#include "objectinstantiatortab.h"
#include "converter.h"

using namespace SceneReconstruction;

/** @class ObjectInstantiatorTab "robotcontrollertab.h"
 *  Tab for the GUI that displays data of the ObjectInstantiator Gazebo Plugin
 *  @author Bastian Klingen
 */

ObjectInstantiatorTab::ObjectInstantiatorTab(gazebo::transport::NodePtr& _node, LoggerTab* _logger, Glib::RefPtr<Gtk::Builder>& builder)
: SceneTab::SceneTab(builder)
{
  node = _node;
  logger = _logger;

  sceneReqPub = node->Advertise<gazebo::msgs::Request>("~/SceneReconstruction/ObjectInstantiator/Request");
  sceneResSub = node->Subscribe("~/SceneReconstruction/ObjectInstantiator/Response", &ObjectInstantiatorTab::OnResponseMsg, this);

  // object list
  _builder->get_widget("objectinstantiator_treeview_spawnedobjects", trv_object);
  obj_store = Glib::RefPtr<Gtk::ListStore>::cast_dynamic(_builder->get_object("objectinstantiator_liststore_spawnedobjects"));
//  obj_store->clear();

  // data list
  _builder->get_widget("objectinstantiator_image_objectdata", img_data);
  _builder->get_widget("objectinstantiator_eventbox_objectdata", evt_data);
  evt_data->add_events(Gdk::BUTTON_RELEASE_MASK);
  evt_data->signal_button_release_event().connect( sigc::mem_fun(*this, &ObjectInstantiatorTab::on_image_button_release) );

  _builder->get_widget("objectinstantiator_combobox_objectdata", com_data);
  img_store = Glib::RefPtr<Gtk::ListStore>::cast_dynamic(_builder->get_object("objectinstantiator_liststore_objectdata"));
  img_store->clear();
  Gtk::TreeModel::Row row;    
  row = *(img_store->append());
  row.set_value(0, (Glib::ustring)"None");
  images["None"] = Gdk::Pixbuf::create_from_file("res/noimg.png");
  img_data->set(images["None"]->scale_simple(270,210,Gdk::INTERP_BILINEAR));
  com_data->set_active(0);
  com_data->signal_changed().connect( sigc::mem_fun(*this, &ObjectInstantiatorTab::on_combo_changed) );

  _builder->get_widget("objectinstantiator_treeview_objectdata", trv_data);
  dat_store = Glib::RefPtr<Gtk::TreeStore>::cast_dynamic(_builder->get_object("objectinstantiator_treestore_objectdata"));
  dat_store->clear();

  // repository list
  _builder->get_widget("objectinstantiator_treeview_objectrepository", trv_repo);
  rep_store = Glib::RefPtr<Gtk::ListStore>::cast_dynamic(_builder->get_object("objectinstantiator_liststore_objectrepository"));
  rep_store->clear();

  _builder->get_widget("objectinstantiator_objectdata_window", win_show);
  win_show->set_visible(false);
  _builder->get_widget("objectinstantiator_objectdata_window_scrolledwindow", win_scroll);
  _builder->get_widget("objectinstantiator_objectdata_window_image", win_image);
  win_image->set(images["None"]);
  _builder->get_widget("objectinstantiator_objectdata_window_combobox", win_combo);
  win_combo->set_active(0);
  win_combo->signal_changed().connect( sigc::mem_fun(*this, &ObjectInstantiatorTab::on_win_combo_changed) );

  // btn_show
  _builder->get_widget("objectinstantiator_toolbutton_display_spawnedobjects", btn_show);
  btn_show->signal_clicked().connect(sigc::mem_fun(*this,&ObjectInstantiatorTab::on_button_show_clicked));

  // btn_refresh_objects
  _builder->get_widget("objectinstantiator_toolbutton_refresh_spawnedobjects", btn_refresh_objects);
  btn_refresh_objects->signal_clicked().connect(sigc::mem_fun(*this,&ObjectInstantiatorTab::on_button_refresh_objects_clicked));

  // btn_refresh_repository
  _builder->get_widget("objectinstantiator_toolbutton_refresh_objectrepository", btn_refresh_repository);
  btn_refresh_repository->signal_clicked().connect(sigc::mem_fun(*this,&ObjectInstantiatorTab::on_button_refresh_repository_clicked));
}

ObjectInstantiatorTab::~ObjectInstantiatorTab() {
}

