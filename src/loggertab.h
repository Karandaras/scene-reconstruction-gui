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
#include "loggingtools.h"

namespace SceneReconstruction {
  /** @class LoggerTab "loggertab.h"
   * Tab for the GUI that is used for logging.
   * @author Bastian Klingen
   */
  class LoggerTab : public SceneTab
  {
    public:
      /** Constructor
       * @param parent parent SceneGUI
       */
      LoggerTab(Glib::RefPtr<Gtk::Builder>&);
      /** Destructor */
      ~LoggerTab();

    private:
      Gtk::TreeView                *trv_logger;
      Glib::RefPtr<Gtk::ListStore>  log_store;
      Gtk::TreeView                *trv_msgs;
      Glib::RefPtr<Gtk::ListStore>  msg_store;
      Gtk::TextView                *txt_console;
      TextBufferStreamBuffer<char> *tbs_cout;
      time_t                        offset;         // offset for current time
      std::streambuf               *old_cout,
                                   *old_cerr;

    private:
      void logmsg(std::string, std::string, std::string);

    public:
      /** logs events to the treeview with timestamp and additional text
       * @param event short eventname
       * @param text longer description, can optionally contain embedded format tags (see sprintf)
       */
      void log(std::string, std::string, ...);

      /** logs msgs to the treeview with timestamp and additional text
       * @param dir direction of the message
       * @param _msg the msg to log
       */
      void msglog(std::string, ConstResponsePtr&);

      /** logs msgs to the treeview with timestamp and additional text
       * @param dir direction of the message
       * @param _msg the msg to log
       */
      void msglog(std::string, ConstRequestPtr&);
 };
}
