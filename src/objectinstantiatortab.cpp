#include "objectinstantiatortab.h"
#include "converter.h"
#include <gazebo/common/Image.hh>

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
  this->responseMutex = new boost::mutex();

  // object list
  _builder->get_widget("objectinstantiator_treeview_spawnedobjects", trv_object);
  obj_store = Glib::RefPtr<Gtk::ListStore>::cast_dynamic(_builder->get_object("objectinstantiator_liststore_spawnedobjects"));

  // data list
  _builder->get_widget("objectinstantiator_image_objectdata", img_data);
  _builder->get_widget("objectinstantiator_eventbox_objectdata", evt_data);
  evt_data->add_events(Gdk::BUTTON_RELEASE_MASK);
  evt_data->signal_button_release_event().connect( sigc::mem_fun(*this, &ObjectInstantiatorTab::on_image_button_release) );

  _builder->get_widget("objectinstantiator_combobox_objectdata", com_data);
  img_store = Glib::RefPtr<Gtk::ListStore>::cast_dynamic(_builder->get_object("objectinstantiator_liststore_objectdata"));
  Gtk::TreeModel::Row row;    
  row = *(img_store->append());
  row.set_value(0, (Glib::ustring)"None");
  images["None"] = Gdk::Pixbuf::create_from_file("res/noimg.png");
  img_data->set(images["None"]->scale_simple(455,240,Gdk::INTERP_BILINEAR));
  com_data->set_active(0);
  com_data->signal_changed().connect( sigc::mem_fun(*this, &ObjectInstantiatorTab::on_combo_changed) );

  _builder->get_widget("objectinstantiator_treeview_objectdata", trv_data);
  dat_store = Glib::RefPtr<Gtk::TreeStore>::cast_dynamic(_builder->get_object("objectinstantiator_treestore_objectdata"));

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

  sceneReqPub = node->Advertise<gazebo::msgs::Request>("~/SceneReconstruction/ObjectInstantiator/Request");
  sceneResSub = node->Subscribe("~/SceneReconstruction/ObjectInstantiator/Response", &ObjectInstantiatorTab::OnResponseMsg, this);
  on_response_msg.connect( sigc::mem_fun( *this , &ObjectInstantiatorTab::ProcessResponseMsg ));
}

ObjectInstantiatorTab::~ObjectInstantiatorTab() {
}

void ObjectInstantiatorTab::OnResponseMsg(ConstResponsePtr& _msg) {
  {
    boost::mutex::scoped_lock lock(*this->responseMutex);
    this->responseMsgs.push_back(*_msg);
  }
  on_response_msg();
}

void ObjectInstantiatorTab::ProcessResponseMsg() {
  boost::mutex::scoped_lock lock(*this->responseMutex);
  std::list<gazebo::msgs::Response>::iterator _msg;
  for(_msg = responseMsgs.begin(); _msg != responseMsgs.end(); _msg++) {
    if(objReq && objReq->id() == _msg->id()) {
      logger->msglog("<<", "~/SceneReconstruction/ObjectInstantiator/Response", *_msg);

      if(_msg->request() == "object_list") {
        gazebo::msgs::GzString_V src;
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
      else if(_msg->request() == "object_data") {
        if(!objRes) {
          gazebo::msgs::Response res;
          res.CopyFrom(*_msg);
          objRes.reset(&res);
          return;
        }
        else if(objRes->type() == _msg->type()) // same message again
          return;

        // message from framework and gazebo present
        gazebo::msgs::SceneObject src1;
        gazebo::msgs::Message_V   src2;

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
        
        dat_store->clear();
        Gtk::TreeModel::Row row;

        logger->log("object instantiator", "receiving object data from ObjectInstantiatorPlugin");
        if(src1.has_model()) {
          row = *(dat_store->append());
          row.set_value(0, (Glib::ustring)"Model");
          row.set_value(1, src1.model());
        }

        if(src1.has_pose()) {
          row = *(dat_store->append());
          row.set_value(0, (Glib::ustring)"Position");
          row.set_value(1, Converter::convert(src1.pose(), 0));
          row = *(dat_store->append());
          row.set_value(0, (Glib::ustring)"Orientation");
          row.set_value(1, Converter::convert(src1.pose(), 1));
        }

        if(src1.has_object()) {
          row = *(dat_store->append());
          row.set_value(0, (Glib::ustring)"Object");
          row.set_value(1, src1.object());
        }

        images.clear();
        img_store->clear();
        row = *(img_store->append());
        row.set_value(0, (Glib::ustring)"None");
        images["None"] = Gdk::Pixbuf::create_from_file("res/noimg.png");

        int n = src2.msgsdata_size();
        gazebo::msgs::SceneDocument doc;
        if(src2.msgtype() == doc.GetTypeName()) {
          for(int m = 0; m<n; m++) {
            doc.ParseFromString(src2.msgsdata(m));

            if(doc.has_image()) {
              row = *(img_store->append());
              row.set_value(0, doc.interface());
              gazebo::common::Image *img = 0;
              gazebo::msgs::Set(*img, doc.image());
              img->SavePNG("tmp_img.png");
              images[doc.interface()] = Gdk::Pixbuf::create_from_file("tmp_img.png");
            }
            
            row = *(dat_store->append());
            row.set_value(0, (Glib::ustring)"Documents");
            row.set_value(1, (Glib::ustring)"");

            Gtk::TreeModel::Row childrow;
            childrow = *(dat_store->append(row.children()));
            childrow.set_value(0, doc.interface());
            childrow.set_value(1, (Glib::ustring)"");

            Gtk::TreeModel::Row cchildrow;
            cchildrow = *(dat_store->append(childrow.children()));
            cchildrow.set_value(0, (Glib::ustring)"");
            cchildrow.set_value(1, Converter::parse_json(doc.document()));
          }
        }    
        image_iter = images.begin();
        img_data->set(image_iter->second);

        objRes.reset();
        objReq.reset();
      }
    }
  }
  responseMsgs.clear();
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
        img_data->set(image_iter->second->scale_simple(455,240,Gdk::INTERP_BILINEAR));
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
        img_data->set(image_iter->second->scale_simple(455,240,Gdk::INTERP_BILINEAR));
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
    logger->msglog(">>", "~/SceneReconstruction/ObjectInstantiator/Request", *objReq);
  }
  else
    logger->log("object instantiator", "no object to request data for selected");
}

void ObjectInstantiatorTab::on_button_refresh_objects_clicked() {
  logger->log("object instantiator", "REFRESH");
  objReq.reset(gazebo::msgs::CreateRequest("object_list"));
  sceneReqPub->Publish(*(objReq.get()));
  logger->log("object instantiator", "requesting list of spawned objects from ObjectInstantiatorPlugin");
  logger->msglog(">>", "~/SceneReconstruction/ObjectInstantiator/Request", *objReq);
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

