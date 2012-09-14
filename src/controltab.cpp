#include <gazebo/math/Pose.hh>

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
  time_offset = 0.0;
  selected_model = "";
  
  resSub = node->Subscribe("~/response", &ControlTab::OnResMsg, this);
  timeSub = node->Subscribe("~/SceneReconstruction/GUI/Time", &ControlTab::OnTimeMsg, this);
  worldSub = node->Subscribe("~/world_stats", &ControlTab::OnWorldStatsMsg, this);
  responseSub = node->Subscribe("~/SceneReconstruction/GUI/Response", &ControlTab::OnResponseMsg, this);
  reqPub = node->Advertise<gazebo::msgs::Request>("~/request");
  controlPub = node->Advertise<gazebo::msgs::SceneFrameworkControl>("~/SceneReconstruction/Framework/Control");
  worldPub = node->Advertise<gazebo::msgs::WorldControl>("~/world_control");
  robotPub = node->Advertise<gazebo::msgs::Request>("~/SceneReconstruction/RobotController/Request");
  objectPub = node->Advertise<gazebo::msgs::Request>("~/SceneReconstruction/ObjectInstantiator/Request");
  framePub = node->Advertise<gazebo::msgs::TransformRequest>("~/SceneReconstruction/Framework/TransformRequest");

  // rng_time setup
  _builder->get_widget("control_scale", rng_time);
  rng_time->signal_button_release_event().connect(sigc::mem_fun(*this,&ControlTab::on_scale_button_event), false);
  rng_time->signal_key_release_event().connect(sigc::mem_fun(*this,&ControlTab::on_scale_key_event), false);
  rng_time->signal_format_value().connect(sigc::mem_fun(*this,&ControlTab::on_scale_format_value), false);
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
  btn_pause->signal_clicked().connect(sigc::mem_fun(*this,&ControlTab::on_button_pause_clicked));

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
  childrow.set_value(0,(Glib::ustring)"Robot");
  childrow.set_value(1,(Glib::ustring)"Position (X: 0 Y: 0 Z: 0) Orientation (X: 0 Y: 0 Z: 0 W: 0)");

  childrow = *(dat_store->append(row.children()));
  childrow.set_value(0,(Glib::ustring)"Sensor");
  childrow.set_value(1,(Glib::ustring)"Position (X: 0 Y: 0 Z: 0) Orientation (X: 0 Y: 0 Z: 0 W: 0)");
  trv_data->expand_all();

  gazebo::math::Pose rob;
  robot = gazebo::msgs::Convert(rob);
  robotRequest = gazebo::msgs::CreateRequest("get_offset");
  robotPub->Publish(*robotRequest);
}

ControlTab::~ControlTab() {
}

void ControlTab::OnTimeMsg(ConstDoublePtr& _msg) {
  rng_time->set_range(0.0, _msg->data());
  Glib::ustring time = Converter::to_ustring_time(_msg->data());
  lbl_max_time->set_text(time);
  size_t p;
  while((p = time.find_first_not_of("0:.")) != Glib::ustring::npos) {
    time = time.replace(p,1,"0");
  }
    
  lbl_min_time->set_text(time);
  logger->log("control", "Range for scale set to (" + Converter::to_ustring_time(0.0) + " , " + Converter::to_ustring_time(_msg->data()) + ")");
}

void ControlTab::OnWorldStatsMsg(ConstWorldStatisticsPtr& _msg) {
  double val;
  val  = _msg->sim_time().sec()*1000;
  val += _msg->sim_time().nsec()/1000000;
  val += time_offset;
  rng_time->set_value(val);
  old_value = rng_time->get_value();

  if(val > rng_time->get_value()) {
    btn_pause->set_active(false);

    gazebo::msgs::WorldControl start;
    start.set_pause(true);
    logger->msglog(">>", "~/world_control", start);
    worldPub->Publish(start);

    gazebo::msgs::SceneFrameworkControl control;
    control.set_pause(true);
    controlPub->Publish(control);
    
    if(selected_model != "") {
      reqPub->Publish(*gazebo::msgs::CreateRequest("entity_info", selected_model));
    }
  }
}

void ControlTab::OnResMsg(ConstResponsePtr& _msg) {
  if (_msg->request() == "entity_info") {
    logger->msglog("<<", "~/response", _msg);

    gazebo::msgs::Model model;
    if (_msg->has_type() && _msg->type() == model.GetTypeName()) {
      model.ParseFromString(_msg->serialized_data());
      logger->log("control", "Coords of Model " + model.name() + " received.");
      selected_model = model.name();
      update_coords(model);
    }
  }
}

