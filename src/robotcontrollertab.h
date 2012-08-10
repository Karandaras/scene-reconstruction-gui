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
  class RobotControllerTab : public SceneTab
  {
    public:
      RobotControllerTab(gazebo::transport::NodePtr&, LoggerTab*);
      ~RobotControllerTab();

    private:
      gazebo::transport::NodePtr  node;
      LoggerTab*                  logger;

      class RCColumns : public Gtk::TreeModel::ColumnRecord {
          public:
              RCColumns() {
                  add(col_simname);
                  add(col_robname);
                  add(col_simangle);
                  add(col_offset);
                  add(col_robangle);
              }

              ~RCColumns() {}

              Gtk::TreeModelColumn<Glib::ustring> col_simname;
              Gtk::TreeModelColumn<Glib::ustring> col_robname;
              Gtk::TreeModelColumn<double>        col_simangle;
              Gtk::TreeModelColumn<double>        col_offset;
              Gtk::TreeModelColumn<double>        col_robangle;
          };

      Gtk::ScrolledWindow           scw_robot;
      Gtk::TreeView                 trv_robot;
      Glib::RefPtr<Gtk::ListStore>  roc_store;

      RCColumns                     roc_cols;

      Gtk::Button                   btn_send;
      Gtk::Button                   btn_reload;
      Gtk::Label                    lbl_pos;
      Gtk::Label                    lbl_posx;
      Gtk::Entry                    ent_posx;
      Gtk::Label                    lbl_posy;
      Gtk::Entry                    ent_posy;
      Gtk::Label                    lbl_posz;
      Gtk::Entry                    ent_posz;
      Gtk::Label                    lbl_rot;
      Gtk::Label                    lbl_rotw;
      Gtk::Entry                    ent_rotw;
      Gtk::Label                    lbl_rotx;
      Gtk::Entry                    ent_rotx;
      Gtk::Label                    lbl_roty;
      Gtk::Entry                    ent_roty;
      Gtk::Label                    lbl_rotz;
      Gtk::Entry                    ent_rotz;

      Gtk::Table                    tbl_robot;

      // subscriber and publisher
      gazebo::transport::SubscriberPtr sceneResSub;
      gazebo::transport::PublisherPtr  sceneReqPub;
      gazebo::msgs::Request *robReq;
 
    private:
      void OnResponseMsg(ConstResponsePtr&);
      void on_button_send_clicked();
      void on_button_reload_clicked();
      void on_cell_simangle_edited(const Glib::ustring&, const Glib::ustring&);
      void on_cell_offset_edited(const Glib::ustring&, const Glib::ustring&);
      void on_cell_robangle_edited(const Glib::ustring&, const Glib::ustring&);
      Gtk::Widget& get_tab();
  };
}
