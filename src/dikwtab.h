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
/** @class DIKWTab "dikwtab.h"
 * Tab for the GUI that builds and represents the DIKW Graph.
 * @author Bastian Klingen
 */
  class DIKWTab : public SceneTab
  {
    public:
      /** Constructor
       * @param _node Gazebo Node Pointer to use
       * @param _logger LoggerTab to use
       * @param builder the ui_builder to access the needed parts
       */
      DIKWTab(gazebo::transport::NodePtr&, LoggerTab*, Glib::RefPtr<Gtk::Builder>&);
      /** Destructor */
      ~DIKWTab();

    private:
      typedef struct {
        struct DIKEdge {
          std::string from;
          std::string to;
          std::string label;
          bool operator==(const DIKEdge& rhs) const
          {
              if(from != rhs.from)
                return false;
              else if(to != rhs.to)
                return false;
              else if(label != rhs.label)
                return false;
              else
                return true;
          }
          bool operator==(const std::string& rhs) const
          {
              if(toString() != rhs)
                return false;
              else
                return true;
          }
          std::string toString() const
          {
            return "\""+from+"\" --"+(label!=""?"\""+label+"\"":"")+"--> \""+to+"\"";
          }
        };

        struct DIKNode {
          std::string node;
          std::list<DIKNode*> parents;
          std::list<DIKNode*> children;

          bool operator==(const std::string& rhs) const
          {
              if(node != rhs)
                return false;
              else
                return true;
          }
          std::string toString() const
          {
            return node;
          }
        };

        std::list<DIKNode>  knowledge_nodes;
        std::list<DIKNode>  information_nodes;
        std::list<DIKNode>  data_nodes;
        std::list<DIKEdge>  edges;
      } DIKGraph;

      DIKGraph                           graph;
      std::list<std::string>             db_collections;
      std::list<std::string>             marked_nodes;
      gazebo::transport::NodePtr         node;
      LoggerTab                         *logger;
      Glib::RefPtr<Gdk::Pixbuf>          pix_graph;

      Gtk::ComboBoxText                 *com_type;
      Gtk::Label                        *lbl_left;
      Gtk::ComboBoxText                 *com_left;
      Gtk::Label                        *lbl_right;
      Gtk::ComboBoxText                 *com_right;
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
      Gtk::Image                        *img_graph;
      Gtk::EventBox                     *evt_graph;
      Gtk::ScrolledWindow               *scw_graph;
      Glib::RefPtr<Gtk::EntryCompletion> cpl_left;
      Glib::RefPtr<Gtk::EntryCompletion> cpl_right;

      Gtk::Window                       *win_show;
      Gtk::Image                        *win_image;

      gazebo::transport::SubscriberPtr   resSub;

    private:
      void OnResponseMsg(ConstResponsePtr&);
      void create_graphviz_dot(std::string);
      bool is_marked(std::string);
      std::string style_edge(std::string, bool);
      bool is_node(std::string);
      DIKGraph::DIKNode* get_node(std::string);
      std::string level_of_node(std::string);
      void mark_children(DIKGraph::DIKNode*);
      void mark_parents(DIKGraph::DIKNode*);
      bool is_edge(DIKGraph::DIKEdge);
      void set_comboboxtext(Gtk::ComboBoxText*,gazebo::msgs::String_V);
      void on_type_changed();
      void on_add_clicked();
      void on_nodes_remove_clicked();
      void on_nodes_mark_clicked();
      void on_edges_remove_clicked();
      void on_edges_mark_clicked();
      bool on_image_button_release(GdkEventButton*);
      bool on_image_resize(GdkEventConfigure*);

    public:
      /** sets the sensitivity of the tab
       * @param enabled true to enable, false to disable the tab
       */
      void set_enabled(bool);
 };
}
