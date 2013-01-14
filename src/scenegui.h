#pragma once
#include <gtkmm.h>
#include <gdk/gdk.h>

#include <google/protobuf/message.h>

#include <gazebo/transport/Transport.hh>
#include <gazebo/transport/TransportTypes.hh>
#include <gazebo/transport/Node.hh>
#include <gazebo/gazebo_config.h>

#include "scenetab.h"
#include "loggertab.h"

namespace SceneReconstruction {
  /** @class SceneGUI "scenegui.h"
   *  Main Class that creates the GUI
   *  @author Bastian Klingen
   */
  class SceneGUI
  {
    public:
      /** Constructor */
      SceneGUI();
      /** Destructor */
      ~SceneGUI();
      /** Function to show the GUI
        * @param minimized if true, the gui will show up minimized
        */
      void present(bool);      

    private:
     // gazebo node
      gazebo::transport::NodePtr         node;

      // vector for all tabs
      std::vector<SceneTab*>             vec_tabs;
      LoggerTab                         *logger;
      Glib::RefPtr<Gtk::Builder>         ui_builder;
      gazebo::transport::PublisherPtr    worldPub;
      gazebo::transport::SubscriberPtr   availSub;

      std::map<std::string, bool>                                       plugin_availability;
      std::map<std::string, gazebo::transport::PublisherPtr>            plugin_pubs;
      unsigned int                                                      missing_plugins;

      Glib::Dispatcher                   on_response_msg;
      boost::mutex                      *responseMutex;
      std::list<gazebo::msgs::Response>  responseMsgs;

    public:
      /** Gtk::Window for the GUI */
      Gtk::Window                     *window;

    private:
      void OnResponseMsg(ConstResponsePtr&);
      void ProcessResponseMsg();
      bool on_close(GdkEventAny*);
  };
}
