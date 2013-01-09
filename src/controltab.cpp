
#include "controltab.h"
#include "converter.h"

using namespace SceneReconstruction;

/** @class ControlTab "controltab.h"
 *  Tab for the GUI that allows the user to control the scene recontruction
 *  by jumping to specific times or (un)pausing. It also displays coordinates
 *  of the currently selected object.
 *  @author Bastian Klingen
 */

ControlTab::ControlTab(gazebo::transport::NodePtr& _node, LoggerTab* _logger, Glib::RefPtr<Gtk::Builder>& builder)
: SceneTab::SceneTab(builder)
{
  node = _node;
  logger = _logger;
  old_value = 0.0;
  ent_info_time = 0.0;
  time_offset = 0.0;
  selected_model = "";
  coords_updated = true;
  this->timeMutex = new boost::mutex();
  this->worldstatsMutex = new boost::mutex();
  this->resMutex = new boost::mutex();
  this->responseMutex = new boost::mutex();
  
  // rng_time setup
  _builder->get_widget("control_scale", rng_time);
  rng_time->signal_button_release_event().connect(sigc::mem_fun(*this,&ControlTab::on_scale_button_event), false);
  rng_time->signal_key_release_event().connect(sigc::mem_fun(*this,&ControlTab::on_scale_key_event), false);
  rng_time->signal_format_value().connect(sigc::mem_fun(*this,&ControlTab::on_scale_format_value), false);
  _builder->get_widget("control_window_scale", rng_win_time);
  rng_win_time->signal_button_release_event().connect(sigc::mem_fun(*this,&ControlTab::on_win_scale_button_event), false);
  rng_win_time->signal_key_release_event().connect(sigc::mem_fun(*this,&ControlTab::on_win_scale_key_event), false);
  rng_win_time->signal_format_value().connect(sigc::mem_fun(*this,&ControlTab::on_win_scale_format_value), false);
  _builder->get_widget("control_label_max_time", lbl_max_time);
  _builder->get_widget("control_label_min_time", lbl_min_time);

  // btn_stop
  _builder->get_widget("control_toolbutton_stop", btn_stop);
  btn_stop->signal_clicked().connect(sigc::mem_fun(*this,&ControlTab::on_button_stop_clicked));

  // btn_play
  _builder->get_widget("control_toolbutton_play", btn_play);
  btn_play->signal_clicked().connect(sigc::mem_fun(*this,&ControlTab::on_button_play_clicked));

  // btn_pause
  _builder->get_widget("control_toolbutton_pause", btn_pause);
  btn_pause->signal_toggled().connect(sigc::mem_fun(*this,&ControlTab::on_button_pause_toggled));

  // btn_shrink
  _builder->get_widget("control_toolbutton_shrink", btn_shrink);
  btn_shrink->signal_clicked().connect(sigc::mem_fun(*this,&ControlTab::on_button_shrink_clicked));

  // data display
  _builder->get_widget("control_treeview", trv_data);
  dat_store = Glib::RefPtr<Gtk::TreeStore>::cast_dynamic(_builder->get_object("control_treestore"));
  dat_store->clear();
  Gtk::TreeModel::Row row = *(dat_store->append());
  row.set_value(0,(Glib::ustring)"Coordinates");
  row.set_value(1,(Glib::ustring)"");

  Gtk::TreeModel::Row childrow = *(dat_store->append(row.children()));
  childrow.set_value(0,(Glib::ustring)"Gazebo");
  childrow.set_value(1,(Glib::ustring)"Position (X: 0 Y: 0 Z: 0) Orientation (X: 0 Y: 0 Z: 0 W: 0)");

  childrow = *(dat_store->append(row.children()));
  childrow.set_value(0,(Glib::ustring)"Robot (\"/map\")");
  childrow.set_value(1,(Glib::ustring)"Position (X: 0 Y: 0 Z: 0) Orientation (X: 0 Y: 0 Z: 0 W: 0)");

  childrow = *(dat_store->append(row.children()));
  childrow.set_value(0,(Glib::ustring)"Frame (\"/map\")");
  childrow.set_value(1,(Glib::ustring)"Position (X: 0 Y: 0 Z: 0) Orientation (X: 0 Y: 0 Z: 0 W: 0)");
  trv_data->expand_all();

  _builder->get_widget("control_window", win_control);
  win_control->signal_delete_event().connect(sigc::mem_fun(*this,&ControlTab::on_control_close), false);
  _builder->get_widget("control_window_toolbutton_stop", btn_win_stop);
  btn_win_stop->signal_clicked().connect(sigc::mem_fun(*this,&ControlTab::on_button_stop_clicked));
  _builder->get_widget("control_window_toolbutton_play", btn_win_play);
  btn_win_play->signal_clicked().connect(sigc::mem_fun(*this,&ControlTab::on_button_play_clicked));
  _builder->get_widget("control_window_toolbutton_pause", btn_win_pause);
  btn_win_pause->signal_toggled().connect(sigc::mem_fun(*this,&ControlTab::on_button_win_pause_toggled));
  _builder->get_widget("control_window_toolbutton_maximize", btn_win_maximize);
  btn_win_maximize->signal_clicked().connect(sigc::mem_fun(*this,&ControlTab::on_button_maximize_clicked));
  no_toggle = false;

  reqPub = node->Advertise<gazebo::msgs::Request>("~/request");
  controlPub = node->Advertise<gazebo::msgs::SceneFrameworkControl>("~/SceneReconstruction/Framework/Control");
  worldPub = node->Advertise<gazebo::msgs::WorldControl>("~/world_control");
  objectPub = node->Advertise<gazebo::msgs::Request>("~/SceneReconstruction/ObjectInstantiator/Request");
  framePub = node->Advertise<gazebo::msgs::TransformRequest>("~/SceneReconstruction/Framework/TransformRequest");
  robotPub = node->Advertise<gazebo::msgs::Request>("~/SceneReconstruction/RobotController/Request");

  gazebo::math::Vector3 rob;
  robot = gazebo::msgs::Convert(rob);
  robotRequest = gazebo::msgs::CreateRequest("get_data");

  gazebo::msgs::WorldControl start;
  start.set_pause(true);
  start.mutable_reset()->set_all(true);
  logger->msglog(">>", "~/world_control", start);
  worldPub->Publish(start);

  gazebo::msgs::SceneFrameworkControl control;
  control.set_pause(true);
  control.set_change_offset(true);
  control.set_offset(time_offset);
  controlPub->Publish(control);

  objectRequest = 0;
  frameRequest = 0;

  resSub = node->Subscribe("~/response", &ControlTab::OnResMsg, this);
  timeSub = node->Subscribe("~/SceneReconstruction/GUI/Time", &ControlTab::OnTimeMsg, this);
  worldSub = node->Subscribe("~/world_stats", &ControlTab::OnWorldStatsMsg, this);
  responseSub = node->Subscribe("~/SceneReconstruction/GUI/Response", &ControlTab::OnResponseMsg, this);
  on_res_msg.connect( sigc::mem_fun( *this , &ControlTab::ProcessResMsg ));
  on_time_msg.connect( sigc::mem_fun( *this , &ControlTab::ProcessTimeMsg ));
  on_worldstats_msg.connect( sigc::mem_fun( *this , &ControlTab::ProcessWorldStatsMsg ));
  on_response_msg.connect( sigc::mem_fun( *this , &ControlTab::ProcessResponseMsg ));
}

