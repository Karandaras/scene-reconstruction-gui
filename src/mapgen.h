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
      Glib::RefPtr<Gtk::FileFilter>    filter_image;
      Gtk::Entry                      *ent_offsetx;
      Gtk::Entry                      *ent_offsety;
      Gtk::Entry                      *ent_offsetz;
      Gtk::Button                     *btn_addobject;
      Gtk::Button                     *btn_removeobject;
      Gtk::TreeView                   *trv_objects;
      Glib::RefPtr<Gtk::ListStore>     lst_objects;

      Gtk::Dialog                     *dia_addobject;
      Gtk::Entry                      *ent_modelname;
      Gtk::Entry                      *ent_interfacename;
      Gtk::FileChooserButton          *fcb_model;
      Glib::RefPtr<Gtk::TextBuffer>    buf_preview;

      Gtk::FileChooserButton          *fcb_robot;

      Gtk::SpinButton                 *spn_resolution;
      Gtk::SpinButton                 *spn_height;
      Gtk::ColorButton                *clb_color;

      Gtk::FileChooserDialog          *fcd_world;
      Glib::RefPtr<Gtk::FileFilter>    filter_world;
      Gtk::Button                     *btn_makeworld;

      Gtk::Dialog                     *dia_map;
      Gtk::Image                      *img_map;
      Gtk::EventBox                   *evt_map;
      Gtk::TreeView                   *trv_map;
      Glib::RefPtr<Gtk::ListStore>     lst_map;
      Gtk::SpinButton                 *spn_map;
      Gtk::ToolButton                 *btn_erase;

      struct Point {
          double x, y;
      };
      struct Line {
          Point p1, p2;
          double width;
      };

      Line                            *new_line;

    private:
      void on_add_object_clicked();
      void on_map_erase_clicked();
      void on_remove_object_clicked();
      bool on_dialog_name_changed(GdkEventKey*);
      void on_dialog_file_set();
      void on_make_world_clicked();
      bool on_map_button_press(GdkEventButton*);
      bool on_map_button_release(GdkEventButton*);
      bool on_map_motion_notify(GdkEventMotion*);
      bool on_map_draw(Cairo::RefPtr<Cairo::Context>);
      void on_map_x1_edited(const Glib::ustring&, const Glib::ustring&);
      void on_map_y1_edited(const Glib::ustring&, const Glib::ustring&);
      void on_map_x2_edited(const Glib::ustring&, const Glib::ustring&);
      void on_map_y2_edited(const Glib::ustring&, const Glib::ustring&);
      void on_map_width_edited(const Glib::ustring&, const Glib::ustring&);
      double ustring_to_double(Glib::ustring);
      void treeviewcolumn_x1_cell_data(Gtk::CellRenderer*, const Gtk::TreeModel::iterator&);
      void treeviewcolumn_y1_cell_data(Gtk::CellRenderer*, const Gtk::TreeModel::iterator&);
      void treeviewcolumn_x2_cell_data(Gtk::CellRenderer*, const Gtk::TreeModel::iterator&);
      void treeviewcolumn_y2_cell_data(Gtk::CellRenderer*, const Gtk::TreeModel::iterator&);
      void treeviewcolumn_width_cell_data(Gtk::CellRenderer*, const Gtk::TreeModel::iterator&);

    public:
      /** Gtk::Window for the MapGenerator */
      Gtk::Window                     *window;
  };
}
