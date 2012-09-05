//#include <gvc.h>
#include <fstream>

#include "dikwtab.h"

using namespace SceneReconstruction;

/** @class DIKWTab "dikwtab.h"
 *  Tab for the GUI that builds and represents the DIKW Graph.
 *  @author Bastian Klingen
 */

DIKWTab::DIKWTab(gazebo::transport::NodePtr& _node, LoggerTab* _logger, Glib::RefPtr<Gtk::Builder>& builder)
: SceneTab::SceneTab(builder)
{
  node = _node;
  logger = _logger;  

  resSub = node->Subscribe("~/SceneReconstruction/GUI/MongoDB", &DIKWTab::OnResponseMsg, this);

  Gtk::Box *box_type;
  _builder->get_widget("dikw_box_top_left", box_type);
  com_type = Gtk::manage(new Gtk::ComboBoxText(false));
  com_type->append("Node");
  com_type->append("Edge");
  com_type->set_active(0);
  box_type->pack_end(*com_type, false, true);
  com_type->signal_changed().connect(sigc::mem_fun(*this,&DIKWTab::on_type_changed));
  _builder->get_widget("dikw_label_top_center_left", lbl_left);
  Gtk::Box *box_left;
  _builder->get_widget("dikw_box_top_center_left", box_left);
  com_left = Gtk::manage(new Gtk::ComboBoxText(true));
  com_left->append("Knowledge");
  com_left->append("Information");
  com_left->append("Data");
  com_left->set_active(0);
  box_left->pack_end(*com_left, true, true);
  cpl_left = Glib::RefPtr<Gtk::EntryCompletion>::cast_dynamic(_builder->get_object("dikw_entrycompletion_level_from"));
  cpl_left->set_model(com_left->get_model());
  com_left->get_entry()->set_completion(cpl_left);
  _builder->get_widget("dikw_label_top_center_right", lbl_right);
  Gtk::Box *box_right;
  _builder->get_widget("dikw_box_top_center_right", box_right);
  com_right = Gtk::manage(new Gtk::ComboBoxText(true));
  box_right->pack_end(*com_right, true, true);
  cpl_right = Glib::RefPtr<Gtk::EntryCompletion>::cast_dynamic(_builder->get_object("dikw_entrycompletion_name_to"));
  cpl_right->set_model(com_right->get_model());
  com_right->get_entry()->set_completion(cpl_right);
  _builder->get_widget("dikw_entry_top_right", ent_label);
  ent_label->set_sensitive(false);
  _builder->get_widget("dikw_toolbutton_top_add", btn_add);
  btn_add->signal_clicked().connect(sigc::mem_fun(*this,&DIKWTab::on_add_clicked));
  _builder->get_widget("dikw_treeview_nodes", trv_nodes);
  nds_store = Glib::RefPtr<Gtk::ListStore>::cast_dynamic(_builder->get_object("dikw_liststore_nodes"));
  nds_store->clear();
  _builder->get_widget("dikw_toolbutton_nodes_delete", btn_nodes_remove);
  btn_nodes_remove->signal_clicked().connect(sigc::mem_fun(*this,&DIKWTab::on_nodes_remove_clicked));
  _builder->get_widget("dikw_toolbutton_nodes_mark", btn_nodes_mark);
  btn_nodes_mark->signal_clicked().connect(sigc::mem_fun(*this,&DIKWTab::on_nodes_mark_clicked));
  _builder->get_widget("dikw_treeview_edges", trv_edges);
  edg_store = Glib::RefPtr<Gtk::ListStore>::cast_dynamic(_builder->get_object("dikw_liststore_edges"));
  edg_store->clear();
  _builder->get_widget("dikw_toolbutton_edges_delete", btn_edges_remove);
  btn_edges_remove->signal_clicked().connect(sigc::mem_fun(*this,&DIKWTab::on_edges_remove_clicked));
  _builder->get_widget("dikw_toolbutton_edges_mark", btn_edges_mark);
  btn_edges_mark->signal_clicked().connect(sigc::mem_fun(*this,&DIKWTab::on_edges_mark_clicked));
  _builder->get_widget("dikw_image_graph", img_graph);
  _builder->get_widget("dikw_eventbox_graph", evt_graph);
  _builder->get_widget("dikw_scrolledwindow_graph", scw_graph);
  evt_graph->add_events(Gdk::BUTTON_RELEASE_MASK);
  evt_graph->signal_button_release_event().connect( sigc::mem_fun(*this, &DIKWTab::on_image_button_release) );
  _builder->get_widget("dikw_graph_window", win_show);
  _builder->get_widget("dikw_graph_window_image", win_image);
  create_graphviz_dot("");
}

