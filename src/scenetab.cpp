#include <gtkmm.h>
#include <gdk/gdk.h>

#include "scenetab.h"

using namespace SceneReconstruction;

/** @class SceneTab "scenetab.h"
 *  Base tab class that needs to be extended
 *  @author Bastian Klingen
 */

SceneTab::SceneTab(Glib::RefPtr<Gtk::Builder> &builder) {
  _builder = builder;
}

SceneTab::~SceneTab() {
}