ControlTab::~ControlTab() {
}

void ControlTab::OnTimeMsg(ConstDoublePtr& _msg) {
  {
    boost::mutex::scoped_lock lock(*this->timeMutex);
    this->timeMsgs.push_back(*_msg);
  }
  on_time_msg();
}

void ControlTab::ProcessTimeMsg() {
  boost::mutex::scoped_lock lock(*this->timeMutex);
  std::list<gazebo::msgs::Double>::iterator _msg;
  for(_msg = timeMsgs.begin(); _msg != timeMsgs.end(); _msg++) {
    rng_time->set_range(0.0, _msg->data());
    rng_win_time->set_range(0.0, _msg->data());
    Glib::ustring time = Converter::to_ustring_time(_msg->data());
    lbl_max_time->set_text(time);
    size_t p;
    while((p = time.find_first_not_of("0:.")) != Glib::ustring::npos) {
      time = time.replace(p,1,"0");
    }
      
    lbl_min_time->set_text(time);
    logger->log("control", "Range for scale set to (" + Converter::to_ustring_time(0.0) + " , " + Converter::to_ustring_time(_msg->data()) + ")");
  }
  timeMsgs.clear();
}

void ControlTab::OnWorldStatsMsg(ConstWorldStatisticsPtr& _msg) {
  {
    boost::mutex::scoped_lock lock(*this->worldstatsMutex);
    this->worldstatsMsgs.push_back(*_msg);
  }
  on_worldstats_msg();
}

