#include "robotcontrollertab.h"
#include "converter.h"

using namespace SceneReconstruction;

/** @class RobotControllerTab "robotcontrollertab.h"
 * Tab for the GUI that displays data of the RobotController Gazebo Plugin
 * @author Bastian Klingen
 */

RobotControllerTab::RobotControllerTab(gazebo::transport::NodePtr& _node, LoggerTab* _logger)
: SceneTab::SceneTab("RobotController"),
  tbl_robot(9,9,false)
{
  node = _node;
  logger = _logger;
  tbl_robot.set_col_spacings(5);

  sceneReqPub = node->Advertise<gazebo::msgs::Request>("~/SceneReconstruction/RobotController/Request");
  sceneResSub = node->Subscribe("~/SceneReconstruction/RobotController/Response", &RobotControllerTab::OnResponseMsg, this);

  trv_robot.set_model(roc_store = Gtk::ListStore::create(roc_cols));

  int c;  

  c = trv_robot.append_column("Simulator Name", roc_cols.simname);
  trv_robot.get_column(c-1)->set_expand(true);
  trv_robot.get_column(c-1)->set_reorderable(true);

  c = trv_robot.append_column("Robot Name", roc_cols.robname);
  trv_robot.get_column(c-1)->set_expand(true);
  trv_robot.get_column(c-1)->set_reorderable(true);

  c = trv_robot.append_column_editable("Simulator Angle", roc_cols.simangle);
  trv_robot.get_column(c-1)->set_expand(true);
  trv_robot.get_column(c-1)->set_reorderable(true);

  c = trv_robot.append_column_editable("Offset", roc_cols.offset);
  trv_robot.get_column(c-1)->set_expand(true);
  trv_robot.get_column(c-1)->set_reorderable(true);

  c = trv_robot.append_column_editable("Robot Angle", roc_cols.robangle);
  trv_robot.get_column(c-1)->set_expand(true);
  trv_robot.get_column(c-1)->set_reorderable(true);

  trv_robot.set_hexpand(true);
  
  std::vector<Gtk::CellRenderer*> rob_col_simangle = trv_robot.get_column(2)->get_cells();
  for(unsigned int i=0; i<rob_col_simangle.size(); i++) {
    dynamic_cast<Gtk::CellRendererText*>(rob_col_simangle[i])->signal_edited().connect(sigc::mem_fun(*this,&RobotControllerTab::on_cell_simangle_edited));
  }
  std::vector<Gtk::CellRenderer*> rob_col_offset = trv_robot.get_column(3)->get_cells();
  for(unsigned int i=0; i<rob_col_offset.size(); i++) {
    dynamic_cast<Gtk::CellRendererText*>(rob_col_offset[i])->signal_edited().connect(sigc::mem_fun(*this,&RobotControllerTab::on_cell_offset_edited));
  }
  std::vector<Gtk::CellRenderer*> rob_col_robangle = trv_robot.get_column(4)->get_cells();
  for(unsigned int i=0; i<rob_col_robangle.size(); i++) {
    dynamic_cast<Gtk::CellRendererText*>(rob_col_robangle[i])->signal_edited().connect(sigc::mem_fun(*this,&RobotControllerTab::on_cell_robangle_edited));
  }

  scw_robot.add(trv_robot);
  scw_robot.set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);
  scw_robot.set_size_request(650,140);
  scw_robot.set_hexpand(true);
  scw_robot.set_vexpand(true);

  // btn_send
  btn_send.set_label("SEND");
  btn_send.set_size_request(70,70);
  btn_send.set_hexpand(false);
  btn_send.set_vexpand(false);
  tbl_robot.attach(btn_send,0,1,0,3, Gtk::EXPAND|Gtk::FILL, Gtk::FILL);
  btn_send.signal_clicked().connect(sigc::mem_fun(*this,&RobotControllerTab::on_button_send_clicked));

  // tbl_position
  lbl_pos.set_text("Position");
  lbl_pos.set_hexpand(false);
  lbl_pos.set_vexpand(false);
  tbl_robot.attach(lbl_pos, 1, 4, 0, 1, Gtk::FILL, Gtk::FILL);

  lbl_posx.set_text("X");
  lbl_posx.set_hexpand(false);
  lbl_posx.set_vexpand(false);
  ent_posx.set_size_request(70,20);
  ent_posx.set_width_chars(6);
  ent_posx.set_hexpand(false);
  ent_posx.set_vexpand(false);
  tbl_robot.attach(lbl_posx, 1, 2, 1, 2, Gtk::FILL, Gtk::FILL);
  tbl_robot.attach(ent_posx, 1, 2, 2, 3, Gtk::FILL, Gtk::FILL);
  lbl_posy.set_text("Y");
  lbl_posy.set_hexpand(false);
  lbl_posy.set_vexpand(false);
  ent_posy.set_size_request(70,20);
  ent_posy.set_width_chars(6);
  ent_posy.set_hexpand(false);
  ent_posy.set_vexpand(false);
  tbl_robot.attach(lbl_posy, 2, 3, 1, 2, Gtk::FILL, Gtk::FILL);
  tbl_robot.attach(ent_posy, 2, 3, 2, 3, Gtk::FILL, Gtk::FILL);
  lbl_posz.set_text("Z");
  lbl_posz.set_hexpand(false);
  lbl_posz.set_vexpand(false);
  ent_posz.set_size_request(70,20);
  ent_posz.set_width_chars(6);
  ent_posz.set_hexpand(false);
  ent_posz.set_vexpand(false);
  tbl_robot.attach(lbl_posz, 3, 4, 1, 2, Gtk::FILL, Gtk::FILL);
  tbl_robot.attach(ent_posz, 3, 4, 2, 3, Gtk::FILL, Gtk::FILL);

  lbl_rot.set_text("Rotation");
  lbl_rot.set_hexpand(false);
  lbl_rot.set_vexpand(false);
  tbl_robot.attach(lbl_rot, 4, 8, 0, 1, Gtk::FILL, Gtk::FILL);

  lbl_rotw.set_text("W");
  lbl_rotw.set_hexpand(false);
  lbl_rotw.set_vexpand(false);
  ent_rotw.set_size_request(70,20);
  ent_rotw.set_width_chars(6);
  ent_rotw.set_hexpand(false);
  ent_rotw.set_vexpand(false);
  tbl_robot.attach(lbl_rotw, 4, 5, 1, 2, Gtk::FILL, Gtk::FILL);
  tbl_robot.attach(ent_rotw, 4, 5, 2, 3, Gtk::FILL, Gtk::FILL);
  lbl_rotx.set_text("X");
  lbl_rotx.set_hexpand(false);
  lbl_rotx.set_vexpand(false);
  ent_rotx.set_size_request(70,20);
  ent_rotx.set_width_chars(6);
  ent_rotx.set_hexpand(false);
  ent_rotx.set_vexpand(false);
  tbl_robot.attach(lbl_rotx, 5, 6, 1, 2, Gtk::FILL, Gtk::FILL);
  tbl_robot.attach(ent_rotx, 5, 6, 2, 3, Gtk::FILL, Gtk::FILL);
  lbl_roty.set_text("Y");
  lbl_roty.set_hexpand(false);
  lbl_roty.set_vexpand(false);
  ent_roty.set_size_request(70,20);
  ent_roty.set_width_chars(6);
  ent_roty.set_hexpand(false);
  ent_roty.set_vexpand(false);
  tbl_robot.attach(lbl_roty, 6, 7, 1, 2, Gtk::FILL, Gtk::FILL);
  tbl_robot.attach(ent_roty, 6, 7, 2, 3, Gtk::FILL, Gtk::FILL);
  lbl_rotz.set_text("Z");
  lbl_rotz.set_hexpand(false);
  lbl_rotz.set_vexpand(false);
  ent_rotz.set_size_request(70,20);
  ent_rotz.set_width_chars(6);
  ent_rotz.set_hexpand(false);
  ent_rotz.set_vexpand(false);
  tbl_robot.attach(lbl_rotz, 7, 8, 1, 2, Gtk::FILL, Gtk::FILL);
  tbl_robot.attach(ent_rotz, 7, 8, 2, 3, Gtk::FILL, Gtk::FILL);

  // btn_send
  btn_reload.set_label("RELOAD");
  btn_reload.set_hexpand(false);
  btn_reload.set_vexpand(false);
  tbl_robot.attach(btn_reload,8,9,0,3, Gtk::EXPAND|Gtk::FILL, Gtk::FILL);
  btn_reload.signal_clicked().connect(sigc::mem_fun(*this,&RobotControllerTab::on_button_reload_clicked));

  tbl_robot.attach(scw_robot, 0, 9, 3, 9, Gtk::EXPAND|Gtk::FILL, Gtk::EXPAND|Gtk::FILL);
  tbl_robot.show_all_children();
}

