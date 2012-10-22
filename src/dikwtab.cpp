#include "dikwtab.h"
#include <fstream>

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

  _builder->get_widget("dikw_toolbutton_new", btn_new);
  btn_new->signal_clicked().connect(sigc::mem_fun(*this,&DIKWTab::on_new_clicked));
  _builder->get_widget("dikw_toolbutton_load", btn_load);
  btn_load->signal_clicked().connect(sigc::mem_fun(*this,&DIKWTab::on_load_clicked));
  _builder->get_widget("dikw_toolbutton_save", btn_save);
  btn_save->signal_clicked().connect(sigc::mem_fun(*this,&DIKWTab::on_save_clicked));
  _builder->get_widget("dikw_toolbutton_close", btn_close);
  btn_close->signal_clicked().connect(sigc::mem_fun(*this,&DIKWTab::on_close_clicked));
  _builder->get_widget("dikw_combobox_graphs", com_graphs);
  com_graphs->signal_changed().connect(sigc::mem_fun(*this,&DIKWTab::on_graphs_changed));
  gra_store = Glib::RefPtr<Gtk::ListStore>::cast_dynamic(_builder->get_object("dikw_liststore_graphs"));
  gra_store->clear();

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
  _builder->get_widget("dikw_scrolledwindow_graph", scw_graph);
  gda_graph = Gtk::manage(new GraphDrawingArea());
  scw_graph->add(*gda_graph);
  gda_graph->show();
  gda_graph->signal_button_release_event().connect(sigc::mem_fun(*this,&DIKWTab::on_graph_release));

  _builder->get_widget("dikw_toolbutton_zoom_in", btn_zoomin);
  btn_zoomin->signal_clicked().connect(sigc::mem_fun(*this,&DIKWTab::on_zoom_in_clicked));
  _builder->get_widget("dikw_toolbutton_zoom_out", btn_zoomout);
  btn_zoomout->signal_clicked().connect(sigc::mem_fun(*this,&DIKWTab::on_zoom_out_clicked));
  _builder->get_widget("dikw_toolbutton_zoom_fit", btn_zoomfit);
  btn_zoomfit->signal_clicked().connect(sigc::mem_fun(*this,&DIKWTab::on_zoom_fit_clicked));
  _builder->get_widget("dikw_toolbutton_zoom_reset", btn_zoomreset);
  btn_zoomreset->signal_clicked().connect(sigc::mem_fun(*this,&DIKWTab::on_zoom_reset_clicked));
  _builder->get_widget("dikw_toolbutton_export", btn_export);
  btn_export->signal_clicked().connect(sigc::mem_fun(*this,&DIKWTab::on_export_clicked));

  _builder->get_widget("dikw_document_window", win_show);
  _builder->get_widget("dikw_document_combobox", win_combo);
  win_combo->signal_changed().connect(sigc::mem_fun(*this,&DIKWTab::on_document_changed));
  _builder->get_widget("dikw_document_image", win_image);
  missing_image = Gdk::Pixbuf::create_from_file("res/noimg.png");
  _builder->get_widget("dikw_document_textview", win_textview);
  win_textbuffer = Glib::RefPtr<Gtk::TextBuffer>::cast_dynamic(_builder->get_object("dikw_document_textbuffer"));
  win_store = Glib::RefPtr<Gtk::ListStore>::cast_dynamic(_builder->get_object("dikw_document_liststore"));

  fcd_save = new Gtk::FileChooserDialog("Save Graph", Gtk::FILE_CHOOSER_ACTION_SAVE);
  fcd_open = new Gtk::FileChooserDialog("Load Graph", Gtk::FILE_CHOOSER_ACTION_OPEN);
  fcd_save->add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
  fcd_save->add_button(Gtk::Stock::SAVE, Gtk::RESPONSE_OK);
  fcd_open->add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
  fcd_open->add_button(Gtk::Stock::OPEN, Gtk::RESPONSE_OK);
  filter_dgf = Gtk::FileFilter::create();
  filter_dgf->set_name("DIK Graph File (DGF)");
  filter_dgf->add_pattern("*.dgf");
  fcd_save->add_filter(filter_dgf);
  fcd_save->set_filter(filter_dgf);
  fcd_open->add_filter(filter_dgf);
  fcd_open->set_filter(filter_dgf);

  dia_new = new Gtk::Dialog("New Graph");
  dia_new->add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
  dia_new->add_button(Gtk::Stock::OK, Gtk::RESPONSE_OK);
  dia_new_entry = Gtk::manage(new Gtk::Entry());
  dia_new->get_content_area()->add(*dia_new_entry);

  resSub = node->Subscribe("~/SceneReconstruction/GUI/MongoDB", &DIKWTab::OnResponseMsg, this);
  framePub = node->Advertise<gazebo::msgs::Request>("~/SceneReconstruction/Framework/Request");
}

