#include "analysistab.h"
#include "converter.h"

using namespace SceneReconstruction;

/** @class AnalysisTab "analysistab.h"
 *  Tab for the GUI that controls the analyis tools.
 *  It displays the buffers for the Robot and the Objects
 *  and controls the a grid for measurements inside the 
 *  simulation as well as the visualization of the lasers
 *  of the robot.
 *  @author Bastian Klingen
 */

AnalysisTab::AnalysisTab(gazebo::transport::NodePtr& _node, LoggerTab* _logger, Glib::RefPtr<Gtk::Builder>& builder)
: SceneTab::SceneTab(builder)
{
  node = _node;
  logger = _logger;
  this->bufferMutex = new boost::mutex();
  this->lasersMutex = new boost::mutex();
  
  _builder->get_widget("analysis_buffer_position_toolbutton_preview", btn_position_preview);
  btn_position_preview->signal_clicked().connect(sigc::mem_fun(*this,&AnalysisTab::on_button_position_preview_clicked));
  _builder->get_widget("analysis_buffer_position_toolbutton_clear", btn_position_clear);
  btn_position_clear->signal_clicked().connect(sigc::mem_fun(*this,&AnalysisTab::on_button_position_clear_clicked));
  _builder->get_widget("analysis_buffer_joints_toolbutton_preview", btn_angles_preview);
  btn_angles_preview->signal_clicked().connect(sigc::mem_fun(*this,&AnalysisTab::on_button_angles_preview_clicked));
  _builder->get_widget("analysis_buffer_joints_toolbutton_clear", btn_angles_clear);
  btn_angles_clear->signal_clicked().connect(sigc::mem_fun(*this,&AnalysisTab::on_button_angles_clear_clicked));
  _builder->get_widget("analysis_buffer_objects_toolbutton_preview", btn_object_preview);
  btn_object_preview->signal_clicked().connect(sigc::mem_fun(*this,&AnalysisTab::on_button_object_preview_clicked));
  _builder->get_widget("analysis_buffer_objects_toolbutton_move", btn_object_move);
  btn_object_move->signal_clicked().connect(sigc::mem_fun(*this,&AnalysisTab::on_button_object_move_clicked));
  _builder->get_widget("analysis_buffer_objects_toolbutton_clear", btn_object_clear);
  btn_object_clear->signal_clicked().connect(sigc::mem_fun(*this,&AnalysisTab::on_button_object_clear_clicked));

  _builder->get_widget("analysis_buffer_position_treeview", trv_positions);
  pos_store = Glib::RefPtr<Gtk::TreeStore>::cast_dynamic(_builder->get_object("analysis_buffer_position_treestore"));

  _builder->get_widget("analysis_buffer_joints_treeview", trv_angles);
  ang_store = Glib::RefPtr<Gtk::TreeStore>::cast_dynamic(_builder->get_object("analysis_buffer_joints_treestore"));

  _builder->get_widget("analysis_buffer_objects_treeview", trv_objects);
  trv_objects->signal_button_release_event().connect(sigc::mem_fun(*this,&AnalysisTab::on_treeview_button_release));
  trv_objects->signal_key_release_event().connect(sigc::mem_fun(*this,&AnalysisTab::on_treeview_key_release));
  obj_store = Glib::RefPtr<Gtk::TreeStore>::cast_dynamic(_builder->get_object("analysis_buffer_objects_treestore"));

  _builder->get_widget("analysis_buffer_objects_spinbutton_position_x", spn_object_pos_x);
  _builder->get_widget("analysis_buffer_objects_spinbutton_position_y", spn_object_pos_y);
  _builder->get_widget("analysis_buffer_objects_spinbutton_position_z", spn_object_pos_z);
  _builder->get_widget("analysis_buffer_objects_spinbutton_orientation_x", spn_object_rot_x);
  _builder->get_widget("analysis_buffer_objects_spinbutton_orientation_y", spn_object_rot_y);
  _builder->get_widget("analysis_buffer_objects_spinbutton_orientation_z", spn_object_rot_z);
  _builder->get_widget("analysis_buffer_objects_spinbutton_orientation_w", spn_object_rot_w);

  _builder->get_widget("analysis_toolbox_grid_spinbutton_position_x", spn_grid_pos_x);
  _builder->get_widget("analysis_toolbox_grid_spinbutton_position_y", spn_grid_pos_y);
  _builder->get_widget("analysis_toolbox_grid_spinbutton_position_z", spn_grid_pos_z);
  _builder->get_widget("analysis_toolbox_grid_spinbutton_rotation_x", spn_grid_rot_x);
  _builder->get_widget("analysis_toolbox_grid_spinbutton_rotation_y", spn_grid_rot_y);
  _builder->get_widget("analysis_toolbox_grid_spinbutton_rotation_z", spn_grid_rot_z);
  _builder->get_widget("analysis_toolbox_grid_spinbutton_rotation_w", spn_grid_rot_w);
  _builder->get_widget("analysis_toolbox_grid_spinbutton_width", spn_grid_width);
  _builder->get_widget("analysis_toolbox_grid_spinbutton_height", spn_grid_height);
  _builder->get_widget("analysis_toolbox_grid_spinbutton_size", spn_grid_size);

  _builder->get_widget("analysis_toolbox_grid_button_show", btn_grid_show);
  btn_grid_show->signal_clicked().connect(sigc::mem_fun(*this,&AnalysisTab::on_button_grid_show_clicked));
  _builder->get_widget("analysis_toolbox_grid_button_move", btn_grid_move);
  btn_grid_move->signal_clicked().connect(sigc::mem_fun(*this,&AnalysisTab::on_button_grid_move_clicked));

  _builder->get_widget("analysis_toolbox_robot_treeview_lasers", trv_lasers);
  lsr_store = Glib::RefPtr<Gtk::ListStore>::cast_dynamic(_builder->get_object("analysis_toolbox_robot_liststore_lasers"));
  _builder->get_widget("analysis_toolbox_robot_button_lasers_update", btn_lasers_update);
  btn_lasers_update->signal_clicked().connect(sigc::mem_fun(*this,&AnalysisTab::on_button_lasers_update_clicked));

  std::vector<Gtk::CellRenderer*> las_col_visible = trv_lasers->get_column(1)->get_cells();
  for(unsigned int i=0; i<las_col_visible.size(); i++) {
    dynamic_cast<Gtk::CellRendererToggle*>(las_col_visible[i])->set_activatable(true);
    dynamic_cast<Gtk::CellRendererToggle*>(las_col_visible[i])->signal_toggled().connect(sigc::mem_fun(*this,&AnalysisTab::on_lasers_visible_toggled));
  }

  positionPub = node->Advertise<gazebo::msgs::SceneRobot>("~/SceneReconstruction/RobotController/BufferPosition");
  anglesPub = node->Advertise<gazebo::msgs::SceneJoint>("~/SceneReconstruction/RobotController/BufferJoints");
  objectPub = node->Advertise<gazebo::msgs::SceneObject>("~/SceneReconstruction/ObjectInstantiator/BufferObject");
  drawingPub = node->Advertise<gazebo::msgs::Drawing>("~/draw");
  lasersPub = node->Advertise<gazebo::msgs::Lasers>("~/SceneReconstruction/Framework/Lasers");

  objBufferSub = node->Subscribe("~/SceneReconstruction/ObjectInstantiator/Object", &AnalysisTab::OnBufferMsg, this);
  robBufferSub = node->Subscribe("~/SceneReconstruction/RobotController/", &AnalysisTab::OnBufferMsg, this);
  on_buffer_msg.connect( sigc::mem_fun( *this , &AnalysisTab::ProcessBufferMsg ));
  lasersSub = node->Subscribe("~/SceneReconstruction/GUI/Lasers", &AnalysisTab::OnLasersMsg, this);
  on_lasers_msg.connect( sigc::mem_fun( *this , &AnalysisTab::ProcessLasersMsg ));
  controlSub = node->Subscribe("~/SceneReconstruction/Framework/Control", &AnalysisTab::OnControlMsg, this);
  on_control_msg.connect( sigc::mem_fun( *this , &AnalysisTab::ProcessControlMsg ));
}

