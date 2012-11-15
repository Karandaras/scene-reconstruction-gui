#pragma once
#include "graph_drawing_area.h"

#include <gtkmm.h>
#include <gdk/gdk.h>

#include <google/protobuf/message.h>

#include <gazebo/transport/Node.hh>
#include <gazebo/transport/Transport.hh>
#include <gazebo/transport/TransportTypes.hh>
#include <gazebo/gazebo_config.h>

#include "scenetab.h"
#include "loggertab.h"
#include "dikgraph.h"

namespace SceneReconstruction {
/** @class DIKWTab "dikwtab.h"
 *  Tab for the GUI that builds and represents the DIKW Graph.
 *  @author Bastian Klingen
 */
  class DIKWTab : public SceneTab
  {
    public:
      /** Constructor
       *  @param _node Gazebo Node Pointer to use
       *  @param _logger LoggerTab to use
       *  @param builder the ui_builder to access the needed parts
       */
      DIKWTab(gazebo::transport::NodePtr&, LoggerTab*, Glib::RefPtr<Gtk::Builder>&);
      /** Destructor */
      ~DIKWTab();

    private:
      DIKGraph                           graph;
      std::list<std::string>             db_collections;
      std::list<std::string>             marked_nodes;
      gazebo::transport::NodePtr         node;
      LoggerTab                         *logger;

      Gtk::ToolButton                   *btn_new;
      Gtk::ToolButton                   *btn_load;
      Gtk::ToolButton                   *btn_save;
      Gtk::ToolButton                   *btn_close;
      Gtk::ComboBox                     *com_graphs;
      Glib::RefPtr<Gtk::ListStore>       gra_store;

      Gtk::ComboBoxText                 *com_type;
      Gtk::Label                        *lbl_left;
      Gtk::ComboBoxText                 *com_left;
      Glib::RefPtr<Gtk::EntryCompletion> cpl_left;
      Gtk::Label                        *lbl_right;
      Gtk::ComboBoxText                 *com_right;
      Glib::RefPtr<Gtk::EntryCompletion> cpl_right;
      Gtk::Entry                        *ent_label;
      Gtk::ToolButton                   *btn_add;
      Gtk::TreeView                     *trv_nodes;
      Glib::RefPtr<Gtk::ListStore>       nds_store;
      Gtk::ToolButton                   *btn_nodes_remove;
      Gtk::ToolButton                   *btn_nodes_mark;
      Gtk::TreeView                     *trv_edges;
      Glib::RefPtr<Gtk::ListStore>       edg_store;
      Gtk::ToolButton                   *btn_edges_remove;
      Gtk::ToolButton                   *btn_edges_mark;
      Gtk::ScrolledWindow               *scw_graph;
      GraphDrawingArea                  *gda_graph;
      Gtk::ToolButton                   *btn_zoomin;
      Gtk::ToolButton                   *btn_zoomout;
      Gtk::ToolButton                   *btn_zoomfit;
      Gtk::ToolButton                   *btn_zoomreset;
      Gtk::ToolButton                   *btn_export;

      Gtk::Window                       *win_show;
      Gtk::ComboBox                     *win_combo;
      Gtk::Image                        *win_image;
      std::map<int, Glib::RefPtr<Gdk::Pixbuf> > win_images;
      std::map<int, gazebo::msgs::Drawing >     win_pointclouds;
      Glib::RefPtr<Gdk::Pixbuf>          missing_image;
      Gtk::TextView                     *win_textview;
      Glib::RefPtr<Gtk::TextBuffer>      win_textbuffer;
      Glib::RefPtr<Gtk::ListStore>       win_store;

      Gtk::FileChooserDialog            *fcd_save;
      Gtk::FileChooserDialog            *fcd_open;
      Gtk::Dialog                       *dia_new;
      Gtk::Entry                        *dia_new_entry;
      Glib::RefPtr<Gtk::FileFilter>      filter_dgf;


      gazebo::transport::SubscriberPtr   resSub;
      gazebo::transport::PublisherPtr    framePub,
                                         pclPub;
      gazebo::msgs::Request             *docreq;

      Glib::Dispatcher                   on_response_msg;
      boost::mutex                      *responseMutex;
      std::list<gazebo::msgs::Response>  responseMsgs;

    private:
      void OnResponseMsg(ConstResponsePtr&);
      void ProcessResponseMsg();
      void create_graphviz_dot(std::string);
      void on_new_clicked();
      void on_load_clicked();
      void on_save_clicked();
      void on_close_clicked();
      void on_graphs_changed();
      void on_type_changed();
      void on_add_clicked();
      void on_nodes_remove_clicked();
      void on_nodes_mark_clicked();
      void on_edges_remove_clicked();
      void on_edges_mark_clicked();
      bool on_graph_release(GdkEventButton*);
      void on_zoom_in_clicked();
      void on_zoom_out_clicked();
      void on_zoom_fit_clicked();
      void on_zoom_reset_clicked();
      void on_export_clicked();
      void on_document_changed();
  };
}
