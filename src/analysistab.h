#pragma once
#include <gtkmm.h>
#include <gdk/gdk.h>

#include <google/protobuf/message.h>

#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>

#include <gazebo/transport/Node.hh>
#include <gazebo/transport/Transport.hh>
#include <gazebo/transport/TransportTypes.hh>
#include <gazebo/gazebo_config.h>

#include "scenetab.h"
#include "loggertab.h"

namespace SceneReconstruction {
  /** @class AnalysisTab "analysistab.h"
   *  Tab for the GUI that controls the analyis tools.
   *  It displays the buffers for the Robot and the Objects
   *  and controls the a grid for measurements inside the 
   *  simulation as well as the visualization of the lasers
   *  of the robot.
   *  @author Bastian Klingen
   */
  class AnalysisTab : public SceneTab
  {
    public:
      /** Constructor
       *  @param _node Gazebo Node Pointer to use
       *  @param _logger LoggerTab to use
       *  @param builder the ui_builder to access the needed parts
       */
      AnalysisTab(gazebo::transport::NodePtr&, LoggerTab*, Glib::RefPtr<Gtk::Builder>&);
      /** Destructor */
      ~AnalysisTab();

    private:
      gazebo::transport::NodePtr         node;
      LoggerTab                         *logger;

      Gtk::ToolButton                   *btn_position_preview;
      Gtk::ToolButton                   *btn_position_clear;
      Gtk::ToolButton                   *btn_angles_preview;
      Gtk::ToolButton                   *btn_angles_clear;
      Gtk::ToolButton                   *btn_object_preview;
      Gtk::ToolButton                   *btn_object_move;
      Gtk::ToolButton                   *btn_object_clear;

      Gtk::TreeView                     *trv_positions;
      Glib::RefPtr<Gtk::TreeStore>       pos_store;
      std::vector<gazebo::msgs::SceneRobot> pos_messages;
      Gtk::TreeView                     *trv_angles;
      Glib::RefPtr<Gtk::TreeStore>       ang_store;
      std::vector<gazebo::msgs::SceneJoint> ang_messages;
      Gtk::TreeView                     *trv_objects;
      Glib::RefPtr<Gtk::TreeStore>       obj_store;
      std::vector<gazebo::msgs::SceneObject> obj_messages;

      gazebo::transport::SubscriberPtr   robBufferSub,
                                         objBufferSub,
                                         controlSub,
                                         lasersSub;
      gazebo::transport::PublisherPtr    positionPub,
                                         anglesPub,
                                         objectPub,
                                         drawingPub,
                                         lasersPub;

      Gtk::SpinButton                   *spn_grid_pos_x;
      Gtk::SpinButton                   *spn_grid_pos_y;
      Gtk::SpinButton                   *spn_grid_pos_z;
      Gtk::SpinButton                   *spn_grid_rot_x;
      Gtk::SpinButton                   *spn_grid_rot_y;
      Gtk::SpinButton                   *spn_grid_rot_z;
      Gtk::SpinButton                   *spn_grid_rot_w;
      Gtk::SpinButton                   *spn_grid_width;
      Gtk::SpinButton                   *spn_grid_height;
      Gtk::SpinButton                   *spn_grid_size;
      Gtk::Button                       *btn_grid_show;
      Gtk::Button                       *btn_grid_move;

      Gtk::SpinButton                   *spn_object_pos_x;
      Gtk::SpinButton                   *spn_object_pos_y;
      Gtk::SpinButton                   *spn_object_pos_z;
      Gtk::SpinButton                   *spn_object_rot_x;
      Gtk::SpinButton                   *spn_object_rot_y;
      Gtk::SpinButton                   *spn_object_rot_z;
      Gtk::SpinButton                   *spn_object_rot_w;
 
      Gtk::TreeView                     *trv_lasers;
      Glib::RefPtr<Gtk::ListStore>       lsr_store;
      Gtk::Button                       *btn_lasers_update;

      Glib::Dispatcher                   on_lasers_msg,
                                         on_buffer_msg,
                                         on_control_msg;
      boost::mutex                      *lasersMutex,
                                        *bufferMutex;
      std::list<gazebo::msgs::Lasers>    lasersMsgs;
      std::list<gazebo::msgs::Message_V> bufferMsgs;

      double                             time_offset;

    private:
      void OnBufferMsg(ConstMessage_VPtr&);
      void ProcessBufferMsg();
      void OnLasersMsg(ConstLasersPtr&);
      void ProcessLasersMsg();
      void OnControlMsg(ConstSceneFrameworkControlPtr&);
      void ProcessControlMsg();
      bool on_treeview_button_release(GdkEventButton*);
      bool on_treeview_key_release(GdkEventKey*);
      void treeview_object_selection();
      void on_button_position_preview_clicked();
      void on_button_position_clear_clicked();
      void on_button_position_refresh_clicked();
      void on_button_angles_preview_clicked();
      void on_button_angles_clear_clicked();
      void on_button_angles_refresh_clicked();
      void on_button_object_preview_clicked();
      void on_button_object_move_clicked();
      void on_button_object_clear_clicked();
      void on_button_object_refresh_clicked();
      void on_lasers_visible_toggled(const Glib::ustring&);
      void on_button_grid_show_clicked();
      void on_button_grid_move_clicked();
      void on_button_lasers_update_clicked();
  };
}