DIKWTab::~DIKWTab() {
}

void DIKWTab::OnResponseMsg(ConstResponsePtr& _msg) {
  if(_msg->response() != "success")
    return;

  if(_msg->request() == "collection_names") {
    logger->log("dikw", "received collections for nodelist");

    gazebo::msgs::String_V src;
    if(_msg->has_type() && _msg->type() == src.GetTypeName()) {
      src.ParseFromString(_msg->serialized_data());
      int n = src.data_size();
      db_collections.clear();
      for(int i=0; i<n; i++) {
        db_collections.push_back(src.data(i));
      }
      if(com_type->get_active_text() == "Node") {
        logger->log("dikw", "updating nodelist");
        com_right->remove_all();
        std::list<std::string>::iterator iter;
        for(iter = db_collections.begin(); iter != db_collections.end(); iter++)
          com_right->append(*iter);
      }
    }
  }
  else if(_msg->request() == "select_database") {
    logger->log("dikw", "received collections for nodelist");

    gazebo::msgs::String_V src;
    if(_msg->has_type() && _msg->type() == src.GetTypeName()) {
      src.ParseFromString(_msg->serialized_data());
      int n = src.data_size();
      db_collections.clear();
      for(int i=0; i<n; i++) {
        db_collections.push_back(src.data(i));
      }
      if(com_type->get_active_text() == "Node") {
        logger->log("dikw", "updating nodelist");
        com_right->remove_all();
        std::list<std::string>::iterator iter;
        for(iter = db_collections.begin(); iter != db_collections.end(); iter++)
          com_right->append(*iter);
      }
    }
  }
}

