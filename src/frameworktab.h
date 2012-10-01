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
  /** @class FrameworkTab "frameworktab.h"
   *  Tab for the GUI that allows the user to receive data from the database of the framework
   *  @author Bastian Klingen
   */
  class FrameworkTab : public SceneTab
  {
    public:
      /** Constructor
       *  @param _node Gazebo Node Pointer to use
       *  @param _logger LoggerTab to use
       *  @param builder the ui_builder to access the needed parts
       */
      FrameworkTab(gazebo::transport::NodePtr&, LoggerTab*, Glib::RefPtr<Gtk::Builder>&);
      /** Destructor */
      ~FrameworkTab();

    private:
      gazebo::transport::NodePtr    node;
      LoggerTab                    *logger;

      Gtk::ToolButton              *btn_collections_refresh;
      Gtk::ToolButton              *btn_collections_select;
      Gtk::SpinButton              *spn_object;

      Gtk::TreeView                *trv_collections;
      Glib::RefPtr<Gtk::ListStore>  col_store;
      Gtk::TextView                *txt_object;
      Gtk::TextBuffer              *buf_object;

      boost::shared_ptr<gazebo::msgs::Request>        objReq;
      gazebo::transport::PublisherPtr                 reqPub;
      gazebo::transport::SubscriberPtr                resSub;

    private:
      void OnResponseMsg(ConstResponsePtr&);
      void on_button_collections_refresh_clicked();
      void on_button_collections_select_clicked();
      void on_button_object_value_changed();
 };
}
