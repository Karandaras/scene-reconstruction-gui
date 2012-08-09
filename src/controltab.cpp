#include "math/Pose.hh"

#include "controltab.h"
#include "converter.h"

using namespace SceneReconstruction;

ControlTab::ControlTab(gazebo::transport::NodePtr& _node, LoggerTab* _logger)
: SceneTab::SceneTab("Control"),
  adj_time(Gtk::Adjustment::create(0.0, 0.0, 101.0, 0.1, 1.0, 1.0) ),
  rng_time(adj_time, Gtk::ORIENTATION_HORIZONTAL),
  btn_stop("STOP"),
  btn_play("PLAY"),
  btn_pause("PAUSE")
{
  node = _node;
  logger = _logger;
  old_value = 0.0;

  reqSub = node->Subscribe("~/request", &ControlTab::OnReqMsg, this);
  resSub = node->Subscribe("~/response", &ControlTab::OnResMsg, this);

  grd_layout.set_row_homogeneous(false);
  grd_layout.set_column_homogeneous(false);

  // rng_time setup
  rng_time.set_digits(2);
  rng_time.set_draw_value(true);
  rng_time.set_size_request(200,50);
  rng_time.set_hexpand(false);
  rng_time.set_vexpand(false);
  grd_layout.attach(rng_time,0,0,200,50);

  // btn_stop
  btn_stop.set_size_request(60,60);
  btn_stop.set_hexpand(false);
  btn_stop.set_vexpand(false);
  box_buttons.pack_start(btn_stop,false,false);

  btn_stop.signal_clicked().connect(sigc::mem_fun(*this,&ControlTab::on_button_stop_clicked));

  // btn_play
  btn_play.set_size_request(60,60);
  btn_play.set_hexpand(false);
  btn_play.set_vexpand(false);
  box_buttons.pack_start(btn_play,false,false);

  btn_play.signal_clicked().connect(sigc::mem_fun(*this,&ControlTab::on_button_play_clicked));

  // btn_pause
  btn_pause.set_size_request(60,60);
  btn_pause.set_hexpand(false);
  btn_pause.set_vexpand(false);
  box_buttons.pack_start(btn_pause,false,false);

  btn_pause.signal_clicked().connect(sigc::mem_fun(*this,&ControlTab::on_button_pause_clicked));

  box_buttons.set_spacing(5);
  grd_layout.attach(box_buttons,0,50,200,90);

  rng_time.signal_button_release_event().connect(sigc::mem_fun(*this,&ControlTab::on_scale_button_event), false);
  rng_time.signal_key_release_event().connect(sigc::mem_fun(*this,&ControlTab::on_scale_key_event), false);

  // data display
  trv_data.set_model(dat_store = Gtk::TreeStore::create(dat_cols));
  trv_data.set_hover_selection(false);
  trv_data.set_hover_expand(true);
  trv_data.set_enable_tree_lines(true);
  trv_data.get_selection()->set_mode(Gtk::SELECTION_NONE);
  Gtk::TreeModel::Row row = *(dat_store->append());
  row[dat_cols.col_name] = "Coordinates";
  row[dat_cols.col_data] = "";

  Gtk::TreeModel::Row childrow = *(dat_store->append(row.children()));
  childrow[dat_cols.col_name] = "Gazebo";
  childrow[dat_cols.col_data] = "X: 0 Y: 0 Z: 0";

  childrow = *(dat_store->append(row.children()));
  childrow[dat_cols.col_name] = "Robot";
  childrow[dat_cols.col_data] = "X: 0 Y: 0 Z: 0";

  childrow = *(dat_store->append(row.children()));
  childrow[dat_cols.col_name] = "Sensor";
  childrow[dat_cols.col_data] = "X: 0 Y: 0 Z: 0";
 
  trv_data.append_column("Name", dat_cols.col_name);
  trv_data.append_column("Data", dat_cols.col_data);

  scw_data.add(trv_data);
  scw_data.set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);
  
  scw_data.set_hexpand();
  scw_data.set_vexpand();
  scw_data.set_size_request(450,140);
  grd_layout.attach(scw_data,200,0,450,140);

}

ControlTab::~ControlTab() {
}

void ControlTab::OnReqMsg(ConstRequestPtr& _msg) {
  if (guiRes && _msg->request() == "entity_info" && _msg->id() == guiRes->id()) {
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
  Gtk::TreeModel::Children rows = trv_data.get_model()->children();
  for(Gtk::TreeModel::Children::iterator iter = rows.begin(); iter != rows.end(); ++iter) {
    Gtk::TreeModel::Row row = *iter;
    if(row[dat_cols.col_name] == "Coordinates" && model.has_pose()) {
      Gtk::TreeModel::Children childrows = row.children();
      Gtk::TreeModel::Children::iterator childiter = childrows.begin();
      Gtk::TreeModel::Row childrow = *childiter;
      childrow[dat_cols.col_data] = Converter::convert(model.pose(), 0, 3);
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
  if(rng_time.get_value() != old_value) {
    logger->log("control", "Time changed from %.2f to %.2f using button %d of the mouse on rng_time", old_value, rng_time.get_value(), b->button);
    old_value = rng_time.get_value();
  }

  return false;
}

bool ControlTab::on_scale_key_event(GdkEventKey* k) {
  if((k->keyval == GDK_KEY_Left || k->keyval == GDK_KEY_Right || k->keyval == GDK_KEY_Up || k->keyval == GDK_KEY_Down || k->keyval == GDK_KEY_KP_Left || k->keyval == GDK_KEY_KP_Right || k->keyval == GDK_KEY_KP_Up || k->keyval == GDK_KEY_KP_Down || k->keyval == GDK_KEY_Home || k->keyval == GDK_KEY_End || k->keyval == GDK_KEY_Page_Up || k->keyval == GDK_KEY_Page_Down) && old_value != rng_time.get_value()) {
    logger->log("control", "Time changed from %.2f to %.2f using key %s of the keyboard on rng_time", old_value, rng_time.get_value(), gdk_keyval_name(k->keyval));
    old_value = rng_time.get_value();
  }

  return false;
}

Gtk::Widget& ControlTab::get_tab() {
  return grd_layout;
}

