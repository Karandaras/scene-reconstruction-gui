#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>

#include <gazebo/transport/Transport.hh>
#include <gazebo/transport/TransportTypes.hh>

#include "scenegui.h"
#include "controltab.h"
#include "robotcontrollertab.h"
#include "objectinstantiatortab.h"
#include "frameworktab.h"
#include "dikwtab.h"
#include "analysistab.h"

using namespace SceneReconstruction;

/** @class SceneGui "scenegui.h"
 *  Main Class that creates the GUI
 *  @author Bastian Klingen
 */

SceneGUI::SceneGUI()
{
  plugin_availability["ObjectInstantiator"] = false;
  plugin_availability["RobotController"]    = false;
  plugin_availability["Framework"]          = false;
  plugin_availability["AnalysisToolbox"]    = true;
  missing_plugins                           = 0;
  std::map<std::string, bool>::iterator plugin;
  for(plugin =  plugin_availability.begin(); plugin != plugin_availability.end(); plugin++) {
    if(!plugin->second) {
      missing_plugins++;
    }
  }
  

  // Setup the GUI
  ui_builder = Gtk::Builder::create_from_file("res/ui.glade");
  ui_builder->get_widget("window", window);

  // create LoggerTab to log all Gazebo output
  logger = new LoggerTab(ui_builder);

  // Init Gazebo
  gazebo::transport::init();
  gazebo::transport::run();
  gazebo::transport::NodePtr _node(new gazebo::transport::Node());
  node = _node;
  node->Init();

  worldPub = node->Advertise<gazebo::msgs::WorldControl>("~/world_control");

  // initially pause the world
  gazebo::msgs::WorldControl start;
  start.set_pause(true);
  logger->msglog(">>", "~/world_control", start);
  worldPub->Publish(start);

  // Create all tabs
  ControlTab*            tab1 = new ControlTab(node, logger, ui_builder);
  RobotControllerTab*    tab2 = new RobotControllerTab(node, logger, ui_builder);
  ObjectInstantiatorTab* tab3 = new ObjectInstantiatorTab(node, logger, ui_builder);
  FrameworkTab*          tab4 = new FrameworkTab(node, logger, ui_builder);
  DIKWTab*               tab5 = new DIKWTab(node, logger, ui_builder);
  AnalysisTab*           tab6 = new AnalysisTab(node, logger, ui_builder);

  vec_tabs.push_back(tab1);
  vec_tabs.push_back(tab2);
  vec_tabs.push_back(tab3);
  vec_tabs.push_back(tab4);
  vec_tabs.push_back(tab5);
  vec_tabs.push_back(tab6);

  window->show_all_children();

  check_components();

  availSub = node->Subscribe("~/SceneReconstruction/GUI/Availability/Response", &SceneGUI::OnResponseMsg, this);
}

SceneGUI::~SceneGUI() {
  node->Fini();
  gazebo::transport::stop();
  gazebo::transport::fini();
}

void SceneGUI::check_components() {
  // publish availability request
  std::map<std::string, bool>::iterator plugin;
  for(plugin =  plugin_availability.begin(); plugin != plugin_availability.end(); plugin++) {
    if(!plugin->second) {
      availPubs[plugin->first] = node->Advertise<gazebo::msgs::Request>("~/SceneReconstruction/GUI/Availability/Request/"+plugin->first);
      avail_requests[plugin->first].reset(gazebo::msgs::CreateRequest("status"));
      availPubs[plugin->first]->Publish(*(avail_requests[plugin->first].get()));
      logger->msglog(">>", "~/SceneReconstruction/GUI/Availability/Request/"+plugin->first, avail_requests[plugin->first]);
    }
    else {
      logger->show_available(plugin->first);
    }
  }
}

void SceneGUI::OnResponseMsg(ConstResponsePtr &_msg) {
  std::map< std::string, boost::shared_ptr<gazebo::msgs::Request> >::iterator avail_request = avail_requests.find(_msg->response());
  if(avail_request == avail_requests.end() && _msg->id() != -1)
    return;
  if(_msg->id() != avail_request->second->id() && _msg->id() != -1)
    return;

  logger->msglog("<<", "~/SceneReconstruction/GUI/Availability/Response", _msg);
 
  // receive availability responses;
  std::map<std::string, bool>::iterator plugin = plugin_availability.find(_msg->response());
  if(plugin != plugin_availability.end()) {
    if(!plugin->second) {
      plugin->second = true;
      missing_plugins--;
      logger->show_available(plugin->first);
      logger->log("available", "component "+plugin->first+" is now available");
    }
  }

  if(missing_plugins == 0) {
    // reset time and pause the world
    gazebo::msgs::WorldControl start;
    start.set_pause(true);
    start.set_reset_time(true);
    logger->msglog(">>", "~/world_control", start);
    worldPub->Publish(start);
  }
}

int main(int argc, char **argv)
{
    Gtk::Main main(argc,argv);
    SceneGUI scene;

    main.run(*(scene.window));
    return 0;
}
