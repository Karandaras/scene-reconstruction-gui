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
  /** @class AnalysisTab "AnalysisTab.h"
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
      Gtk::ToolButton                   *btn_angles_preview;
      Gtk::ToolButton                   *btn_object_preview;

      Gtk::TreeView                     *trv_positions;
      Glib::RefPtr<Gtk::TreeStore>       pos_store;
      std::vector<gazebo::msgs::BufferPosition> pos_messages;
      Gtk::TreeView                     *trv_angles;
      Glib::RefPtr<Gtk::TreeStore>       ang_store;
      std::vector<gazebo::msgs::BufferJoints> ang_messages;
      Gtk::TreeView                     *trv_objects;
      Glib::RefPtr<Gtk::TreeStore>       obj_store;
      std::vector<gazebo::msgs::BufferObjects> obj_messages;

      gazebo::transport::SubscriberPtr   bufferSub,
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

      Gtk::TreeView                     *trv_lasers;
      Glib::RefPtr<Gtk::TreeStore>       lsr_store;
      Gtk::Button                       *btn_lasers_update;

    private:
      void OnBufferMsg(ConstMessage_VPtr&);
      void OnLasersMsg(ConstLasersPtr&);
      void on_button_position_preview_clicked();
      void on_button_angles_preview_clicked();
      void on_button_object_preview_clicked();
      void on_lasers_visible_toggled(const Glib::ustring&);
      void on_button_grid_show_clicked();
      void on_button_grid_move_clicked();
      void on_button_lasers_update_clicked();
  };
}
