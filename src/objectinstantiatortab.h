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
  /** @class ObjectInstantiatorTab "objectinstantiatortab.h"
   * Tab for the GUI that displays data of the ObjectInstantiator Gazebo Plugin
   * @author Bastian Klingen
   */
  class ObjectInstantiatorTab : public SceneTab
  {
    public:
      /** Constructor
       * @param _node Gazebo Node Pointer to use
       * @param _logger LoggerTab to use
       */
      ObjectInstantiatorTab(gazebo::transport::NodePtr&, LoggerTab*);
      /** Destructor */
      ~ObjectInstantiatorTab();

    private:
      gazebo::transport::NodePtr  node;
      LoggerTab*                  logger;

      class DataColumns : public Gtk::TreeModel::ColumnRecord {
          public:
              /** Constructor */
              DataColumns() {
                  add(desc);
                  add(data);
              }

              /** Destructor */
              ~DataColumns() {}

              /** Gtk::TreeModelColumn that holds the description for the data */
              Gtk::TreeModelColumn<Glib::ustring> desc;
              /** Gtk::TreeModelColumn that holds the data */
              Gtk::TreeModelColumn<Glib::ustring> data;
          };

      class ListColumns : public Gtk::TreeModel::ColumnRecord {
          public:
              /** Constructor */
              ListColumns() {
                  add(name);
              }

              /** Destructor */
              ~ListColumns() {}

              /** Gtk::TreeModelColumn that holds the name of the object*/
              Gtk::TreeModelColumn<Glib::ustring> name;
          };

      Gtk::Label                    lbl_object;
      Gtk::ScrolledWindow           scw_object;
      Gtk::TreeView                 trv_object;
      Glib::RefPtr<Gtk::ListStore>  obj_store;
      ListColumns                   obj_cols;

      Gtk::Label                    lbl_data;
      Gtk::ScrolledWindow           scw_data;
      Gtk::TreeView                 trv_data;
      Glib::RefPtr<Gtk::ListStore>  obj_data;
      DataColumns                   dat_cols;

      Gtk::Label                    lbl_repo;
      Gtk::ScrolledWindow           scw_repo;
      Gtk::TreeView                 trv_repo;
      Glib::RefPtr<Gtk::ListStore>  obj_repo;
      ListColumns                   rep_cols;


      Gtk::Button                   btn_refresh_objects;
      Gtk::Button                   btn_refresh_repository;
      Gtk::Button                   btn_show;

      Gtk::Table                    tbl_objects;

      // subscriber and publisher
      gazebo::transport::SubscriberPtr sceneResSub;
      gazebo::transport::PublisherPtr  sceneReqPub;
      boost::shared_ptr<gazebo::msgs::Request>        objReq;
      boost::shared_ptr<gazebo::msgs::Response const> objRes;
 
    private:
      void OnResponseMsg(ConstResponsePtr&);
      void on_button_refresh_repository_clicked();
      void on_button_refresh_objects_clicked();
      void on_button_show_clicked();
    public:
      /** Getter for the tab of this class
       * @return Gtk::Widget the tab generated by this class
       */
      Gtk::Widget& get_tab();
  };
}
