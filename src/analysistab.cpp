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
  pos_store->clear();

  _builder->get_widget("analysis_buffer_joints_treeview", trv_angles);
  ang_store = Glib::RefPtr<Gtk::TreeStore>::cast_dynamic(_builder->get_object("analysis_buffer_joints_treestore"));
  ang_store->clear();

  _builder->get_widget("analysis_buffer_objects_treeview", trv_objects);
  obj_store = Glib::RefPtr<Gtk::TreeStore>::cast_dynamic(_builder->get_object("analysis_buffer_objects_treestore"));
  obj_store->clear();

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
//  btn_grid_show->signal_clicked().connect(sigc::mem_fun(*this,&AnalysisTab::on_button_grid_show_clicked));
  _builder->get_widget("analysis_toolbox_grid_button_move", btn_grid_move);
//  btn_grid_move->signal_clicked().connect(sigc::mem_fun(*this,&AnalysisTab::on_button_grid_move_clicked));

  _builder->get_widget("analysis_toolbox_robot_treeview_lasers", trv_lasers);
  lsr_store = Glib::RefPtr<Gtk::TreeStore>::cast_dynamic(_builder->get_object("analysis_toolbox_robot_liststore_lasers"));
//  lsr_store->clear();
  _builder->get_widget("analysis_toolbox_robot_button_lasers_update", btn_lasers_update);
//  btn_lasers_update->signal_clicked().connect(sigc::mem_fun(*this,&AnalysisTab::on_button_grid_show_clicked));

  std::vector<Gtk::CellRenderer*> las_col_visible = trv_lasers->get_column(1)->get_cells();
  for(unsigned int i=0; i<las_col_visible.size(); i++) {
    dynamic_cast<Gtk::CellRendererToggle*>(las_col_visible[i])->set_activatable(true);
    dynamic_cast<Gtk::CellRendererToggle*>(las_col_visible[i])->signal_toggled().connect(sigc::mem_fun(*this,&AnalysisTab::on_lasers_visible_toggled));
  }

  positionPub = node->Advertise<gazebo::msgs::Pose>("~/SceneReconstruction/RobotController/Position");
  anglesPub = node->Advertise<gazebo::msgs::Message_V>("~/SceneReconstruction/RobotController/Joints");
  objectPub = node->Advertise<gazebo::msgs::SceneObject>("~/SceneReconstruction/ObjectInstantiator/Object");

  bufferSub = node->Subscribe("~/SceneReconstruction/GUI/Buffer", &AnalysisTab::OnBufferMsg, this);
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
    for(int i=0; i<_msg->msgsdata_size(); i++) {
      jnt.ParseFromString(_msg->msgsdata(i));
      Gtk::TreeModel::Row row = *(ang_store->append());
      row.set_value(0, Converter::to_ustring_time(jnt.timestamp()));
      row.set_value(1,(Glib::ustring)"");

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
    for(int i=0; i<_msg->msgsdata_size(); i++) {
      obj.ParseFromString(_msg->msgsdata(i));
      Gtk::TreeModel::Row row = *(obj_store->append());
      row.set_value(0, Converter::to_ustring_time(obj.timestamp()));
      row.set_value(1,(Glib::ustring)"");

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
    for(int i=0; i<_msg->msgsdata_size(); i++) {
      pos.ParseFromString(_msg->msgsdata(i));
      Gtk::TreeModel::Row childrow = *(pos_store->append());
      childrow.set_value(0,obj.object(i).object());
      childrow.set_value(1,Converter::convert(pos.position(), 2, 3));
    }
  }
}

void AnalysisTab::on_button_position_preview_clicked() {
  // TODO: send message to robotcontroller to move robot
  gazebo::msgs::Pose position;

  // TODO: fill pose message from selected row

  positionPub->Publish(position);
}

void AnalysisTab::on_button_angles_preview_clicked() {
  // TODO: send message to robotcontroller to move joints
  gazebo::msgs::Message_V angles;
  gazebo::msgs::SceneJoint jnt;
  angles.set_msgtype(jnt.GetTypeName());

  // TODO: fill joint messages from selected row

  anglesPub->Publish(angles);
}

void AnalysisTab::on_button_object_preview_clicked() {
  // TODO: send message to objectinstantiator to clone object and set different position
  gazebo::msgs::SceneObject object;

  // TODO: fill object message from selected row

  objectPub->Publish(object);
}

void AnalysisTab::on_lasers_visible_toggled(const Glib::ustring& path) {
  bool val;
  Gtk::TreeModel::iterator it = trv_lasers->get_model()->get_iter(path);
  it->get_value(1,val);
  it->set_value(1,!val);
}