void ControlTab::ProcessWorldStatsMsg() {
  boost::mutex::scoped_lock lock(*this->worldstatsMutex);
  std::list<gazebo::msgs::WorldStatistics>::iterator _msg;
  for(_msg = worldstatsMsgs.begin(); _msg != worldstatsMsgs.end(); _msg++) {
    if(!btn_pause->get_active()) {
      double val;
      val  = _msg->sim_time().sec()*1000;
      val += _msg->sim_time().nsec()/1000000;
      val += time_offset;
      rng_time->set_value(val);
      rng_win_time->set_value(val);
      old_value = rng_time->get_value();

      if(val > rng_time->get_value()) {
        btn_pause->set_active(true);
      }

      if(val >= ent_info_time+1000 && coords_updated && selected_model != "") {
        ent_info_time = val;
        reqPub->Publish(*gazebo::msgs::CreateRequest("entity_info", selected_model));
      }
    }
  }
}

void ControlTab::OnResMsg(ConstResponsePtr& _msg) {
  {
    boost::mutex::scoped_lock lock(*this->resMutex);
    this->resMsgs.push_back(*_msg);
  }
  on_res_msg();
}

void ControlTab::ProcessResMsg() {
  boost::mutex::scoped_lock lock(*this->resMutex);
  std::list<gazebo::msgs::Response>::iterator _msg;
  for(_msg = resMsgs.begin(); _msg != resMsgs.end(); _msg++) {
    if (_msg->request() == "entity_info") {
      logger->msglog("<<", "~/response", *_msg);

      gazebo::msgs::Model model;
      if (_msg->has_type() && _msg->type() == model.GetTypeName()) {
        if(selected_model != model.name() || coords_updated) {
          coords_updated = false;
          model.ParseFromString(_msg->serialized_data());
          logger->log("control", "Coords of Model " + model.name() + " received.");
          selected_model = model.name();
          update_coords(model);
        }
      }
    }
  }
  resMsgs.clear();
}

void ControlTab::OnResponseMsg(ConstResponsePtr& _msg) {
  {
    boost::mutex::scoped_lock lock(*this->responseMutex);
    this->responseMsgs.push_back(*_msg);
  }
  on_response_msg();
}

