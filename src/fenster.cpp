#include <gtkmm.h>
#include <gdk/gdk.h>
#include <iostream>
#include <time.h>

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
    // data display
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

    // layout
    Gtk::Grid                     grd_layout;

    // tabs for log etc.
    Gtk::Notebook                 ntb_tabs;

    // temporary variable to detect changes in navigation
    double                        old_value;

    // offset for current time
    time_t offset;

  private:
    void attach_widgets_to_grid();
    void connect_signals();
    void on_button_stop_clicked();
    void on_button_play_clicked();
    void on_button_pause_clicked();
    bool on_scale_button_event(GdkEventButton*);
    bool on_scale_key_event(GdkEventKey*);
    void log(std::string, std::string, ...);
};

MyWindow::MyWindow()
: Gtk::Window(),
  adj_time(Gtk::Adjustment::create(0.0, 0.0, 101.0, 0.1, 1.0, 1.0) ),
  rng_time(adj_time, Gtk::ORIENTATION_HORIZONTAL),
  btn_stop("STOP"),
  btn_play("PLAY"),
  btn_pause("PAUSE")
{
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

    grd_layout.set_row_homogeneous(false);
    grd_layout.set_column_homogeneous(false);

    attach_widgets_to_grid();
    connect_signals();
   
    Gtk::Label lbl_tab1;
    lbl_tab1.set_text("Main");
    Gtk::Label lbl_tab2;
    lbl_tab2.set_text("Logger");
    ntb_tabs.append_page(grd_layout, lbl_tab1);
    ntb_tabs.append_page(scw_logger, lbl_tab2);
    add(ntb_tabs);
   
    show_all_children();
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
    childrow[dat_cols.col_name] = "Gazebo Coordinates";
    childrow[dat_cols.col_data] = "X: 0 Y: 0 Z: 0";

    childrow = *(dat_store->append(row.children()));
    childrow[dat_cols.col_name] = "Robot Coordinates";
    childrow[dat_cols.col_data] = "X: 0 Y: 0 Z: 0";

    childrow = *(dat_store->append(row.children()));
    childrow[dat_cols.col_name] = "Sensor Coordinates";
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
  char * time_buffer;
  buffer = (char *) malloc (buf_len+1);
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

int main(int argc, char **argv)
{
    Gtk::Main main(argc,argv);
    MyWindow window;
    main.run(window);
    return 0;
}
