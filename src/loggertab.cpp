#include "loggertab.h"

using namespace SceneReconstruction;

/** @class LoggerTab "loggertab.h"
 *  Tab for the GUI that is used for logging.
 *  @author Bastian Klingen
 */

LoggerTab::LoggerTab(Glib::RefPtr<Gtk::Builder>& builder) : SceneTab::SceneTab(builder) {
  offset = time(NULL);

  _builder->get_widget("logger_event_treeview", trv_logger);
  log_store = Glib::RefPtr<Gtk::ListStore>::cast_dynamic(_builder->get_object("logger_event_liststore"));
  log_store->clear();
  
  _builder->get_widget("logger_messages_treeview", trv_msgs);
  msg_store = Glib::RefPtr<Gtk::ListStore>::cast_dynamic(_builder->get_object("logger_messages_liststore"));
  msg_store->clear();

  _builder->get_widget("logger_console_textview", txt_console);
  txt_console->set_editable(false);
  txt_console->override_font(Pango::FontDescription("monospace"));
  tbs_cout = new TextBufferStreamBuffer<char>(txt_console->get_buffer());

  old_cout = std::cout.rdbuf();
  old_cerr = std::cerr.rdbuf();

  std::cout.rdbuf(tbs_cout);
  std::cerr.rdbuf(tbs_cout);
}

LoggerTab::~LoggerTab() {
  std::cout.rdbuf(old_cout);
  std::cerr.rdbuf(old_cerr);
}

void LoggerTab::log(std::string event, std::string text, ...)
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
  row.set_value(0, (Glib::ustring)event);
  row.set_value(1, (Glib::ustring)time_buffer);
  row.set_value(2, (Glib::ustring)buffer);

  free(time_buffer);
  free(buffer);
  va_end(Args);
}

void LoggerTab::logmsg(std::string dir, std::string type, std::string topic, std::string msg)
{
  char * time_buffer;
  time_buffer = (char *) malloc (8);  //hh:mm:ss
  
  int cur_time = time(NULL) - offset;

  sprintf(time_buffer, "%02d:%02d:%02d", cur_time/3600, (cur_time/60)%60, cur_time%60);

  Gtk::TreeModel::Row row = *(msg_store->append());
  row.set_value(0, (Glib::ustring)dir);
  row.set_value(1, (Glib::ustring)time_buffer);
  row.set_value(2, (Glib::ustring)type);
  row.set_value(3, (Glib::ustring)topic);
  row.set_value(4, (Glib::ustring)msg);

  free(time_buffer);
}

void LoggerTab::msglog(std::string dir, std::string topic, ConstRequestPtr &_msg)
{
  std::ostringstream msg;
  msg << "Request: ";
  msg << _msg->request();
  msg << ", ID: ";
  msg << _msg->id();
  if(_msg->has_data() && _msg->data() != "") {
    msg << ", Data: ";
    msg << _msg->data();
  }
  if(_msg->has_dbl_data()) {
    msg << ", Double Data: ";
    msg << _msg->dbl_data();
  }

  logmsg(dir, "Request", topic, msg.str());
}

void LoggerTab::msglog(std::string dir, std::string topic, gazebo::msgs::WorldControl &_msg)
{
  std::ostringstream msg;
  if(_msg.has_pause()) {
    msg << "Pause: ";
    msg << (_msg.pause()?"true":"false");
  }
  if(_msg.has_step()) {
    if(msg.str().length()>0)
      msg << ", ";
    msg << "Step: ";
    msg << (_msg.step()?"true":"false");
  }
  if(_msg.has_reset_time()) {
    if(msg.str().length()>0)
      msg << ", ";
    msg << "Reset Time: ";
    msg << (_msg.reset_time()?"true":"false");
  }
  if(_msg.has_reset_world()) {
    if(msg.str().length()>0)
      msg << ", ";
    msg << "Reset World: ";
    msg << (_msg.reset_world()?"true":"false");
  }

  logmsg(dir, "WorldControl", topic, msg.str());
}

void LoggerTab::msglog(std::string dir, std::string topic, ConstResponsePtr &_msg)
{
  std::ostringstream msg;
  msg << "Request: ";
  msg << _msg->request();
  msg << ", ID: ";
  msg << _msg->id();
  msg << ", Response: ";
  msg << _msg->response();
  if(_msg->has_type()) {
    msg << ", Type: ";
    msg << _msg->type();

    gazebo::msgs::String error;
    if(_msg->response() != "success" && _msg->type() == error.GetTypeName() && _msg->has_serialized_data()) {
      error.ParseFromString(_msg->serialized_data());
      msg << ", Error: ";
      msg << error.data();
    }
  }

  logmsg(dir, "Response", topic, msg.str());
}

void LoggerTab::msglog(std::string dir, std::string topic, ConstDoublePtr &_msg)
{
  std::ostringstream msg;
  msg << "Data: ";
  msg << _msg->data();

  logmsg(dir, "Double", topic, msg.str());
}

void LoggerTab::msglog(std::string dir, std::string topic, gazebo::msgs::SceneRobotController &_msg)
{
  std::ostringstream msg;

  int sn, rn, o, sa, ra;
  sn = _msg.simulator_name_size();
  rn = _msg.robot_name_size();
  o  = _msg.offset_size();
  sa = _msg.simulator_angle_size();
  ra = _msg.robot_angle_size();

  msg << "Position (X: ";
  if(_msg.has_pos_x()) {
    msg << _msg.pos_x();
  }  
  else {
    msg << "0.0";
  }

  msg << ", Y: ";
  if(_msg.has_pos_y()) {
    msg << _msg.pos_y();
  }
  else {
    msg << "0.0";
  }

  msg << ", Z: ";
  if(_msg.has_pos_z()) {
    msg << _msg.pos_z();
  }
  else {
    msg << "0.0";
  }

  msg << "), Rotation (W: ";
  if (_msg.has_ori_w()) {
    msg << _msg.ori_w();
  }
  else {
    msg << "0.0";
  }

  msg << ", X: ";
  if (_msg.has_ori_x()) {
    msg << _msg.ori_x();
  }
  else {
    msg << "0.0";
  }

  msg << ", Y: ";
  if (_msg.has_ori_y()) {
    msg << _msg.ori_y();
  }
  else {
    msg << "0.0";
  }

  msg << ", Z: ";
  if (_msg.has_ori_z()) {
    msg << _msg.ori_z();
  }
  else {
    msg << "0.0";
  }
  msg << ")";

  if(sn == rn && rn == o && o == sa && sa == ra && ra == sn) {
    for(int i=0; i<sn; i++) {
      msg << ", Joint(";
      msg << _msg.simulator_name(i) << ", ";
      msg << _msg.robot_name(i) << ", ";
      msg << _msg.offset(i) << ", ";
      msg << _msg.simulator_angle(i) << ", ";
      msg << _msg.robot_angle(i) << ")";
    }
  }

  logmsg(dir, "SceneRobotController", topic, msg.str());
}

void LoggerTab::set_enabled(bool enabled) {
  Gtk::Widget* tab;
  _builder->get_widget("logger_tab", tab);
  tab->set_sensitive(enabled);
}

void LoggerTab::show_available(std::string comp) {
  Gtk::Image *img;
  std::transform(comp.begin(), comp.end(), comp.begin(), ::tolower);
  _builder->get_widget("logger_availability_status_image_"+comp, img);
  img->set(Gtk::Stock::YES, Gtk::ICON_SIZE_BUTTON);
}
