#pragma once
#include <gtkmm.h>
#include <gdk/gdk.h>

#include <google/protobuf/message.h>

#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>

#include "transport/Transport.hh"
#include "transport/TransportTypes.hh"
#include "transport/Node.hh"
#include "gazebo_config.h"

#include "scenetab.h"
#include "loggertab.h"

namespace SceneReconstruction {
  class ModelTab : public SceneTab
  {
    public:
      ModelTab(gazebo::transport::NodePtr&, LoggerTab*);
      virtual ~ModelTab();

    private:
      gazebo::transport::NodePtr  node;
      LoggerTab*                  logger;

      class ModelColumns : public Gtk::TreeModel::ColumnRecord {
          public:
              ModelColumns() {
                  add(col_desc);
                  add(col_val);
              }
         
              ~ModelColumns() {}
         
              Gtk::TreeModelColumn<Glib::ustring> col_desc;
              Gtk::TreeModelColumn<Glib::ustring> col_val;
          };

      Gtk::ScrolledWindow           scw_model;
      Gtk::TreeView                 trv_model;
      Glib::RefPtr<Gtk::TreeStore>  mdl_store;

      ModelColumns                  mdl_cols; 

      // request message to detect selection through gui
      boost::shared_ptr<gazebo::msgs::Request const> guiReq;
      boost::shared_ptr<gazebo::msgs::Response const> guiRes;
      gazebo::transport::SubscriberPtr resSub, reqSub;

    private:
      void fill_model_treeview(gazebo::msgs::Model);
      void OnReqMsg(ConstRequestPtr&);
      void OnResMsg(ConstResponsePtr&);
      Gtk::Widget& get_tab();
  };
}
