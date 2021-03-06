#pragma once
#include <gtkmm.h>
#include <gdk/gdk.h>

#include <google/protobuf/message.h>

#include <gazebo/transport/Transport.hh>
#include <gazebo/transport/TransportTypes.hh>
#include <gazebo/transport/Node.hh>
#include <gazebo/gazebo_config.h>

#include "scenetab.h"
#include "loggertab.h"

namespace SceneReconstruction {
  /** @class ObjectInstantiatorTab "objectinstantiatortab.h"
   *  Tab for the GUI that displays data of the ObjectInstantiator Gazebo Plugin
   *  @author Bastian Klingen
   */
  class ObjectInstantiatorTab : public SceneTab
  {
    public:
      /** Constructor
       *  @param _node Gazebo Node Pointer to use
       *  @param _logger LoggerTab to use
       *  @param builder the ui_builder to access the needed parts
       */
      ObjectInstantiatorTab(gazebo::transport::NodePtr&, LoggerTab*, Glib::RefPtr<Gtk::Builder>&);
      /** Destructor */
      ~ObjectInstantiatorTab();

    private:
      gazebo::transport::NodePtr                                   node;
      LoggerTab                                                   *logger;
      Gtk::TreeView                                               *trv_object;
      Glib::RefPtr<Gtk::ListStore>                                 obj_store;
      Gtk::TreeView                                               *trv_data;
      Glib::RefPtr<Gtk::TreeStore>                                 dat_store;
      Gtk::Image                                                  *img_data;
      Gtk::EventBox                                               *evt_data;
      std::map< std::string, Glib::RefPtr<Gdk::Pixbuf> >           images;
      std::map< std::string, Glib::RefPtr<Gdk::Pixbuf> >::iterator image_iter;
      Gtk::ComboBox                                               *com_data;
      Glib::RefPtr<Gtk::ListStore>                                 img_store;

      Gtk::ToolButton                                             *btn_refresh_objects;
      Gtk::ToolButton                                             *btn_show;
  
      Gtk::Window                                                 *win_show;
      Gtk::Image                                                  *win_image;
      Gtk::ScrolledWindow                                         *win_scroll;
      Gtk::ComboBox                                               *win_combo;
      Gtk::Button                                                 *win_close;
      bool                                                         win_change;

      // subscriber and publisher
      gazebo::transport::SubscriberPtr                             sceneResSub;
      gazebo::transport::PublisherPtr                              sceneReqPub;
      gazebo::msgs::Request                                       *objReq;
      gazebo::msgs::Response                                      *objRes;
      bool                                                         object_data_part1,
                                                                   object_data_part2,
                                                                   set_documents;
      Gtk::TreeModel::Row                                          documents_row;

      Glib::Dispatcher                                             on_response_msg;
      boost::mutex                                                *responseMutex;
      std::list<gazebo::msgs::Response>                            responseMsgs;

    private:
      void OnResponseMsg(ConstResponsePtr&);
      void ProcessResponseMsg();
      void on_button_refresh_objects_clicked();
      void on_button_show_clicked();
      void on_combo_changed();
      void on_win_combo_changed();
      void on_win_button_close_clicked();
      bool on_image_button_release(GdkEventButton*);
  };
}