void ObjectInstantiatorTab::OnResponseMsg(ConstResponsePtr& _msg) {
  if(!objReq || objReq->id() != _msg->id()) 
    return;

  logger->msglog("<<", "~/SceneReconstruction/ObjectInstantiator/Response", _msg);

  if(_msg->request() == "object_list") {
    gazebo::msgs::String_V src;
    if(_msg->has_type() && _msg->type() == src.GetTypeName()) {
      src.ParseFromString(_msg->serialized_data());
      logger->log("object instantiator", "receiving object list from ObjectInstantiatorPlugin");

      int n = src.data_size();
      obj_store->clear();
      Gtk::TreeModel::Row row;

      for(int i = 0; i<n; i++) {
        row = *(obj_store->append());
        row.set_value(0, src.data(i));
      }
    }

    objReq.reset();
  }
  else if(_msg->request() == "object_repository") {
    gazebo::msgs::String_V src;
    if(_msg->has_type() && _msg->type() == src.GetTypeName()) {
      src.ParseFromString(_msg->serialized_data());
      logger->log("object instantiator", "receiving object repository from ObjectInstantiatorPlugin");

      int n = src.data_size();
      rep_store->clear();
      Gtk::TreeModel::Row row;

      for(int i = 0; i<n; i++) {
        row = *(rep_store->append());
        row.set_value(0, src.data(i));
      }
    }

    objReq.reset();
  }
  else if(_msg->request() == "object_data") {
    if(!objRes) {
      objRes = _msg;
      return;
    }
    else if(objRes->type() == _msg->type()) // same message again
      return;

    // message from framework and gazebo present
    gazebo::msgs::SceneObject src1;
    gazebo::msgs::SceneObjectData src2;

    if((_msg->has_type() && _msg->type() == src1.GetTypeName()) && (objRes->has_type() && objRes->type() == src2.GetTypeName())) {
      src1.ParseFromString(_msg->serialized_data());
      src2.ParseFromString(objRes->serialized_data());
    }
    else if((_msg->has_type() && _msg->type() == src2.GetTypeName()) && (objRes->has_type() && objRes->type() == src1.GetTypeName())) {
      src2.ParseFromString(_msg->serialized_data());
      src1.ParseFromString(objRes->serialized_data());
    }
    else {
      logger->log("object instantiator", "received messages not matching required types");
      return;
    }
    
    if(!src1.has_objectids() || src1.objectids() != src2.objectids()) {
      logger->log("object instantiator", "received messages refer to different objectids");
      return;
    }

    dat_store->clear();
    Gtk::TreeModel::Row row;

    logger->log("object instantiator", "receiving object data from ObjectInstantiatorPlugin");

    if(src1.has_object_type()) {
      row = *(dat_store->append());
      row.set_value(0, (Glib::ustring)"Object Type");
      row.set_value(1, src1.object_type());
    }

    if(src1.has_pos_x() && src1.has_pos_y() && src1.has_pos_z()) {
      row = *(dat_store->append());
      row.set_value(0, (Glib::ustring)"Position");
      row.set_value(1, Converter::convert(src1.pos_x(), src1.pos_y(), src1.pos_z()));
    }

    if(src1.has_ori_w() && src1.has_ori_x() && src1.has_ori_y() && src1.has_ori_z()) {
      row = *(dat_store->append());
      row.set_value(0, (Glib::ustring)"Orientation");
      row.set_value(1, Converter::convert(src1.ori_w(), src1.ori_x(), src1.ori_y(), src1.ori_z()));
    }

    if(src1.has_frame()) {
      row = *(dat_store->append());
      row.set_value(0, (Glib::ustring)"Frame");
      row.set_value(1, src1.frame());
    }

    if(src1.has_objectids()) {
      row = *(dat_store->append());
      row.set_value(0, (Glib::ustring)"ObjectIDs");
      row.set_value(1, src1.objectids());
    }

    if(src1.has_name()) {
      row = *(dat_store->append());
      row.set_value(0, (Glib::ustring)"Name");
      row.set_value(1, src1.name());
    }

    images.clear();
    img_store->clear();

    if(src2.images_size() > 0) {
      for(int i=0; i<src2.images_size(); i++) {
        row = *(img_store->append());
        row.set_value(0, src2.images(i).name());
        // TODO: use data obtained from the framework to create an image
        images[src2.images(i).name()] = Gdk::Pixbuf::create_from_file("res/noimg.png");
      }
    }
    else {
      row = *(img_store->append());
      row.set_value(0, (Glib::ustring)"None");
      images["None"] = Gdk::Pixbuf::create_from_file("res/noimg.png");
    }

    image_iter = images.begin();
    img_data->set(image_iter->second);

    objRes.reset();
    objReq.reset();
  }
}

