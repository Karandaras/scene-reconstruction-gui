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
   *  Tab for the GUI
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

      // data display
      Gtk::TreeView                     *trv_positions;
      Glib::RefPtr<Gtk::TreeStore>       pos_store;
      Gtk::TreeView                     *trv_angles;
      Glib::RefPtr<Gtk::TreeStore>       ang_store;
      Gtk::TreeView                     *trv_objects;
      Glib::RefPtr<Gtk::TreeStore>       obj_store;

      gazebo::transport::SubscriberPtr   bufferSub;
      gazebo::transport::PublisherPtr    positionPub,
                                         anglesPub,
                                         objectPub;


    private:
      void OnBufferMsg(ConstMessage_VPtr&);
      void on_button_position_preview_clicked();
      void on_button_angles_preview_clicked();
      void on_button_object_preview_clicked();
  };
}
