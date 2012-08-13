#pragma once
#include <gtkmm.h>
#include <gdk/gdk.h>

#include "transport/Node.hh"
#include "gazebo_config.h"

#include "scenetab.h"
#include "loggertab.h"

namespace SceneReconstruction {
  /** @class SceneGUI "scenegui.h"
   * Main Class that creates the GUI
   * @author Bastian Klingen
   */
  class SceneGUI
  {
    public:
      /** Constructor */
      SceneGUI();
      /** Destructor */
      ~SceneGUI();

    private:
      // tabs for log etc.
      Gtk::Notebook                 ntb_tabs;

      // gazebo node
      gazebo::transport::NodePtr    node;

      // vector for all tabs
      std::vector<SceneTab*>        vec_tabs;
      LoggerTab*                    logger;

    public:
      /** Gtk::Window for the GUI */
      Gtk::Window                   window;
  };
}