void ControlTab::OnResponseMsg(ConstResponsePtr& _msg) {
  logger->msglog("<<", "~/SceneReconstruction/GUI/Response", _msg);
  if (_msg->request() == robotRequest->request() && _msg->id() == robotRequest->id()) {
    if (_msg->has_type() && _msg->type() == robot.GetTypeName())
      robot.ParseFromString(_msg->serialized_data());
  }
  else if (_msg->request() == objectRequest->request() && _msg->id() == objectRequest->id()) {
    gazebo::msgs::String obj;
    if (_msg->has_type() && _msg->type() == obj.GetTypeName() && _msg->response() == "success") {
      obj.ParseFromString(_msg->serialized_data());
      model_frame = obj.data();

      // TODO: request sensor position from framework
      gazebo::msgs::Request tmp = *gazebo::msgs::CreateRequest("transform_request");
      frameRequest.set_id(tmp.id());
      frameRequest.set_request(tmp.request());
      frameRequest.set_source_frame("/map");
      frameRequest.set_target_frame(model_frame);
      frameRequest.set_pos_x(gazebo.position().x() + robot.position().x());
      frameRequest.set_pos_y(gazebo.position().y() + robot.position().y());
      frameRequest.set_pos_z(gazebo.position().z() + robot.position().z());
      frameRequest.set_ori_w(gazebo.orientation().w() + robot.orientation().w());
      frameRequest.set_ori_x(gazebo.orientation().x() + robot.orientation().x());
      frameRequest.set_ori_y(gazebo.orientation().y() + robot.orientation().y());
      frameRequest.set_ori_z(gazebo.orientation().z() + robot.orientation().z());
      framePub->Publish(frameRequest);
    }
    else {
      gazebo::math::Pose tmp_pose = gazebo::msgs::Convert(robot);
      tmp_pose.pos.x += gazebo.position().x();
      tmp_pose.pos.y += gazebo.position().y();
      tmp_pose.pos.z += gazebo.position().z();
      tmp_pose.rot.w += gazebo.orientation().w();
      tmp_pose.rot.x += gazebo.orientation().x();
      tmp_pose.rot.y += gazebo.orientation().y();
      tmp_pose.rot.z += gazebo.orientation().z();

      sensor = gazebo::msgs::Convert(tmp_pose);
      sensor.set_name("/base_link");
    }
  }
  else if (_msg->request() == frameRequest.request() && _msg->id() == frameRequest.id()) {
    if (_msg->has_type() && _msg->type() == sensor.GetTypeName())
      sensor.ParseFromString(_msg->serialized_data());
  }
}

void ControlTab::update_coords(gazebo::msgs::Model model) {
  // Update Coords-Treeview
  if(model.has_pose()) {
    gazebo = model.pose();
  }

  gazebo::math::Pose sen;
  sensor = gazebo::msgs::Convert(sen);
  objectRequest = gazebo::msgs::CreateRequest("get_object", model.name());
  objectPub->Publish(*objectRequest);

  update_coords();
}

void ControlTab::update_coords() {
  // Update Coords-Treeview
  Gtk::TreeModel::Children rows = trv_data->get_model()->children();
  for(Gtk::TreeModel::Children::iterator iter = rows.begin(); iter != rows.end(); ++iter) {
    Gtk::TreeModel::Row row = *iter;
    Glib::ustring tmp;
    row.get_value(0, tmp);
    if(tmp == "Coordinates") {
      Gtk::TreeModel::Children childrows = row.children();
      Gtk::TreeModel::Children::iterator childiter;
      Gtk::TreeModel::Row childrow;

      childiter = childrows.begin();
      childrow = *childiter;
      childrow.set_value(1, Converter::convert(gazebo, 2, 3));
    
      gazebo::math::Pose tmp_pose = gazebo::msgs::Convert(robot);
      tmp_pose.pos.x += gazebo.position().x();
      tmp_pose.pos.y += gazebo.position().y();
      tmp_pose.pos.z += gazebo.position().z();
      tmp_pose.rot.w += gazebo.orientation().w();
      tmp_pose.rot.x += gazebo.orientation().x();
      tmp_pose.rot.y += gazebo.orientation().y();
      tmp_pose.rot.z += gazebo.orientation().z();
      childiter++;
      childrow = *childiter;
      childrow.set_value(1, Converter::convert(gazebo::msgs::Convert(tmp_pose), 2, 3));

      childiter++;
      childrow = *childiter;
      childrow.set_value(0, "Frame("+sensor.name()+")");
      childrow.set_value(1, Converter::convert(sensor, 2, 3));
    }
  }
}