void ControlTab::ProcessResponseMsg() {
  boost::mutex::scoped_lock lock(*this->responseMutex);
  std::list<gazebo::msgs::Response>::iterator _msg;
  for(_msg = responseMsgs.begin(); _msg != responseMsgs.end(); _msg++) {
    logger->msglog("<<", "~/SceneReconstruction/GUI/Response", *_msg);
    if (robotRequest) {
      if (_msg->request() == robotRequest->request() && (_msg->id() == robotRequest->id() || _msg->id() == -1) && _msg->has_type() && _msg->type() == robot.GetTypeName() && _msg->response() != "unknown") {
        robot.ParseFromString(_msg->serialized_data());
        selected_model = _msg->response();
      }
    }
    
    if (objectRequest) {
      gazebo::msgs::GzString obj;
      if (_msg->request() == objectRequest->request() && _msg->id() == objectRequest->id() && _msg->has_type() && _msg->type() == obj.GetTypeName() && _msg->response() == "success") {
        if(!frameRequest) {
          obj.ParseFromString(_msg->serialized_data());
          model_frame = obj.data();
          logger->log("control", "getting coords for frame: "+model_frame);

          gazebo::msgs::Request *tmp = gazebo::msgs::CreateRequest("transform_request");
          frameRequest = new gazebo::msgs::TransformRequest;
          frameRequest->set_id(tmp->id());
          frameRequest->set_request(tmp->request());
          frameRequest->set_source_frame("/gazebo");
          frameRequest->set_target_frame(model_frame);
          gazebo::math::Vector3 pos = gazebo::msgs::Convert(gazebo.position()) - gazebo::msgs::Convert(robot);
          frameRequest->set_pos_x(pos.x);
          frameRequest->set_pos_y(pos.y);
          frameRequest->set_pos_z(pos.z);
          frameRequest->set_ori_w(gazebo.orientation().w());
          frameRequest->set_ori_x(gazebo.orientation().x());
          frameRequest->set_ori_y(gazebo.orientation().y());
          frameRequest->set_ori_z(gazebo.orientation().z());
          framePub->Publish(*frameRequest);
        }
      }
      else {
        logger->log("control", "getting coords for frame: /map");
        gazebo::math::Pose tmp_pose = gazebo::msgs::Convert(gazebo);
        tmp_pose.pos -= gazebo::msgs::Convert(robot);
        sensor = gazebo::msgs::Convert(tmp_pose);
        sensor.set_name("/map");

        update_coords();
      }
    }
    
    if (frameRequest) {
      if (_msg->request() == frameRequest->request() && _msg->id() == frameRequest->id() && _msg->has_type() && _msg->type() == sensor.GetTypeName()) {
        sensor.ParseFromString(_msg->serialized_data());
        delete frameRequest;
        frameRequest = 0;
        update_coords();      
      }
    }
  }
  responseMsgs.clear();
}

void ControlTab::update_coords(gazebo::msgs::Model model) {
  // Update Coords-Treeview
  if(model.has_pose()) {
    gazebo = model.pose();
  }

  gazebo::math::Pose sen;
  sensor = gazebo::msgs::Convert(sen);
  objectRequest = gazebo::msgs::CreateRequest("get_frame", model.name());
  objectPub->Publish(*objectRequest);
  logger->msglog(">>", "~/SceneReconstruction/ObjectInstantiator/Request", *objectRequest);
}

void ControlTab::update_coords() {
  // Update Coords-Treeview
  Gtk::TreeModel::Children rows = trv_data->get_model()->children();
  for(Gtk::TreeModel::Children::iterator iter = rows.begin(); iter != rows.end(); ++iter) {
    Gtk::TreeModel::Row row = *iter;
    Glib::ustring tmp;
    row.get_value(0, tmp);
    Gtk::TreeModel::Children childrows = row.children();
    Gtk::TreeModel::Children::iterator childiter;
    Gtk::TreeModel::Row childrow;

    childiter = childrows.begin();
    childrow = *childiter;
    childrow.set_value(1, Converter::convert(gazebo, 2, 3));
  
    gazebo::math::Pose tmp_pose = gazebo::msgs::Convert(gazebo);
    tmp_pose.pos -= gazebo::msgs::Convert(robot);
    childiter++;
    childrow = *childiter;
    childrow.set_value(1, Converter::convert(gazebo::msgs::Convert(tmp_pose), 2, 3));

    childiter++;
    childrow = *childiter;
    childrow.set_value(0, "Frame (\""+sensor.name()+"\")");
    childrow.set_value(1, Converter::convert(sensor, 2, 3));
  }
  coords_updated = true;
}