AnalysisTab::~AnalysisTab() {
}

void AnalysisTab::OnControlMsg(ConstSceneFrameworkControlPtr& _msg) {
  if(_msg->has_change_offset() && _msg->change_offset()) {
    time_offset = _msg->offset();
    on_control_msg();
  }
}

void AnalysisTab::ProcessControlMsg() {
  {
    boost::mutex::scoped_lock lock(*this->bufferMutex);
    ang_store->clear();
    ang_messages.clear();
    obj_store->clear();
    obj_messages.clear();
    pos_store->clear();
    pos_messages.clear();
  }
}

void AnalysisTab::OnBufferMsg(ConstMessage_VPtr& _msg) {
  {
    boost::mutex::scoped_lock lock(*this->bufferMutex);
    this->bufferMsgs.push_back(*_msg);
  }
  on_buffer_msg();
}

void AnalysisTab::ProcessBufferMsg() {
  boost::mutex::scoped_lock lock(*this->bufferMutex);
  std::list<gazebo::msgs::Message_V>::iterator _msg;
  for(_msg = bufferMsgs.begin(); _msg != bufferMsgs.end(); _msg++) {
    logger->msglog("<<", "~/SceneReconstruction/GUI/Buffer", *_msg);
    gazebo::msgs::SceneJoint jnt;
    gazebo::msgs::SceneRobot pos;
    gazebo::msgs::SceneObject obj;
    
    if(_msg->msgtype() == jnt.GetTypeName()) {
      for(int i=0; i<_msg->msgsdata_size(); i++) {
        jnt.ParseFromString(_msg->msgsdata(i));
        Gtk::TreeModel::Row row = *(ang_store->append());
        int msg = ang_messages.size();
        row.set_value(0, Converter::to_ustring_time((jnt.controltime()+time_offset)*1000));
        row.set_value(1, (Glib::ustring)"");
        row.set_value(2, msg);

        int n = jnt.joint_size();
        int a = jnt.angle_size();
        if(n==a) {
          for(int j=0; j<n; j++) {
            Gtk::TreeModel::Row childrow = *(ang_store->append(row.children()));
            childrow.set_value(0, jnt.joint(j));
            childrow.set_value(1, Converter::to_ustring(jnt.angle(j)));
            childrow.set_value(2, (int)ang_messages.size());
          }
        }
        ang_messages.push_back(jnt);
      }
    }
    else if(_msg->msgtype() == obj.GetTypeName()) {
      obj_messages.resize(obj_messages.size()+_msg->msgsdata_size());
      for(int i=0; i<_msg->msgsdata_size(); i++) {
        obj.ParseFromString(_msg->msgsdata(i));
        Gtk::TreeModel::Row row = *(obj_store->append());
        int msg = obj_messages.size();
        row.set_value(0, Converter::to_ustring_time((obj.time()+time_offset)*1000));
        row.set_value(1, obj.object()+(obj.visible()?" (Visible)":""));
        row.set_value(2, msg);

        Gtk::TreeModel::Row childrow = *(obj_store->append(row.children()));
        childrow.set_value(0, (Glib::ustring)"Visible");
        childrow.set_value(1, Converter::to_ustring(obj.visible()));
        childrow.set_value(2, msg);

        childrow = *(obj_store->append(row.children()));
        childrow.set_value(0, (Glib::ustring)"Pose");
        childrow.set_value(1, Converter::convert(obj.pose(), 2, 3));
        childrow.set_value(2, msg);

        if(obj.has_query()) {
          childrow = *(obj_store->append(row.children()));
          childrow.set_value(0, (Glib::ustring)"Query");
          childrow.set_value(1, obj.query());
          childrow.set_value(2, msg);
        }

        if(obj.has_frame()) {
          childrow = *(obj_store->append(row.children()));
          childrow.set_value(0, (Glib::ustring)"Frame");
          childrow.set_value(1, obj.frame());
          childrow.set_value(2, msg);
        }

        obj_messages.push_back(obj);
      }
    }
    else if(_msg->msgtype() == pos.GetTypeName()) {
      for(int i=0; i<_msg->msgsdata_size(); i++) {
        pos.ParseFromString(_msg->msgsdata(i));
        Gtk::TreeModel::Row row = *(pos_store->append());
        int msg = pos_messages.size();
        row.set_value(0, Converter::to_ustring_time((pos.controltime()+time_offset)*1000));
        row.set_value(1, Converter::convert(pos.pose(), 2, 3));
        row.set_value(2, msg);
        Gtk::TreeModel::Row childrow = *(pos_store->append(row.children()));
        childrow.set_value(0, (Glib::ustring)"Position");
        childrow.set_value(1, Converter::convert(pos.pose(), 0, 3));
        childrow.set_value(2, msg);
        childrow = *(pos_store->append(row.children()));
        childrow.set_value(0, (Glib::ustring)"Orientation");
        childrow.set_value(1, Converter::convert(pos.pose(), 1, 3));
        childrow.set_value(2, msg);
        pos_messages.push_back(pos);
      }
    }
  }

  bufferMsgs.clear();
}

