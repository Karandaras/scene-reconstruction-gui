#include "kidtab.h"
#include "converter.h"
#include <fstream>
#include <gazebo/common/Image.hh>
#include <boost/filesystem/path.hpp>

using namespace SceneReconstruction;

/** @class KIDTab "kidtab.h"
 *  Tab for the GUI that builds and represents the KID Graph.
 *  @author Bastian Klingen
 */

KIDTab::KIDTab(gazebo::transport::NodePtr& _node, LoggerTab* _logger, Glib::RefPtr<Gtk::Builder>& builder)
: SceneTab::SceneTab(builder)
{
  node = _node;
  logger = _logger;  
  this->responseMutex = new boost::mutex();

  _builder->get_widget("kid_toolbutton_new", btn_new);
  btn_new->signal_clicked().connect(sigc::mem_fun(*this,&KIDTab::on_new_clicked));
  _builder->get_widget("kid_toolbutton_load", btn_load);
  btn_load->signal_clicked().connect(sigc::mem_fun(*this,&KIDTab::on_load_clicked));
  _builder->get_widget("kid_toolbutton_save", btn_save);
  btn_save->signal_clicked().connect(sigc::mem_fun(*this,&KIDTab::on_save_clicked));
  _builder->get_widget("kid_toolbutton_close", btn_close);
  btn_close->signal_clicked().connect(sigc::mem_fun(*this,&KIDTab::on_close_clicked));
  _builder->get_widget("kid_combobox_graphs", com_graphs);
  com_graphs->signal_changed().connect(sigc::mem_fun(*this,&KIDTab::on_graphs_changed));
  gra_store = Glib::RefPtr<Gtk::ListStore>::cast_dynamic(_builder->get_object("kid_liststore_graphs"));
  gra_store->clear();

  Gtk::Box *box_type;
  _builder->get_widget("kid_box_top_left", box_type);
  com_type = Gtk::manage(new Gtk::ComboBoxText(false));
  com_type->append("Node");
  com_type->append("Edge");
  com_type->set_active(0);
  box_type->pack_end(*com_type, false, true);
  com_type->signal_changed().connect(sigc::mem_fun(*this,&KIDTab::on_type_changed));
  _builder->get_widget("kid_label_top_center_left", lbl_left);
  Gtk::Box *box_left;
  _builder->get_widget("kid_box_top_center_left", box_left);
  com_left = Gtk::manage(new Gtk::ComboBoxText(true));
  com_left->append("Knowledge");
  com_left->append("Information");
  com_left->append("Data");
  com_left->set_active(0);
  box_left->pack_end(*com_left, true, true);
  cpl_left = Glib::RefPtr<Gtk::EntryCompletion>::cast_dynamic(_builder->get_object("kid_entrycompletion_level_from"));
  cpl_left->set_model(com_left->get_model());
  com_left->get_entry()->set_completion(cpl_left);
  _builder->get_widget("kid_label_top_center_right", lbl_right);
  Gtk::Box *box_right;
  _builder->get_widget("kid_box_top_center_right", box_right);
  com_right = Gtk::manage(new Gtk::ComboBoxText(true));
  box_right->pack_end(*com_right, true, true);
  cpl_right = Glib::RefPtr<Gtk::EntryCompletion>::cast_dynamic(_builder->get_object("kid_entrycompletion_name_to"));
  cpl_right->set_model(com_right->get_model());
  com_right->get_entry()->set_completion(cpl_right);
  _builder->get_widget("kid_entry_top_right", ent_label);
  ent_label->set_sensitive(false);
  _builder->get_widget("kid_toolbutton_top_add", btn_add);
  btn_add->signal_clicked().connect(sigc::mem_fun(*this,&KIDTab::on_add_clicked));
  _builder->get_widget("kid_treeview_nodes", trv_nodes);
  nds_store = Glib::RefPtr<Gtk::ListStore>::cast_dynamic(_builder->get_object("kid_liststore_nodes"));
  nds_store->clear();
  _builder->get_widget("kid_toolbutton_nodes_delete", btn_nodes_remove);
  btn_nodes_remove->signal_clicked().connect(sigc::mem_fun(*this,&KIDTab::on_nodes_remove_clicked));
  _builder->get_widget("kid_toolbutton_unmark", btn_unmark);
  btn_unmark->signal_clicked().connect(sigc::mem_fun(*this,&KIDTab::on_unmark_clicked));
  _builder->get_widget("kid_treeview_edges", trv_edges);
  edg_store = Glib::RefPtr<Gtk::ListStore>::cast_dynamic(_builder->get_object("kid_liststore_edges"));
  edg_store->clear();
  _builder->get_widget("kid_toolbutton_edges_delete", btn_edges_remove);
  btn_edges_remove->signal_clicked().connect(sigc::mem_fun(*this,&KIDTab::on_edges_remove_clicked));
  _builder->get_widget("kid_scrolledwindow_graph", scw_graph);
  gda_graph = Gtk::manage(new GraphDrawingArea());
  scw_graph->add(*gda_graph);
  gda_graph->show();
  gda_graph->signal_button_release_event().connect(sigc::mem_fun(*this,&KIDTab::on_graph_release));

  _builder->get_widget("kid_toolbutton_zoom_in", btn_zoomin);
  btn_zoomin->signal_clicked().connect(sigc::mem_fun(*this,&KIDTab::on_zoom_in_clicked));
  _builder->get_widget("kid_toolbutton_zoom_out", btn_zoomout);
  btn_zoomout->signal_clicked().connect(sigc::mem_fun(*this,&KIDTab::on_zoom_out_clicked));
  _builder->get_widget("kid_toolbutton_zoom_fit", btn_zoomfit);
  btn_zoomfit->signal_clicked().connect(sigc::mem_fun(*this,&KIDTab::on_zoom_fit_clicked));
  _builder->get_widget("kid_toolbutton_zoom_reset", btn_zoomreset);
  btn_zoomreset->signal_clicked().connect(sigc::mem_fun(*this,&KIDTab::on_zoom_reset_clicked));
  _builder->get_widget("kid_toolbutton_export", btn_export);
  btn_export->signal_clicked().connect(sigc::mem_fun(*this,&KIDTab::on_export_clicked));

  _builder->get_widget("kid_document_window", win_show);
  _builder->get_widget("kid_document_combobox", win_combo);
  win_combo->signal_changed().connect(sigc::mem_fun(*this,&KIDTab::on_document_changed));
  _builder->get_widget("kid_document_image", win_image);
  missing_image = Gdk::Pixbuf::create_from_file("res/noimg.png");
  _builder->get_widget("kid_document_textview", win_textview);
  win_textbuffer = Glib::RefPtr<Gtk::TextBuffer>::cast_dynamic(_builder->get_object("kid_document_textbuffer"));
  win_store = Glib::RefPtr<Gtk::ListStore>::cast_dynamic(_builder->get_object("kid_document_liststore"));

  fcd_save = new Gtk::FileChooserDialog("Save Graph", Gtk::FILE_CHOOSER_ACTION_SAVE);
  fcd_open = new Gtk::FileChooserDialog("Load Graph", Gtk::FILE_CHOOSER_ACTION_OPEN);
  fcd_save->add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
  fcd_save->add_button(Gtk::Stock::SAVE, Gtk::RESPONSE_OK);
  fcd_open->add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
  fcd_open->add_button(Gtk::Stock::OPEN, Gtk::RESPONSE_OK);
  filter_kgf = Gtk::FileFilter::create();
  filter_kgf->set_name("KID Graph File (KGF)");
  filter_kgf->add_pattern("*.kgf");
  fcd_save->add_filter(filter_kgf);
  fcd_save->set_filter(filter_kgf);
  fcd_open->add_filter(filter_kgf);
  fcd_open->set_filter(filter_kgf);

  _builder->get_widget("kid_dialog_newgraph", dia_new);
  _builder->get_widget("kid_dialog_newgraph_entry", dia_new_entry);

  resSub = node->Subscribe("~/SceneReconstruction/GUI/MongoDB", &KIDTab::OnResponseMsg, this);
  framePub = node->Advertise<gazebo::msgs::Request>("~/SceneReconstruction/Framework/Request");
  pclPub = node->Advertise<gazebo::msgs::Drawing>("~/SceneReconstruction/RobotController/Draw");
  on_response_msg.connect( sigc::mem_fun( *this , &KIDTab::ProcessResponseMsg ));
}