void ControlTab::on_button_stop_clicked() {
  time_offset = 0.0;
  rng_time->set_value(0.0);
  rng_win_time->set_value(0.0);
  ent_info_time = 0.0;
  btn_pause->set_active(true);

  gazebo::msgs::WorldControl start;
  start.set_pause(true);
  start.mutable_reset()->set_all(true);
  logger->msglog(">>", "~/world_control", start);
  worldPub->Publish(start);

  gazebo::msgs::SceneFrameworkControl control;
  control.set_pause(true);
  control.set_change_offset(true);
  control.set_offset(time_offset);
  controlPub->Publish(control);
}

void ControlTab::on_button_play_clicked() {
  btn_pause->set_active(false);
}

void ControlTab::on_button_pause_toggled() {
  no_toggle = true;
  btn_win_pause->set_active(btn_pause->get_active());

  if(btn_pause->get_active()) {
    gazebo::msgs::WorldControl start;
    start.set_pause(true);
    logger->msglog(">>", "~/world_control", start);
    worldPub->Publish(start);

    gazebo::msgs::SceneFrameworkControl control;
    control.set_pause(true);
    controlPub->Publish(control);
  }
  else {
    gazebo::msgs::WorldControl start;
    start.set_pause(false);
    logger->msglog(">>", "~/world_control", start);
    worldPub->Publish(start);

    gazebo::msgs::SceneFrameworkControl control;
    control.set_pause(false);
    controlPub->Publish(control);
  }
  no_toggle = false;
}

void ControlTab::on_button_win_pause_toggled() {
  if(!no_toggle)
    btn_pause->set_active(!(btn_pause->get_active()));
}

void ControlTab::on_button_shrink_clicked() {
  Gtk::Window *w;
  _builder->get_widget("window", w);

  int x, y;
  w->get_position(x,y);
  win_control->move(x,y);

  win_control->present();
  w->hide();
}

void ControlTab::on_button_maximize_clicked() {
  Gtk::Window *w;
  _builder->get_widget("window", w);

  int x, y;
  win_control->get_position(x,y);
  w->move(x,y);

  w->present();
  win_control->hide();
}

bool ControlTab::on_control_close(GdkEventAny* /*e*/) {
  Gtk::Window *w;
  _builder->get_widget("window", w);

  int x, y;
  win_control->get_position(x,y);
  w->move(x,y);

  w->present();
  win_control->hide();
  return true;
}

Glib::ustring ControlTab::on_scale_format_value(double value) {
  return Converter::to_ustring_time(value);
}

Glib::ustring ControlTab::on_win_scale_format_value(double value) {
  return Converter::to_ustring_time(value)+" / "+lbl_max_time->get_text();
}

bool ControlTab::on_scale_button_event(GdkEventButton* b) {
  if(rng_time->get_value() != old_value && btn_pause->get_active()) {

    logger->log("control", "Time changed from %.2f to %.2f using button %d of the mouse on rng_time", old_value, rng_time->get_value(), b->button);
    time_offset = rng_time->get_value();
    old_value = rng_time->get_value();
    rng_win_time->set_value(old_value);
    ent_info_time = time_offset;

    gazebo::msgs::WorldControl start;
    start.set_pause(true);
    start.set_step(true);
    start.mutable_reset()->set_all(true);
    logger->msglog(">>", "~/world_control", start);
    worldPub->Publish(start);

    gazebo::msgs::SceneFrameworkControl control;
    control.set_pause(true);
    control.set_change_offset(true);
    control.set_offset(time_offset);
    control.set_step(true);
    controlPub->Publish(control);
  }

  return false;
}