bool AnalysisTab::on_treeview_button_release(GdkEventButton */*event*/) {
  treeview_object_selection();  
  return true;
}

bool AnalysisTab::on_treeview_key_release(GdkEventKey */*event*/) {
  treeview_object_selection();  
  return true;
}

void AnalysisTab::treeview_object_selection() {
  if(trv_objects->get_selection()->count_selected_rows() == 1) {
    Gtk::TreeModel::iterator row = trv_objects->get_selection()->get_selected();
    int msg = -1;

    row->get_value(2, msg);

    if(msg != -1) {
      spn_object_pos_x->set_value(obj_messages[msg].pose().position().x());
      spn_object_pos_y->set_value(obj_messages[msg].pose().position().y());
      spn_object_pos_z->set_value(obj_messages[msg].pose().position().z());
      spn_object_rot_x->set_value(obj_messages[msg].pose().orientation().x());
      spn_object_rot_y->set_value(obj_messages[msg].pose().orientation().y());
      spn_object_rot_z->set_value(obj_messages[msg].pose().orientation().z());
      spn_object_rot_w->set_value(obj_messages[msg].pose().orientation().w());
    }
    else {
      spn_object_pos_x->set_value(0.0);
      spn_object_pos_y->set_value(0.0);
      spn_object_pos_z->set_value(0.0);
      spn_object_rot_x->set_value(0.0);
      spn_object_rot_y->set_value(0.0);
      spn_object_rot_z->set_value(0.0);
      spn_object_rot_w->set_value(0.0);
    }
  }
}

