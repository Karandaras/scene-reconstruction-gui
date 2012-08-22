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
  /** @class RobotControllerTab "robotcontrollertab.h"
   * Tab for the GUI that displays data of the RobotController Gazebo Plugin
   * @author Bastian Klingen
   */
  class RobotControllerTab : public SceneTab
  {
    public:
      /** Constructor
       * @param _node Gazebo Node Pointer to use
       * @param _logger LoggerTab to use
       * @param builder the ui_builder to access the needed parts
       */
      RobotControllerTab(gazebo::transport::NodePtr&, LoggerTab*, Glib::RefPtr<Gtk::Builder>&);
      /** Destructor */
      ~RobotControllerTab();

    private:
      gazebo::transport::NodePtr               node;
      LoggerTab                               *logger;

      Gtk::TreeView                           *trv_robot;
      Glib::RefPtr<Gtk::ListStore>             rob_store;
      Gtk::ToolButton                         *btn_send;
      Gtk::ToolButton                         *btn_reload;
      Gtk::Entry                              *ent_posx;
      Gtk::Entry                              *ent_posy;
      Gtk::Entry                              *ent_posz;
      Gtk::Entry                              *ent_rotw;
      Gtk::Entry                              *ent_rotx;
      Gtk::Entry                              *ent_roty;
      Gtk::Entry                              *ent_rotz;

      // subscriber and publisher
      gazebo::transport::SubscriberPtr         sceneResSub;
      gazebo::transport::PublisherPtr          sceneReqPub;
      boost::shared_ptr<gazebo::msgs::Request> robReq;
 
    private:
      void OnResponseMsg(ConstResponsePtr&);
      void on_button_send_clicked();
      void on_button_reload_clicked();
      void on_cell_simangle_edited(const Glib::ustring&, const Glib::ustring&);
      void on_cell_offset_edited(const Glib::ustring&, const Glib::ustring&);
      void on_cell_robangle_edited(const Glib::ustring&, const Glib::ustring&);
  };
}
