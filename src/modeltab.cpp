#include "modeltab.h"
#include "converter.h"

using namespace SceneReconstruction;

/** @class ModelTab "modeltab.h"
 *  Tab for the GUI that displays additional data about the selected object.
 *  @author Bastian Klingen
 */

ModelTab::ModelTab(gazebo::transport::NodePtr& _node, LoggerTab* _logger, Glib::RefPtr<Gtk::Builder>& builder) : SceneTab::SceneTab(builder)
{
  node = _node;
  logger = _logger;

  reqSub = node->Subscribe("~/request", &ModelTab::OnReqMsg, this);
  resSub = node->Subscribe("~/response", &ModelTab::OnResMsg, this);

  _builder->get_widget("model_treeview", trv_model);
  mdl_store = Glib::RefPtr<Gtk::TreeStore>::cast_dynamic(_builder->get_object("model_treestore"));
  mdl_store->clear();
}

ModelTab::~ModelTab() {
}

void ModelTab::fill_model_treeview(gazebo::msgs::Model model) {
  mdl_store->clear();
  Gtk::TreeModel::Row row = *(mdl_store->append());
  Gtk::TreeModel::Row childrow;
  Gtk::TreeModel::Row cchildrow;
  Gtk::TreeModel::Row ccchildrow;
  Gtk::TreeModel::Row cccchildrow;
  row.set_value(0, (Glib::ustring)"Name");
  row.set_value(1, (Glib::ustring)model.name());
  if(model.has_id()) {
    row = *(mdl_store->append());
    row.set_value(0, (Glib::ustring)"ID");
    row.set_value(1, Converter::to_ustring(model.id()));
  }
  if(model.has_is_static()) {
    row = *(mdl_store->append());
    row.set_value(0, (Glib::ustring)"is_static");
    row.set_value(1, Converter::to_ustring(model.is_static()));
  }
  if(model.has_pose()) {
    gazebo::msgs::Pose pose = model.pose();
    row = *(mdl_store->append());
    row.set_value(0, (Glib::ustring)"Pose");
    if(pose.has_name()) {
      row.set_value(1, (Glib::ustring)pose.name());
    } else {
      row.set_value(1, (Glib::ustring)"");
    }

    childrow = *(mdl_store->append(row.children()));
    childrow.set_value(0, (Glib::ustring)"Position");
    childrow.set_value(1, Converter::convert(pose.position()));
    childrow = *(mdl_store->append(row.children()));
    childrow.set_value(0, (Glib::ustring)"Orientation Q");
    childrow.set_value(1, Converter::convert(pose.orientation()));
    childrow = *(mdl_store->append(row.children()));
    childrow.set_value(0, (Glib::ustring)"Orientation E");
    childrow.set_value(1, Converter::convert(pose.orientation(), -1, true));
  }
  int joints = model.joint_size();
  if(joints > 0) {
    row = *(mdl_store->append());
    row.set_value(0, (Glib::ustring)"Joints");
    row.set_value(1, (Glib::ustring)"");

    for(int i=0; i<joints; i++) {
      gazebo::msgs::Joint joint = model.joint(i);
      childrow = *(mdl_store->append(row.children()));
      childrow.set_value(0, (Glib::ustring)joint.name());
      childrow.set_value(1, (Glib::ustring)"");
      if(joint.has_type()) {
        cchildrow = *(mdl_store->append(childrow.children()));
        cchildrow.set_value(0, (Glib::ustring)"Type");
        cchildrow.set_value(1, (Glib::ustring)gazebo::msgs::Joint::Type_Name(joint.type()));
      }
      if(joint.has_parent()) {
        cchildrow = *(mdl_store->append(childrow.children()));
        cchildrow.set_value(0, (Glib::ustring)"Parent");
        cchildrow.set_value(1, (Glib::ustring)joint.parent());
      }
      if(joint.has_child()) {
        cchildrow = *(mdl_store->append(childrow.children()));
        cchildrow.set_value(0, (Glib::ustring)"Child");
        cchildrow.set_value(1, (Glib::ustring)joint.child());
      }
      if(joint.has_pose()) {
        gazebo::msgs::Pose pose = joint.pose();
        cchildrow = *(mdl_store->append(childrow.children()));
        cchildrow.set_value(0, (Glib::ustring)"Pose");
        if(pose.has_name()) {
          cchildrow.set_value(1, (Glib::ustring)pose.name());
        } else {
          cchildrow.set_value(1, (Glib::ustring)"");
        }

        ccchildrow = *(mdl_store->append(cchildrow.children()));
        ccchildrow.set_value(0, (Glib::ustring)"Position");
        ccchildrow.set_value(1, Converter::convert(pose.position()));
        ccchildrow = *(mdl_store->append(cchildrow.children()));
        ccchildrow.set_value(0, (Glib::ustring)"Orientation Q");
        ccchildrow.set_value(1, Converter::convert(pose.orientation()));
        ccchildrow = *(mdl_store->append(cchildrow.children()));
        ccchildrow.set_value(0, (Glib::ustring)"Orientation E");
        ccchildrow.set_value(1, Converter::convert(pose.orientation(), -1, true));
      }
      if(joint.has_axis1()) {
        gazebo::msgs::Axis axis = joint.axis1();
        cchildrow = *(mdl_store->append(childrow.children()));
        cchildrow.set_value(0, (Glib::ustring)"Axis1");
        cchildrow.set_value(1, (Glib::ustring)"");

        ccchildrow = *(mdl_store->append(cchildrow.children()));
        ccchildrow.set_value(0, (Glib::ustring)"XYZ");
        ccchildrow.set_value(1, Converter::convert(axis.xyz()));
        ccchildrow = *(mdl_store->append(cchildrow.children()));
        ccchildrow.set_value(0, (Glib::ustring)"Limit");
        ccchildrow.set_value(0, (Glib::ustring)"");
        cccchildrow = *(mdl_store->append(ccchildrow.children()));
        cccchildrow.set_value(0, (Glib::ustring)"Lower");
        cccchildrow.set_value(1, Converter::to_ustring(axis.limit_lower()));
        cccchildrow = *(mdl_store->append(ccchildrow.children()));
        cccchildrow.set_value(0, (Glib::ustring)"Upper");
        cccchildrow.set_value(1, Converter::to_ustring(axis.limit_upper()));
        cccchildrow = *(mdl_store->append(ccchildrow.children()));
        cccchildrow.set_value(0, (Glib::ustring)"Effort");
        cccchildrow.set_value(1, Converter::to_ustring(axis.limit_effort()));
        cccchildrow = *(mdl_store->append(ccchildrow.children()));
        cccchildrow.set_value(0, (Glib::ustring)"Velocity");
        cccchildrow.set_value(1, Converter::to_ustring(axis.limit_velocity()));
        ccchildrow = *(mdl_store->append(cchildrow.children()));
        ccchildrow.set_value(0, (Glib::ustring)"Damping");
        ccchildrow.set_value(1, Converter::to_ustring(axis.damping()));
        ccchildrow = *(mdl_store->append(cchildrow.children()));
        ccchildrow.set_value(0, (Glib::ustring)"Friction");
        ccchildrow.set_value(1, Converter::to_ustring(axis.friction()));
      }
      if(joint.has_axis2()) {
        gazebo::msgs::Axis axis = joint.axis2();
        cchildrow = *(mdl_store->append(childrow.children()));
        cchildrow.set_value(0, (Glib::ustring)"Axis2");
        cchildrow.set_value(1, (Glib::ustring)"");

        ccchildrow = *(mdl_store->append(cchildrow.children()));
        ccchildrow.set_value(0, (Glib::ustring)"XYZ");
        ccchildrow.set_value(1, Converter::convert(axis.xyz()));
        ccchildrow = *(mdl_store->append(cchildrow.children()));
        ccchildrow.set_value(0, (Glib::ustring)"Limit");
        ccchildrow.set_value(1, (Glib::ustring)"");
        cccchildrow = *(mdl_store->append(ccchildrow.children()));
        cccchildrow.set_value(0, (Glib::ustring)"Lower");
        cccchildrow.set_value(1, Converter::to_ustring(axis.limit_lower()));
        cccchildrow = *(mdl_store->append(ccchildrow.children()));
        cccchildrow.set_value(0, (Glib::ustring)"Upper");
        cccchildrow.set_value(1, Converter::to_ustring(axis.limit_upper()));
        cccchildrow = *(mdl_store->append(ccchildrow.children()));
        cccchildrow.set_value(0, (Glib::ustring)"Effort");
        cccchildrow.set_value(1, Converter::to_ustring(axis.limit_effort()));
        cccchildrow = *(mdl_store->append(ccchildrow.children()));
        cccchildrow.set_value(0, (Glib::ustring)"Velocity");
        cccchildrow.set_value(1, Converter::to_ustring(axis.limit_velocity()));
        ccchildrow = *(mdl_store->append(cchildrow.children()));
        ccchildrow.set_value(0, (Glib::ustring)"Damping");
        ccchildrow.set_value(1, Converter::to_ustring(axis.damping()));
        ccchildrow = *(mdl_store->append(cchildrow.children()));
        ccchildrow.set_value(0, (Glib::ustring)"Friction");
        ccchildrow.set_value(1, Converter::to_ustring(axis.friction()));
      }
    }
  }
  int links = model.link_size();
  if(links > 0) {
    row = *(mdl_store->append());
    row.set_value(0, (Glib::ustring)"Links");
    row.set_value(1, (Glib::ustring)"");

    for(int i=0; i<links; i++) {
      gazebo::msgs::Link link = model.link(i);
      childrow = *(mdl_store->append(row.children()));
      childrow.set_value(0, (Glib::ustring)link.name());
      childrow.set_value(1, (Glib::ustring)"");

      cchildrow = *(mdl_store->append(childrow.children()));
      cchildrow.set_value(0, (Glib::ustring)"ID");
      cchildrow.set_value(1, Converter::to_ustring(link.id()));

      if(link.has_self_collide()) {
        cchildrow = *(mdl_store->append(childrow.children()));
        cchildrow.set_value(0, (Glib::ustring)"self_collide");
        cchildrow.set_value(1, Converter::to_ustring(link.self_collide()));
      }          
      if(link.has_gravity()) {
        cchildrow = *(mdl_store->append(childrow.children()));
        cchildrow.set_value(0, (Glib::ustring)"gravity");
        cchildrow.set_value(1, Converter::to_ustring(link.gravity()));
      }          
      if(link.has_kinematic()) {
        cchildrow = *(mdl_store->append(childrow.children()));
        cchildrow.set_value(0, (Glib::ustring)"kinematic");
        cchildrow.set_value(1, Converter::to_ustring(link.kinematic()));
      }          
      if(link.has_enabled()) {
        cchildrow = *(mdl_store->append(childrow.children()));
        cchildrow.set_value(0, (Glib::ustring)"enabled");
        cchildrow.set_value(1, Converter::to_ustring(link.enabled()));
      }          
      if(link.has_inertial()) {
        gazebo::msgs::Inertial inert = link.inertial();
        cchildrow = *(mdl_store->append(childrow.children()));
        cchildrow.set_value(0, (Glib::ustring)"Inertial");
        cchildrow.set_value(1, (Glib::ustring)"");
        if(inert.has_mass()) {
          ccchildrow = *(mdl_store->append(cchildrow.children()));
          ccchildrow.set_value(0, (Glib::ustring)"Mass");
          ccchildrow.set_value(1, Converter::to_ustring(inert.mass()));
        }
        if(inert.has_pose()) {
          gazebo::msgs::Pose pose = link.pose();
          ccchildrow = *(mdl_store->append(cchildrow.children()));
          ccchildrow.set_value(0, (Glib::ustring)"Pose");
          if(pose.has_name()) {
            ccchildrow.set_value(1, (Glib::ustring)pose.name());
          } else {
            ccchildrow.set_value(1, (Glib::ustring)"");
          }

          cccchildrow = *(mdl_store->append(ccchildrow.children()));
          cccchildrow.set_value(0, (Glib::ustring)"Position");
          cccchildrow.set_value(1, Converter::convert(pose.position()));
          cccchildrow = *(mdl_store->append(ccchildrow.children()));
          cccchildrow.set_value(0, (Glib::ustring)"Orientation Q");
          cccchildrow.set_value(1, Converter::convert(pose.orientation()));
          cccchildrow = *(mdl_store->append(ccchildrow.children()));
          cccchildrow.set_value(0, (Glib::ustring)"Orientation E");
          cccchildrow.set_value(1, Converter::convert(pose.orientation(), -1, true));
        }
        if(inert.has_ixx()) {
          ccchildrow = *(mdl_store->append(cchildrow.children()));
          ccchildrow.set_value(0, (Glib::ustring)"IXX");
          ccchildrow.set_value(1, Converter::to_ustring(inert.ixx()));
        }
        if(inert.has_ixy()) {
          ccchildrow = *(mdl_store->append(cchildrow.children()));
          ccchildrow.set_value(0, (Glib::ustring)"IXY");
          ccchildrow.set_value(1, Converter::to_ustring(inert.ixy()));
        }
        if(inert.has_ixz()) {
          ccchildrow = *(mdl_store->append(cchildrow.children()));
          ccchildrow.set_value(0, (Glib::ustring)"IXZ");
          ccchildrow.set_value(1, Converter::to_ustring(inert.ixz()));
        }
        if(inert.has_iyy()) {
          ccchildrow = *(mdl_store->append(cchildrow.children()));
          ccchildrow.set_value(0, (Glib::ustring)"IYY");
          ccchildrow.set_value(1, Converter::to_ustring(inert.iyy()));
        }
        if(inert.has_iyz()) {
          ccchildrow = *(mdl_store->append(cchildrow.children()));
          ccchildrow.set_value(0, (Glib::ustring)"IYZ");
          ccchildrow.set_value(1, Converter::to_ustring(inert.iyz()));
        }
        if(inert.has_izz()) {
          ccchildrow = *(mdl_store->append(cchildrow.children()));
          ccchildrow.set_value(0, (Glib::ustring)"IZZ");
          ccchildrow.set_value(1, Converter::to_ustring(inert.izz()));
        }
      }          
      if(link.has_pose()) {
        gazebo::msgs::Pose pose = link.pose();
        cchildrow = *(mdl_store->append(childrow.children()));
        cchildrow.set_value(0, (Glib::ustring)"Pose");
        if(pose.has_name()) {
          cchildrow.set_value(1, (Glib::ustring)pose.name());
        } else {
          cchildrow.set_value(1, (Glib::ustring)"");
        }

        ccchildrow = *(mdl_store->append(cchildrow.children()));
        ccchildrow.set_value(0, (Glib::ustring)"Position");
        ccchildrow.set_value(1, Converter::convert(pose.position()));
        ccchildrow = *(mdl_store->append(cchildrow.children()));
        ccchildrow.set_value(0, (Glib::ustring)"Orientation Q");
        ccchildrow.set_value(1, Converter::convert(pose.orientation()));
        ccchildrow = *(mdl_store->append(cchildrow.children()));
        ccchildrow.set_value(0, (Glib::ustring)"Orientation E");
        ccchildrow.set_value(1, Converter::convert(pose.orientation(), -1, true));
      }
/*
      int visuals = link.visual_size();
      int collisions = link.collision_size();
      int sensors = link.sensor_size();
      int projectors = link.projector_size();
*/
    }
  }
  if(model.has_deleted()) {
    row = *(mdl_store->append());
    row.set_value(0, (Glib::ustring)"deleted");
    row.set_value(1, Converter::to_ustring(model.deleted()));
  }
// Visual
}

