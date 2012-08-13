#include "modeltab.h"
#include "converter.h"

using namespace SceneReconstruction;

/** @class ModelTab "modeltab.h"
 * Tab for the GUI that displays additional data about the selected object.
 * @author Bastian Klingen
 */

ModelTab::ModelTab(gazebo::transport::NodePtr& _node, LoggerTab* _logger) : SceneTab::SceneTab("ModelInfo")
{
  node = _node;
  logger = _logger;

  reqSub = node->Subscribe("~/request", &ModelTab::OnReqMsg, this);
  resSub = node->Subscribe("~/response", &ModelTab::OnResMsg, this);

  trv_model.set_model(mdl_store = Gtk::TreeStore::create(mdl_cols));
  trv_model.set_hover_selection(true);
  trv_model.set_enable_tree_lines(true);
  trv_model.set_headers_visible(false);

  int c = trv_model.append_column("Description", mdl_cols.desc);
  trv_model.get_column(c-1)->set_resizable(true);
  c = trv_model.append_column_editable("Value", mdl_cols.val);
  trv_model.get_column(c-1)->set_resizable(true);

  scw_model.add(trv_model);
  scw_model.set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);
}

ModelTab::~ModelTab() {
}

void ModelTab::fill_model_treeview(gazebo::msgs::Model model) {
  mdl_store.clear();
  trv_model.set_model(mdl_store = Gtk::TreeStore::create(mdl_cols));
  Gtk::TreeModel::Row row = *(mdl_store->append());
  Gtk::TreeModel::Row childrow;
  Gtk::TreeModel::Row cchildrow;
  Gtk::TreeModel::Row ccchildrow;
  Gtk::TreeModel::Row cccchildrow;
  row[mdl_cols.desc] = "Name";
  row[mdl_cols.val] = model.name();
  if(model.has_id()) {
    row = *(mdl_store->append());
    row[mdl_cols.desc] = "ID";
    row[mdl_cols.val] = Converter::to_ustring(model.id());
  }
  if(model.has_is_static()) {
    row = *(mdl_store->append());
    row[mdl_cols.desc] = "is_static";
    row[mdl_cols.val] = Converter::to_ustring(model.is_static());
  }
  if(model.has_pose()) {
    gazebo::msgs::Pose pose = model.pose();
    row = *(mdl_store->append());
    row[mdl_cols.desc] = "Pose";
    if(pose.has_name()) {
      row[mdl_cols.val] = pose.name();
    } else {
      row[mdl_cols.val] = "";
    }

    childrow = *(mdl_store->append(row.children()));
    childrow[mdl_cols.desc] = "Position";
    childrow[mdl_cols.val] = Converter::convert(pose.position());
    childrow = *(mdl_store->append(row.children()));
    childrow[mdl_cols.desc] = "Orientation Q";
    childrow[mdl_cols.val] = Converter::convert(pose.orientation());
    childrow = *(mdl_store->append(row.children()));
    childrow[mdl_cols.desc] = "Orientation E";
    childrow[mdl_cols.val] = Converter::convert(pose.orientation(), -1, true);
  }
  int joints = model.joint_size();
  if(joints > 0) {
    row = *(mdl_store->append());
    row[mdl_cols.desc] = "Joints";
    row[mdl_cols.val] = "";

    for(int i=0; i<joints; i++) {
      gazebo::msgs::Joint joint = model.joint(i);
      childrow = *(mdl_store->append(row.children()));
      childrow[mdl_cols.desc] = joint.name();
      childrow[mdl_cols.val] = "";
      if(joint.has_type()) {
        cchildrow = *(mdl_store->append(childrow.children()));
        cchildrow[mdl_cols.desc] = "Type";
        cchildrow[mdl_cols.val] = gazebo::msgs::Joint::Type_Name(joint.type());
      }
      if(joint.has_parent()) {
        cchildrow = *(mdl_store->append(childrow.children()));
        cchildrow[mdl_cols.desc] = "Parent";
        cchildrow[mdl_cols.val] = joint.parent();
      }
      if(joint.has_child()) {
        cchildrow = *(mdl_store->append(childrow.children()));
        cchildrow[mdl_cols.desc] = "Child";
        cchildrow[mdl_cols.val] = joint.child();
      }
      if(joint.has_pose()) {
        gazebo::msgs::Pose pose = joint.pose();
        cchildrow = *(mdl_store->append(childrow.children()));
        cchildrow[mdl_cols.desc] = "Pose";
        if(pose.has_name()) {
          cchildrow[mdl_cols.val] = pose.name();
        } else {
          cchildrow[mdl_cols.val] = "";
        }

        ccchildrow = *(mdl_store->append(cchildrow.children()));
        ccchildrow[mdl_cols.desc] = "Position";
        ccchildrow[mdl_cols.val] = Converter::convert(pose.position());
        ccchildrow = *(mdl_store->append(cchildrow.children()));
        ccchildrow[mdl_cols.desc] = "Orientation Q";
        ccchildrow[mdl_cols.val] = Converter::convert(pose.orientation());
        ccchildrow[mdl_cols.desc] = "Orientation E";
        ccchildrow[mdl_cols.val] = Converter::convert(pose.orientation(), -1, true);
      }
      if(joint.has_axis1()) {
        gazebo::msgs::Axis axis = joint.axis1();
        cchildrow = *(mdl_store->append(childrow.children()));
        cchildrow[mdl_cols.desc] = "Axis1";
        cchildrow[mdl_cols.val] = "";

        ccchildrow = *(mdl_store->append(cchildrow.children()));
        ccchildrow[mdl_cols.desc] = "XYZ";
        ccchildrow[mdl_cols.val] = Converter::convert(axis.xyz());
        ccchildrow = *(mdl_store->append(cchildrow.children()));
        ccchildrow[mdl_cols.desc] = "Limit";
        ccchildrow[mdl_cols.val] = "";
        cccchildrow = *(mdl_store->append(ccchildrow.children()));
        cccchildrow[mdl_cols.desc] = "Lower";
        cccchildrow[mdl_cols.val] = Converter::to_ustring(axis.limit_lower());
        cccchildrow = *(mdl_store->append(ccchildrow.children()));
        cccchildrow[mdl_cols.desc] = "Upper";
        cccchildrow[mdl_cols.val] = Converter::to_ustring(axis.limit_upper());
        cccchildrow = *(mdl_store->append(ccchildrow.children()));
        cccchildrow[mdl_cols.desc] = "Effort";
        cccchildrow[mdl_cols.val] = Converter::to_ustring(axis.limit_effort());
        cccchildrow = *(mdl_store->append(ccchildrow.children()));
        cccchildrow[mdl_cols.desc] = "Velocity";
        cccchildrow[mdl_cols.val] = Converter::to_ustring(axis.limit_velocity());
        ccchildrow = *(mdl_store->append(cchildrow.children()));
        ccchildrow[mdl_cols.desc] = "Damping";
        ccchildrow[mdl_cols.val] = Converter::to_ustring(axis.damping());
        ccchildrow = *(mdl_store->append(cchildrow.children()));
        ccchildrow[mdl_cols.desc] = "Friction";
        ccchildrow[mdl_cols.val] = Converter::to_ustring(axis.friction());
      }
      if(joint.has_axis2()) {
        gazebo::msgs::Axis axis = joint.axis2();
        cchildrow = *(mdl_store->append(childrow.children()));
        cchildrow[mdl_cols.desc] = "Axis2";
        cchildrow[mdl_cols.val] = "";

        ccchildrow = *(mdl_store->append(cchildrow.children()));
        ccchildrow[mdl_cols.desc] = "XYZ";
        ccchildrow[mdl_cols.val] = Converter::convert(axis.xyz());
        ccchildrow = *(mdl_store->append(cchildrow.children()));
        ccchildrow[mdl_cols.desc] = "Limit";
        ccchildrow[mdl_cols.val] = "";
        cccchildrow = *(mdl_store->append(ccchildrow.children()));
        cccchildrow[mdl_cols.desc] = "Lower";
        cccchildrow[mdl_cols.val] = Converter::to_ustring(axis.limit_lower());
        cccchildrow = *(mdl_store->append(ccchildrow.children()));
        cccchildrow[mdl_cols.desc] = "Upper";
        cccchildrow[mdl_cols.val] = Converter::to_ustring(axis.limit_upper());
        cccchildrow = *(mdl_store->append(ccchildrow.children()));
        cccchildrow[mdl_cols.desc] = "Effort";
        cccchildrow[mdl_cols.val] = Converter::to_ustring(axis.limit_effort());
        cccchildrow = *(mdl_store->append(ccchildrow.children()));
        cccchildrow[mdl_cols.desc] = "Velocity";
        cccchildrow[mdl_cols.val] = Converter::to_ustring(axis.limit_velocity());
        ccchildrow = *(mdl_store->append(cchildrow.children()));
        ccchildrow[mdl_cols.desc] = "Damping";
        ccchildrow[mdl_cols.val] = Converter::to_ustring(axis.damping());
        ccchildrow = *(mdl_store->append(cchildrow.children()));
        ccchildrow[mdl_cols.desc] = "Friction";
        ccchildrow[mdl_cols.val] = Converter::to_ustring(axis.friction());
      }
    }
  }
  int links = model.link_size();
  if(links > 0) {
    row = *(mdl_store->append());
    row[mdl_cols.desc] = "Links";
    row[mdl_cols.val] = "";

    for(int i=0; i<links; i++) {
      gazebo::msgs::Link link = model.link(i);
      childrow = *(mdl_store->append(row.children()));
      childrow[mdl_cols.desc] = link.name();
      childrow[mdl_cols.val] = "";

      cchildrow = *(mdl_store->append(childrow.children()));
      cchildrow[mdl_cols.desc] = "ID";
      cchildrow[mdl_cols.val] = Converter::to_ustring(link.id());

      if(link.has_self_collide()) {
        cchildrow = *(mdl_store->append(childrow.children()));
        cchildrow[mdl_cols.desc] = "self_collide";
        cchildrow[mdl_cols.val] = Converter::to_ustring(link.self_collide());
      }          
      if(link.has_gravity()) {
        cchildrow = *(mdl_store->append(childrow.children()));
        cchildrow[mdl_cols.desc] = "gravity";
        cchildrow[mdl_cols.val] = Converter::to_ustring(link.gravity());
      }          
      if(link.has_kinematic()) {
        cchildrow = *(mdl_store->append(childrow.children()));
        cchildrow[mdl_cols.desc] = "kinematic";
        cchildrow[mdl_cols.val] = Converter::to_ustring(link.kinematic());
      }          
      if(link.has_enabled()) {
        cchildrow = *(mdl_store->append(childrow.children()));
        cchildrow[mdl_cols.desc] = "enabled";
        cchildrow[mdl_cols.val] = Converter::to_ustring(link.enabled());
      }          
      if(link.has_inertial()) {
        gazebo::msgs::Inertial inert = link.inertial();
        cchildrow = *(mdl_store->append(childrow.children()));
        cchildrow[mdl_cols.desc] = "Inertial";
        cchildrow[mdl_cols.val] = "";
        if(inert.has_mass()) {
          ccchildrow = *(mdl_store->append(cchildrow.children()));
          ccchildrow[mdl_cols.desc] = "Mass";
          ccchildrow[mdl_cols.val] = Converter::to_ustring(inert.mass());
        }
        if(inert.has_pose()) {
          gazebo::msgs::Pose pose = link.pose();
          ccchildrow = *(mdl_store->append(cchildrow.children()));
          ccchildrow[mdl_cols.desc] = "Pose";
          if(pose.has_name()) {
            ccchildrow[mdl_cols.val] = pose.name();
          } else {
            ccchildrow[mdl_cols.val] = "";
          }

          cccchildrow = *(mdl_store->append(ccchildrow.children()));
          cccchildrow[mdl_cols.desc] = "Position";
          cccchildrow[mdl_cols.val] = Converter::convert(pose.position());
          cccchildrow = *(mdl_store->append(ccchildrow.children()));
          cccchildrow[mdl_cols.desc] = "Orientation Q";
          cccchildrow[mdl_cols.val] = Converter::convert(pose.orientation());
          cccchildrow[mdl_cols.desc] = "Orientation E";
          cccchildrow[mdl_cols.val] = Converter::convert(pose.orientation(), -1, true);
        }
        if(inert.has_ixx()) {
          ccchildrow = *(mdl_store->append(cchildrow.children()));
          ccchildrow[mdl_cols.desc] = "IXX";
          ccchildrow[mdl_cols.val] = Converter::to_ustring(inert.ixx());
        }
        if(inert.has_ixy()) {
          ccchildrow = *(mdl_store->append(cchildrow.children()));
          ccchildrow[mdl_cols.desc] = "IXY";
          ccchildrow[mdl_cols.val] = Converter::to_ustring(inert.ixy());
        }
        if(inert.has_ixz()) {
          ccchildrow = *(mdl_store->append(cchildrow.children()));
          ccchildrow[mdl_cols.desc] = "IXZ";
          ccchildrow[mdl_cols.val] = Converter::to_ustring(inert.ixz());
        }
        if(inert.has_iyy()) {
          ccchildrow = *(mdl_store->append(cchildrow.children()));
          ccchildrow[mdl_cols.desc] = "IYY";
          ccchildrow[mdl_cols.val] = Converter::to_ustring(inert.iyy());
        }
        if(inert.has_iyz()) {
          ccchildrow = *(mdl_store->append(cchildrow.children()));
          ccchildrow[mdl_cols.desc] = "IYZ";
          ccchildrow[mdl_cols.val] = Converter::to_ustring(inert.iyz());
        }
        if(inert.has_izz()) {
          ccchildrow = *(mdl_store->append(cchildrow.children()));
          ccchildrow[mdl_cols.desc] = "IZZ";
          ccchildrow[mdl_cols.val] = Converter::to_ustring(inert.izz());
        }
      }          
      if(link.has_pose()) {
        gazebo::msgs::Pose pose = link.pose();
        cchildrow = *(mdl_store->append(childrow.children()));
        cchildrow[mdl_cols.desc] = "Pose";
        if(pose.has_name()) {
          cchildrow[mdl_cols.val] = pose.name();
        } else {
          cchildrow[mdl_cols.val] = "";
        }

        ccchildrow = *(mdl_store->append(cchildrow.children()));
        ccchildrow[mdl_cols.desc] = "Position";
        ccchildrow[mdl_cols.val] = Converter::convert(pose.position());
        ccchildrow = *(mdl_store->append(cchildrow.children()));
        ccchildrow[mdl_cols.desc] = "Orientation Q";
        ccchildrow[mdl_cols.val] = Converter::convert(pose.orientation());
        ccchildrow[mdl_cols.desc] = "Orientation E";
        ccchildrow[mdl_cols.val] = Converter::convert(pose.orientation(), -1, true);
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
    row[mdl_cols.desc] = "deleted";
    row[mdl_cols.val] = Converter::to_ustring(model.deleted());
  }
// Visual
}

void ModelTab::OnReqMsg(ConstRequestPtr& _msg) {
  if (guiRes && _msg->request() == "entity_info" && _msg->id() == guiRes->id()) {
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

Gtk::Widget& ModelTab::get_tab() {
  return scw_model;
}

