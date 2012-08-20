#include "objectinstantiatortab.h"

using namespace SceneReconstruction;

/** @class ObjectInstantiatorTab "robotcontrollertab.h"
 * Tab for the GUI that displays data of the ObjectInstantiator Gazebo Plugin
 * @author Bastian Klingen
 */

ObjectInstantiatorTab::ObjectInstantiatorTab(gazebo::transport::NodePtr& _node, LoggerTab* _logger)
: SceneTab::SceneTab("ObjectInstantiator"),
  tbl_objects(9,9,false)
{
  node = _node;
  logger = _logger;
  tbl_objects.set_col_spacings(5);

  sceneReqPub = node->Advertise<gazebo::msgs::Request>("~/SceneReconstruction/ObjectInstantiator/Request");
  sceneResSub = node->Subscribe("~/SceneReconstruction/ObjectInstantiator/Response", &ObjectInstantiatorTab::OnResponseMsg, this);

  // object list
  lbl_object.set_text("Spawned Objects");
  tbl_objects.attach(lbl_object, 0, 2, 0, 1, Gtk::FILL, Gtk::FILL);

  trv_object.set_model(obj_store = Gtk::ListStore::create(obj_cols));
  trv_object.set_hover_selection(false);
  trv_object.set_enable_tree_lines(false);
  trv_object.get_selection()->set_mode(Gtk::SELECTION_SINGLE);

  trv_object.append_column("Name", obj_cols.name);
  trv_object.set_hexpand(false);
  
  scw_object.add(trv_object);
  scw_object.set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);
  scw_object.set_size_request(150,140);
  scw_object.set_hexpand(false);
  scw_object.set_vexpand(true);
  tbl_objects.attach(scw_object, 0, 2, 1, 8, Gtk::FILL, Gtk::EXPAND|Gtk::FILL);

  // data list
  lbl_data.set_text("Object Data");
  tbl_objects.attach(lbl_data, 2, 7, 0, 1, Gtk::EXPAND|Gtk::FILL, Gtk::FILL);

  trv_data.set_model(obj_data = Gtk::ListStore::create(dat_cols));
  trv_data.set_hover_selection(false);
  trv_data.set_enable_tree_lines(false);
  trv_data.get_selection()->set_mode(Gtk::SELECTION_NONE);

  trv_data.append_column("Description", dat_cols.desc);
  trv_data.append_column("Data", dat_cols.data);
  trv_data.set_hexpand(true);
  
  scw_data.add(trv_data);
  scw_data.set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);
  scw_data.set_size_request(350,140);
  scw_data.set_hexpand(true);
  scw_data.set_vexpand(true);
  tbl_objects.attach(scw_data, 2, 7, 1, 8, Gtk::EXPAND|Gtk::FILL, Gtk::EXPAND|Gtk::FILL);

  // repository list
  lbl_repo.set_text("Object-Repository");
  tbl_objects.attach(lbl_repo, 7, 9, 0, 1, Gtk::EXPAND|Gtk::FILL, Gtk::FILL);

  trv_repo.set_model(obj_repo = Gtk::ListStore::create(rep_cols));
  trv_repo.set_hover_selection(false);
  trv_repo.set_enable_tree_lines(false);
  trv_repo.get_selection()->set_mode(Gtk::SELECTION_NONE);

  trv_repo.append_column("Name", rep_cols.name);
  trv_repo.set_hexpand(false);
  
  scw_repo.add(trv_repo);
  scw_repo.set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);
  scw_repo.set_size_request(150,140);
  scw_repo.set_hexpand(false);
  scw_repo.set_vexpand(true);
  tbl_objects.attach(scw_repo, 7, 9, 1, 8, Gtk::FILL, Gtk::EXPAND|Gtk::FILL);

  // btn_show
  btn_show.set_label("Display\nData");
  btn_show.set_size_request(10,30);
  btn_show.set_hexpand(false);
  btn_show.set_vexpand(false);
  tbl_objects.attach(btn_show,1,2,8,9, Gtk::FILL, Gtk::FILL);
  btn_show.signal_clicked().connect(sigc::mem_fun(*this,&ObjectInstantiatorTab::on_button_show_clicked));

  // btn_refresh_objects
  btn_refresh_objects.set_label("Refresh");
  btn_refresh_objects.set_size_request(10,30);
  btn_refresh_objects.set_hexpand(false);
  btn_refresh_objects.set_vexpand(false);
  tbl_objects.attach(btn_refresh_objects,0,1,8,9, Gtk::FILL, Gtk::FILL);
  btn_refresh_objects.signal_clicked().connect(sigc::mem_fun(*this,&ObjectInstantiatorTab::on_button_refresh_objects_clicked));

  // btn_refresh_repository
  btn_refresh_repository.set_label("Refresh");
  btn_refresh_repository.set_size_request(10,30);
  btn_refresh_repository.set_hexpand(false);
  btn_refresh_repository.set_vexpand(false);
  tbl_objects.attach(btn_refresh_repository,7,9,8,9, Gtk::FILL, Gtk::FILL);
  btn_refresh_repository.signal_clicked().connect(sigc::mem_fun(*this,&ObjectInstantiatorTab::on_button_refresh_repository_clicked));

  tbl_objects.show_all_children();
}

ObjectInstantiatorTab::~ObjectInstantiatorTab() {
}

void ObjectInstantiatorTab::OnResponseMsg(ConstResponsePtr& _msg) {
  if(!objReq || objReq->id() != _msg->id()) 
    return;

  logger->msglog("<<", _msg);

  if(_msg->request() == "object_list") {
    // process message

    objReq.reset();
  }
  else if(_msg->request() == "object_repository") {
    // process message

    objReq.reset();
  }
  else if(_msg->request() == "object_data") {
    if(!objRes) {
      objRes = _msg;
      return;
    }
    else if(objRes->type() == _msg->type())
      return;

    // message from framework and gazebo present
    // process messages

    objRes.reset();
    objReq.reset();
  }
}

void ObjectInstantiatorTab::on_button_show_clicked() {
  logger->log("object instantiator", "SHOW");
  if(trv_object.get_selection()->count_selected_rows() == 1) {
    objReq.reset(gazebo::msgs::CreateRequest("object_data"));
    objReq->set_data(trv_object.get_selection()->get_selected()->get_value(obj_cols.name));
    sceneReqPub->Publish(*(objReq.get()));
    logger->log("object instantiator", "requesting data of selected spawned object from ObjectInstantiatorPlugin");
    logger->msglog(">>", objReq);
  }
  else
    logger->log("object instantiator", "no object to request data for selected");
}

void ObjectInstantiatorTab::on_button_refresh_objects_clicked() {
  logger->log("object instantiator", "REFRESH");
  objReq.reset(gazebo::msgs::CreateRequest("object_list"));
  sceneReqPub->Publish(*(objReq.get()));
  logger->log("object instantiator", "requesting list of spawned objects from ObjectInstantiatorPlugin");
  logger->msglog(">>", objReq);
}

void ObjectInstantiatorTab::on_button_refresh_repository_clicked() {
  logger->log("object instantiator", "REFRESH");
  objReq.reset(gazebo::msgs::CreateRequest("object_repository"));
  sceneReqPub->Publish(*(objReq.get()));
  logger->log("object instantiator", "requesting repository list from ObjectInstantiatorPlugin");
  logger->msglog(">>", objReq);
}

Gtk::Widget& ObjectInstantiatorTab::get_tab() {
  return tbl_objects;
}

