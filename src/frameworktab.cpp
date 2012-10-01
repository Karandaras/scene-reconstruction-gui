#include <gazebo/math/Pose.hh>

#include "frameworktab.h"
#include "converter.h"

using namespace SceneReconstruction;

/** @class FrameworkTab "frameworktab.h"
 *  Tab for the GUI that allows the user to receive data from the database of the framework
 *  @author Bastian Klingen
 */

FrameworkTab::FrameworkTab(gazebo::transport::NodePtr& _node, LoggerTab* _logger, Glib::RefPtr<Gtk::Builder>& builder)
: SceneTab::SceneTab(builder)
{
  node = _node;
  logger = _logger;

  _builder->get_widget("framework_spinbutton_object", spn_object);
  spn_object->signal_value_changed().connect(sigc::mem_fun(*this,&FrameworkTab::on_button_object_value_changed), false);

  _builder->get_widget("framework_toolbutton_collections_refresh", btn_collections_refresh);
  btn_collections_refresh->signal_clicked().connect(sigc::mem_fun(*this,&FrameworkTab::on_button_collections_refresh_clicked));

  _builder->get_widget("framework_toolbutton_collections_select", btn_collections_select);
  btn_collections_select->signal_clicked().connect(sigc::mem_fun(*this,&FrameworkTab::on_button_collections_select_clicked));

  _builder->get_widget("framework_treeview_collections", trv_collections);
  col_store = Glib::RefPtr<Gtk::ListStore>::cast_dynamic(_builder->get_object("framework_liststore_collections"));
  col_store->clear();

  _builder->get_widget("framework_textview_object", txt_object);
  txt_object->override_font(Pango::FontDescription("monospace"));
  
  reqPub = node->Advertise<gazebo::msgs::Request>("~/SceneReconstruction/Framework/Request");
  resSub = node->Subscribe("~/SceneReconstruction/GUI/MongoDB", &FrameworkTab::OnResponseMsg, this);
}

FrameworkTab::~FrameworkTab() {
}

void FrameworkTab::OnResponseMsg(ConstResponsePtr& _msg) {
  if ((objReq && _msg->id() == objReq->id()) || (!objReq && _msg->id() == -1)){
    logger->msglog("<<", "~/SceneReconstruction/GUI/MongoDB", _msg);

    if(_msg->response() != "success")
      return;

    if(_msg->request() == "collection_names") {
      gazebo::msgs::String_V src;
      if(_msg->has_type() && _msg->type() == src.GetTypeName()) {
        src.ParseFromString(_msg->serialized_data());
        int n = src.data_size();
        col_store->clear();
        Gtk::TreeModel::Row row;
        for(int i=0; i<n; i++) {
          row = *(col_store->append());
          row.set_value(0, src.data(i));
        }
      }
    }
    else if(_msg->request() == "select_collection") {
      gazebo::msgs::String_V src;
      if(_msg->has_type() && _msg->type() == src.GetTypeName()) {
        src.ParseFromString(_msg->serialized_data());
        char* t;
        double max = strtod(src.data(0).c_str(), &t);
        if(*t != 0) {
          max = 0.0;
        }
        spn_object->set_range(0.0,max);

        if(src.data_size()>1) {
          txt_object->get_buffer()->set_text(Converter::parse_json(src.data(1)));
        }
      }
    }
    else if(_msg->request() == "select_object") {
      gazebo::msgs::String src;
      if(_msg->has_type() && _msg->type() == src.GetTypeName()) {
        src.ParseFromString(_msg->serialized_data());
        txt_object->get_buffer()->set_text(Converter::parse_json(src.data()));
      }
    }
  }
}

void FrameworkTab::on_button_collections_refresh_clicked() {
  objReq.reset(gazebo::msgs::CreateRequest("collection_names"));
  logger->log("framework", "refreshing collections");

  logger->msglog(">>", "~/SceneReconstruction/Framework/Request", objReq);
  reqPub->Publish(*(objReq.get()));
}

void FrameworkTab::on_button_collections_select_clicked() {
  if(trv_collections->get_selection()->count_selected_rows() == 1) {
    Glib::ustring tmp;
    trv_collections->get_selection()->get_selected()->get_value(0, tmp);
    logger->log("framework", "selecting collection "+tmp);
    objReq.reset(gazebo::msgs::CreateRequest("select_collection"));
    objReq->set_data(tmp);
    logger->msglog(">>", "~/SceneReconstruction/Framework/Request", objReq);
    reqPub->Publish(*(objReq.get()));
  }
  else {
    logger->log("framework", "no collection selected");
  }
}

void FrameworkTab::on_button_object_value_changed() {
  logger->log("framework", "select object %d", (int)spn_object->get_value());
  objReq.reset(gazebo::msgs::CreateRequest("select_object"));
  objReq->set_dbl_data(spn_object->get_value());
  logger->msglog(">>", "~/SceneReconstruction/Framework/Request", objReq);
  reqPub->Publish(*(objReq.get()));
}

