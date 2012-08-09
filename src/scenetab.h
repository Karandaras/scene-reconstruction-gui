#pragma once
#include <gtkmm.h>
#include <gdk/gdk.h>

namespace SceneReconstruction {
  class SceneTab {
    public:
      SceneTab(Glib::ustring);
      virtual ~SceneTab();

    private:
      Gtk::Label*  label;

    public:
	    Gtk::Label&  get_label();
      virtual Gtk::Widget& get_tab();
  };
}

