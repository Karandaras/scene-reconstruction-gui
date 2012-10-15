#pragma once
#include <gtkmm.h>
#include <gdk/gdk.h>

namespace SceneReconstruction {
  /** @class SceneGUI "scenegui.h"
   *  Main Class that creates the GUI
   *  @author Bastian Klingen
   */
  class MapGenerator
  {
    public:
      /** Constructor */
      MapGenerator();
      /** Destructor */
      ~MapGenerator();

    private:
      Glib::RefPtr<Gtk::Builder>       ui_builder;
      Gtk::FileChooserButton          *fcb_image;
      Gtk::Entry                      *ent_offsetx;
      Gtk::Entry                      *ent_offsety;
      Gtk::Entry                      *ent_offsetz;
      Gtk::Button                     *btn_addobject;
      Gtk::Button                     *btn_removeobject;
      Gtk::TreeView                   *trv_objects;
      Glib::RefPtr<Gtk::ListStore>     lst_objects;

      Gtk::Dialog                     *dia_addobject;
      Gtk::Button                     *btn_diacancel;
      Gtk::Button                     *btn_diaadd;
      Gtk::Entry                      *ent_modelname;
      Gtk::Entry                      *ent_interfacename;
      Gtk::FileChooserButton          *fcb_model;
      Glib::RefPtr<Gtk::TextBuffer>    buf_preview;

      Gtk::FileChooserButton          *fcb_robot;
      Gtk::FileChooserDialog          *fcd_world;
      Glib::RefPtr<Gtk::FileFilter>    filter_world;
      Gtk::Button                     *btn_makeworld;

    private:
      void on_add_object_clicked();
      void on_remove_object_clicked();
      bool on_dialog_name_changed(GdkEventKey*);
      void on_dialog_file_set();
      void on_make_world_clicked();

    public:
      /** Gtk::Window for the MapGenerator */
      Gtk::Window                     *window;
  };
}
