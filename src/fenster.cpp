#define _USE_MATH_DEFINES
#include <math.h> 

#include <gtkmm.h>
#include <gdk/gdk.h>
#include <iostream>
#include <time.h>

#include <google/protobuf/message.h>

#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>

#include "common/Time.hh"
#include "transport/Transport.hh"
#include "transport/TransportTypes.hh"
#include "transport/Node.hh"
#include "gazebo_config.h"
#include "math/Pose.hh"

struct MyWindow : Gtk::Window
{
  MyWindow();
  private:
    // timeline to display current time and allow navigation
    Glib::RefPtr<Gtk::Adjustment> adj_time;
    Gtk::Scale                    rng_time;

    // buttons for basic commands
    Gtk::ButtonBox                box_buttons;
    Gtk::Button                   btn_stop;
    Gtk::Button                   btn_play;
    Gtk::Button                   btn_pause;

    // data display
    class Columns : public Gtk::TreeModel::ColumnRecord {
        public:
            Columns() {
                add(col_name);
                add(col_data);
            }
       
            ~Columns() {}
       
            Gtk::TreeModelColumn<Glib::ustring> col_name;
            Gtk::TreeModelColumn<Glib::ustring> col_data;
        };

    Gtk::ScrolledWindow           scw_data;
    Gtk::TreeView                 trv_data;
    Glib::RefPtr<Gtk::TreeStore>  dat_store;

    Columns                       dat_cols; 
    
    // logger
    class LogColumns : public Gtk::TreeModel::ColumnRecord {
        public:
            LogColumns() {
                add(col_event);
                add(col_time);
                add(col_text);
            }
       
            ~LogColumns() {}
       
            Gtk::TreeModelColumn<Glib::ustring> col_event;
            Gtk::TreeModelColumn<Glib::ustring> col_time;
            Gtk::TreeModelColumn<Glib::ustring> col_text;
        };

    Gtk::ScrolledWindow           scw_logger;
    Gtk::TreeView                 trv_logger;
    Glib::RefPtr<Gtk::ListStore>  log_store;

    LogColumns                    log_cols; 

    // topics
    class TopicColumns : public Gtk::TreeModel::ColumnRecord {
        public:
            TopicColumns() {
                add(col_name);
                add(col_listen);
            }
       
            ~TopicColumns() {}
       
            Gtk::TreeModelColumn<Glib::ustring> col_name;
            Gtk::TreeModelColumn<bool> col_listen;
        };

    Gtk::ScrolledWindow           scw_topics;
    Gtk::TreeView                 trv_topics;
    Glib::RefPtr<Gtk::ListStore>  top_store;

    TopicColumns                  top_cols;

    std::map<std::string, gazebo::transport::SubscriberPtr> top_subs;

    // listener
    class ListenerColumns : public Gtk::TreeModel::ColumnRecord {
        public:
            ListenerColumns() {
                add(col_time);
                add(col_type);
                add(col_msgs);
            }
       
            ~ListenerColumns() {}
       
            Gtk::TreeModelColumn<Glib::ustring> col_time;
            Gtk::TreeModelColumn<Glib::ustring> col_type;
            Gtk::TreeModelColumn<Glib::ustring> col_msgs;
        };

    Gtk::ScrolledWindow           scw_listen;
    Gtk::TreeView                 trv_listen;
    Glib::RefPtr<Gtk::ListStore>  lst_store;

    ListenerColumns               lst_cols; 

    // model
    class ModelColumns : public Gtk::TreeModel::ColumnRecord {
        public:
            ModelColumns() {
                add(col_desc);
                add(col_val);
            }
       
            ~ModelColumns() {}
       
            Gtk::TreeModelColumn<Glib::ustring> col_desc;
            Gtk::TreeModelColumn<Glib::ustring> col_val;
        };

    Gtk::ScrolledWindow           scw_model;
    Gtk::TreeView                 trv_model;
    Glib::RefPtr<Gtk::TreeStore>  mdl_store;

    ModelColumns                  mdl_cols; 


    // layout
    Gtk::Grid                     grd_layout;

    // tabs for log etc.
    Gtk::Notebook                 ntb_tabs;

    // temporary variable to detect changes in navigation
    double                        old_value;

    // offset for current time
    time_t offset;

    // gazebo node
    gazebo::transport::NodePtr node;

    // request message to detect selection through gui
    boost::shared_ptr<gazebo::msgs::Request const> guiReq;
    boost::shared_ptr<gazebo::msgs::Response const> guiRes;