KIDTab::~KIDTab() {
}

void KIDTab::OnResponseMsg(ConstResponsePtr& _msg) {
  {
    boost::mutex::scoped_lock lock(*this->responseMutex);
    this->responseMsgs.push_back(*_msg);
  }
  on_response_msg();
}

void KIDTab::ProcessResponseMsg() {
  boost::mutex::scoped_lock lock(*this->responseMutex);
  std::list<gazebo::msgs::Response>::iterator _msg;
  for(_msg = responseMsgs.begin(); _msg != responseMsgs.end(); _msg++) {
    if(_msg->response() == "success" || _msg->response() == "part") {
      if(_msg->request() == "collection_names") {
        logger->log("kid", "received collections for nodelist");

        gazebo::msgs::GzString_V src;
        if(_msg->has_type() && _msg->type() == src.GetTypeName()) {
          src.ParseFromString(_msg->serialized_data());
          int n = src.data_size();
          db_collections.clear();
          for(int i=0; i<n; i++) {
            db_collections.push_back(src.data(i));
          }
          if(com_type->get_active_text() == "Node") {
            logger->log("kid", "updating nodelist");
            com_right->remove_all();
            std::list<std::string>::iterator iter;
            for(iter = db_collections.begin(); iter != db_collections.end(); iter++)
              com_right->append(*iter);
          }
        }
      }
      else if(_msg->request() == "documents" && docreq->id() == _msg->id()) {
        logger->log("kid", "received documents for selected node: "+docreq->data());

        gazebo::msgs::Message_V docs;
        if(_msg->has_type() && _msg->type() == docs.GetTypeName()) {
          docs.ParseFromString(_msg->serialized_data());
          gazebo::msgs::SceneDocument doc;
          if(docs.msgtype() == doc.GetTypeName()) {
            int n = docs.msgsdata_size();
            
            for(int i=0; i<n; i++) {
              logger->log("kid", "processing document for selected node: "+docreq->data());
              doc.ParseFromString(docs.msgsdata(i));
              Gtk::TreeModel::Row row;
              row = *(win_store->append());
              row.set_value(0, "Time: "+Converter::to_ustring_time(doc.timestamp()));
              Glib::RefPtr<Gdk::Pixbuf> img;
              if(doc.has_image()) {
                gazebo::common::Image tmpimg;
                gazebo::msgs::Set(tmpimg, doc.image());
                tmpimg.SavePNG("tmp_img.png");
                img = Gdk::Pixbuf::create_from_file("tmp_img.png");
              }
              else {
                img = missing_image;
              }

              gazebo::msgs::Drawing pcl;
              if(doc.has_pointcloud()) {
                pcl.CopyFrom(doc.pointcloud());
              }
              else {
                pcl.set_name("pointcloud");
                pcl.set_visible(false);
              }
              
              row.set_value(1, i);
              win_images[i] = img;
              win_pointclouds[i] = pcl;
              row.set_value(2, doc.document());
            }
            win_combo->set_active(win_store->children().begin());
          }
        }
      }
    }
  }
  responseMsgs.clear();
}

