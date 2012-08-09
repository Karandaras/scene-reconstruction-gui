#pragma once
#include <gtkmm.h>
#include <gdk/gdk.h>

#include <google/protobuf/message.h>

#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>

#include "transport/Node.hh"
#include "transport/Transport.hh"
#include "transport/TransportTypes.hh"
#include "gazebo_config.h"

#include "scenetab.h"
#include "loggertab.h"

namespace SceneReconstruction {
  class ControlTab : public SceneTab
  {
    public:
      ControlTab(gazebo::transport::NodePtr&, LoggerTab*);
      ~ControlTab();

    private:
      gazebo::transport::NodePtr    node;
      LoggerTab*                    logger;
      // timeline to display current time and allow navigation
      Glib::RefPtr<Gtk::Adjustment> adj_time;
      Gtk::Scale                    rng_time;

      // buttons for basic commands
      Gtk::ButtonBox                box_buttons;
      Gtk::Button                   btn_stop;
      Gtk::Button                   btn_play;
      Gtk::Button                   btn_pause;

      // data display
      class Columns : public Gtk::TreeModel::ColumnRecord {
          public:
              Columns() {
                  add(col_name);
                  add(col_data);
              }
         
              ~Columns() {}
         
              Gtk::TreeModelColumn<Glib::ustring> col_name;
              Gtk::TreeModelColumn<Glib::ustring> col_data;
          };
      Gtk::ScrolledWindow           scw_data;
      Gtk::TreeView                 trv_data;
      Glib::RefPtr<Gtk::TreeStore>  dat_store;
      Columns                       dat_cols; 

      Gtk::Grid                     grd_layout;

      // temporary variable to detect changes in navigation
      double                        old_value;

      // request message to detect selection through gui
      boost::shared_ptr<gazebo::msgs::Request const> guiReq;
      boost::shared_ptr<gazebo::msgs::Response const> guiRes;
      gazebo::transport::SubscriberPtr resSub, reqSub;

    private:
      void OnReqMsg(ConstRequestPtr&);
      void OnResMsg(ConstResponsePtr&);
      void update_coords(gazebo::msgs::Model);
      void on_button_stop_clicked();
      void on_button_play_clicked();
      void on_button_pause_clicked();
      bool on_scale_button_event(GdkEventButton*);
      bool on_scale_key_event(GdkEventKey*);
      Gtk::Widget& get_tab();
 };
}