  private:
    void init_transport_and_setup_topics();
    void attach_widgets_to_grid();
    void connect_signals();
    void on_button_stop_clicked();
    void on_button_play_clicked();
    void on_button_pause_clicked();
    bool on_scale_button_event(GdkEventButton*);
    bool on_scale_key_event(GdkEventKey*);
    void on_listener(const Gtk::TreeModel::Path&, const Gtk::TreeModel::iterator&);
    void OnWSMsg(ConstWorldStatisticsPtr&);
    void OnReqMsg(ConstRequestPtr&);
    void OnResMsg(ConstResponsePtr&);
    void log(std::string, std::string, ...);
    Glib::ustring to_ustring(google::protobuf::uint32);
    Glib::ustring to_ustring(bool);
    Glib::ustring to_ustring(double);
    Glib::ustring convert(gazebo::msgs::Vector3d, int);
    Glib::ustring convert(gazebo::msgs::Quaternion, int, bool);
    Glib::ustring convert(gazebo::msgs::Pose, int, int, bool);
    void fill_model_treeview(gazebo::msgs::Model);
    void update_coords(gazebo::msgs::Model);
};

MyWindow::MyWindow()
: Gtk::Window(),
  adj_time(Gtk::Adjustment::create(0.0, 0.0, 101.0, 0.1, 1.0, 1.0) ),
  rng_time(adj_time, Gtk::ORIENTATION_HORIZONTAL),
  btn_stop("STOP"),
  btn_play("PLAY"),
  btn_pause("PAUSE")
{
    init_transport_and_setup_topics();

    old_value = 0.0;
    offset = time(NULL);

    set_default_size(600,120);
    set_title("GUI-Tool for Scene Reconstruction");

    // logger
    trv_logger.set_model(log_store = Gtk::ListStore::create(log_cols));
    trv_logger.set_hover_selection(false);
    trv_logger.set_enable_tree_lines(false);
    trv_logger.get_selection()->set_mode(Gtk::SELECTION_NONE);

    Gtk::CellRendererText *cell1 = new Gtk::CellRendererText;
    cell1->set_alignment(0.0,0.0);
    int column_count = trv_logger.append_column("Event", *cell1);
    Gtk::TreeViewColumn *column1 = trv_logger.get_column(column_count-1); 
    if (column1) {
#ifndef GLIBMM_PROPERTIES_ENABLED
        column1->add_attribute(cell1->property_text(), log_cols.col_event);
#else
        column1->add_attribute(*cell1, "text", log_cols.col_event);
#endif
    } 
    
    Gtk::CellRendererText *cell2 = new Gtk::CellRendererText;
    cell2->set_alignment(0.0,0.0);
    column_count = trv_logger.append_column("Time", *cell2);
    Gtk::TreeViewColumn *column2 = trv_logger.get_column(column_count-1); 
    if (column2) {
#ifndef GLIBMM_PROPERTIES_ENABLED
        column2->add_attribute(cell2->property_text(), log_cols.col_time);
#else
        column2->add_attribute(*cell2, "text", log_cols.col_time);
#endif
    } 
    
    Gtk::CellRendererText *cell3 = new Gtk::CellRendererText;
    cell3->property_wrap_mode() = Pango::WRAP_WORD;
    cell3->property_wrap_width() = 600;
    cell3->set_alignment(0.0,0.0);
    column_count = trv_logger.append_column("Text", *cell3);
    Gtk::TreeViewColumn *column3 = trv_logger.get_column(column_count-1); 
    if (column3) {
#ifndef GLIBMM_PROPERTIES_ENABLED
        column3->add_attribute(cell3->property_text(), log_cols.col_text);
#else
        column3->add_attribute(*cell3, "text", log_cols.col_text);
#endif
    } 
    scw_logger.add(trv_logger);
    scw_logger.set_policy(Gtk::POLICY_NEVER, Gtk::POLICY_AUTOMATIC);

    // Topics
    trv_topics.set_model(top_store = Gtk::ListStore::create(top_cols));
    trv_topics.set_hover_selection(false);
    trv_topics.set_enable_tree_lines(false);

  std::string data, namespacesData, publishersData;
  gazebo::msgs::Packet packet;

  // Connect to the master
  gazebo::transport::ConnectionPtr connection(new gazebo::transport::Connection());
  connection->Connect("localhost", 11345);

  // Read the verification message
  connection->Read(data);
  connection->Read(namespacesData);
  connection->Read(publishersData);

  packet.ParseFromString(data);
  if (packet.type() == "init")
  {
    gazebo::msgs::String msg;
    msg.ParseFromString(packet.serialized_data());
    if (msg.data() != std::string("gazebo ") + GAZEBO_VERSION)
      std::cerr << "Conflicting gazebo versions\n";
  }

        gazebo::msgs::Request request;
        gazebo::msgs::Publishers pubs;

        request.set_id(0);
        request.set_request("get_publishers");
        connection->EnqueueMsg(gazebo::msgs::Package("request", request), true);
        connection->Read(data);

        packet.ParseFromString(data);
        pubs.ParseFromString(packet.serialized_data());

        for (int i = 0; i < pubs.publisher_size(); i++)
        {
          const gazebo::msgs::Publish &p = pubs.publisher(i);
          if (p.topic().find("__dbg") == std::string::npos) {
            Gtk::TreeModel::Row row = *(top_store->append());
            row[top_cols.col_name] = p.topic();
            row[top_cols.col_listen] = false;
          }
          
          connection.reset();
        }
  
    trv_topics.append_column("Topic", top_cols.col_name);
    trv_topics.append_column_editable("Subscribe", top_cols.col_listen);

    scw_topics.add(trv_topics);
    scw_topics.set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);

    // listener
    trv_listen.set_model(lst_store = Gtk::ListStore::create(lst_cols));
    trv_listen.set_hover_selection(false);
    trv_listen.set_enable_tree_lines(false);

    trv_listen.append_column("Time", lst_cols.col_time);
    trv_listen.append_column("Type", lst_cols.col_type);
    trv_listen.append_column("Data", lst_cols.col_msgs);

    scw_listen.add(trv_listen);
    scw_listen.set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);

    // model
    trv_model.set_model(mdl_store = Gtk::TreeStore::create(mdl_cols));
    trv_model.set_hover_selection(true);
    trv_model.set_enable_tree_lines(true);

    trv_model.append_column("Description", mdl_cols.col_desc);
    trv_model.append_column_editable("Value", mdl_cols.col_val);

    scw_model.add(trv_model);
    scw_model.set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);

    grd_layout.set_row_homogeneous(false);
    grd_layout.set_column_homogeneous(false);

    attach_widgets_to_grid();
    connect_signals();
   
    Gtk::Label lbl_tab1;
    lbl_tab1.set_text("Main");
    Gtk::Label lbl_tab2;
    lbl_tab2.set_text("Logger");
    Gtk::Label lbl_tab3;
    lbl_tab3.set_text("Topics");
    Gtk::Label lbl_tab4;
    lbl_tab4.set_text("Listener");
    Gtk::Label lbl_tab5;
    lbl_tab5.set_text("Model");
    ntb_tabs.append_page(grd_layout, lbl_tab1);
    ntb_tabs.append_page(scw_logger, lbl_tab2);
    ntb_tabs.append_page(scw_topics, lbl_tab3);
    ntb_tabs.append_page(scw_listen, lbl_tab4);
    ntb_tabs.append_page(scw_model, lbl_tab5);
    add(ntb_tabs);
   
    show_all_children();
}

