#pragma once
#include <gtkmm.h>
#include <gdk/gdk.h>
#include <time.h>

#include <google/protobuf/message.h>

#include <gazebo/common/Time.hh>
#include <gazebo/transport/Transport.hh>
#include <gazebo/transport/TransportTypes.hh>
#include <gazebo/transport/Node.hh>
#include <gazebo/gazebo_config.h>

#include "scenetab.h"
#include "loggingtools.h"

namespace SceneReconstruction {
  /** @class LoggerTab "loggertab.h"
   *  Tab for the GUI that is used for logging.
   *  @author Bastian Klingen
   */
  class LoggerTab : public SceneTab
  {
    public:
      /** Constructor
       *  @param builder the ui_builder to access the needed parts
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
      void logmsg(std::string, std::string, std::string, std::string);

    public:
      /** logs events to the treeview with timestamp and additional text
       *  @param event short eventname
       *  @param text longer description, can optionally contain embedded format tags (see sprintf)
       */
      void log(std::string, std::string, ...);

      /** logs msgs to the treeview with timestamp and topic
       *  @param dir direction of the message
       *  @param topic topic of the message
       *  @param _msg the msg to log
       */
      void msglog(std::string, std::string, const gazebo::msgs::Response&);

      /** logs msgs to the treeview with timestamp and topic
       *  @param dir direction of the message
       *  @param topic topic of the message
       *  @param _msg the msg to log
       */
      void msglog(std::string, std::string, const gazebo::msgs::WorldControl&);

      /** logs msgs to the treeview with timestamp and topic
       *  @param dir direction of the message
       *  @param topic topic of the message
       *  @param _msg the msg to log
       */
      void msglog(std::string, std::string, const gazebo::msgs::SceneRobotController&);

      /** logs msgs to the treeview with timestamp and topic
       *  @param dir direction of the message
       *  @param topic topic of the message
       *  @param _msg the msg to log
       */
      void msglog(std::string, std::string, const gazebo::msgs::Request&);

      /** logs msgs to the treeview with timestamp and topic
       *  @param dir direction of the message
       *  @param topic topic of the message
       *  @param _msg the msg to log
       */
      void msglog(std::string, std::string, const gazebo::msgs::Double&);

      /** logs msgs to the treeview with timestamp and topic
       *  @param dir direction of the message
       *  @param topic topic of the message
       *  @param _msg the msg to log
       */
      void msglog(std::string, std::string, const gazebo::msgs::Message_V&);

      /** switches the image for the given component to represent that the
       *  component has respondedto the availability request
       *  @param comp name of the component
       */
      void show_available(std::string);
 };
}
