#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>

#include "transport/Transport.hh"
#include "transport/TransportTypes.hh"

#include "scenegui.h"
#include "controltab.h"
#include "robotcontrollertab.h"
#include "modeltab.h"
#include "objectinstantiatortab.h"

using namespace SceneReconstruction;

/** @class SceneGui "scenegui.h"
 * Main Class that creates the GUI
 * @author Bastian Klingen
 */

SceneGUI::SceneGUI()
{
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

  // Create all tabs
  ControlTab*            tab1 = new ControlTab(node, logger, ui_builder);
  ModelTab*              tab2 = new ModelTab(node, logger, ui_builder);
  RobotControllerTab*    tab3 = new RobotControllerTab(node, logger, ui_builder);
  ObjectInstantiatorTab* tab4 = new ObjectInstantiatorTab(node, logger, ui_builder);

  vec_tabs.push_back(tab1);
  vec_tabs.push_back(tab2);
  vec_tabs.push_back(tab3);
  vec_tabs.push_back(tab4);
  vec_tabs.push_back(logger);

  window->show_all_children();
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

    main.run(*(scene.window));
    return 0;
}
