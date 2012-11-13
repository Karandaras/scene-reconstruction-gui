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
  /** @class RobotControllerTab "robotcontrollertab.h"
   *  Tab for the GUI that displays data of the RobotController Gazebo Plugin
   *  @author Bastian Klingen
   */
  class RobotControllerTab : public SceneTab
  {
    public:
      /** Constructor
       *  @param _node Gazebo Node Pointer to use
       *  @param _logger LoggerTab to use
       *  @param builder the ui_builder to access the needed parts
       */
      RobotControllerTab(gazebo::transport::NodePtr&, LoggerTab*, Glib::RefPtr<Gtk::Builder>&);
      /** Destructor */
      ~RobotControllerTab();

    private:
      gazebo::transport::NodePtr               node;
      LoggerTab                               *logger;

      Gtk::TreeView                           *trv_robot;
      Glib::RefPtr<Gtk::ListStore>             rob_store;
      Gtk::Entry                              *ent_posx;
      Gtk::Entry                              *ent_posy;
      Gtk::Entry                              *ent_posz;
      Gtk::Entry                              *ent_oriw;
      Gtk::Entry                              *ent_orix;
      Gtk::Entry                              *ent_oriy;
      Gtk::Entry                              *ent_oriz;

      // subscriber and publisher
      gazebo::transport::SubscriberPtr         controllerSub;
 
      Glib::Dispatcher                         on_controllerinfo_msg;
      boost::mutex                            *controllerinfoMutex;
      std::list<gazebo::msgs::SceneRobotController>    controllerinfoMsgs;

    private:
      void OnControllerInfoMsg(ConstSceneRobotControllerPtr&);
      void ProcessControllerInfoMsg();
  };
}