void AnalysisTab::on_button_position_preview_clicked() {
  if(trv_positions->get_selection()->count_selected_rows() == 1) {
    int msgid = -1;
    Gtk::TreeModel::iterator row = trv_positions->get_selection()->get_selected();
    row->get_value(2, msgid);

    if(msgid != -1)
      positionPub->Publish(pos_messages[msgid]);
  }
}

void AnalysisTab::on_button_position_clear_clicked() {
  gazebo::msgs::SceneRobot buf;
  buf.set_controltime(-1.0);
  gazebo::math::Pose p(0.0, 0.0, 0.0, 0.0, 0.0, 0.0);
  buf.mutable_pose()->CopyFrom(gazebo::msgs::Convert(p));
  positionPub->Publish(buf);
}

void AnalysisTab::on_button_angles_preview_clicked() {
  if(trv_angles->get_selection()->count_selected_rows() == 1) {
    int msgid = -1;
    Gtk::TreeModel::iterator row = trv_angles->get_selection()->get_selected();
    row->get_value(2, msgid);

    anglesPub->Publish(ang_messages[msgid]);
  }
}

void AnalysisTab::on_button_angles_clear_clicked() {
  gazebo::msgs::SceneJoint buf;
  buf.set_controltime(-1.0);
  anglesPub->Publish(buf);
}

void AnalysisTab::on_button_object_preview_clicked() {
  if(trv_objects->get_selection()->count_selected_rows() == 1) {
    int msgid = -1;
    Gtk::TreeModel::iterator row = trv_objects->get_selection()->get_selected();

    row->get_value(2, msgid);;
    if(msgid != -1)
      objectPub->Publish(obj_messages[msgid]);
  }
}

