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

  _builder->get_widget("robotcontroller_treeview", trv_robot);
  rob_store = Glib::RefPtr<Gtk::ListStore>::cast_dynamic(_builder->get_object("robotcontroller_liststore"));
  rob_store->clear();

  // tbl_position
  _builder->get_widget("robotcontroller_entry_position_x", ent_posx);
  _builder->get_widget("robotcontroller_entry_position_y", ent_posy);
  _builder->get_widget("robotcontroller_entry_position_z", ent_posz);

  _builder->get_widget("robotcontroller_entry_orientation_w", ent_oriw);
  _builder->get_widget("robotcontroller_entry_orientation_x", ent_orix);
  _builder->get_widget("robotcontroller_entry_orientation_y", ent_oriy);
  _builder->get_widget("robotcontroller_entry_orientation_z", ent_oriz);

  controllerSub = node->Subscribe("~/SceneReconstruction/RobotController/ControllerInfo", &RobotControllerTab::OnControllerInfoMsg, this);
}

RobotControllerTab::~RobotControllerTab() {
}

void RobotControllerTab::OnControllerInfoMsg(ConstSceneRobotControllerPtr& _msg) {
  logger->msglog("<<", "~/SceneReconstruction/RobotController/ControllerInfo", *_msg);
  logger->log("robot controller", "receiving info from RobotControllerPlugin");

  int sn, rn, gr, sn2, o, sa, ra;
  sn  = _msg->simulator_name_size();
  rn  = _msg->robot_name_size();
  gr  = _msg->gripper_size();
  sn2 = _msg->simulator_name2_size();
  o   = _msg->offset_size();
  sa  = _msg->simulator_angle_size();
  ra  = _msg->robot_angle_size();

  if(_msg->has_pos_x() && _msg->has_pos_y()) {	
    ent_posx->set_text(Converter::to_ustring(_msg->pos_x(), 3));
    ent_posy->set_text(Converter::to_ustring(_msg->pos_y(), 3));
    if(_msg->has_pos_z()) {
      ent_posz->set_text(Converter::to_ustring(_msg->pos_z(), 3));
    }
    else {
      ent_posz->set_text(Converter::to_ustring(0.0, 3));
    }
  }

  if (_msg->has_ori_w() && _msg->has_ori_x() && _msg->has_ori_y() && _msg->has_ori_z()) {
    ent_oriw->set_text(Converter::to_ustring(_msg->ori_w(), 3));
    ent_orix->set_text(Converter::to_ustring(_msg->ori_x(), 3));
    ent_oriy->set_text(Converter::to_ustring(_msg->ori_y(), 3));
    ent_oriz->set_text(Converter::to_ustring(_msg->ori_z(), 3));
  }
  else {
    ent_oriw->set_text(Converter::to_ustring(0.0, 3));
    ent_orix->set_text(Converter::to_ustring(0.0, 3));
    ent_oriy->set_text(Converter::to_ustring(0.0, 3));
    ent_oriz->set_text(Converter::to_ustring(0.0, 3));
  }

  if(sn == rn && rn == gr && gr == sn2 && sn2 == o && o == sa && sa == ra && ra == sn) {
    rob_store->clear();
    Gtk::TreeModel::Row row;

    for(int i=0; i<sn; i++) {
      row = *(rob_store->append());
      row.set_value(0, (Glib::ustring)_msg->simulator_name(i));
      row.set_value(1, (Glib::ustring)_msg->robot_name(i));
      row.set_value(2, _msg->gripper(i));
      row.set_value(3, (Glib::ustring)_msg->simulator_name2(i));
      row.set_value(4, _msg->offset(i));
      row.set_value(5, _msg->simulator_angle(i));
      row.set_value(6, _msg->robot_angle(i));
    }
  }
}

