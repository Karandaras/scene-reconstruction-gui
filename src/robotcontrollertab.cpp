#include "robotcontrollertab.h"

using namespace SceneReconstruction;

RobotControllerTab::RobotControllerTab(gazebo::transport::NodePtr& _node, LoggerTab* _logger)
: SceneTab::SceneTab("RobotController"),
  btn_send("SEND"),
  btn_reload("RELOAD") {
  node = _node;
  logger = _logger;

  sceneReqPub = node->Advertise<gazebo::msgs::Request>("~/SceneReconstruction/Request");
  sceneResSub = node->Subscribe("~/SceneReconstruction/Response", &RobotControllerTab::OnResponseMsg, this);

  trv_robot.set_model(roc_store = Gtk::ListStore::create(roc_cols));
  
  trv_robot.append_column("Simulator Name", roc_cols.col_simname);
  trv_robot.append_column("Robot Name", roc_cols.col_robname);
  trv_robot.append_column_editable("Simulator Angle", roc_cols.col_simangle);
  trv_robot.append_column_editable("Offset", roc_cols.col_offset);
  trv_robot.append_column_editable("Robot Angle", roc_cols.col_robangle);

  scw_robot.add(trv_robot);
  scw_robot.set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);
  scw_robot.set_size_request(650,140);
  scw_robot.set_hexpand(true);
  scw_robot.set_vexpand(true);

  // btn_send
  btn_send.set_size_request(60,60);
  btn_send.set_hexpand(false);
  btn_send.set_vexpand(false);
  box_robot.pack_start(btn_send,false,false);
  btn_send.signal_clicked().connect(sigc::mem_fun(*this,&RobotControllerTab::on_button_send_clicked));

  // btn_send
  btn_reload.set_size_request(60,60);
  btn_reload.set_hexpand(false);
  btn_reload.set_vexpand(false);
  box_robot.pack_start(btn_reload,false,false);
  btn_reload.signal_clicked().connect(sigc::mem_fun(*this,&RobotControllerTab::on_button_reload_clicked));

  box_robot.set_spacing(5);
  grd_robot.attach(box_robot,0,0,650,70);
  grd_robot.attach(scw_robot, 0, 70, 650, 140);
}

RobotControllerTab::~RobotControllerTab() {
}

void RobotControllerTab::OnResponseMsg(ConstResponsePtr& _msg) {
  if(!robReq || robReq->id() != _msg->id()) 
    return;

  if(_msg->request() == "controller_info") {
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
	// ROBOT POSITION (X,Y) AVAILABLE
	if(src.has_pos_z()) {
	  // ROBOT POSITION (X,Y,Z) AVAILABLE
	}
      }

      if (src.has_rot_w() && src.has_rot_x() && src.has_rot_y() && src.has_rot_z()) {
        // ROBOT ORIENTATION (W,X,Y,Z) AVAILABLE
      }

      if(sn == rn && rn == o && o == sa && sa == ra && ra == sn) {
        trv_robot.set_model(roc_store = Gtk::ListStore::create(roc_cols));
        Gtk::TreeModel::Row row;

        for(int i=0; i<sn; i++) {
          row = *(roc_store->append());
          row[roc_cols.col_simname]  = src.simulator_name(i);
          row[roc_cols.col_robname]  = src.robot_name(i);
          row[roc_cols.col_offset]   = src.offset(i);
          row[roc_cols.col_simangle] = src.simulator_angle(i);
	        row[roc_cols.col_robangle] = src.robot_angle(i);
        }
      }
    }
  }
}

void RobotControllerTab::on_button_send_clicked() {
  logger->log("robot controller", "SEND");
}

void RobotControllerTab::on_button_reload_clicked() {
  logger->log("robot controller", "RELOAD");
  robReq = gazebo::msgs::CreateRequest("controller_info");
  sceneReqPub->Publish(*robReq);
  logger->log("robot controller", "requesting info from RobotControllerPlugin");
}

void RobotControllerTab::on_cell_simangle_edited(const Glib::ustring& path, const Glib::ustring& new_text) {
  logger->log("robot controller", "Simulation Angle changed");  
  Gtk::TreeIter it = trv_robot.get_model()->get_iter(path);
  
  double simangle =  strtod(new_text.data(), NULL);
  double offset;
  offset = it->get_value(roc_cols.col_offset);
  double robangle = simangle - offset;

  it->set_value(roc_cols.col_simangle, simangle);
  it->set_value(roc_cols.col_offset, offset);
  it->set_value(roc_cols.col_robangle, robangle);
}

void RobotControllerTab::on_cell_offset_edited(const Glib::ustring& path, const Glib::ustring& new_text) {
  logger->log("robot controller", "Offset changed");
  Gtk::TreeIter it = trv_robot.get_model()->get_iter(path);

  double offset = strtod(new_text.data(), NULL);
  double robangle;
  robangle = it->get_value(roc_cols.col_robangle);
  double simangle = robangle + offset;

  it->set_value(roc_cols.col_simangle, simangle);
  it->set_value(roc_cols.col_offset, offset);
  it->set_value(roc_cols.col_robangle, robangle);
}

void RobotControllerTab::on_cell_robangle_edited(const Glib::ustring& path, const Glib::ustring& new_text) {
  logger->log("robot controller", "Robot Angle changed");
  Gtk::TreeIter it = trv_robot.get_model()->get_iter(path);

  double offset;
  offset = it->get_value(roc_cols.col_offset);
  double robangle = strtod(new_text.data(), NULL);
  double simangle = robangle + offset;

  it->set_value(roc_cols.col_simangle, simangle);
  it->set_value(roc_cols.col_offset, offset);
  it->set_value(roc_cols.col_robangle, robangle);
}

Gtk::Widget& RobotControllerTab::get_tab() {
  return grd_robot;
}