void DIKWTab::create_graphviz_dot(std::string markupnode) {
  std::list<DIKGraph::DIKNode>::iterator iter;
  std::list<DIKGraph::DIKEdge>::iterator edgeiter;
  // standard "header" of the graph
  std::string dot = "digraph G {\n"\
                    "  ranksep=1;\n"\
                    "  edge[style=invis];\n"\
                    "  node[shape=box,fontsize=20,fixedsize=true,width=2];\n"\
                    "  \"Knowledge\" -> \"Information\" -> \"Data\";\n"\
                    "  edge[style=solid,dir=back];\n"\
                    "  node[shape=ellipse,fontsize=14,fixedsize=false,width=0.75];\n";

  //find nodes and edges connected to the markupnode
  marked_nodes.clear();
  if(markupnode != "") {
    marked_nodes.push_back(markupnode);
    //mark nodes pointing to marked nodes
    mark_children(get_node(markupnode));

    //mark nodes pointed from marked nodes
    mark_parents(get_node(markupnode));
  }
  
  // create knowledge nodes
  dot            += "  subgraph knowledge {\n"\
                    "    rank = same;\n"\
                    "    \"Knowledge\";\n";
  for(iter = graph.knowledge_nodes.begin(); iter != graph.knowledge_nodes.end(); iter++) {
    dot          += "    \""+(iter->node)+"\""+(is_marked(iter->node)?" [style=bold,color=red]":"")+";\n";
  }
  dot            += "  }\n";

  // create information nodes
  dot            += "  subgraph information {\n"\
                    "    rank = same;\n"\
                    "    \"Information\";\n";
  for(iter = graph.information_nodes.begin(); iter != graph.information_nodes.end(); iter++) {
    dot          += "    \""+(iter->node)+"\""+(is_marked(iter->node)?" [style=bold,color=red]":"")+";\n";
  }
  dot            += "  }\n";

  // create data nodes
  dot            += "  subgraph data {\n"\
                    "    rank = same;\n"\
                    "    \"Data\";\n";
  for(iter = graph.data_nodes.begin(); iter != graph.data_nodes.end(); iter++) {
    dot          += "    \""+(iter->node)+"\""+(is_marked(iter->node)?" [style=bold,color=red]":"")+";\n";
  }
  dot            += "  }\n";

  // create edges
  for(edgeiter = graph.edges.begin(); edgeiter != graph.edges.end(); edgeiter++) {
    dot          += "    \""+(edgeiter->to)+"\" -> \""+(edgeiter->from)+"\""+style_edge(edgeiter->label, is_marked(edgeiter->from) && is_marked(edgeiter->to))+";\n";
  }

  // end the graph
  dot            += "}";

  char tmpdot [L_tmpnam];
  tmpnam(tmpdot);
  std::ofstream tmpdotf;
  tmpdotf.open(tmpdot);
  tmpdotf << dot;
  tmpdotf.close();

  char tmppng [L_tmpnam];
  tmpnam(tmppng);

  system(std::string("dot -Tpng -o"+std::string(tmppng)+" "+std::string(tmpdot)).c_str());

  pix_graph = Gdk::Pixbuf::create_from_file(std::string(tmppng));

  int w,h;
  w = pix_graph->get_width();
  h = pix_graph->get_height();
  int maxw, maxh;
  maxw = scw_graph->get_width()-10;
  maxh = scw_graph->get_height()-10;
  double ratio = (double)w/(double)h;
  if(maxw<1)
    maxw=300;
  if(maxh<1)
    maxh=200;

  if(w>maxw || h>maxh) {
    if((int)(maxh*ratio) > maxw) {
      w = maxw;
      h = (int)(maxw/ratio);
    }
    else {
      h = maxh;
      w = (int)(maxh*ratio);
    }
  }
  
  img_graph->set(pix_graph->scale_simple(w,h,Gdk::INTERP_BILINEAR));
  if(win_show->get_visible()) {
    win_image->set(pix_graph);
  }

  remove(tmpdot);
  remove(tmppng);

/*
  Agraph_t* G;
  GVC_t* gvc;
  gvc = gvContext();
  G = agmemread((char*)dot.c_str());
  gvLayout(gvc, G, "dot");
  char *graphdata;
  unsigned char *ugraphdata;
  unsigned int graphdatalen;
  gvRenderData(gvc, G, "png", &graphdata, &graphdatalen);
  ugraphdata = (unsigned char*)malloc(graphdatalen+1);
  strcpy((char *)ugraphdata, graphdata);
  pix_graph = Gdk::Pixbuf::create_from_inline(graphdatalen,ugraphdata);
  int w,h;
  w = pix_graph->get_width();
  h = pix_graph->get_height();
  int maxw, maxh;
  maxw = evt_graph->get_width()-10;
  maxh = evt_graph->get_height()-10;
  double ratio = (double)w/(double)h;
  if((int)(maxh*ratio) > maxw) {
    w = maxw;
    h = (int)(maxw/ratio);
  }
  else {
    h = maxh;
    w = (int)(maxh*ratio);
  }

  img_graph->set(pix_graph->scale_simple(w,h,Gdk::INTERP_BILINEAR));

  gvFreeLayout(gvc, G);
  agclose(G);
  gvFreeContext(gvc);
*/
}

bool DIKWTab::is_marked(std::string node) {
  return (find(marked_nodes.begin(), marked_nodes.end(), node) != marked_nodes.end());
}

bool DIKWTab::is_node(std::string node) {
  std::list<DIKGraph::DIKNode>::iterator iter;
  for(iter = graph.knowledge_nodes.begin(); iter != graph.knowledge_nodes.end(); iter++)
    if(*iter == node)
      return true;
  for(iter = graph.information_nodes.begin(); iter != graph.information_nodes.end(); iter++)
    if(*iter == node)
      return true;
  for(iter = graph.data_nodes.begin(); iter != graph.data_nodes.end(); iter++)
    if(*iter == node)
      return true;

  return false;
}