void ObjectInstantiatorTab::on_combo_changed() {
  if(!win_change) {
    logger->log("object instantiator", "COMBOBOX CHANGED");
    Gtk::TreeModel::iterator iter = com_data->get_active();
    if(iter) {
      win_change = true;
      win_combo->set_active(iter);
      Gtk::TreeModel::Row row = *iter;
      if(row) {
        Glib::ustring name;
        row.get_value(0, name);
        image_iter = images.find(name);
        img_data->set(image_iter->second->scale_simple(270,210,Gdk::INTERP_BILINEAR));
        logger->log("object instantiator", "displaying image "+image_iter->first);
        if(win_show->get_visible()) {
          win_image->set(image_iter->second);
        }
      }
    }
    win_change = false;
  }
}

void ObjectInstantiatorTab::on_win_combo_changed() {
  if(!win_change) {
    logger->log("object instantiator", "COMBOBOX CHANGED");
    Gtk::TreeModel::iterator iter = win_combo->get_active();
    if(iter) {
      win_change = true;
      com_data->set_active(iter);
      Gtk::TreeModel::Row row = *iter;
      if(row) {
        Glib::ustring name;
        row.get_value(0, name);
        image_iter = images.find(name);
        img_data->set(image_iter->second->scale_simple(270,210,Gdk::INTERP_BILINEAR));
        logger->log("object instantiator", "displaying image "+image_iter->first);
        if(win_show->get_visible()) {
          win_image->set(image_iter->second);
        }
      }
    }
    win_change = false;
  }
}

void ObjectInstantiatorTab::on_button_show_clicked() {
  logger->log("object instantiator", "SHOW");
  if(trv_object->get_selection()->count_selected_rows() == 1) {
    objReq.reset(gazebo::msgs::CreateRequest("object_data"));
    Glib::ustring tmp;
    trv_object->get_selection()->get_selected()->get_value(0, tmp);
    objReq->set_data(tmp);
    sceneReqPub->Publish(*(objReq.get()));
    logger->log("object instantiator", "requesting data of selected spawned object from ObjectInstantiatorPlugin");
    logger->msglog(">>", "~/SceneReconstruction/ObjectInstantiator/Request", objReq);
  }
  else
    logger->log("object instantiator", "no object to request data for selected");
}

void ObjectInstantiatorTab::on_button_refresh_objects_clicked() {
  logger->log("object instantiator", "REFRESH");
  objReq.reset(gazebo::msgs::CreateRequest("object_list"));
  sceneReqPub->Publish(*(objReq.get()));
  logger->log("object instantiator", "requesting list of spawned objects from ObjectInstantiatorPlugin");
  logger->msglog(">>", "~/SceneReconstruction/ObjectInstantiator/Request", objReq);
}

void ObjectInstantiatorTab::on_button_refresh_repository_clicked() {
  logger->log("object instantiator", "REFRESH");
  objReq.reset(gazebo::msgs::CreateRequest("object_repository"));
  sceneReqPub->Publish(*(objReq.get()));
  logger->log("object instantiator", "requesting repository list from ObjectInstantiatorPlugin");
  logger->msglog(">>", "~/SceneReconstruction/ObjectInstantiator/Request", objReq);
}

bool ObjectInstantiatorTab::on_image_button_release(GdkEventButton *b) {
  if(b->button == 1) {
    if(win_show->get_visible()) {
      win_show->set_visible(false);
      logger->log("object instantiator", "closing originale sized image in new window");
    }
    else {
      Gtk::TreeModel::iterator iter = com_data->get_active();
      if(iter) {
        Gtk::TreeModel::Row row = *iter;
        if(row) {
          Glib::ustring name;
          row.get_value(0, name);
          image_iter = images.find(name);
          win_image->set(image_iter->second);
        }
      }
      win_show->present();
      logger->log("object instantiator", "opening originale sized image in new window");
    }
  }

  return false;
}

void ObjectInstantiatorTab::on_win_button_close_clicked() {
  win_show->set_visible(false);
  logger->log("object instantiator", "closing originale sized image in new window");
}


void ObjectInstantiatorTab::set_enabled(bool enabled) {
  Gtk::Widget* tab;
  _builder->get_widget("objectinstantiator_tab", tab);
  tab->set_sensitive(enabled);
}