DIKWTab::~DIKWTab() {
}

void DIKWTab::OnResponseMsg(ConstResponsePtr& _msg) {
  if(_msg->response() != "success")
    return;

  if(_msg->request() == "collection_names") {
    logger->log("dikw", "received collections for nodelist");

    gazebo::msgs::GzString_V src;
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
  else if(_msg->request() == "documents" && docreq->id() == _msg->id()) {
    logger->log("dikw", "received documents for selected node: "+docreq->data());

    gazebo::msgs::Message_V docs;
    if(_msg->has_type() && _msg->type() == docs.GetTypeName()) {
      docs.ParseFromString(_msg->serialized_data());
      gazebo::msgs::SceneDocument doc;
      if(docs.msgtype() == doc.GetTypeName()) {
        int n = docs.msgsdata_size();
        win_store->clear();
        win_images.clear();
        
        for(int i=0; i<n; i++) {
          doc.ParseFromString(docs.msgsdata(i));
          Gtk::TreeModel::Row row;
          row = *(win_store->append());
          row.set_value(0, doc.timestamp());
          Glib::RefPtr<Gdk::Pixbuf> img;
          if(doc.has_image()) {
            // TODO: set Gdk::Pixbuf from Image message
          }
          else {
            img = missing_image;
          }
          row.set_value(1, i);
          win_images[i] = img;
          row.set_value(2, doc.document());
        }
      }
    }
  }
}

void DIKWTab::create_graphviz_dot(std::string markupnode) {
  std::string dot = graph.get_dot(markupnode);
  gda_graph->set_graph(dot);
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
    if(!graph.is_node(com_right->get_entry_text())) {
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
        logger->log("dikw", "information node \""+com_right->get_entry_text()+"\" added");
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
      logger->log("dikw", "node \""+com_right->get_entry_text()+"\" already exists at level "+graph.level_of_node(com_right->get_entry_text()));
    }
  }
  else if(com_type->get_active_text() == "Edge") {
    DIKGraph::DIKEdge edge;
    edge.from = com_left->get_entry_text();
    edge.to = com_right->get_entry_text();
    edge.label = ent_label->get_text();
    if(!graph.is_edge(edge) && graph.is_node(edge.from) && graph.is_node(edge.to)) {
      logger->log("dikw", "edge "+edge.toString()+" added");
      graph.edges.push_back(edge);
      DIKGraph::DIKNode *from = graph.get_node(edge.from);
      DIKGraph::DIKNode *to   = graph.get_node(edge.to);
      from->children.push_back(to);
      to->parents.push_back(from);
      Gtk::TreeModel::Row row;
      row = *(edg_store->append());
      row.set_value(0, edge.toString());
    }
    else {
      std::string error = "edge not added";
      if(graph.is_edge(edge))
        error += ", edge already exists";
      if(!graph.is_node(com_left->get_entry_text()))
        error += ", node \""+com_left->get_entry_text()+"\" is not known";
      if(!graph.is_node(com_right->get_entry_text()))
        error += ", node \""+com_right->get_entry_text()+"\" is not known";

      logger->log("dikw", error);
    }
  }

  com_graphs->get_active()->set_value(1,graph.save_to_string());

  create_graphviz_dot("");
}