void MyWindow::init_transport_and_setup_topics()
{
    gazebo::transport::init();
    gazebo::transport::run();
    gazebo::transport::NodePtr _node(new gazebo::transport::Node());
    node = _node;
    node->Init();
}

void MyWindow::attach_widgets_to_grid()
{
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

    // btn_play
    btn_play.set_size_request(60,60);
    btn_play.set_hexpand(false);
    btn_play.set_vexpand(false);
    box_buttons.pack_start(btn_play,false,false);

    // btn_pause
    btn_pause.set_size_request(60,60);
    btn_pause.set_hexpand(false);
    btn_pause.set_vexpand(false);
    box_buttons.pack_start(btn_pause,false,false);

    box_buttons.set_spacing(5);
    grd_layout.attach(box_buttons,0,50,200,90);

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

void MyWindow::connect_signals()
{
    // Signale verbinden:
    btn_stop.signal_clicked().connect(sigc::mem_fun(*this,&MyWindow::on_button_stop_clicked));
    btn_play.signal_clicked().connect(sigc::mem_fun(*this,&MyWindow::on_button_play_clicked));
    btn_pause.signal_clicked().connect(sigc::mem_fun(*this,&MyWindow::on_button_pause_clicked));
    rng_time.signal_button_release_event().connect(sigc::mem_fun(*this,&MyWindow::on_scale_button_event), false);
    rng_time.signal_key_release_event().connect(sigc::mem_fun(*this,&MyWindow::on_scale_key_event), false);
    trv_topics.get_model()->signal_row_changed().connect(sigc::mem_fun(*this,&MyWindow::on_listener), false);
}

bool MyWindow::on_scale_button_event(GdkEventButton *b)
{
    if(rng_time.get_value() != old_value) {
      log("mouse event", "Time changed from %.2f to %.2f using button %d of the mouse on rng_time", old_value, rng_time.get_value(), b->button);
      old_value = rng_time.get_value();
    }

    return false;
}

bool MyWindow::on_scale_key_event(GdkEventKey *k)
{
    if((k->keyval == GDK_KEY_Left || k->keyval == GDK_KEY_Right || k->keyval == GDK_KEY_Up || k->keyval == GDK_KEY_Down || k->keyval == GDK_KEY_KP_Left || k->keyval == GDK_KEY_KP_Right || k->keyval == GDK_KEY_KP_Up || k->keyval == GDK_KEY_KP_Down || k->keyval == GDK_KEY_Home || k->keyval == GDK_KEY_End || k->keyval == GDK_KEY_Page_Up || k->keyval == GDK_KEY_Page_Down) && old_value != rng_time.get_value()) {
      log("key event", "Time changed from %.2f to %.2f using key %s of the keyboard on rng_time", old_value, rng_time.get_value(), gdk_keyval_name(k->keyval));
      old_value = rng_time.get_value();
    }

    return false;
}

void MyWindow::on_button_stop_clicked()
{
      log("button pressed", "STOP");
}


void MyWindow::on_button_play_clicked()
{
      log("button pressed", "PLAY");
}


void MyWindow::on_button_pause_clicked()
{
      log("button pressed", "PAUSE");
}

void MyWindow::on_listener(const Gtk::TreeModel::Path& path, const Gtk::TreeModel::iterator& iter)
{
      std::string topic = iter->get_value(top_cols.col_name);
      if(iter->get_value(top_cols.col_listen))
      {
        if(topic.find("world_stats") != std::string::npos) {
          top_subs[topic] = node->Subscribe(topic, &MyWindow::OnWSMsg, this);
          log("topic subscribed", "Subscribed to topic " + topic);
        } else if (topic.find("request") != std::string::npos) {
          top_subs[topic] = node->Subscribe(topic, &MyWindow::OnReqMsg, this);
          log("topic subscribed", "Subscribed to topic " + topic);
        } else if (topic.find("response") != std::string::npos) {
          top_subs[topic] = node->Subscribe(topic, &MyWindow::OnResMsg, this);
          log("topic subscribed", "Subscribed to topic " + topic);
        }
      }
      else
      {
        log("topic subscribed", "Unsubscribed from topic " + topic);
        if(top_subs[topic])
          top_subs[topic]->Unsubscribe();
      }
}

void MyWindow::OnWSMsg(ConstWorldStatisticsPtr &_msg)
{
  char * time_buffer;
  time_buffer = (char *) malloc (8);  //hh:mm:ss
  
  int cur_time = time(NULL) - offset;

  sprintf(time_buffer, "%02d:%02d:%02d", cur_time/3600, (cur_time/60)%60, cur_time%60);
  Gtk::TreeModel::Row row = *(lst_store->append());
  row[lst_cols.col_time] = time_buffer;
  row[lst_cols.col_type] = "WorldStatistics";
  row[lst_cols.col_msgs] = _msg->ShortDebugString();
}

void MyWindow::OnReqMsg(ConstRequestPtr &_msg)
{
  char * time_buffer;
  time_buffer = (char *) malloc (8);  //hh:mm:ss
  
  int cur_time = time(NULL) - offset;

  sprintf(time_buffer, "%02d:%02d:%02d", cur_time/3600, (cur_time/60)%60, cur_time%60);
  Gtk::TreeModel::Row row = *(lst_store->append());
  row[lst_cols.col_time] = time_buffer;
  row[lst_cols.col_type] = "Request";
  row[lst_cols.col_msgs] = _msg->ShortDebugString();

  if (guiRes && _msg->request() == "entity_info" && _msg->id() == guiRes->id()) {
    gazebo::msgs::Model model;
    if (guiRes->has_type() && guiRes->type() == model.GetTypeName()) {
      model.ParseFromString(guiRes->serialized_data());
      log("selection", "Model " + model.name() + " was selected.");
    } else {
      guiRes.reset();
      guiReq = _msg;
    }
  } else {
    guiReq = _msg;
  }
}

void MyWindow::OnResMsg(ConstResponsePtr &_msg)
{
  char * time_buffer;
  time_buffer = (char *) malloc (8);  //hh:mm:ss
  
  int cur_time = time(NULL) - offset;

  sprintf(time_buffer, "%02d:%02d:%02d", cur_time/3600, (cur_time/60)%60, cur_time%60);
  Gtk::TreeModel::Row row = *(lst_store->append());
  row[lst_cols.col_time] = time_buffer;
  row[lst_cols.col_type] = "Response";
  row[lst_cols.col_msgs] = _msg->ShortDebugString();

  if (guiReq && _msg->request() == "entity_info" && _msg->id() == guiReq->id()) {
    gazebo::msgs::Model model;
    if (_msg->has_type() && _msg->type() == model.GetTypeName()) {
      model.ParseFromString(_msg->serialized_data());
      log("selection", "Model " + model.name() + " was selected.");

      fill_model_treeview(model);
      update_coords(model);
    } else {
      guiReq.reset();
      guiRes = _msg;
    }
  } else {
    guiRes = _msg;
  }
}

void MyWindow::log(std::string event, std::string text, ...)
{
  va_list Args;
  va_start(Args, text); 

  int buf_len = text.length();
  for(size_t i = text.find("%", 0); i != std::string::npos; i = text.find("%", i)) {
    i++;
    if(text.substr(i,1) == "s")
      buf_len += 30;
    else
      buf_len += 10;
  }

  char * buffer;
  buffer = (char *) malloc (buf_len+1);
  char * time_buffer;
  time_buffer = (char *) malloc (8);  //hh:mm:ss
  
  int cur_time = time(NULL) - offset;

  sprintf(time_buffer, "%02d:%02d:%02d", cur_time/3600, (cur_time/60)%60, cur_time%60);
  vsprintf(buffer, text.c_str(), Args);

  buffer[buf_len] = '\0';

  Gtk::TreeModel::Row row = *(log_store->append());
  row[log_cols.col_event] = "["+event+"]";
  row[log_cols.col_time] = time_buffer;
  row[log_cols.col_text] = buffer;

  free(time_buffer);
  free(buffer);
  va_end(Args);
}

Glib::ustring MyWindow::to_ustring(google::protobuf::uint32 in) {
  std::stringstream convert;
  convert << in;
  Glib::ustring ret(convert.str());
  return ret;
}

Glib::ustring MyWindow::to_ustring(bool in) {
  std::stringstream convert;
  convert << in;
  Glib::ustring ret(convert.str());
  return ret;
}

Glib::ustring MyWindow::to_ustring(double in) {
  std::stringstream convert;
  convert << in;
  Glib::ustring ret(convert.str());
  return ret;
}

Glib::ustring MyWindow::convert(gazebo::msgs::Vector3d in, int round = -1) {
  gazebo::math::Vector3 v = gazebo::msgs::Convert(in); 
  if(round != -1)
    v.Round(round);
  
  std::stringstream convert;
  convert << "X: ";
  convert << v.x;
  convert << " Y: ";
  convert << v.y;
  convert << " Z: ";
  convert << v.z;

  Glib::ustring ret(convert.str());
  return ret;
}

Glib::ustring MyWindow::convert(gazebo::msgs::Quaternion in, int round = -1, bool as_euler = false) {
  gazebo::math::Quaternion q = gazebo::msgs::Convert(in); 
  if(round != -1)
    q.Round(round);

  std::stringstream convert;
  if(as_euler) {
    gazebo::math::Vector3 v = q.GetAsEuler();
    v *= *180/M_PI;
    if(round != -1)
      v.Round(round);
  
    convert << "R: ";
    convert << v.x;
    convert << " P: ";
    convert << v.y;
    convert << " Y: ";
    convert << v.z;
  }
  else {
    convert << "W: ";
    convert << q.w;
    convert << " X: ";
    convert << q.x;
    convert << " Y: ";
    convert << q.y;
    convert << " Z: ";
    convert << q.z;
  }
  
  Glib::ustring ret(convert.str());
  return ret;
}

Glib::ustring MyWindow::convert(gazebo::msgs::Pose in, int part, int round = -1, bool as_euler = false) {
  if(part == 0) { // position
    return convert(in.position(), round);
  }
  else if (part == 1) { // orientation
    return convert(in.orientation(), round, as_euler);
  }
  else
    return "";
}


void MyWindow::fill_model_treeview(gazebo::msgs::Model model) {
  mdl_store.clear();
  trv_model.set_model(mdl_store = Gtk::TreeStore::create(mdl_cols));
  Gtk::TreeModel::Row row = *(mdl_store->append());
  Gtk::TreeModel::Row childrow;
  Gtk::TreeModel::Row cchildrow;
  Gtk::TreeModel::Row ccchildrow;
  Gtk::TreeModel::Row cccchildrow;
  row[mdl_cols.col_desc] = "Name";
  row[mdl_cols.col_val] = model.name();
  if(model.has_id()) {
    row = *(mdl_store->append());
    row[mdl_cols.col_desc] = "ID";
    row[mdl_cols.col_val] = to_ustring(model.id());
  }
  if(model.has_is_static()) {
    row = *(mdl_store->append());
    row[mdl_cols.col_desc] = "is_static";
    row[mdl_cols.col_val] = to_ustring(model.is_static());
  }
  if(model.has_pose()) {
    gazebo::msgs::Pose pose = model.pose();
    row = *(mdl_store->append());
    row[mdl_cols.col_desc] = "Pose";
    if(pose.has_name()) {
      row[mdl_cols.col_val] = pose.name();
    } else {
      row[mdl_cols.col_val] = "";
    }

    childrow = *(mdl_store->append(row.children()));
    childrow[mdl_cols.col_desc] = "Position";
    childrow[mdl_cols.col_val] = convert(pose.position());
    childrow = *(mdl_store->append(row.children()));
    childrow[mdl_cols.col_desc] = "Orientation Q";
    childrow[mdl_cols.col_val] = convert(pose.orientation());
    childrow = *(mdl_store->append(row.children()));
    childrow[mdl_cols.col_desc] = "Orientation E";
    childrow[mdl_cols.col_val] = convert(pose.orientation(), -1, true);
  }
  int joints = model.joint_size();
  if(joints > 0) {
    row = *(mdl_store->append());
    row[mdl_cols.col_desc] = "Joints";
    row[mdl_cols.col_val] = "";

    for(int i=0; i<joints; i++) {
      gazebo::msgs::Joint joint = model.joint(i);
      childrow = *(mdl_store->append(row.children()));
      childrow[mdl_cols.col_desc] = joint.name();
      childrow[mdl_cols.col_val] = "";
      if(joint.has_type()) {
        cchildrow = *(mdl_store->append(childrow.children()));
        cchildrow[mdl_cols.col_desc] = "Type";
        cchildrow[mdl_cols.col_val] = gazebo::msgs::Joint::Type_Name(joint.type());
      }
      if(joint.has_parent()) {
        cchildrow = *(mdl_store->append(childrow.children()));
        cchildrow[mdl_cols.col_desc] = "Parent";
        cchildrow[mdl_cols.col_val] = joint.parent();
      }
      if(joint.has_child()) {
        cchildrow = *(mdl_store->append(childrow.children()));
        cchildrow[mdl_cols.col_desc] = "Child";
        cchildrow[mdl_cols.col_val] = joint.child();
      }
      if(joint.has_pose()) {
        gazebo::msgs::Pose pose = joint.pose();
        cchildrow = *(mdl_store->append(childrow.children()));
        cchildrow[mdl_cols.col_desc] = "Pose";
        if(pose.has_name()) {
          cchildrow[mdl_cols.col_val] = pose.name();
        } else {
          cchildrow[mdl_cols.col_val] = "";
        }

        ccchildrow = *(mdl_store->append(cchildrow.children()));
        ccchildrow[mdl_cols.col_desc] = "Position";
        ccchildrow[mdl_cols.col_val] = convert(pose.position());
        ccchildrow = *(mdl_store->append(cchildrow.children()));
        ccchildrow[mdl_cols.col_desc] = "Orientation Q";
        ccchildrow[mdl_cols.col_val] = convert(pose.orientation());
        ccchildrow[mdl_cols.col_desc] = "Orientation E";
        ccchildrow[mdl_cols.col_val] = convert(pose.orientation(), -1, true);
      }
      if(joint.has_axis1()) {
        gazebo::msgs::Axis axis = joint.axis1();
        cchildrow = *(mdl_store->append(childrow.children()));
        cchildrow[mdl_cols.col_desc] = "Axis1";
        cchildrow[mdl_cols.col_val] = "";

        ccchildrow = *(mdl_store->append(cchildrow.children()));
        ccchildrow[mdl_cols.col_desc] = "XYZ";
        ccchildrow[mdl_cols.col_val] = convert(axis.xyz());
        ccchildrow = *(mdl_store->append(cchildrow.children()));
        ccchildrow[mdl_cols.col_desc] = "Limit";
        ccchildrow[mdl_cols.col_val] = "";
        cccchildrow = *(mdl_store->append(ccchildrow.children()));
        cccchildrow[mdl_cols.col_desc] = "Lower";
        cccchildrow[mdl_cols.col_val] = to_ustring(axis.limit_lower());
        cccchildrow = *(mdl_store->append(ccchildrow.children()));
        cccchildrow[mdl_cols.col_desc] = "Upper";
        cccchildrow[mdl_cols.col_val] = to_ustring(axis.limit_upper());
        cccchildrow = *(mdl_store->append(ccchildrow.children()));
        cccchildrow[mdl_cols.col_desc] = "Effort";
        cccchildrow[mdl_cols.col_val] = to_ustring(axis.limit_effort());
        cccchildrow = *(mdl_store->append(ccchildrow.children()));
        cccchildrow[mdl_cols.col_desc] = "Velocity";
        cccchildrow[mdl_cols.col_val] = to_ustring(axis.limit_velocity());
        ccchildrow = *(mdl_store->append(cchildrow.children()));
        ccchildrow[mdl_cols.col_desc] = "Damping";
        ccchildrow[mdl_cols.col_val] = to_ustring(axis.damping());
        ccchildrow = *(mdl_store->append(cchildrow.children()));
        ccchildrow[mdl_cols.col_desc] = "Friction";
        ccchildrow[mdl_cols.col_val] = to_ustring(axis.friction());
      }
      if(joint.has_axis2()) {
        gazebo::msgs::Axis axis = joint.axis2();
        cchildrow = *(mdl_store->append(childrow.children()));
        cchildrow[mdl_cols.col_desc] = "Axis2";
        cchildrow[mdl_cols.col_val] = "";

        ccchildrow = *(mdl_store->append(cchildrow.children()));
        ccchildrow[mdl_cols.col_desc] = "XYZ";
        ccchildrow[mdl_cols.col_val] = convert(axis.xyz());
        ccchildrow = *(mdl_store->append(cchildrow.children()));
        ccchildrow[mdl_cols.col_desc] = "Limit";
        ccchildrow[mdl_cols.col_val] = "";
        cccchildrow = *(mdl_store->append(ccchildrow.children()));
        cccchildrow[mdl_cols.col_desc] = "Lower";
        cccchildrow[mdl_cols.col_val] = to_ustring(axis.limit_lower());
        cccchildrow = *(mdl_store->append(ccchildrow.children()));
        cccchildrow[mdl_cols.col_desc] = "Upper";
        cccchildrow[mdl_cols.col_val] = to_ustring(axis.limit_upper());
        cccchildrow = *(mdl_store->append(ccchildrow.children()));
        cccchildrow[mdl_cols.col_desc] = "Effort";
        cccchildrow[mdl_cols.col_val] = to_ustring(axis.limit_effort());
        cccchildrow = *(mdl_store->append(ccchildrow.children()));
        cccchildrow[mdl_cols.col_desc] = "Velocity";
        cccchildrow[mdl_cols.col_val] = to_ustring(axis.limit_velocity());
        ccchildrow = *(mdl_store->append(cchildrow.children()));
        ccchildrow[mdl_cols.col_desc] = "Damping";
        ccchildrow[mdl_cols.col_val] = to_ustring(axis.damping());
        ccchildrow = *(mdl_store->append(cchildrow.children()));
        ccchildrow[mdl_cols.col_desc] = "Friction";
        ccchildrow[mdl_cols.col_val] = to_ustring(axis.friction());
      }
    }
  }
  int links = model.link_size();
  if(links > 0) {
    row = *(mdl_store->append());
    row[mdl_cols.col_desc] = "Links";
    row[mdl_cols.col_val] = "";

    for(int i=0; i<links; i++) {
      gazebo::msgs::Link link = model.link(i);
      childrow = *(mdl_store->append(row.children()));
      childrow[mdl_cols.col_desc] = link.name();
      childrow[mdl_cols.col_val] = "";

      cchildrow = *(mdl_store->append(childrow.children()));
      cchildrow[mdl_cols.col_desc] = "ID";
      cchildrow[mdl_cols.col_val] = to_ustring(link.id());

      if(link.has_self_collide()) {
        cchildrow = *(mdl_store->append(childrow.children()));
        cchildrow[mdl_cols.col_desc] = "self_collide";
        cchildrow[mdl_cols.col_val] = to_ustring(link.self_collide());
      }          
      if(link.has_gravity()) {
        cchildrow = *(mdl_store->append(childrow.children()));
        cchildrow[mdl_cols.col_desc] = "gravity";
        cchildrow[mdl_cols.col_val] = to_ustring(link.gravity());
      }          
      if(link.has_kinematic()) {
        cchildrow = *(mdl_store->append(childrow.children()));
        cchildrow[mdl_cols.col_desc] = "kinematic";
        cchildrow[mdl_cols.col_val] = to_ustring(link.kinematic());
      }          
      if(link.has_enabled()) {
        cchildrow = *(mdl_store->append(childrow.children()));
        cchildrow[mdl_cols.col_desc] = "enabled";
        cchildrow[mdl_cols.col_val] = to_ustring(link.enabled());
      }          
      if(link.has_inertial()) {
        gazebo::msgs::Inertial inert = link.inertial();
        cchildrow = *(mdl_store->append(childrow.children()));
        cchildrow[mdl_cols.col_desc] = "Inertial";
        cchildrow[mdl_cols.col_val] = "";
        if(inert.has_mass()) {
          ccchildrow = *(mdl_store->append(cchildrow.children()));
          ccchildrow[mdl_cols.col_desc] = "Mass";
          ccchildrow[mdl_cols.col_val] = to_ustring(inert.mass());
        }
        if(inert.has_pose()) {
          gazebo::msgs::Pose pose = link.pose();
          ccchildrow = *(mdl_store->append(cchildrow.children()));
          ccchildrow[mdl_cols.col_desc] = "Pose";
          if(pose.has_name()) {
            ccchildrow[mdl_cols.col_val] = pose.name();
          } else {
            ccchildrow[mdl_cols.col_val] = "";
          }

          cccchildrow = *(mdl_store->append(ccchildrow.children()));
          cccchildrow[mdl_cols.col_desc] = "Position";
          cccchildrow[mdl_cols.col_val] = convert(pose.position());
          cccchildrow = *(mdl_store->append(ccchildrow.children()));
          cccchildrow[mdl_cols.col_desc] = "Orientation Q";
          cccchildrow[mdl_cols.col_val] = convert(pose.orientation());
          cccchildrow[mdl_cols.col_desc] = "Orientation E";
          cccchildrow[mdl_cols.col_val] = convert(pose.orientation(), -1, true);
        }
        if(inert.has_ixx()) {
          ccchildrow = *(mdl_store->append(cchildrow.children()));
          ccchildrow[mdl_cols.col_desc] = "IXX";
          ccchildrow[mdl_cols.col_val] = to_ustring(inert.ixx());
        }
        if(inert.has_ixy()) {
          ccchildrow = *(mdl_store->append(cchildrow.children()));
          ccchildrow[mdl_cols.col_desc] = "IXY";
          ccchildrow[mdl_cols.col_val] = to_ustring(inert.ixy());
        }
        if(inert.has_ixz()) {
          ccchildrow = *(mdl_store->append(cchildrow.children()));
          ccchildrow[mdl_cols.col_desc] = "IXZ";
          ccchildrow[mdl_cols.col_val] = to_ustring(inert.ixz());
        }
        if(inert.has_iyy()) {
          ccchildrow = *(mdl_store->append(cchildrow.children()));
          ccchildrow[mdl_cols.col_desc] = "IYY";
          ccchildrow[mdl_cols.col_val] = to_ustring(inert.iyy());
        }
        if(inert.has_iyz()) {
          ccchildrow = *(mdl_store->append(cchildrow.children()));
          ccchildrow[mdl_cols.col_desc] = "IYZ";
          ccchildrow[mdl_cols.col_val] = to_ustring(inert.iyz());
        }
        if(inert.has_izz()) {
          ccchildrow = *(mdl_store->append(cchildrow.children()));
          ccchildrow[mdl_cols.col_desc] = "IZZ";
          ccchildrow[mdl_cols.col_val] = to_ustring(inert.izz());
        }
      }          
      if(link.has_pose()) {
        gazebo::msgs::Pose pose = link.pose();
        cchildrow = *(mdl_store->append(childrow.children()));
        cchildrow[mdl_cols.col_desc] = "Pose";
        if(pose.has_name()) {
          cchildrow[mdl_cols.col_val] = pose.name();
        } else {
          cchildrow[mdl_cols.col_val] = "";
        }

        ccchildrow = *(mdl_store->append(cchildrow.children()));
        ccchildrow[mdl_cols.col_desc] = "Position";
        ccchildrow[mdl_cols.col_val] = convert(pose.position());
        ccchildrow = *(mdl_store->append(cchildrow.children()));
        ccchildrow[mdl_cols.col_desc] = "Orientation Q";
        ccchildrow[mdl_cols.col_val] = convert(pose.orientation());
        ccchildrow[mdl_cols.col_desc] = "Orientation E";
        ccchildrow[mdl_cols.col_val] = convert(pose.orientation(), -1, true);
      }
/*
      int visuals = link.visual_size();
      int collisions = link.collision_size();
      int sensors = link.sensor_size();
      int projectors = link.projector_size();
*/
    }
  }
  if(model.has_deleted()) {
    row = *(mdl_store->append());
    row[mdl_cols.col_desc] = "deleted";
    row[mdl_cols.col_val] = to_ustring(model.deleted());
  }
// Visual
}

void MyWindow::update_coords(gazebo::msgs::Model model) {
  // Update Coords-Treeview
  Gtk::TreeModel::Children rows = trv_data.get_model()->children();
  for(Gtk::TreeModel::Children::iterator iter = rows.begin(); iter != rows.end(); ++iter) {
    Gtk::TreeModel::Row row = *iter;
    if(row[dat_cols.col_name] == "Coordinates" && model.has_pose()) {
      Gtk::TreeModel::Children childrows = row.children();
      Gtk::TreeModel::Children::iterator childiter = childrows.begin();
      Gtk::TreeModel::Row childrow = *childiter;
      childrow[dat_cols.col_data] = convert(model.pose(), 0, 3);
    }
  }
}

int main(int argc, char **argv)
{
    Gtk::Main main(argc,argv);
    MyWindow window;
    main.run(window);
    return 0;
}