DIKWTab::DIKGraph::DIKNode* DIKWTab::get_node(std::string node) {
  DIKGraph::DIKNode* n = new DIKGraph::DIKNode;
  std::list<DIKGraph::DIKNode>::iterator iter;
  iter = find(graph.knowledge_nodes.begin(), graph.knowledge_nodes.end(), node);
  if(iter != graph.knowledge_nodes.end()) {
    n=&(*iter);
    return n;
  }
  iter = find(graph.information_nodes.begin(), graph.information_nodes.end(), node);
  if(iter != graph.information_nodes.end()) {
    n=&(*iter);
    return n;
  }
  iter = find(graph.data_nodes.begin(), graph.data_nodes.end(), node);
  if(iter != graph.data_nodes.end()) {
    n=&(*iter);
    return n;
  }

  return n;
}

void DIKWTab::mark_children(DIKGraph::DIKNode* node) {
  std::list<DIKGraph::DIKNode*>::iterator iter;
  for(iter = node->children.begin(); iter != node->children.end(); iter++) {
    if(!is_marked((*iter)->node)) {
      marked_nodes.push_back((*iter)->node);
      mark_children(*iter);
    }
  }
}

void DIKWTab::mark_parents(DIKGraph::DIKNode* node) {
  std::list<DIKGraph::DIKNode*>::iterator iter;
  for(iter = node->parents.begin(); iter != node->parents.end(); iter++) {
    if(!is_marked((*iter)->node)) {
      marked_nodes.push_back((*iter)->node);
      mark_parents(*iter);
    }
  }
}

bool DIKWTab::is_edge(DIKGraph::DIKEdge edge) {
  return (find(graph.edges.begin(), graph.edges.end(), edge) != graph.edges.end());
}

std::string DIKWTab::level_of_node(std::string node) {
  if (find(graph.knowledge_nodes.begin(), graph.knowledge_nodes.end(), node) != graph.knowledge_nodes.end())
    return "knowledge";
  else if (find(graph.information_nodes.begin(), graph.information_nodes.end(), node) != graph.information_nodes.end())
    return "information";
  else if (find(graph.data_nodes.begin(), graph.data_nodes.end(), node) != graph.data_nodes.end())
    return "data";
  else
    return "";
}

std::string DIKWTab::style_edge(std::string label, bool marked) {
  std::string style = "";
  if(!label.empty() || marked) {
    style += " [dir=back";
      if(!label.empty())
        style += ",label=\""+label+"\"";
      if(marked)
        style += ",style=bold,color=red";
    style += "]";
  }

  return style;
}

void DIKWTab::on_type_changed() {
  if(com_type->get_active_text() == "Node") {
    lbl_left->set_text("Level");
    com_left->remove_all();
    com_left->append("Knowledge");
    com_left->append("Information");
    com_left->append("Data");
    lbl_right->set_text("Name");
    ent_label->set_sensitive(false);
    com_right->remove_all();
    std::list<std::string>::iterator iter;
    for(iter = db_collections.begin(); iter != db_collections.end(); iter++)
      com_right->append(*iter);
  }
  else if(com_type->get_active_text() == "Edge") {
    lbl_left->set_text("From");
    lbl_right->set_text("To");
    com_left->remove_all();
    com_right->remove_all();
    ent_label->set_sensitive(true);
    std::list<DIKGraph::DIKNode>::iterator iter;
    for(iter = graph.knowledge_nodes.begin(); iter != graph.knowledge_nodes.end(); iter++) {
      com_left->append(iter->node);
      com_right->append(iter->node);
    }
    for(iter = graph.information_nodes.begin(); iter != graph.information_nodes.end(); iter++) {
      com_left->append(iter->node);
      com_right->append(iter->node);
    }
    for(iter = graph.data_nodes.begin(); iter != graph.data_nodes.end(); iter++) {
      com_left->append(iter->node);
      com_right->append(iter->node);
    }
    
    com_left->get_entry()->set_editable(true);
  }
}