void KIDTab::create_graphviz_dot() {
  std::string dot = graph.get_dot();
  gda_graph->set_graph(dot);
  gda_graph->zoom_fit();  
}

void KIDTab::on_type_changed() {
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
    std::list<KIDGraph::KIDNode>::iterator iter;
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

void KIDTab::on_add_clicked() {
  if(com_type->get_active_text() == "Node") {
    if(!graph.is_node(com_right->get_entry_text())) {
      if(com_left->get_active_text() == "Knowledge") {
        logger->log("kid", "knowledge node \""+com_right->get_entry_text()+"\" added");
        KIDGraph::KIDNode node;
        node.node = com_right->get_entry_text();
        graph.knowledge_nodes.push_back(node);
        Gtk::TreeModel::Row row;
        row = *(nds_store->append());
        row.set_value(0, com_right->get_entry_text());
      }
      else if(com_left->get_active_text() == "Information") {
        logger->log("kid", "information node \""+com_right->get_entry_text()+"\" added");
        KIDGraph::KIDNode node;
        node.node = com_right->get_entry_text();
        graph.information_nodes.push_back(node);
        Gtk::TreeModel::Row row;
        row = *(nds_store->append());
        row.set_value(0, com_right->get_entry_text());
      }
      else if(com_left->get_active_text() == "Data") {
        logger->log("kid", "data node \""+com_right->get_entry_text()+"\" added");
        KIDGraph::KIDNode node;
        node.node = com_right->get_entry_text();
        graph.data_nodes.push_back(node);
        Gtk::TreeModel::Row row;
        row = *(nds_store->append());
        row.set_value(0, com_right->get_entry_text());
      }
      else {
        logger->log("kid", "level \""+com_left->get_entry_text()+"\" not known");
      }
    }
    else {
      logger->log("kid", "node \""+com_right->get_entry_text()+"\" already exists at level "+graph.level_of_node(com_right->get_entry_text()));
    }
  }
  else if(com_type->get_active_text() == "Edge") {
    KIDGraph::KIDEdge edge;
    edge.from = com_left->get_entry_text();
    edge.to = com_right->get_entry_text();
    edge.label = ent_label->get_text();
    if(!graph.is_edge(edge) && graph.is_node(edge.from) && graph.is_node(edge.to)) {
      logger->log("kid", "edge "+edge.toString()+" added");
      graph.edges.push_back(edge);
      KIDGraph::KIDNode *from = graph.get_node(edge.from);
      KIDGraph::KIDNode *to   = graph.get_node(edge.to);
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

      logger->log("kid", error);
    }
  }

  com_graphs->get_active()->set_value(1,graph.save_to_string());

  create_graphviz_dot();
}

void KIDTab::on_nodes_remove_clicked() {
  if(trv_nodes->get_selection()->count_selected_rows() == 1){
    Glib::ustring node;
    trv_nodes->get_selection()->get_selected()->get_value(0, node);
    nds_store->erase(trv_nodes->get_selection()->get_selected());
    logger->log("kid", "removed node: "+node);
    if(graph.level_of_node(node) == "knowledge") {
      std::list<KIDGraph::KIDNode>::iterator iter;
      iter = find(graph.knowledge_nodes.begin(), graph.knowledge_nodes.end(), (std::string)node);
      if(iter != graph.knowledge_nodes.end()) {
        std::list<KIDGraph::KIDNode*>::iterator subiter;
        KIDGraph::KIDNode *p;
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
      std::list<KIDGraph::KIDNode>::iterator iter;
      iter = find(graph.information_nodes.begin(), graph.information_nodes.end(), (std::string)node);
      if(iter != graph.information_nodes.end()) {
        std::list<KIDGraph::KIDNode*>::iterator subiter;
        KIDGraph::KIDNode *p;
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
      std::list<KIDGraph::KIDNode>::iterator iter;
      iter = find(graph.data_nodes.begin(), graph.data_nodes.end(), (std::string)node);
      if(iter != graph.data_nodes.end()) {
        std::list<KIDGraph::KIDNode*>::iterator subiter;
        KIDGraph::KIDNode *p;
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
    std::list<KIDGraph::KIDEdge>::iterator edgeiter;
    std::list<KIDGraph::KIDEdge> newedges;
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

  create_graphviz_dot();
}

void KIDTab::on_unmark_clicked() {
  graph.clear_markup();
  com_graphs->get_active()->set_value(1,graph.save_to_string());
  create_graphviz_dot();
  logger->log("kid", "unmarked graph");
}

void KIDTab::on_edges_remove_clicked() {
  if(trv_edges->get_selection()->count_selected_rows() == 1) {
    Glib::ustring edge;
    trv_edges->get_selection()->get_selected()->get_value(0, edge);
    edg_store->erase(trv_edges->get_selection()->get_selected());
    logger->log("kid", "removed edge: "+edge);
    std::list<KIDGraph::KIDEdge>::iterator iter;
    iter = find(graph.edges.begin(), graph.edges.end(), (std::string)edge);
    
    if(iter != graph.edges.end()) {
      KIDGraph::KIDNode *from;
      KIDGraph::KIDNode *to;
      from = graph.get_node(iter->from);
      from->parents.remove(to);
      to   = graph.get_node(iter->to);
      to->children.remove(from);
      graph.edges.erase(iter);
    }
  }

  com_graphs->get_active()->set_value(1,graph.save_to_string());

  create_graphviz_dot();
}

bool KIDTab::on_graph_release(GdkEventButton *b) {
  std::string node = gda_graph->get_clicked_node(b->x, b->y);
  if(graph.is_node(node)) {
    if(b->button == 1) {
      if(find(db_collections.begin(), db_collections.end(), node) != db_collections.end()) {
        if(!win_show->get_visible()) {
          win_show->present();
          logger->log("kid", "opening document window for node: "+node);
        }
        else
          logger->log("kid", "refreshing document window for node: "+node);

        // clear previous data
        win_store->clear();
        win_images.clear();

        // get new data from framework for selected collection node
        docreq = gazebo::msgs::CreateRequest("documents", node);
        framePub->Publish(*docreq);

      }
      else if (node != "")
        logger->log("kid", "node: "+node+" does not refer to a collection");
    }
    else if(b->button == 3) {
      if(graph.is_marked(node)) {
        logger->log("kid", "unmark node: "+node);
        graph.unmark_node(node);
        com_graphs->get_active()->set_value(1,graph.save_to_string());
        create_graphviz_dot();
      }
      else {
        logger->log("kid", "mark node: "+node);
        graph.mark_node(node);
        com_graphs->get_active()->set_value(1,graph.save_to_string());
        create_graphviz_dot();
      }
    }
  }

  return false;
}

void KIDTab::on_document_changed() {
  // set textbuffer and image according to selection
  // send pointcloud to gazebo or remove currently shown one
  int id;
  if(win_combo->get_active_row_number() != -1) {
    win_combo->get_active()->get_value(1,id);
    win_image->set(win_images[id]);
    pclPub->Publish(win_pointclouds[id]);
    Glib::ustring doc;
    win_combo->get_active()->get_value(2,doc);
    win_textbuffer->set_text(Converter::parse_json(doc));
  }
  else {
    win_image->set(Gtk::Stock::MISSING_IMAGE, Gtk::ICON_SIZE_BUTTON);
    gazebo::msgs::Drawing pcl;
    pcl.set_name("pointcloud");
    pcl.set_visible(false);
    pclPub->Publish(pcl);
    win_textbuffer->set_text("");
  }
}

void KIDTab::on_new_clicked() {
  // create new graph inside combobox
  Gtk::Window *w;
  _builder->get_widget("window", w);
  
  dia_new->set_transient_for(*w);
  int result = dia_new->run();
  if (result == Gtk::RESPONSE_OK) {
    std::string __name = dia_new_entry->get_text();
    std::string __graph = __name+"\n";
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
      com_graphs->set_active(row);
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

void KIDTab::on_load_clicked() {
  // load existing graph file into combobox (.kgf)
  Gtk::Window *w;
  _builder->get_widget("window", w);
  
  fcd_open->set_transient_for(*w);

  int result = fcd_open->run();
  if (result == Gtk::RESPONSE_OK) {
    std::string filename = fcd_open->get_filename();
    std::string __graph = "";
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

      size_t pos = __graph.find("\n");
      std::string __name = __graph.substr(0, pos);

      Gtk::TreeModel::Row row;
      row = *(gra_store->append());
      row.set_value(0, __name);
      row.set_value(1, __graph);
      com_graphs->set_active(row);
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

void KIDTab::on_save_clicked() {
  // save selected graph to file (.kgf)
  Gtk::Window *w;
  _builder->get_widget("window", w);
  
  fcd_save->set_transient_for(*w);

  int result = fcd_save->run();
  if (result == Gtk::RESPONSE_OK) {

    std::string filename = fcd_save->get_filename();
    if (filename != "") {
      size_t ext_pos = filename.rfind(".");
      std::string extension = "";
      if(ext_pos != std::string::npos) {
        extension = filename.substr(ext_pos+1);
        std::transform(extension.begin(), extension.end(), extension.begin(), ::tolower);
      }
      if(extension != "kgf")
        filename += ".kgf";

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

void KIDTab::on_close_clicked() {
  // remove selected graph from combobox
  Gtk::TreeModel::iterator oldgraph = com_graphs->get_active();
  Gtk::TreeModel::iterator newgraph = com_graphs->get_active();
  newgraph++;
  if(newgraph == gra_store->children().end())
    newgraph = gra_store->children().begin();

  if(gra_store->children().size() == 1) {
    gra_store->clear();
  }
  else {
    com_graphs->set_active(newgraph);
    gra_store->erase(oldgraph);
  }
}

void KIDTab::on_graphs_changed() {
  if(gra_store->children().size() == 0) {
    graph.load("");
    nds_store->clear();
    edg_store->clear();
  }
  else {
    Glib::ustring g;
    com_graphs->get_active()->get_value(1,g);
    graph.load(g);

    nds_store->clear();
    std::list<KIDGraph::KIDNode>::iterator iter;
    Gtk::TreeModel::Row row;
    for(iter = graph.knowledge_nodes.begin(); iter != graph.knowledge_nodes.end(); iter++) {
      row = *(nds_store->append());
      row.set_value(0, iter->node);
    }
    for(iter = graph.information_nodes.begin(); iter != graph.information_nodes.end(); iter++) {
      row = *(nds_store->append());
      row.set_value(0, iter->node);
    }
    for(iter = graph.data_nodes.begin(); iter != graph.data_nodes.end(); iter++) {
      row = *(nds_store->append());
      row.set_value(0, iter->node);
    }

    edg_store->clear();
    std::list<KIDGraph::KIDEdge>::iterator eiter;
    for(eiter = graph.edges.begin(); eiter != graph.edges.end(); eiter++) {
      row = *(edg_store->append());
      row.set_value(0, eiter->toString());
    }
  }

  create_graphviz_dot();
}

void KIDTab::on_zoom_in_clicked() {
  gda_graph->zoom_in();
}

void KIDTab::on_zoom_out_clicked() {
  gda_graph->zoom_out();
}

void KIDTab::on_zoom_fit_clicked() {
  gda_graph->zoom_fit();
}

void KIDTab::on_zoom_reset_clicked() {
  gda_graph->zoom_reset();
}

void KIDTab::on_export_clicked() {
  gda_graph->save();  
}

