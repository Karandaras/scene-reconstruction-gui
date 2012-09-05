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
      /** simple structure to contruct and store the DIK Graph
       */
      typedef struct {
        /** simple structure for the edges of the DIK Graph
         */
        struct DIKEdge {
          /** node from which the edge comes */
          std::string from;
          /** node to which the edge points */
          std::string to;
          /** label of the edge */
          std::string label;
          /** equality operator
           *  @param rhs an edge to compare with
           *  @return true if edges are equal
           */
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
          /** equality operator
           *  @param rhs string representation of an edge to compare with
           *  @return true if edges are equal
           */
          bool operator==(const std::string& rhs) const
          {
              if(toString() != rhs)
                return false;
              else
                return true;
          }
          /** get the string representation of the edge
           *  @return string representation of this edge
           */
          std::string toString() const
          {
            return "\""+from+"\" --"+(label!=""?"\""+label+"\"":"")+"--> \""+to+"\"";
          }
        };

        /** simple structure for the nodes of the DIK Graph
         */
        struct DIKNode {
          /** name of the node */
          std::string node;
          /** list of pointers to parent nodes */
          std::list<DIKNode*> parents;
          /** list of pointers to child nodes */
          std::list<DIKNode*> children;

          /** equality operator
           *  only checks for names since the graph does not allow two
           *  nodes with equal names
           *  @param rhs node name
           *  @return true if names are equal
           */
          bool operator==(const std::string& rhs) const
          {
              if(node != rhs)
                return false;
              else
                return true;
          }

          /** get the name of the node
           *  @return name of the node
           */
          std::string toString() const
          {
            return node;
          }
        };

        /** list of knowledge nodes */
        std::list<DIKNode>  knowledge_nodes;
        /** list of information nodes */
        std::list<DIKNode>  information_nodes;
        /** list of data nodes */
        std::list<DIKNode>  data_nodes;
        /** list of edges */
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
       *  @param enabled true to enable, false to disable the tab
       */
      void set_enabled(bool);
 };
}
