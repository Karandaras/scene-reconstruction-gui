#include "math/Pose.hh"

#include "controltab.h"
#include "converter.h"

using namespace SceneReconstruction;

/** @class ControlTab "controltab.h"
 * Tab for the GUI that allows the user to control the scene recontruction
 * by jumping to specific times or (un)pausing. It also displays coordinates
 * of the currently selected object.
 * @author Bastian Klingen
 */

ControlTab::ControlTab(gazebo::transport::NodePtr& _node, LoggerTab* _logger, Glib::RefPtr<Gtk::Builder>& builder)
: SceneTab::SceneTab(builder)
{
  node = _node;
  logger = _logger;
  old_value = 0.0;
  
  reqSub = node->Subscribe("~/request", &ControlTab::OnReqMsg, this);
  resSub = node->Subscribe("~/response", &ControlTab::OnResMsg, this);

  // rng_time setup
  _builder->get_widget("control_scale", rng_time);
  rng_time->signal_button_release_event().connect(sigc::mem_fun(*this,&ControlTab::on_scale_button_event), false);
  rng_time->signal_key_release_event().connect(sigc::mem_fun(*this,&ControlTab::on_scale_key_event), false);

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
  dat_store = Glib::RefPtr<Gtk::TreeStore>::cast_dynamic(trv_data->get_model());
  Gtk::TreeModel::Row row = *(dat_store->append());
  row.set_value(0,(Glib::ustring)"Coordinates");
  row.set_value(1,(Glib::ustring)"");

  Gtk::TreeModel::Row childrow = *(dat_store->append(row.children()));
  childrow.set_value(0,(Glib::ustring)"Gazebo");
  childrow.set_value(1,(Glib::ustring)"X: 0 Y: 0 Z: 0");

  childrow = *(dat_store->append(row.children()));
  childrow.set_value(0,(Glib::ustring)"Robot");
  childrow.set_value(1,(Glib::ustring)"X: 0 Y: 0 Z: 0");

  childrow = *(dat_store->append(row.children()));
  childrow.set_value(0,(Glib::ustring)"Sensor");
  childrow.set_value(1,(Glib::ustring)"X: 0 Y: 0 Z: 0");
  trv_data->expand_all();
}

ControlTab::~ControlTab() {
}

void ControlTab::OnReqMsg(ConstRequestPtr& _msg) {
  if (guiRes && _msg->request() == "entity_info" && _msg->id() == guiRes->id()) {
    logger->msglog("<<", _msg);

    gazebo::msgs::Model model;
    if (guiRes->has_type() && guiRes->type() == model.GetTypeName()) {
      model.ParseFromString(guiRes->serialized_data());
      logger->log("control", "Coords of Model " + model.name() + " received.");
      update_coords(model);
    } else {
      guiRes.reset();
      guiReq = _msg;
    }
  } else {
    guiReq = _msg;
  }
}

void ControlTab::OnResMsg(ConstResponsePtr& _msg) {
  if (guiReq && _msg->request() == "entity_info" && _msg->id() == guiReq->id()) {
    logger->msglog("<<", _msg);

    gazebo::msgs::Model model;
    if (_msg->has_type() && _msg->type() == model.GetTypeName()) {
      model.ParseFromString(_msg->serialized_data());
      logger->log("control", "Coords of Model " + model.name() + " received.");
      update_coords(model);
    } else {
      guiReq.reset();
      guiRes = _msg;
    }
  } else {
    guiRes = _msg;
  }
}

void ControlTab::update_coords(gazebo::msgs::Model model) {
  // Update Coords-Treeview
  Gtk::TreeModel::Children rows = trv_data->get_model()->children();
  for(Gtk::TreeModel::Children::iterator iter = rows.begin(); iter != rows.end(); ++iter) {
    Gtk::TreeModel::Row row = *iter;
    Glib::ustring tmp;
    row.get_value(0, tmp);
    if(tmp == "Coordinates" && model.has_pose()) {
      Gtk::TreeModel::Children childrows = row.children();
      Gtk::TreeModel::Children::iterator childiter = childrows.begin();
      Gtk::TreeModel::Row childrow = *childiter;
      childrow.set_value(1, Converter::convert(model.pose(), 0, 3));
    }
  }
}

void ControlTab::on_button_stop_clicked() {
  logger->log("control", "STOP");
}

void ControlTab::on_button_play_clicked() {
  logger->log("control", "PLAY");
}

void ControlTab::on_button_pause_clicked() {
  logger->log("control", "PAUSE");
}

bool ControlTab::on_scale_button_event(GdkEventButton* b) {
  if(rng_time->get_value() != old_value) {
    logger->log("control", "Time changed from %.2f to %.2f using button %d of the mouse on rng_time", old_value, rng_time->get_value(), b->button);
    old_value = rng_time->get_value();
  }

  return false;
}

bool ControlTab::on_scale_key_event(GdkEventKey* k) {
  if((k->keyval == GDK_KEY_Left || k->keyval == GDK_KEY_Right || k->keyval == GDK_KEY_Up || k->keyval == GDK_KEY_Down || k->keyval == GDK_KEY_KP_Left || k->keyval == GDK_KEY_KP_Right || k->keyval == GDK_KEY_KP_Up || k->keyval == GDK_KEY_KP_Down || k->keyval == GDK_KEY_Home || k->keyval == GDK_KEY_End || k->keyval == GDK_KEY_Page_Up || k->keyval == GDK_KEY_Page_Down) && old_value != rng_time->get_value()) {
    logger->log("control", "Time changed from %.2f to %.2f using key %s of the keyboard on rng_time", old_value, rng_time->get_value(), gdk_keyval_name(k->keyval));
    old_value = rng_time->get_value();
  }

  return false;
}

