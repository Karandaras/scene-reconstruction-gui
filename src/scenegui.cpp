#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>

#include "transport/Transport.hh"
#include "transport/TransportTypes.hh"

#include "scenegui.h"
#include "controltab.h"
#include "robotcontrollertab.h"
#include "modeltab.h"

using namespace SceneReconstruction;

SceneGUI::SceneGUI()
{
  // Init Gazebo
  gazebo::transport::init();
  gazebo::transport::run();
  gazebo::transport::NodePtr _node(new gazebo::transport::Node());
  node = _node;
  node->Init();

  // Connect to the master
  gazebo::transport::ConnectionPtr connection(new gazebo::transport::Connection());

  std::string host;
  unsigned int port;
  if(gazebo::transport::get_master_uri(host,port))
    connection->Connect(host,port);
  else
    connection->Connect("localhost", 11345);

  // Read the verification message
  std::string data, namespacesData, publishersData;
  connection->Read(data);
  connection->Read(namespacesData);
  connection->Read(publishersData);
  connection->Shutdown();

  gazebo::msgs::Packet packet;
  packet.ParseFromString(data);
  if (packet.type() == "init")
  {
    gazebo::msgs::String msg;
    msg.ParseFromString(packet.serialized_data());
    if (msg.data() != std::string("gazebo ") + GAZEBO_VERSION)
      std::cerr << "Conflicting gazebo versions\n";
  }

  // Setup the GUI
  window.set_default_size(600,120);
  window.set_title("GUI-Tool for Scene Reconstruction");


  // Add all tabs
  logger = new LoggerTab();
  ControlTab*         tab1 = new ControlTab(node, logger);
  ModelTab*           tab2 = new ModelTab(node, logger);
  RobotControllerTab* tab3 = new RobotControllerTab(node, logger);

  vec_tabs.push_back(tab1);
  vec_tabs.push_back(tab2);
  vec_tabs.push_back(tab3);
  vec_tabs.push_back(logger);

  for(std::vector<SceneTab*>::iterator it = vec_tabs.begin(); it != vec_tabs.end(); it++) {
    ntb_tabs.append_page((*it)->get_tab(), (*it)->get_label());
  }

  window.add(ntb_tabs);
  window.show_all_children();
}

SceneGUI::~SceneGUI() {
  node->Fini();
  gazebo::transport::stop();
  gazebo::transport::fini();
}

int main(int argc, char **argv)
{
    Gtk::Main main(argc,argv);
    SceneGUI scene;
    main.run(scene.window);
    return 0;
}
