#pragma once
#include <gtkmm.h>
#include <gdk/gdk.h>
#include <time.h>

#include <google/protobuf/message.h>

#include "common/Time.hh"
#include "transport/Transport.hh"
#include "transport/TransportTypes.hh"
#include "transport/Node.hh"
#include "gazebo_config.h"

#include "scenetab.h"

namespace SceneReconstruction {
  class LoggerTab : public SceneTab
  {
    public:
      LoggerTab();
      ~LoggerTab();

    private:
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

       // offset for current time
      time_t                        offset;

    private:

    public:
      void log(std::string, std::string, ...);
      Gtk::Widget& get_tab();
  };
}
