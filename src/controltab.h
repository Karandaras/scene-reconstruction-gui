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
  /** @class ControlTab "controltab.h"
   * Tab for the GUI that allows the user to control the scene recontruction
   * by jumping to specific times or (un)pausing. It also displays coordinates
   * of the currently selected object.
   * @author Bastian Klingen
   */
  class ControlTab : public SceneTab
  {
    public:
      /** Constructor
       * @param _node Gazebo Node Pointer to use
       * @param _logger LoggerTab to use
       * @param builder the ui_builder to access the needed parts
       */
      ControlTab(gazebo::transport::NodePtr&, LoggerTab*, Glib::RefPtr<Gtk::Builder>&);
      /** Destructor */
      ~ControlTab();

    private:
      gazebo::transport::NodePtr    node;
      LoggerTab                    *logger;
      // timeline to display current time and allow navigation
      Gtk::Scale                   *rng_time;
      Gtk::Label                   *lbl_max_time;

      // buttons for basic commands
      Gtk::ToolButton              *btn_stop;
      Gtk::ToolButton              *btn_play;
      Gtk::ToolButton              *btn_pause;

      // data display
      Gtk::TreeView                *trv_data;
      Glib::RefPtr<Gtk::TreeStore>  dat_store;

      // temporary variable to detect changes in navigation
      double                        old_value;

      // request message to detect selection through gui
      boost::shared_ptr<gazebo::msgs::Request const>  guiReq;
      boost::shared_ptr<gazebo::msgs::Response const> guiRes;
      gazebo::transport::SubscriberPtr                resSub,
                                                      reqSub,
                                                      timeSub;
      gazebo::transport::PublisherPtr                 worldPub;

    private:
      void OnTimeMsg(ConstDoublePtr&);
      void OnReqMsg(ConstRequestPtr&);
      void OnResMsg(ConstResponsePtr&);
      void update_coords(gazebo::msgs::Model);
      void on_button_stop_clicked();
      void on_button_play_clicked();
      void on_button_pause_clicked();
      bool on_scale_button_event(GdkEventButton*);
      bool on_scale_key_event(GdkEventKey*);

    public:
      /** sets the sensitivity of the tab
       * @param enabled true to enable, false to disable the tab
       */
      void set_enabled(bool);
 };
}
