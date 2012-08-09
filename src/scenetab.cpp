#include <gtkmm.h>
#include <gdk/gdk.h>

#include "scenetab.h"

using namespace SceneReconstruction;

SceneTab::SceneTab(Glib::ustring text) {
  label = new Gtk::Label(text);
}

SceneTab::~SceneTab() {
}

Gtk::Label& SceneTab::get_label() {
  return *label;
}

Gtk::Widget& SceneTab::get_tab() {
  return *label;
}