bool ControlTab::on_scale_key_event(GdkEventKey* k) {
  if(((k->keyval == GDK_KEY_Left || k->keyval == GDK_KEY_Right || k->keyval == GDK_KEY_Up || k->keyval == GDK_KEY_Down || k->keyval == GDK_KEY_KP_Left || k->keyval == GDK_KEY_KP_Right || k->keyval == GDK_KEY_KP_Up || k->keyval == GDK_KEY_KP_Down || k->keyval == GDK_KEY_Home || k->keyval == GDK_KEY_End || k->keyval == GDK_KEY_Page_Up || k->keyval == GDK_KEY_Page_Down) && old_value != rng_time->get_value()) && btn_pause->get_active()) {

    logger->log("control", "Time changed from %.2f to %.2f using key %s of the keyboard on rng_time", old_value, rng_time->get_value(), gdk_keyval_name(k->keyval));

    time_offset = rng_time->get_value();
    old_value = rng_time->get_value();
    rng_win_time->set_value(old_value);
    ent_info_time = time_offset;

    gazebo::msgs::WorldControl start;
    start.set_pause(true);
    start.set_step(true);
    start.mutable_reset()->set_all(true);
    logger->msglog(">>", "~/world_control", start);
    worldPub->Publish(start);

    gazebo::msgs::SceneFrameworkControl control;
    control.set_pause(true);
    control.set_change_offset(true);
    control.set_offset(time_offset);
    control.set_step(true);
    controlPub->Publish(control);
  }

  return false;
}

bool ControlTab::on_win_scale_button_event(GdkEventButton* b) {
  if(rng_win_time->get_value() != old_value && btn_pause->get_active()) {

    logger->log("control", "Time changed from %.2f to %.2f using button %d of the mouse on rng_win_time", old_value, rng_win_time->get_value(), b->button);
    time_offset = rng_win_time->get_value();
    old_value = rng_win_time->get_value();
    rng_time->set_value(old_value);
    ent_info_time = time_offset;

    gazebo::msgs::WorldControl start;
    start.set_pause(true);
    start.set_step(true);
    start.mutable_reset()->set_all(true);
    logger->msglog(">>", "~/world_control", start);
    worldPub->Publish(start);

    gazebo::msgs::SceneFrameworkControl control;
    control.set_pause(true);
    control.set_change_offset(true);
    control.set_offset(time_offset);
    control.set_step(true);
    controlPub->Publish(control);
  }

  return false;
}

bool ControlTab::on_win_scale_key_event(GdkEventKey* k) {
  if(((k->keyval == GDK_KEY_Left || k->keyval == GDK_KEY_Right || k->keyval == GDK_KEY_Up || k->keyval == GDK_KEY_Down || k->keyval == GDK_KEY_KP_Left || k->keyval == GDK_KEY_KP_Right || k->keyval == GDK_KEY_KP_Up || k->keyval == GDK_KEY_KP_Down || k->keyval == GDK_KEY_Home || k->keyval == GDK_KEY_End || k->keyval == GDK_KEY_Page_Up || k->keyval == GDK_KEY_Page_Down) && old_value != rng_win_time->get_value()) && btn_pause->get_active()) {

    logger->log("control", "Time changed from %.2f to %.2f using key %s of the keyboard on rng_win_time", old_value, rng_win_time->get_value(), gdk_keyval_name(k->keyval));

    time_offset = rng_win_time->get_value();
    old_value = rng_win_time->get_value();
    rng_time->set_value(old_value);
    ent_info_time = time_offset;

    gazebo::msgs::WorldControl start;
    start.set_pause(true);
    start.set_step(true);
    start.mutable_reset()->set_all(true);
    logger->msglog(">>", "~/world_control", start);
    worldPub->Publish(start);

    gazebo::msgs::SceneFrameworkControl control;
    control.set_pause(true);
    control.set_change_offset(true);
    control.set_offset(time_offset);
    control.set_step(true);
    controlPub->Publish(control);
  }

  return false;
}

