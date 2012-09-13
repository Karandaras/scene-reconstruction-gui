#include "robotcontrollertab.h"
#include "converter.h"

using namespace SceneReconstruction;

/** @class RobotControllerTab "robotcontrollertab.h"
 *  Tab for the GUI that displays data of the RobotController Gazebo Plugin
 *  @author Bastian Klingen
 */

RobotControllerTab::RobotControllerTab(gazebo::transport::NodePtr& _node, LoggerTab* _logger, Glib::RefPtr<Gtk::Builder>& builder)
: SceneTab::SceneTab(builder)
{
  node = _node;
  logger = _logger;

  sceneReqPub = node->Advertise<gazebo::msgs::Request>("~/SceneReconstruction/RobotController/Request");
  setupPub = node->Advertise<gazebo::msgs::SceneRobotController>("~/SceneReconstruction/RobotController/Setup");
  sceneResSub = node->Subscribe("~/SceneReconstruction/RobotController/Response", &RobotControllerTab::OnResponseMsg, this);

  _builder->get_widget("robotcontroller_treeview", trv_robot);
  rob_store = Glib::RefPtr<Gtk::ListStore>::cast_dynamic(_builder->get_object("robotcontroller_liststore"));
  rob_store->clear();
  std::vector<Gtk::CellRenderer*> rob_col_simangle = trv_robot->get_column(2)->get_cells();
  for(unsigned int i=0; i<rob_col_simangle.size(); i++) {
    dynamic_cast<Gtk::CellRendererText*>(rob_col_simangle[i])->property_editable() = true;
    dynamic_cast<Gtk::CellRendererText*>(rob_col_simangle[i])->signal_edited().connect(sigc::mem_fun(*this,&RobotControllerTab::on_cell_simangle_edited));
  }
  std::vector<Gtk::CellRenderer*> rob_col_offset = trv_robot->get_column(3)->get_cells();
  for(unsigned int i=0; i<rob_col_offset.size(); i++) {
    dynamic_cast<Gtk::CellRendererText*>(rob_col_offset[i])->property_editable() = true;
    dynamic_cast<Gtk::CellRendererText*>(rob_col_offset[i])->signal_edited().connect(sigc::mem_fun(*this,&RobotControllerTab::on_cell_offset_edited));
  }
  std::vector<Gtk::CellRenderer*> rob_col_robangle = trv_robot->get_column(4)->get_cells();
  for(unsigned int i=0; i<rob_col_robangle.size(); i++) {
    dynamic_cast<Gtk::CellRendererText*>(rob_col_robangle[i])->property_editable() = true;
    dynamic_cast<Gtk::CellRendererText*>(rob_col_robangle[i])->signal_edited().connect(sigc::mem_fun(*this,&RobotControllerTab::on_cell_robangle_edited));
  }

  // btn_send
  _builder->get_widget("robotcontroller_toolbutton_send", btn_send);
  btn_send->signal_clicked().connect(sigc::mem_fun(*this,&RobotControllerTab::on_button_send_clicked));

  // tbl_position
  _builder->get_widget("robotcontroller_entry_position_x", ent_posx);
  _builder->get_widget("robotcontroller_entry_position_y", ent_posy);
  _builder->get_widget("robotcontroller_entry_position_z", ent_posz);

  _builder->get_widget("robotcontroller_entry_orientation_w", ent_rotw);
  _builder->get_widget("robotcontroller_entry_orientation_x", ent_rotx);
  _builder->get_widget("robotcontroller_entry_orientation_y", ent_roty);
  _builder->get_widget("robotcontroller_entry_orientation_z", ent_rotz);

  // btn_send
  _builder->get_widget("robotcontroller_toolbutton_reload", btn_reload);
  btn_reload->signal_clicked().connect(sigc::mem_fun(*this,&RobotControllerTab::on_button_reload_clicked));
}

RobotControllerTab::~RobotControllerTab() {
}

void RobotControllerTab::OnResponseMsg(ConstResponsePtr& _msg) {
  if(!robReq || robReq->id() != _msg->id()) 
    return;

  if(_msg->request() == "controller_info") {
    logger->msglog("<<", "~/SceneReconstruction/RobotController/Response", _msg);

    gazebo::msgs::SceneRobotController src;
    if(_msg->has_type() && _msg->type() == src.GetTypeName()) {
      src.ParseFromString(_msg->serialized_data());
      logger->log("robot controller", "receiving info from RobotControllerPlugin");

      int sn, rn, o, sa, ra;
      sn = src.simulator_name_size();
      rn = src.robot_name_size();
      o  = src.offset_size();
      sa = src.simulator_angle_size();
      ra = src.robot_angle_size();

      if(src.has_pos_x() && src.has_pos_y()) {	
	      ent_posx->set_text(Converter::to_ustring(src.pos_x(), 3));
	      ent_posy->set_text(Converter::to_ustring(src.pos_y(), 3));
	      if(src.has_pos_z()) {
  	      ent_posz->set_text(Converter::to_ustring(src.pos_z(), 3));
	      }
        else {
  	      ent_posz->set_text(Converter::to_ustring(0.0, 3));
        }
      }

      if (src.has_rot_w() && src.has_rot_x() && src.has_rot_y() && src.has_rot_z()) {
	      ent_rotw->set_text(Converter::to_ustring(src.rot_w(), 3));
	      ent_rotx->set_text(Converter::to_ustring(src.rot_x(), 3));
	      ent_roty->set_text(Converter::to_ustring(src.rot_y(), 3));
	      ent_rotz->set_text(Converter::to_ustring(src.rot_z(), 3));
      }
      else {
	      ent_rotw->set_text(Converter::to_ustring(0.0, 3));
	      ent_rotx->set_text(Converter::to_ustring(0.0, 3));
	      ent_roty->set_text(Converter::to_ustring(0.0, 3));
	      ent_rotz->set_text(Converter::to_ustring(0.0, 3));
      }

      if(sn == rn && rn == o && o == sa && sa == ra && ra == sn) {
        rob_store->clear();
        Gtk::TreeModel::Row row;

        for(int i=0; i<sn; i++) {
          row = *(rob_store->append());
          row.set_value(0, (Glib::ustring)src.simulator_name(i));
          row.set_value(1, (Glib::ustring)src.robot_name(i));
          row.set_value(2, src.offset(i));
          row.set_value(3, src.simulator_angle(i));
	        row.set_value(4, src.robot_angle(i));
        }
      }
    }

    robReq.reset();
  }
}

