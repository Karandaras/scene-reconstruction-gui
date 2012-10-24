#include "analysistab.h"
#include "converter.h"

using namespace SceneReconstruction;

/** @class AnalysisTab "AnalysisTab.h"
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
  
  _builder->get_widget("analysis_buffer_position_toolbutton_preview", btn_position_preview);
  btn_position_preview->signal_clicked().connect(sigc::mem_fun(*this,&AnalysisTab::on_button_position_preview_clicked));
  _builder->get_widget("analysis_buffer_joints_toolbutton_preview", btn_angles_preview);
  btn_angles_preview->signal_clicked().connect(sigc::mem_fun(*this,&AnalysisTab::on_button_angles_preview_clicked));
  _builder->get_widget("analysis_buffer_objects_toolbutton_preview", btn_object_preview);
  btn_object_preview->signal_clicked().connect(sigc::mem_fun(*this,&AnalysisTab::on_button_object_preview_clicked));

  _builder->get_widget("analysis_buffer_position_treeview", trv_positions);
  pos_store = Glib::RefPtr<Gtk::TreeStore>::cast_dynamic(_builder->get_object("analysis_buffer_position_treestore"));

  _builder->get_widget("analysis_buffer_joints_treeview", trv_angles);
  ang_store = Glib::RefPtr<Gtk::TreeStore>::cast_dynamic(_builder->get_object("analysis_buffer_joints_treestore"));

  _builder->get_widget("analysis_buffer_objects_treeview", trv_objects);
  obj_store = Glib::RefPtr<Gtk::TreeStore>::cast_dynamic(_builder->get_object("analysis_buffer_objects_treestore"));

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

  positionPub = node->Advertise<gazebo::msgs::BufferPosition>("~/SceneReconstruction/RobotController/Position");
  anglesPub = node->Advertise<gazebo::msgs::BufferJoints>("~/SceneReconstruction/RobotController/Joints");
  objectPub = node->Advertise<gazebo::msgs::BufferObjects>("~/SceneReconstruction/ObjectInstantiator/Object");
  drawingPub = node->Advertise<gazebo::msgs::Drawing>(std::string("~/draw"));
  lasersPub = node->Advertise<gazebo::msgs::Lasers>("~/SceneReconstruction/Framework/Lasers");

  bufferSub = node->Subscribe("~/SceneReconstruction/GUI/Buffer", &AnalysisTab::OnBufferMsg, this);
  lasersSub = node->Subscribe("~/SceneReconstruction/GUI/Lasers", &AnalysisTab::OnLasersMsg, this);
}

AnalysisTab::~AnalysisTab() {
}

void AnalysisTab::OnBufferMsg(ConstMessage_VPtr& _msg) {
  logger->msglog("<<", "~/SceneReconstruction/GUI/Buffer", _msg);
  gazebo::msgs::BufferJoints jnt;
  gazebo::msgs::BufferPosition pos;
  gazebo::msgs::BufferObjects obj;
  
  if(_msg->msgtype() == jnt.GetTypeName()) {
    ang_store->clear();
    ang_messages.clear();
    ang_messages.resize(_msg->msgsdata_size());
    for(int i=0; i<_msg->msgsdata_size(); i++) {
      jnt.ParseFromString(_msg->msgsdata(i));
      Gtk::TreeModel::Row row = *(ang_store->append());
      row.set_value(0, Converter::to_ustring_time(jnt.timestamp()));
      row.set_value(1, (Glib::ustring)"");
      row.set_value(2, i);
      row.set_value(3, false);
      ang_messages[i] = jnt;

      int n = jnt.name_size();
      int a = jnt.angle_size();
      if(n==a) {
        for(int j=0; j<n; j++) {
          Gtk::TreeModel::Row childrow = *(ang_store->append(row.children()));
          childrow.set_value(0,jnt.name(i));
          childrow.set_value(1,Converter::to_ustring(jnt.angle(i)));
        }
      }
    }
  }
  else if(_msg->msgtype() == obj.GetTypeName()) {
    obj_store->clear();
    obj_messages.clear();
    obj_messages.resize(_msg->msgsdata_size());
    for(int i=0; i<_msg->msgsdata_size(); i++) {
      obj.ParseFromString(_msg->msgsdata(i));
      Gtk::TreeModel::Row row = *(obj_store->append());
      row.set_value(0, Converter::to_ustring_time(obj.timestamp()));
      row.set_value(1,(Glib::ustring)"");
      row.set_value(2, i);
      row.set_value(3, false);
      obj_messages[i] = obj;

      int o = obj.object_size();
      for(int j=0; j<o; j++) {
        Gtk::TreeModel::Row childrow = *(obj_store->append(row.children()));
        childrow.set_value(0,obj.object(i).object());
        childrow.set_value(1,(Glib::ustring)"");

        Gtk::TreeModel::Row cchildrow;
        cchildrow = *(obj_store->append(childrow.children()));
        cchildrow.set_value(0,(Glib::ustring)"Visible");
        cchildrow.set_value(1,obj.object(i).visible());

        if(obj.object(i).has_model()) {
          cchildrow = *(obj_store->append(childrow.children()));
          cchildrow.set_value(0,(Glib::ustring)"Model");
          cchildrow.set_value(1,obj.object(i).model());
        }

        cchildrow = *(obj_store->append(childrow.children()));
        cchildrow.set_value(0,(Glib::ustring)"Pose");
        cchildrow.set_value(1,Converter::convert(obj.object(i).pose(), 2, 3));

        if(obj.object(i).has_query()) {
          cchildrow = *(obj_store->append(childrow.children()));
          cchildrow.set_value(0,(Glib::ustring)"Query");
          cchildrow.set_value(1,obj.object(i).query());
        }
      }
    }
  }
  else if(_msg->msgtype() == pos.GetTypeName()) {
    pos_store->clear();
    pos_messages.clear();
    pos_messages.resize(_msg->msgsdata_size());
    for(int i=0; i<_msg->msgsdata_size(); i++) {
      pos.ParseFromString(_msg->msgsdata(i));
      Gtk::TreeModel::Row childrow = *(pos_store->append());
      childrow.set_value(0,obj.object(i).object());
      childrow.set_value(1,Converter::convert(pos.position(), 2, 3));
      childrow.set_value(2, i);
      childrow.set_value(3, false);
      pos_messages[i] = pos;
    }
  }
}

void AnalysisTab::on_button_position_preview_clicked() {
  if(trv_positions->get_selection()->count_selected_rows() == 1) {
    int msgid;
    Gtk::TreeModel::iterator row = trv_positions->get_selection()->get_selected();
    row->get_value(2, msgid);

    positionPub->Publish(pos_messages[msgid]);
  }
}

void AnalysisTab::on_button_angles_preview_clicked() {
  if(trv_angles->get_selection()->count_selected_rows() == 1) {
    int msgid;
    Gtk::TreeModel::iterator row = trv_angles->get_selection()->get_selected();
    if(row->parent())
      row = row->parent();
    row->get_value(2, msgid);

    anglesPub->Publish(ang_messages[msgid]);
  }
}

void AnalysisTab::on_button_object_preview_clicked() {
  if(trv_objects->get_selection()->count_selected_rows() == 1) {
    int msgid;
    Gtk::TreeModel::iterator row = trv_objects->get_selection()->get_selected();
    if(row->parent())
      row = row->parent();
    row->get_value(2, msgid);

    objectPub->Publish(obj_messages[msgid]);
  }
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
  gazebo::msgs::Drawing drw;
  drw.set_name("grid");
  if(btn_grid_show->get_label() == "Hide") {
    drw.set_visible(true);
    drw.set_material("WhiteGlow");
    drw.set_mode(gazebo::msgs::Drawing::LINE_LIST);
    gazebo::math::Vector3 pos(spn_grid_pos_x->get_value(), spn_grid_pos_y->get_value(), spn_grid_pos_z->get_value());
    gazebo::math::Quaternion rot(spn_grid_rot_w->get_value(), spn_grid_rot_x->get_value(), spn_grid_rot_y->get_value(), spn_grid_rot_z->get_value());

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
    drw.set_visible(false);
  }

  drawingPub->Publish(drw);
}

void AnalysisTab::on_button_lasers_update_clicked() {
  gazebo::msgs::Lasers lasers;

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
  Gtk::TreeModel::Row row;
  int i, v;
  i = _msg->interface_size();
  v = _msg->visible_size();
  if(i != v)
    return;

  lsr_store->clear();
  for(int l=0; l<i; l++) {
    row = *(lsr_store->append());
    row.set_value(0, _msg->interface(l));
    row.set_value(1, _msg->visible(l));
  }
}
