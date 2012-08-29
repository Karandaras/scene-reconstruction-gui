#include "robotcontrollertab.h"
#include "converter.h"

using namespace SceneReconstruction;

/** @class RobotControllerTab "robotcontrollertab.h"
 * Tab for the GUI that displays data of the RobotController Gazebo Plugin
 * @author Bastian Klingen
 */

RobotControllerTab::RobotControllerTab(gazebo::transport::NodePtr& _node, LoggerTab* _logger, Glib::RefPtr<Gtk::Builder>& builder)
: SceneTab::SceneTab(builder)
{
  node = _node;
  logger = _logger;

  sceneReqPub = node->Advertise<gazebo::msgs::Request>("~/SceneReconstruction/RobotController/Request");
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
	      ent_posx->set_text(Converter::to_ustring(src.pos_x()));
	      ent_posy->set_text(Converter::to_ustring(src.pos_y()));
	      if(src.has_pos_z()) {
  	      ent_posz->set_text(Converter::to_ustring(src.pos_z()));
	      }
        else {
  	      ent_posz->set_text("");
        }
      }

      if (src.has_rot_w() && src.has_rot_x() && src.has_rot_y() && src.has_rot_z()) {
	      ent_rotw->set_text(Converter::to_ustring(src.rot_w()));
	      ent_rotx->set_text(Converter::to_ustring(src.rot_x()));
	      ent_roty->set_text(Converter::to_ustring(src.rot_y()));
	      ent_rotz->set_text(Converter::to_ustring(src.rot_z()));
      }
      else {
	      ent_rotw->set_text("");
	      ent_rotx->set_text("");
	      ent_roty->set_text("");
	      ent_rotz->set_text("");
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
  logger->log("robot controller", "SEND");
}

void RobotControllerTab::on_button_reload_clicked() {
  logger->log("robot controller", "RELOAD");
  robReq.reset(gazebo::msgs::CreateRequest("controller_info"));
  sceneReqPub->Publish(*(robReq.get()));
  logger->log("robot controller", "requesting info from RobotControllerPlugin");
  logger->msglog(">>", "~/SceneReconstruction/RobotController/Request", robReq);
}

void RobotControllerTab::on_cell_simangle_edited(const Glib::ustring& path, const Glib::ustring& new_text) {
  logger->log("robot controller", "Simulation Angle changed");  
  Gtk::TreeIter it = rob_store->get_iter(path);
  
  double simangle =  strtod(new_text.data(), NULL);
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

  double offset = strtod(new_text.data(), NULL);
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
  double robangle = strtod(new_text.data(), NULL);
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