void RobotControllerTab::on_button_send_clicked() {
  logger->log("robot controller", "sending data to the robotcontrollerplugin");
  gazebo::msgs::SceneRobotController src;
  Gtk::TreeModel::Children rows = rob_store->children();
  for(Gtk::TreeModel::Children::iterator it = rows.begin(); it != rows.end(); it++) {
    Gtk::TreeModel::Row row = *it;

    Glib::ustring simulator_name;
    row.get_value(0, simulator_name);
    src.add_simulator_name(simulator_name);

    Glib::ustring robot_name;
    row.get_value(1, robot_name);
    src.add_robot_name(robot_name);

    double offset;
    row.get_value(2, offset);
    src.add_offset(offset);

    double simulator_angle;
    row.get_value(3, simulator_angle);
    src.add_simulator_angle(simulator_angle);

    double robot_angle;
    row.get_value(4, robot_angle);
    src.add_robot_angle(robot_angle);
  }

  double pos_x = 0.0;
  double pos_y = 0.0;
  double pos_z = 0.0;
  double rot_w = 0.0;
  double rot_x = 0.0;
  double rot_y = 0.0;
  double rot_z = 0.0;

  pos_x = Converter::ustring_to_double(ent_posx->get_text());
  pos_y = Converter::ustring_to_double(ent_posy->get_text());
  pos_z = Converter::ustring_to_double(ent_posz->get_text());
  rot_w = Converter::ustring_to_double(ent_rotw->get_text());
  rot_x = Converter::ustring_to_double(ent_rotx->get_text());
  rot_y = Converter::ustring_to_double(ent_roty->get_text());
  rot_z = Converter::ustring_to_double(ent_rotz->get_text());

  src.set_pos_x(pos_x);
  src.set_pos_y(pos_y);
  src.set_pos_z(pos_z);
  src.set_rot_w(rot_w);
  src.set_rot_x(rot_x);
  src.set_rot_y(rot_y);
  src.set_rot_z(rot_z);

  setupPub->Publish(src);
  
  logger->msglog(">>", "~/SceneReconstruction/RobotController/Setup", src);
}

void RobotControllerTab::on_button_reload_clicked() {
  logger->log("robot controller", "requesting info from RobotControllerPlugin");
  robReq.reset(gazebo::msgs::CreateRequest("controller_info"));
  sceneReqPub->Publish(*(robReq.get()));
  logger->msglog(">>", "~/SceneReconstruction/RobotController/Request", robReq);
}

void RobotControllerTab::on_cell_simangle_edited(const Glib::ustring& path, const Glib::ustring& new_text) {
  logger->log("robot controller", "Simulation Angle changed");  
  Gtk::TreeIter it = rob_store->get_iter(path);
  
  double simangle =  Converter::ustring_to_double(new_text.data());
  double offset;
  it->get_value(3, offset);
  double robangle = simangle - offset;

  it->set_value(2, simangle);
  it->set_value(3, offset);
  it->set_value(4, robangle);
}

void RobotControllerTab::on_cell_offset_edited(const Glib::ustring& path, const Glib::ustring& new_text) {
  logger->log("robot controller", "Offset changed");
  Gtk::TreeIter it = rob_store->get_iter(path);

  double offset = Converter::ustring_to_double(new_text.data());
  double robangle;
  it->get_value(4, robangle);
  double simangle = robangle + offset;

  it->set_value(2, simangle);
  it->set_value(3, offset);
  it->set_value(4, robangle);
}

void RobotControllerTab::on_cell_robangle_edited(const Glib::ustring& path, const Glib::ustring& new_text) {
  logger->log("robot controller", "Robot Angle changed");
  Gtk::TreeIter it = rob_store->get_iter(path);

  double offset;
  it->get_value(3, offset);
  double robangle = Converter::ustring_to_double(new_text.data());
  double simangle = robangle + offset;

  it->set_value(2, simangle);
  it->set_value(3, offset);
  it->set_value(4, robangle);
}

void RobotControllerTab::set_enabled(bool enabled) {
  Gtk::Widget* tab;
  _builder->get_widget("robotcontroller_tab", tab);
  tab->set_sensitive(enabled);
}