void ControlTab::on_button_stop_clicked() {
  logger->log("control", "STOP");
  time_offset = 0.0;
  rng_time->set_value(0.0);
  btn_pause->set_active(true);

  gazebo::msgs::WorldControl start;
  start.set_pause(true);
  start.set_reset_time(true);
  start.set_reset_world(true);
  logger->msglog(">>", "~/world_control", start);
  worldPub->Publish(start);

  gazebo::msgs::SceneFrameworkControl control;
  control.set_pause(true);
  control.set_change_offset(true);
  control.set_offset(time_offset);
  controlPub->Publish(control);
}

void ControlTab::on_button_play_clicked() {
  logger->log("control", "PLAY");
  btn_pause->set_active(false);

  gazebo::msgs::WorldControl start;
  start.set_pause(false);
  logger->msglog(">>", "~/world_control", start);
  worldPub->Publish(start);

  gazebo::msgs::SceneFrameworkControl control;
  control.set_pause(false);
  controlPub->Publish(control);
}

void ControlTab::on_button_pause_clicked() {
  logger->log("control", "PAUSE");
  if(!btn_pause->get_active()) {
    btn_pause->set_active(true);
    gazebo::msgs::WorldControl start;
    start.set_pause(true);
    logger->msglog(">>", "~/world_control", start);
    worldPub->Publish(start);

    gazebo::msgs::SceneFrameworkControl control;
    control.set_pause(true);
    controlPub->Publish(control);
  }
  else {
    btn_pause->set_active(false);

    gazebo::msgs::WorldControl start;
    start.set_pause(false);
    logger->msglog(">>", "~/world_control", start);
    worldPub->Publish(start);

    gazebo::msgs::SceneFrameworkControl control;
    control.set_pause(false);
    controlPub->Publish(control);
  }
}

bool ControlTab::on_scale_button_event(GdkEventButton* b) {
  if(rng_time->get_value() != old_value) {
    logger->log("control", "Time changed from %.2f to %.2f using button %d of the mouse on rng_time", old_value, rng_time->get_value(), b->button);
    gazebo::msgs::WorldControl start;
    start.set_reset_time(true);
    logger->msglog(">>", "~/world_control", start);
    worldPub->Publish(start);
    time_offset = rng_time->get_value();
    old_value = rng_time->get_value();

    gazebo::msgs::SceneFrameworkControl control;
    control.set_change_offset(true);
    control.set_offset(time_offset);
    controlPub->Publish(control);
  }

  return false;
}

Glib::ustring ControlTab::on_scale_format_value(double value) {
  return Converter::to_ustring_time(value);
}

bool ControlTab::on_scale_key_event(GdkEventKey* k) {
  if((k->keyval == GDK_KEY_Left || k->keyval == GDK_KEY_Right || k->keyval == GDK_KEY_Up || k->keyval == GDK_KEY_Down || k->keyval == GDK_KEY_KP_Left || k->keyval == GDK_KEY_KP_Right || k->keyval == GDK_KEY_KP_Up || k->keyval == GDK_KEY_KP_Down || k->keyval == GDK_KEY_Home || k->keyval == GDK_KEY_End || k->keyval == GDK_KEY_Page_Up || k->keyval == GDK_KEY_Page_Down) && old_value != rng_time->get_value()) {
    logger->log("control", "Time changed from %.2f to %.2f using key %s of the keyboard on rng_time", old_value, rng_time->get_value(), gdk_keyval_name(k->keyval));
    gazebo::msgs::WorldControl start;
    start.set_reset_time(true);
    logger->msglog(">>", "~/world_control", start);
    worldPub->Publish(start);
    time_offset = rng_time->get_value();
    old_value = rng_time->get_value();

    gazebo::msgs::SceneFrameworkControl control;
    control.set_change_offset(true);
    control.set_offset(time_offset);
    controlPub->Publish(control);
  }

  return false;
}

void ControlTab::set_enabled(bool enabled) {
  Gtk::Widget* tab;
  _builder->get_widget("control_tab", tab);
  tab->set_sensitive(enabled);
}