void AnalysisTab::on_button_object_move_clicked() {
  if(trv_objects->get_selection()->count_selected_rows() == 1) {
    int msgid = -1;
    Gtk::TreeModel::iterator row = trv_objects->get_selection()->get_selected();

    gazebo::msgs::SceneObject buf;
    row->get_value(2, msgid);;
    if(msgid != -1) {
      buf.CopyFrom(obj_messages[msgid]);

      buf.mutable_pose()->mutable_position()->set_x(spn_object_pos_x->get_value());
      buf.mutable_pose()->mutable_position()->set_y(spn_object_pos_y->get_value());
      buf.mutable_pose()->mutable_position()->set_z(spn_object_pos_z->get_value());
      buf.mutable_pose()->mutable_orientation()->set_x(spn_object_rot_x->get_value());
      buf.mutable_pose()->mutable_orientation()->set_y(spn_object_rot_y->get_value());
      buf.mutable_pose()->mutable_orientation()->set_z(spn_object_rot_z->get_value());
      buf.mutable_pose()->mutable_orientation()->set_w(spn_object_rot_w->get_value());

      objectPub->Publish(buf);
    }
  }
}

void AnalysisTab::on_button_object_clear_clicked() {
  gazebo::msgs::SceneObject buf;
  buf.set_object("");
  buf.set_visible(false);
  gazebo::math::Pose p(0.0, 0.0, 0.0, 0.0, 0.0, 0.0);
  buf.mutable_pose()->CopyFrom(gazebo::msgs::Convert(p));
  buf.set_time(-1.0);
  objectPub->Publish(buf);
}

void AnalysisTab::on_lasers_visible_toggled(const Glib::ustring& path) {
  bool val;
  Gtk::TreeModel::iterator it = trv_lasers->get_model()->get_iter(path);
  it->get_value(1,val);
  it->set_value(1,!val);
}

void AnalysisTab::on_button_grid_show_clicked() {
  gazebo::msgs::Drawing drw;
  drw.set_name("grid");
  if(btn_grid_show->get_label() == "Show") {
    btn_grid_show->set_label("Hide");
    drw.set_visible(true);
    drw.set_material("WhiteGlow");
    drw.set_mode(gazebo::msgs::Drawing::LINE_LIST);
    gazebo::msgs::Pose *pose = drw.mutable_pose();
    gazebo::msgs::Vector3d *pos = pose->mutable_position();
    gazebo::msgs::Quaternion *ori = pose->mutable_orientation();

    pos->set_x(spn_grid_pos_x->get_value());
    pos->set_y(spn_grid_pos_y->get_value());
    pos->set_z(spn_grid_pos_z->get_value());
    ori->set_w(spn_grid_rot_w->get_value());
    ori->set_x(spn_grid_rot_x->get_value());
    ori->set_y(spn_grid_rot_y->get_value());
    ori->set_z(spn_grid_rot_z->get_value());

    int width = spn_grid_width->get_value_as_int();
    int height = spn_grid_height->get_value_as_int();
    double cellsize = spn_grid_size->get_value();

    for(int x = 0; x <= width; x++) {
      gazebo::msgs::Vector3d *p0 = drw.add_point()->mutable_position();
      gazebo::msgs::Vector3d *p1 = drw.add_point()->mutable_position();
      gazebo::math::Vector3 mp0((-width/2+x)*cellsize, -height/2*cellsize, 0.01);
      gazebo::math::Vector3 mp1((-width/2+x)*cellsize, +height/2*cellsize, 0.01);

      *p0 = gazebo::msgs::Convert(mp0);
      *p1 = gazebo::msgs::Convert(mp1);
    }
    for(int y = 0; y <= height; y++) {
      gazebo::msgs::Vector3d *p0 = drw.add_point()->mutable_position();
      gazebo::msgs::Vector3d *p1 = drw.add_point()->mutable_position();
      gazebo::math::Vector3 mp0(-width/2*cellsize, (-height/2+y)*cellsize, 0.01);
      gazebo::math::Vector3 mp1(+width/2*cellsize, (-height/2+y)*cellsize, 0.01);

      *p0 = gazebo::msgs::Convert(mp0);
      *p1 = gazebo::msgs::Convert(mp1);
    }
  }
  else {
    btn_grid_show->set_label("Show");
    drw.set_visible(false);
  }

  drawingPub->Publish(drw);
}

