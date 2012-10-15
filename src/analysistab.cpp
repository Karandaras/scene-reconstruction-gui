
#include "AnalysisTab.h"
#include "converter.h"

using namespace SceneReconstruction;

/** @class AnalysisTab "AnalysisTab.h"
 *  Tab for the GUI
 *  @author Bastian Klingen
 */

AnalysisTab::AnalysisTab(gazebo::transport::NodePtr& _node, LoggerTab* _logger, Glib::RefPtr<Gtk::Builder>& builder)
: SceneTab::SceneTab(builder)
{
  node = _node;
  logger = _logger;
  
  // rng_time setup
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

  positionPub = node->Advertise<gazebo::msgs::Request>("~/SceneReconstruction/RobotController/Position");
  anglesPub = node->Advertise<gazebo::msgs::SceneFrameworkControl>("~/SceneReconstruction/RobotController/Joints");
  objectPub = node->Advertise<gazebo::msgs::WorldControl>("~/SceneReconstruction/ObjectInstantiator/Object");

  bufferSub = node->Subscribe("~/SceneReconstruction/GUI/Buffer", &AnalysisTab::OnBufferMsg, this);
}

AnalysisTab::~AnalysisTab() {
}

void AnalysisTab::OnBufferMsg(ConstMessage_VPtr& _msg) {
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
          childrow.set_value(1,Converter::to_ustring(jnt.angle(i));
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
    for(unsigned int i=0; i<_msg->msgsdata_size(); i++) {
      pos.ParseFromString(_msg->msgsdata(i));
      Gtk::TreeModel::Row childrow = *(pos_store->append(row.children()));
      childrow.set_value(0,obj.object(i).object());
      childrow.set_value(1,Converter::convert(pos.pose(), 2, 3));
    }
  }
}

void AnalysisTab::on_button_position_preview_clicked() {
  // TODO: send message to robotcontroller to move robot
}

void AnalysisTab::on_button_angles_preview_clicked() {
  // TODO: send message to robotcontroller to move joints
}

void AnalysisTab::on_button_object_preview_clicked() {
  // TODO: send message to objectinstantiator to clone object and set different position
}