RobotControllerTab::~RobotControllerTab() {
}

void RobotControllerTab::OnResponseMsg(ConstResponsePtr& _msg) {
  if(!robReq || robReq->id() != _msg->id()) 
    return;

  if(_msg->request() == "controller_info") {
    logger->msglog("<<", _msg);

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
	      ent_posx.set_text(Converter::to_ustring(src.pos_x()));
	      ent_posy.set_text(Converter::to_ustring(src.pos_y()));
	      if(src.has_pos_z()) {
  	      ent_posz.set_text(Converter::to_ustring(src.pos_z()));
	      }
      }

      if (src.has_rot_w() && src.has_rot_x() && src.has_rot_y() && src.has_rot_z()) {
	      ent_rotw.set_text(Converter::to_ustring(src.rot_w()));
	      ent_rotx.set_text(Converter::to_ustring(src.rot_x()));
	      ent_roty.set_text(Converter::to_ustring(src.rot_y()));
	      ent_rotz.set_text(Converter::to_ustring(src.rot_z()));
      }

      if(sn == rn && rn == o && o == sa && sa == ra && ra == sn) {
        trv_robot.set_model(roc_store = Gtk::ListStore::create(roc_cols));
        Gtk::TreeModel::Row row;

        for(int i=0; i<sn; i++) {
          row = *(roc_store->append());
          row[roc_cols.simname]  = src.simulator_name(i);
          row[roc_cols.robname]  = src.robot_name(i);
          row[roc_cols.offset]   = src.offset(i);
          row[roc_cols.simangle] = src.simulator_angle(i);
	        row[roc_cols.robangle] = src.robot_angle(i);
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
  logger->msglog(">>", robReq);
}

void RobotControllerTab::on_cell_simangle_edited(const Glib::ustring& path, const Glib::ustring& new_text) {
  logger->log("robot controller", "Simulation Angle changed");  
  Gtk::TreeIter it = trv_robot.get_model()->get_iter(path);
  
  double simangle =  strtod(new_text.data(), NULL);
  double offset;
  offset = it->get_value(roc_cols.offset);
  double robangle = simangle - offset;

  it->set_value(roc_cols.simangle, simangle);
  it->set_value(roc_cols.offset, offset);
  it->set_value(roc_cols.robangle, robangle);
}

void RobotControllerTab::on_cell_offset_edited(const Glib::ustring& path, const Glib::ustring& new_text) {
  logger->log("robot controller", "Offset changed");
  Gtk::TreeIter it = trv_robot.get_model()->get_iter(path);

  double offset = strtod(new_text.data(), NULL);
  double robangle;
  robangle = it->get_value(roc_cols.robangle);
  double simangle = robangle + offset;

  it->set_value(roc_cols.simangle, simangle);
  it->set_value(roc_cols.offset, offset);
  it->set_value(roc_cols.robangle, robangle);
}

void RobotControllerTab::on_cell_robangle_edited(const Glib::ustring& path, const Glib::ustring& new_text) {
  logger->log("robot controller", "Robot Angle changed");
  Gtk::TreeIter it = trv_robot.get_model()->get_iter(path);

  double offset;
  offset = it->get_value(roc_cols.offset);
  double robangle = strtod(new_text.data(), NULL);
  double simangle = robangle + offset;

  it->set_value(roc_cols.simangle, simangle);
  it->set_value(roc_cols.offset, offset);
  it->set_value(roc_cols.robangle, robangle);
}

Gtk::Widget& RobotControllerTab::get_tab() {
  return tbl_robot;
}