void AnalysisTab::on_button_grid_move_clicked() {
  if(btn_grid_show->get_label() == "Hide") {
    gazebo::msgs::Drawing drw;
    drw.set_name("grid");
    drw.set_visible(true);
    drw.set_material("Gazebo/WhiteGlow");
    drw.set_mode(gazebo::msgs::Drawing::LINE_LIST);
    gazebo::msgs::Pose *pose = drw.mutable_pose();
    gazebo::msgs::Vector3d *pos = pose->mutable_position();
    gazebo::msgs::Quaternion *ori = pose->mutable_orientation();

    pos->set_x(spn_grid_pos_x->get_value());
    pos->set_y(spn_grid_pos_y->get_value());
    pos->set_z(spn_grid_pos_z->get_value());
    ori->set_w(spn_grid_rot_w->get_value());
    ori->set_x(spn_grid_rot_x->get_value());
    ori->set_y(spn_grid_rot_y->get_value());
    ori->set_z(spn_grid_rot_z->get_value());

    int width = spn_grid_width->get_value_as_int();
    int height = spn_grid_height->get_value_as_int();
    double cellsize = spn_grid_size->get_value();

    for(int x = 0; x <= width; x++) {
      gazebo::msgs::Vector3d *p0 = drw.add_point()->mutable_position();
      gazebo::msgs::Vector3d *p1 = drw.add_point()->mutable_position();
      gazebo::math::Vector3 mp0((-width/2.0+x)*cellsize, -height/2.0*cellsize, 0.01);
      gazebo::math::Vector3 mp1((-width/2.0+x)*cellsize, +height/2.0*cellsize, 0.01);

      *p0 = gazebo::msgs::Convert(mp0);
      *p1 = gazebo::msgs::Convert(mp1);
    }
    for(int y = 0; y <= height; y++) {
      gazebo::msgs::Vector3d *p0 = drw.add_point()->mutable_position();
      gazebo::msgs::Vector3d *p1 = drw.add_point()->mutable_position();
      gazebo::math::Vector3 mp0(-width/2.0*cellsize, (-height/2.0+y)*cellsize, 0.01);
      gazebo::math::Vector3 mp1(+width/2.0*cellsize, (-height/2.0+y)*cellsize, 0.01);

      *p0 = gazebo::msgs::Convert(mp0);
      *p1 = gazebo::msgs::Convert(mp1);
    }
    drawingPub->Publish(drw);
  }
}

void AnalysisTab::on_button_lasers_update_clicked() {
  gazebo::msgs::Lasers lasers;
  lasers.set_update(true);

  Gtk::TreeModel::Children rows = trv_lasers->get_model()->children();
  for(Gtk::TreeModel::Children::iterator row = rows.begin(); row != rows.end(); row++) {
    Glib::ustring interface;
    bool visible;
    row->get_value(0, interface);
    row->get_value(1, visible);
    lasers.add_interface(interface);
    lasers.add_visible(visible);
  }

  lasersPub->Publish(lasers);
}

void AnalysisTab::OnLasersMsg(ConstLasersPtr &_msg) {
  {
    boost::mutex::scoped_lock lock(*this->lasersMutex);
    this->lasersMsgs.push_back(*_msg);
  }
  on_lasers_msg();
}

void AnalysisTab::ProcessLasersMsg() {
  boost::mutex::scoped_lock lock(*this->lasersMutex);
  std::list<gazebo::msgs::Lasers>::iterator _msg;
  for(_msg = lasersMsgs.begin(); _msg != lasersMsgs.end(); _msg++) {
    Gtk::TreeModel::Row row;
    int i, v;
    i = _msg->interface_size();
    v = _msg->visible_size();
    if(i == v) {
      lsr_store->clear();
      for(int l=0; l<i; l++) {
        row = *(lsr_store->append());
        row.set_value(0, _msg->interface(l));
        row.set_value(1, _msg->visible(l));
      }
    }
  }
  lasersMsgs.clear();
}