void DIKWTab::on_nodes_remove_clicked() {
  if(trv_nodes->get_selection()->count_selected_rows() == 1){
    Glib::ustring node;
    trv_nodes->get_selection()->get_selected()->get_value(0, node);
    nds_store->erase(trv_nodes->get_selection()->get_selected());
    logger->log("dikw", "removed node: "+node);
    if(graph.level_of_node(node) == "knowledge") {
      std::list<DIKGraph::DIKNode>::iterator iter;
      iter = find(graph.knowledge_nodes.begin(), graph.knowledge_nodes.end(), (std::string)node);
      if(iter != graph.knowledge_nodes.end()) {
        std::list<DIKGraph::DIKNode*>::iterator subiter;
        DIKGraph::DIKNode *p;
        p = graph.get_node(node);
        for(subiter = iter->children.begin(); subiter != iter->children.end(); subiter++) {
          (*subiter)->parents.remove(p);
        }
        for(subiter = iter->parents.begin(); subiter != iter->parents.end(); subiter++) {
          (*subiter)->children.remove(p);
        }

        graph.knowledge_nodes.erase(iter);
      }
    }
    else if(graph.level_of_node(node) == "information") {
      std::list<DIKGraph::DIKNode>::iterator iter;
      iter = find(graph.information_nodes.begin(), graph.information_nodes.end(), (std::string)node);
      if(iter != graph.information_nodes.end()) {
        std::list<DIKGraph::DIKNode*>::iterator subiter;
        DIKGraph::DIKNode *p;
        p = graph.get_node(node);
        for(subiter = iter->children.begin(); subiter != iter->children.end(); subiter++) {
          (*subiter)->parents.remove(p);
        }
        for(subiter = iter->parents.begin(); subiter != iter->parents.end(); subiter++) {
          (*subiter)->children.remove(p);
        }

        graph.information_nodes.erase(iter);
      }
    }
    else if(graph.level_of_node(node) == "data") {
      std::list<DIKGraph::DIKNode>::iterator iter;
      iter = find(graph.data_nodes.begin(), graph.data_nodes.end(), (std::string)node);
      if(iter != graph.data_nodes.end()) {
        std::list<DIKGraph::DIKNode*>::iterator subiter;
        DIKGraph::DIKNode *p;
        p = graph.get_node(node);
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

  com_graphs->get_active()->set_value(1,graph.save_to_string());

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
      from = graph.get_node(iter->from);
      from->parents.remove(to);
      to   = graph.get_node(iter->to);
      to->children.remove(from);
      graph.edges.erase(iter);
    }
  }

  com_graphs->get_active()->set_value(1,graph.save_to_string());

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

bool DIKWTab::on_graph_release(GdkEventButton *b) {
  if(b->button == 1) {
    std::string node = gda_graph->get_clicked_node(b->x, b->y);
    if(graph.is_node(node) && find(db_collections.begin(), db_collections.end(), node) != db_collections.end()) {
      if(!win_show->get_visible()) {
        win_show->present();
        logger->log("dikw", "opening document window for node: "+node);
      }
      else
        logger->log("dikw", "refreshing document window for node: "+node);

      // TODO: get data from framework for selected collection node
      docreq = gazebo::msgs::CreateRequest("documents", node);
      framePub->Publish(*docreq);
    }
    else if (node != "")
      logger->log("dikw", "node: "+node+" does not refer to a collection");

  }

  return false;
}

void DIKWTab::on_document_changed() {
  // TODO: set textbuffer and image according to selection
  int imgid;
  win_combo->get_active()->get_value(1,imgid);
//  win_image->set(win_images[imgid]);
  Glib::ustring doc;
  win_combo->get_active()->get_value(2,doc);
  win_textbuffer->set_text(doc);
}

void DIKWTab::on_new_clicked() {
  // TODO: create new graph inside combobox
  Gtk::Window *w;
  _builder->get_widget("window", w);
  
  dia_new->set_transient_for(*w);
  int result = dia_new->run();
  if (result == Gtk::RESPONSE_OK) {
    std::string __graph = "";
    std::string __name = dia_new_entry->get_text();
    bool validname = true;
    Gtk::TreeModel::Children tmc = gra_store->children();
    Gtk::TreeModel::iterator tmi = tmc.begin();
    while(validname && tmi != tmc.end()) {
      std::string name;
      tmi->get_value(0, name);
      if(name == __name)
        validname = false;

      tmi++;
    }
    if(validname) {
      Gtk::TreeModel::Row row;
      row = *(gra_store->append());
      row.set_value(0, __name);
      row.set_value(1, __graph);
    }
    else {
      Gtk::MessageDialog md(*w, "Graphname already exists",
			    /* markup */ false, Gtk::MESSAGE_ERROR,
			    Gtk::BUTTONS_OK, /* modal */ true);
      md.set_title("Duplicate Graph Name");
      md.run();
    }
  }

  dia_new->hide();
}

void DIKWTab::on_load_clicked() {
  // TODO: load existing graph file into combobox (.dgf)
  Gtk::Window *w;
  _builder->get_widget("window", w);
  
  fcd_open->set_transient_for(*w);

  int result = fcd_open->run();
  if (result == Gtk::RESPONSE_OK) {

    std::string filename = fcd_open->get_filename();
    char *basec = strdup(filename.c_str());
    char *basen = basename(basec);
    free(basec);
    std::string __graph = "";
    std::string __name = basen;
    if (filename != "") {      
      FILE *f = fopen(filename.c_str(), "r");
      while (! feof(f)) {
        char tmp[4096];
        size_t s;
        if ((s = fread(tmp, 1, 4096, f)) > 0) {
         	__graph.append(tmp, s);
        }
      }
      fclose(f);
      Gtk::TreeModel::Row row;
      row = *(gra_store->append());
      row.set_value(0, __name);
      row.set_value(1, __graph);
    } else {
      Gtk::MessageDialog md(*w, "Invalid filename",
			    /* markup */ false, Gtk::MESSAGE_ERROR,
			    Gtk::BUTTONS_OK, /* modal */ true);
      md.set_title("Invalid File Name");
      md.run();
    }
  }

  fcd_open->hide();
}

void DIKWTab::on_save_clicked() {
  // TODO: save selected graph to file (.dgf)
  Gtk::Window *w;
  _builder->get_widget("window", w);
  
  fcd_save->set_transient_for(*w);

  int result = fcd_save->run();
  if (result == Gtk::RESPONSE_OK) {

    std::string filename = fcd_save->get_filename();
    if (filename != "") {
     	graph.save(filename.c_str());
    } else {
      Gtk::MessageDialog md(*w, "Invalid filename",
			    /* markup */ false, Gtk::MESSAGE_ERROR,
			    Gtk::BUTTONS_OK, /* modal */ true);
      md.set_title("Invalid File Name");
      md.run();
    }
  }

  fcd_save->hide();
}

void DIKWTab::on_close_clicked() {
  // TODO: remove selected graph from combobox
  gra_store->erase(com_graphs->get_active());
}

void DIKWTab::on_graphs_changed() {
  // TODO: display selected graph
  Glib::ustring g;
  com_graphs->get_active()->get_value(1,g);
  graph.load(g);
}

void DIKWTab::on_zoom_in_clicked() {
  // TODO: zoom in
  gda_graph->zoom_in();
}

void DIKWTab::on_zoom_out_clicked() {
  // TODO: zoom out
  gda_graph->zoom_out();
}

void DIKWTab::on_zoom_fit_clicked() {
  // TODO: zoom to fit given space
  gda_graph->zoom_fit();
}

void DIKWTab::on_zoom_reset_clicked() {
  // TODO: zoom to fit given space
  gda_graph->zoom_reset();
}

void DIKWTab::on_export_clicked() {
  // TODO: export graph to other formats
  gda_graph->save();  
}

