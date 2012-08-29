#pragma once
#include <gtkmm.h>
#include <gdk/gdk.h>

#include <google/protobuf/message.h>

#include "transport/Transport.hh"
#include "transport/TransportTypes.hh"
#include "transport/Node.hh"
#include "gazebo_config.h"

#include "scenetab.h"
#include "loggertab.h"

namespace SceneReconstruction {
  /** @class SceneGUI "scenegui.h"
   * Main Class that creates the GUI
   * @author Bastian Klingen
   */
  class SceneGUI
  {
    public:
      /** Constructor */
      SceneGUI();
      /** Destructor */
      ~SceneGUI();

    private:
     // gazebo node
      gazebo::transport::NodePtr               node;

      // vector for all tabs
      std::vector<SceneTab*>                   vec_tabs;
      LoggerTab*                               logger;
      Glib::RefPtr<Gtk::Builder>               ui_builder;
      gazebo::transport::PublisherPtr          availPub,
                                               worldPub;
      gazebo::transport::SubscriberPtr         availSub;

      std::map<std::string, bool>              plugin_availability;
      unsigned int                             missing_plugins;
      boost::shared_ptr<gazebo::msgs::Request> avail_request;

    public:
      /** Gtk::Window for the GUI */
      Gtk::Window                             *window;

    private:
      void check_components();
      void OnResponseMsg(ConstResponsePtr&);
  };
}