void DIKWTab::on_add_clicked() {
  if(com_type->get_active_text() == "Node") {
    if(!is_node(com_right->get_entry_text())) {
      if(com_left->get_active_text() == "Knowledge") {
        logger->log("dikw", "knowledge node \""+com_right->get_entry_text()+"\" added");
        DIKGraph::DIKNode node;
        node.node = com_right->get_entry_text();
        graph.knowledge_nodes.push_back(node);
        Gtk::TreeModel::Row row;
        row = *(nds_store->append());
        row.set_value(0, com_right->get_entry_text());
      }
      else if(com_left->get_active_text() == "Information") {
        logger->log("dikw", "information node \""+com_right->get_entry_text()+"\" adde");
        DIKGraph::DIKNode node;
        node.node = com_right->get_entry_text();
        graph.information_nodes.push_back(node);
        Gtk::TreeModel::Row row;
        row = *(nds_store->append());
        row.set_value(0, com_right->get_entry_text());
      }
      else if(com_left->get_active_text() == "Data") {
        logger->log("dikw", "data node \""+com_right->get_entry_text()+"\" added");
        DIKGraph::DIKNode node;
        node.node = com_right->get_entry_text();
        graph.data_nodes.push_back(node);
        Gtk::TreeModel::Row row;
        row = *(nds_store->append());
        row.set_value(0, com_right->get_entry_text());
      }
      else {
        logger->log("dikw", "level \""+com_left->get_entry_text()+"\" not known");
      }
    }
    else {
      logger->log("dikw", "node \""+com_right->get_entry_text()+"\" already exists at level "+level_of_node(com_right->get_entry_text()));
    }
  }
  else if(com_type->get_active_text() == "Edge") {
    DIKGraph::DIKEdge edge;
    edge.from = com_left->get_entry_text();
    edge.to = com_right->get_entry_text();
    edge.label = ent_label->get_text();
    if(!is_edge(edge) && is_node(edge.from) && is_node(edge.to)) {
      logger->log("dikw", "edge "+edge.toString()+" added");
      graph.edges.push_back(edge);
      DIKGraph::DIKNode *from = get_node(edge.from);
      DIKGraph::DIKNode *to   = get_node(edge.to);
      from->parents.push_back(to);
      to->children.push_back(from);
      Gtk::TreeModel::Row row;
      row = *(edg_store->append());
      row.set_value(0, edge.toString());
    }
    else {
      std::string error = "edge not added";
      if(is_edge(edge))
        error += ", edge already exists";
      if(!is_node(com_left->get_entry_text()))
        error += ", node \""+com_left->get_entry_text()+"\" is not known";
      if(!is_node(com_right->get_entry_text()))
        error += ", node \""+com_right->get_entry_text()+"\" is not known";

      logger->log("dikw", error);
    }
  }

  create_graphviz_dot("");
}

