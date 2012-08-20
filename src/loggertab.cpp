#include "loggertab.h"

using namespace SceneReconstruction;

/** @class LoggerTab "loggertab.h"
 * Tab for the GUI that is used for logging.
 * @author Bastian Klingen
 */

LoggerTab::LoggerTab() : SceneTab::SceneTab("Logger") {
  offset = time(NULL);
  trv_logger.set_model(log_store = Gtk::ListStore::create(log_cols));
  trv_logger.set_hover_selection(false);
  trv_logger.set_enable_tree_lines(false);
  trv_logger.get_selection()->set_mode(Gtk::SELECTION_NONE);

  Gtk::CellRendererText *cell1 = new Gtk::CellRendererText;
  cell1->set_alignment(0.0,0.0);
  int column_count = trv_logger.append_column("Event", *cell1);
  Gtk::TreeViewColumn *column1 = trv_logger.get_column(column_count-1); 
  if (column1) {
    column1->set_reorderable(true);
    column1->set_resizable(true);
#ifndef GLIBMM_PROPERTIES_ENABLED
      column1->add_attribute(cell1->property_text(), log_cols.event);
#else
      column1->add_attribute(*cell1, "text", log_cols.event);
#endif
  } 
  
  Gtk::CellRendererText *cell2 = new Gtk::CellRendererText;
  cell2->set_alignment(0.0,0.0);
  column_count = trv_logger.append_column("Time", *cell2);
  Gtk::TreeViewColumn *column2 = trv_logger.get_column(column_count-1); 
  if (column2) {
    column2->set_reorderable(true);
    column2->set_resizable(true);
#ifndef GLIBMM_PROPERTIES_ENABLED
      column2->add_attribute(cell2->property_text(), log_cols.time);
#else
      column2->add_attribute(*cell2, "text", log_cols.time);
#endif
  } 
  
  Gtk::CellRendererText *cell3 = new Gtk::CellRendererText;
  cell3->property_wrap_mode() = Pango::WRAP_WORD;
  cell3->property_wrap_width() = 600;
  cell3->set_alignment(0.0,0.0);
  column_count = trv_logger.append_column("Text", *cell3);
  Gtk::TreeViewColumn *column3 = trv_logger.get_column(column_count-1); 
  if (column3) {
    column3->set_reorderable(true);
    column3->set_resizable(true);
    column3->set_expand(true);
#ifndef GLIBMM_PROPERTIES_ENABLED
      column3->add_attribute(cell3->property_text(), log_cols.text);
#else
      column3->add_attribute(*cell3, "text", log_cols.text);
#endif
  } 
  scw_logger.add(trv_logger);
  scw_logger.set_policy(Gtk::POLICY_NEVER, Gtk::POLICY_AUTOMATIC);

  log_tabs.append_page(scw_logger, "Events");

  trv_msgs.set_model(msg_store = Gtk::ListStore::create(msg_cols));
  trv_msgs.set_hover_selection(false);
  trv_msgs.set_enable_tree_lines(false);
  trv_msgs.get_selection()->set_mode(Gtk::SELECTION_NONE);

  Gtk::CellRendererText *msgscell1 = new Gtk::CellRendererText;
  msgscell1->set_alignment(0.0,0.0);
  column_count = trv_msgs.append_column("Dir", *msgscell1);
  Gtk::TreeViewColumn *msgscolumn1 = trv_msgs.get_column(column_count-1); 
  if (msgscolumn1) {
    msgscolumn1->set_reorderable(true);
    msgscolumn1->set_resizable(true);
#ifndef GLIBMM_PROPERTIES_ENABLED
      msgscolumn1->add_attribute(msgscell1->property_text(), msg_cols.dir);
#else
      msgscolumn1->add_attribute(*msgscell1, "text", msg_cols.dir);
#endif
  } 
  
  Gtk::CellRendererText *msgscell2 = new Gtk::CellRendererText;
  msgscell2->set_alignment(0.0,0.0);
  column_count = trv_msgs.append_column("Time", *msgscell2);
  Gtk::TreeViewColumn *msgscolumn2 = trv_msgs.get_column(column_count-1); 
  if (msgscolumn2) {
    msgscolumn2->set_reorderable(true);
    msgscolumn2->set_resizable(true);
#ifndef GLIBMM_PROPERTIES_ENABLED
      msgscolumn2->add_attribute(msgscell2->property_text(), msg_cols.time);
#else
      msgscolumn2->add_attribute(*msgscell2, "text", msg_cols.time);
#endif
  } 
  
  Gtk::CellRendererText *msgscell3 = new Gtk::CellRendererText;
  msgscell3->set_alignment(0.0,0.0);
  column_count = trv_msgs.append_column("Type", *msgscell3);
  Gtk::TreeViewColumn *msgscolumn3 = trv_msgs.get_column(column_count-1); 
  if (msgscolumn3) {
    msgscolumn3->set_reorderable(true);
    msgscolumn3->set_resizable(true);
#ifndef GLIBMM_PROPERTIES_ENABLED
      msgscolumn3->add_attribute(msgscell3->property_text(), msg_cols.type);
#else
      msgscolumn3->add_attribute(*msgscell3, "text", msg_cols.type);
#endif
  } 
  
  Gtk::CellRendererText *msgscell4 = new Gtk::CellRendererText;
  msgscell4->property_wrap_mode() = Pango::WRAP_WORD;
  msgscell4->property_wrap_width() = 600;
  msgscell4->set_alignment(0.0,0.0);
  column_count = trv_msgs.append_column("Message", *msgscell4);
  Gtk::TreeViewColumn *msgscolumn4 = trv_msgs.get_column(column_count-1); 
  if (msgscolumn4) {
    msgscolumn4->set_reorderable(true);
    msgscolumn4->set_resizable(true);
    msgscolumn4->set_expand(true);
#ifndef GLIBMM_PROPERTIES_ENABLED
      msgscolumn4->add_attribute(msgscell4->property_text(), msg_cols.msg);
#else
      msgscolumn4->add_attribute(*msgscell4, "text", msg_cols.msg);
#endif
  } 
  scw_msgs.add(trv_msgs);
  scw_msgs.set_policy(Gtk::POLICY_NEVER, Gtk::POLICY_AUTOMATIC);

  log_tabs.append_page(scw_msgs, "Messages");

  txt_console.set_editable(false);
  txt_console.override_font(Pango::FontDescription("monospace"));
  tbs_cout = new TextBufferStreamBuffer<char>(txt_console.get_buffer());

  old_cout = std::cout.rdbuf();
  old_cerr = std::cerr.rdbuf();

  std::cout.rdbuf(tbs_cout);
  std::cerr.rdbuf(tbs_cout);

  scw_console.add(txt_console);
  scw_console.set_policy(Gtk::POLICY_NEVER, Gtk::POLICY_AUTOMATIC);

  log_tabs.append_page(scw_console, "Console");
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
  row[log_cols.event] = event;
  row[log_cols.time] = time_buffer;
  row[log_cols.text] = buffer;

  free(time_buffer);
  free(buffer);
  va_end(Args);
}

void LoggerTab::logmsg(std::string dir, std::string type, std::string msg)
{
  char * time_buffer;
  time_buffer = (char *) malloc (8);  //hh:mm:ss
  
  int cur_time = time(NULL) - offset;

  sprintf(time_buffer, "%02d:%02d:%02d", cur_time/3600, (cur_time/60)%60, cur_time%60);

  Gtk::TreeModel::Row row = *(msg_store->append());
  row[msg_cols.dir] = dir;
  row[msg_cols.time] = time_buffer;
  row[msg_cols.type] = type;
  row[msg_cols.msg] = msg;

  free(time_buffer);
}

void LoggerTab::msglog(std::string dir, ConstRequestPtr &_msg)
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

  logmsg(dir, "Request", msg.str());
}

void LoggerTab::msglog(std::string dir, ConstResponsePtr &_msg)
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
  }

  logmsg(dir, "Reponse", msg.str());
}

Gtk::Widget& LoggerTab::get_tab() {
  return log_tabs;
}

