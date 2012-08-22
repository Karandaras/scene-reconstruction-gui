#pragma once
#include <gtkmm.h>
#include <gdk/gdk.h>

namespace SceneReconstruction {
  /** @class SceneTab "scenetab.h"
   * Base tab class that needs to be extended
   * @author Bastian Klingen
   */
  class SceneTab {
    public:
      /** Constructor
       * @param parent SceneGUI parent
       */
      SceneTab(Glib::RefPtr<Gtk::Builder>&);
      /** Destructor */
      virtual ~SceneTab();

    public:
      /** Refrerence to the Gtk::Builder to access the necessary widgets */
      Glib::RefPtr<Gtk::Builder>     _builder;
  };
}

