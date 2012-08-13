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
}

LoggerTab::~LoggerTab() {
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
  row[log_cols.event] = "["+event+"]";
  row[log_cols.time] = time_buffer;
  row[log_cols.text] = buffer;

  free(time_buffer);
  free(buffer);
  va_end(Args);
}

Gtk::Widget& LoggerTab::get_tab() {
  return scw_logger;
}