void DIKWTab::on_nodes_remove_clicked() {
  if(trv_nodes->get_selection()->count_selected_rows() == 1){
    Glib::ustring node;
    trv_nodes->get_selection()->get_selected()->get_value(0, node);
    nds_store->erase(trv_nodes->get_selection()->get_selected());
    logger->log("dikw", "removed node: "+node);
    if(level_of_node(node) == "knowledge") {
      std::list<DIKGraph::DIKNode>::iterator iter;
      iter = find(graph.knowledge_nodes.begin(), graph.knowledge_nodes.end(), (std::string)node);
      if(iter != graph.knowledge_nodes.end()) {
        std::list<DIKGraph::DIKNode*>::iterator subiter;
        DIKGraph::DIKNode *p;
        p = get_node(node);
        for(subiter = iter->children.begin(); subiter != iter->children.end(); subiter++) {
          (*subiter)->parents.remove(p);
        }
        for(subiter = iter->parents.begin(); subiter != iter->parents.end(); subiter++) {
          (*subiter)->children.remove(p);
        }

        graph.knowledge_nodes.erase(iter);
      }
    }
    else if(level_of_node(node) == "information") {
      std::list<DIKGraph::DIKNode>::iterator iter;
      iter = find(graph.information_nodes.begin(), graph.information_nodes.end(), (std::string)node);
      if(iter != graph.information_nodes.end()) {
        std::list<DIKGraph::DIKNode*>::iterator subiter;
        DIKGraph::DIKNode *p;
        p = get_node(node);
        for(subiter = iter->children.begin(); subiter != iter->children.end(); subiter++) {
          (*subiter)->parents.remove(p);
        }
        for(subiter = iter->parents.begin(); subiter != iter->parents.end(); subiter++) {
          (*subiter)->children.remove(p);
        }

        graph.information_nodes.erase(iter);
      }
    }
    else if(level_of_node(node) == "data") {
      std::list<DIKGraph::DIKNode>::iterator iter;
      iter = find(graph.data_nodes.begin(), graph.data_nodes.end(), (std::string)node);
      if(iter != graph.data_nodes.end()) {
        std::list<DIKGraph::DIKNode*>::iterator subiter;
        DIKGraph::DIKNode *p;
        p = get_node(node);
        for(subiter = iter->children.begin(); subiter != iter->children.end(); subiter++) {
          (*subiter)->parents.remove(p);
        }
        for(subiter = iter->parents.begin(); subiter != iter->parents.end(); subiter++) {
          (*subiter)->children.remove(p);
        }

        graph.data_nodes.erase(iter);
      }
    }

    // remove edges that are no longer valid
    std::list<DIKGraph::DIKEdge>::iterator edgeiter;
    std::list<DIKGraph::DIKEdge> newedges;
    edg_store->clear();
    Gtk::TreeModel::Row row;
    for(edgeiter = graph.edges.begin(); edgeiter != graph.edges.end(); edgeiter++) {
      if(!(edgeiter->from == node || edgeiter->to == node)) {
        row = *(edg_store->append());
        row.set_value(0, edgeiter->toString());
        newedges.push_back(*edgeiter);
      }
    }
    graph.edges = newedges;
  }

  create_graphviz_dot("");
}

void DIKWTab::on_nodes_mark_clicked() {
  if(trv_nodes->get_selection()->count_selected_rows() == 1) {
    Glib::ustring node;
    trv_nodes->get_selection()->get_selected()->get_value(0, node);
    create_graphviz_dot(node);
    logger->log("dikw", "marked node: "+node);
  }
}

void DIKWTab::on_edges_remove_clicked() {
  if(trv_edges->get_selection()->count_selected_rows() == 1) {
    Glib::ustring edge;
    trv_edges->get_selection()->get_selected()->get_value(0, edge);
    edg_store->erase(trv_edges->get_selection()->get_selected());
    logger->log("dikw", "removed edge: "+edge);
    std::list<DIKGraph::DIKEdge>::iterator iter;
    iter = find(graph.edges.begin(), graph.edges.end(), (std::string)edge);
    
    if(iter != graph.edges.end()) {
      DIKGraph::DIKNode *from;
      DIKGraph::DIKNode *to;
      from = get_node(iter->from);
      from->parents.remove(to);
      to   = get_node(iter->to);
      to->children.remove(from);
      graph.edges.erase(iter);
    }
  }

  create_graphviz_dot("");
}

void DIKWTab::on_edges_mark_clicked() {
  if(trv_edges->get_selection()->count_selected_rows() == 1) {
    Glib::ustring edge;
    trv_edges->get_selection()->get_selected()->get_value(0, edge);    
    create_graphviz_dot(edge.substr(1,edge.find("\"",1)-1));
    logger->log("dikw", "marked node: "+edge.substr(1,edge.find("\"",1)-1)+" from edge: "+edge);
  }
}

bool DIKWTab::on_image_button_release(GdkEventButton *b) {
  if(b->button == 1) {
    if(win_show->get_visible()) {
      win_show->set_visible(false);
      logger->log("dikw", "closing originale sized image in new window");
    }
    else {
      win_image->set(pix_graph);
      win_show->present();
      logger->log("dikw", "opening originale sized image in new window");
    }
  }

  return false;
}

void DIKWTab::set_enabled(bool enabled) {
  Gtk::Widget* tab;
  _builder->get_widget("dikw_tab", tab);
  tab->set_sensitive(enabled);
}


