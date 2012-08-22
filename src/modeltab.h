#pragma once
#include <gtkmm.h>
#include <gdk/gdk.h>

#include <google/protobuf/message.h>

#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>

#include "transport/Transport.hh"
#include "transport/TransportTypes.hh"
#include "transport/Node.hh"
#include "gazebo_config.h"

#include "scenetab.h"
#include "loggertab.h"

namespace SceneReconstruction {
  /** @class ModelTab "modeltab.h"
   * Tab for the GUI that displays additional data about the selected object.
   * @author Bastian Klingen
   */
  class ModelTab : public SceneTab
  {
    public:
      /** Constructor
       * @param _node Gazebo Node Pointer to use
       * @param _logger LoggerTab to use
       * @param builder the ui_builder to access the needed parts
       */
      ModelTab(gazebo::transport::NodePtr&, LoggerTab*, Glib::RefPtr<Gtk::Builder>&);
      /** Destructor */
      ~ModelTab();

    private:
      gazebo::transport::NodePtr                       node;
      LoggerTab                                       *logger;
      Gtk::TreeView                                   *trv_model;
      Glib::RefPtr<Gtk::TreeStore>                     mdl_store;

      // request message to detect selection through gui
      boost::shared_ptr<gazebo::msgs::Request const>   guiReq;
      boost::shared_ptr<gazebo::msgs::Response const>  guiRes;
      gazebo::transport::SubscriberPtr                 resSub,
                                                       reqSub;

    private:
      void fill_model_treeview(gazebo::msgs::Model);
      void OnReqMsg(ConstRequestPtr&);
      void OnResMsg(ConstResponsePtr&);
  };
}