void ModelTab::OnReqMsg(ConstRequestPtr& _msg) {
  if (guiRes && _msg->request() == "entity_info" && _msg->id() == guiRes->id()) {
    logger->msglog("<<", "~/request", _msg);

    gazebo::msgs::Model model;
    if (guiRes->has_type() && guiRes->type() == model.GetTypeName()) {
      model.ParseFromString(guiRes->serialized_data());
      logger->log("model", "Data of Model" + model.name() + " received.");
      fill_model_treeview(model);
    } else {
      guiRes.reset();
      guiReq = _msg;
    }
  } else {
    guiReq = _msg;
  }
}

void ModelTab::OnResMsg(ConstResponsePtr& _msg) {
  if (guiReq && _msg->request() == "entity_info" && _msg->id() == guiReq->id()) {
    logger->msglog("<<", "~/response", _msg);

    gazebo::msgs::Model model;
    if (_msg->has_type() && _msg->type() == model.GetTypeName()) {
      model.ParseFromString(_msg->serialized_data());
      logger->log("model", "Data of Model" + model.name() + " received.");
      fill_model_treeview(model);
    } else {
      guiReq.reset();
      guiRes = _msg;
    }
  } else {
    guiRes = _msg;
  }
}

void ModelTab::set_enabled(bool enabled) {
  Gtk::Widget* tab;
  _builder->get_widget("model_tab", tab);
  tab->set_sensitive(enabled);
}
