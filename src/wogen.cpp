#include "wogen.h"
#include <iostream>
#include <fstream>
#include <math.h>

using namespace SceneReconstruction;

/** @class SceneGui "mapgen.h"
 *  Class that creates the WorldfileGenerator GUI for Worldfile generation.
 *  It loads an image and allows the user to create walls according to that image. Additionally
 *  it helps in setting up the objectinstantiator and allows one to include the robot file.
 *  @author Bastian Klingen
 */

WorldfileGenerator::WorldfileGenerator()
{
  // Setup the GUI
  ui_builder = Gtk::Builder::create_from_file("res/wogen.glade");
  ui_builder->get_widget("window", window);

  ui_builder->get_widget("filechooserbutton_image", fcb_image);
  filter_image = Gtk::FileFilter::create();
  filter_image->set_name("Image Files (for GdkPixBuf)");
  filter_image->add_pixbuf_formats();
  fcb_image->add_filter(filter_image);
  fcb_image->set_filter(filter_image);
  ui_builder->get_widget("entry_objectinstantiator_offset_x", ent_offsetx);
  ui_builder->get_widget("entry_objectinstantiator_offset_y", ent_offsety);
  ui_builder->get_widget("entry_objectinstantiator_offset_z", ent_offsetz);
  ui_builder->get_widget("button_objectinstantiator_add_object", btn_addobject);
  btn_addobject->signal_clicked().connect(sigc::mem_fun(*this,&WorldfileGenerator::on_add_object_clicked));
  ui_builder->get_widget("button_objectinstantiator_remove_object", btn_removeobject);
  btn_removeobject->signal_clicked().connect(sigc::mem_fun(*this,&WorldfileGenerator::on_remove_object_clicked));
  ui_builder->get_widget("treeview_objectinstantiator", trv_objects);
  lst_objects = Glib::RefPtr<Gtk::ListStore>::cast_dynamic(ui_builder->get_object("liststore_objectinstantiator_objects"));

  ui_builder->get_widget("dialog_add_object", dia_addobject);
  ui_builder->get_widget("dialog_entry_modelname", ent_modelname);
  ent_modelname->signal_key_release_event().connect(sigc::mem_fun(*this,&WorldfileGenerator::on_dialog_name_changed));
  ui_builder->get_widget("dialog_entry_interfacename", ent_interfacename);
  ui_builder->get_widget("dialog_filechooserbutton_model_file", fcb_model);
  fcb_model->signal_file_set().connect(sigc::mem_fun(*this,&WorldfileGenerator::on_dialog_file_set));
  buf_preview = Glib::RefPtr<Gtk::TextBuffer>::cast_dynamic(ui_builder->get_object("dialog_textbuffer_model_file"));

  ui_builder->get_widget("filechooserbutton_robot", fcb_robot);

  fcd_world = new Gtk::FileChooserDialog("Save World", Gtk::FILE_CHOOSER_ACTION_SAVE);
  fcd_world->add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
  fcd_world->add_button(Gtk::Stock::SAVE, Gtk::RESPONSE_OK);
  filter_world = Gtk::FileFilter::create();
  filter_world->set_name("Gazebo World (.world)");
  filter_world->add_pattern("*.world");
  fcd_world->add_filter(filter_world);
  fcd_world->set_filter(filter_world);
  fcd_world->set_do_overwrite_confirmation();

  ui_builder->get_widget("spinbutton_settings_resolution", spn_resolution);
  ui_builder->get_widget("spinbutton_settings_height", spn_height);
  ui_builder->get_widget("colorbutton_settings_color", clb_color);

  ui_builder->get_widget("button_make_world", btn_makeworld);
  btn_makeworld->signal_clicked().connect(sigc::mem_fun(*this,&WorldfileGenerator::on_make_world_clicked));

  ui_builder->get_widget("dialog_save", dia_map);
  ui_builder->get_widget("dialog_save_image", img_map);
  ui_builder->get_widget("dialog_save_eventbox", evt_map);
  evt_map->set_events(Gdk::BUTTON_MOTION_MASK | Gdk::BUTTON_PRESS_MASK | Gdk::BUTTON_RELEASE_MASK);
  evt_map->signal_button_press_event().connect(sigc::mem_fun(*this,&WorldfileGenerator::on_map_button_press));
  evt_map->signal_button_release_event().connect(sigc::mem_fun(*this,&WorldfileGenerator::on_map_button_release));
  evt_map->signal_motion_notify_event().connect(sigc::mem_fun(*this,&WorldfileGenerator::on_map_motion_notify));
  img_map->signal_draw().connect(sigc::mem_fun(*this,&WorldfileGenerator::on_map_draw));
  ui_builder->get_widget("dialog_save_spinbutton", spn_map);
  ui_builder->get_widget("dialog_save_toolbutton_erase", btn_erase);
  btn_erase->signal_clicked().connect(sigc::mem_fun(*this,&WorldfileGenerator::on_map_erase_clicked));
  ui_builder->get_widget("dialog_save_treeview", trv_map);
  lst_map = Glib::RefPtr<Gtk::ListStore>::cast_dynamic(ui_builder->get_object("dialog_save_liststore"));


  Glib::RefPtr<Gtk::CellRendererText> crt_x1 = Glib::RefPtr<Gtk::CellRendererText>::cast_dynamic(ui_builder->get_object("dialog_save_cellrenderertext_x1"));
  crt_x1->property_editable() = true;
  crt_x1->signal_edited().connect(sigc::mem_fun(*this,&WorldfileGenerator::on_map_x1_edited));
  Glib::RefPtr<Gtk::TreeViewColumn> tvc_x1 = Glib::RefPtr<Gtk::TreeViewColumn>::cast_dynamic(ui_builder->get_object("dialog_save_treeviewcolumn_x1"));
  std::vector<Gtk::CellRenderer*> col_x1 = trv_map->get_column(0)->get_cells();
  for(unsigned int i=0; i<col_x1.size(); i++) {
    tvc_x1->set_cell_data_func(*dynamic_cast<Gtk::CellRendererText*>(col_x1[i]), sigc::mem_fun(*this, &WorldfileGenerator::treeviewcolumn_x1_cell_data) );
  }

  Glib::RefPtr<Gtk::CellRendererText> crt_y1 = Glib::RefPtr<Gtk::CellRendererText>::cast_dynamic(ui_builder->get_object("dialog_save_cellrenderertext_y1"));
  crt_y1->property_editable() = true;
  crt_y1->signal_edited().connect(sigc::mem_fun(*this,&WorldfileGenerator::on_map_y1_edited));
  Glib::RefPtr<Gtk::TreeViewColumn> tvc_y1 = Glib::RefPtr<Gtk::TreeViewColumn>::cast_dynamic(ui_builder->get_object("dialog_save_treeviewcolumn_y1"));
  std::vector<Gtk::CellRenderer*> col_y1 = trv_map->get_column(1)->get_cells();
  for(unsigned int i=0; i<col_y1.size(); i++) {
    tvc_y1->set_cell_data_func(*dynamic_cast<Gtk::CellRendererText*>(col_y1[i]), sigc::mem_fun(*this, &WorldfileGenerator::treeviewcolumn_y1_cell_data) );
  }

  Glib::RefPtr<Gtk::CellRendererText> crt_x2 = Glib::RefPtr<Gtk::CellRendererText>::cast_dynamic(ui_builder->get_object("dialog_save_cellrenderertext_x2"));
  crt_x2->property_editable() = true;
  crt_x2->signal_edited().connect(sigc::mem_fun(*this,&WorldfileGenerator::on_map_x2_edited));
  Glib::RefPtr<Gtk::TreeViewColumn> tvc_x2 = Glib::RefPtr<Gtk::TreeViewColumn>::cast_dynamic(ui_builder->get_object("dialog_save_treeviewcolumn_x2"));
  std::vector<Gtk::CellRenderer*> col_x2 = trv_map->get_column(3)->get_cells();
  for(unsigned int i=0; i<col_x2.size(); i++) {
    tvc_x2->set_cell_data_func(*dynamic_cast<Gtk::CellRendererText*>(col_x2[i]), sigc::mem_fun(*this, &WorldfileGenerator::treeviewcolumn_x2_cell_data) );
  }

  Glib::RefPtr<Gtk::CellRendererText> crt_y2 = Glib::RefPtr<Gtk::CellRendererText>::cast_dynamic(ui_builder->get_object("dialog_save_cellrenderertext_y2"));
  crt_y2->property_editable() = true;
  crt_y2->signal_edited().connect(sigc::mem_fun(*this,&WorldfileGenerator::on_map_y2_edited));
  Glib::RefPtr<Gtk::TreeViewColumn> tvc_y2 = Glib::RefPtr<Gtk::TreeViewColumn>::cast_dynamic(ui_builder->get_object("dialog_save_treeviewcolumn_y2"));
  std::vector<Gtk::CellRenderer*> col_y2 = trv_map->get_column(4)->get_cells();
  for(unsigned int i=0; i<col_y2.size(); i++) {
    tvc_y2->set_cell_data_func(*dynamic_cast<Gtk::CellRendererText*>(col_y2[i]), sigc::mem_fun(*this, &WorldfileGenerator::treeviewcolumn_y2_cell_data) );
  }

  Glib::RefPtr<Gtk::CellRendererText> crt_width = Glib::RefPtr<Gtk::CellRendererText>::cast_dynamic(ui_builder->get_object("dialog_save_cellrenderertext_width"));
  crt_width->property_editable() = true;
  crt_width->signal_edited().connect(sigc::mem_fun(*this,&WorldfileGenerator::on_map_width_edited));
  Glib::RefPtr<Gtk::TreeViewColumn> tvc_width = Glib::RefPtr<Gtk::TreeViewColumn>::cast_dynamic(ui_builder->get_object("dialog_save_treeviewcolumn_width"));
  std::vector<Gtk::CellRenderer*> col_width = trv_map->get_column(6)->get_cells();
  for(unsigned int i=0; i<col_width.size(); i++) {
    tvc_width->set_cell_data_func(*dynamic_cast<Gtk::CellRendererText*>(col_width[i]), sigc::mem_fun(*this, &WorldfileGenerator::treeviewcolumn_width_cell_data) );
  }

}

WorldfileGenerator::~WorldfileGenerator() {
}

void WorldfileGenerator::on_add_object_clicked() {
  dia_addobject->set_transient_for(*window);
  int result = dia_addobject->run();
  if(result == Gtk::RESPONSE_APPLY) {
    // add object to liststore
    Gtk::TreeModel::Row row;
    row = *(lst_objects->append());
    row.set_value(0, ent_modelname->get_text());    
    row.set_value(1, ent_interfacename->get_text());    
    row.set_value(2, buf_preview->get_text());    

    std::string clonesdf = buf_preview->get_text();
    size_t model_pos = clonesdf.find("<model");
    if(model_pos != std::string::npos) {
      size_t name_start_pos = clonesdf.find("name=\"", model_pos)+6;
      size_t name_end_pos = clonesdf.find("\"", name_start_pos);
      clonesdf.insert(name_end_pos, "_clone");
    }

    row.set_value(3, clonesdf);    
  }

  ent_modelname->set_text("");
  ent_interfacename->set_text("");
  fcb_model->unselect_all();
  buf_preview->set_text("");

  dia_addobject->hide();
}

void WorldfileGenerator::on_map_erase_clicked() {
  if(trv_map->get_selection()->count_selected_rows() == 1) {
    lst_map->erase(trv_map->get_selection()->get_selected());
  }
  img_map->queue_draw();
}

void WorldfileGenerator::on_remove_object_clicked() {
  Glib::RefPtr<Gtk::TreeView::Selection> sel = trv_objects->get_selection();
  if(sel->count_selected_rows() == 1)
    lst_objects->erase(sel->get_selected());
}

bool WorldfileGenerator::on_dialog_name_changed(GdkEventKey* /*event*/) {
  // replace name in textbuffer by new modelname if file already loaded
  std::string modelsdf = buf_preview->get_text();
  size_t model_pos = modelsdf.find("<model");
  if(model_pos != std::string::npos) {
    size_t name_start_pos = modelsdf.find("name=\"", model_pos)+6;
    size_t name_end_pos = modelsdf.find("\"", name_start_pos);
    buf_preview->set_text(modelsdf.replace(name_start_pos, name_end_pos-name_start_pos, ent_modelname->get_text()));
  }

  return false;
}

void WorldfileGenerator::on_dialog_file_set() {  
  // load file to textbuffer and replace name by modelname
  std::string filename = fcb_model->get_filename();
  std::string modelsdf = "";
  if (filename != "") {      
    FILE *f = fopen(filename.c_str(), "r");
    while (! feof(f)) {
      char tmp[4096];
      size_t s;
      if ((s = fread(tmp, 1, 4096, f)) > 0) {
       	modelsdf.append(tmp, s);
      }
    }
    fclose(f);

    size_t model_pos = modelsdf.find("<model");
    size_t name_start_pos = modelsdf.find("name=\"", model_pos)+6;
    size_t name_end_pos = modelsdf.find("\"", name_start_pos);
    buf_preview->set_text(modelsdf.replace(name_start_pos, name_end_pos-name_start_pos, ent_modelname->get_text()));
  }
}

void WorldfileGenerator::on_make_world_clicked() {
  dia_map->set_transient_for(*window);
  // check and load image file
  if(fcb_image->get_filename() != "") {
    Glib::RefPtr<Gdk::Pixbuf> image = Gdk::Pixbuf::create_from_file(fcb_image->get_filename());
    img_map->set(image);
    img_map->set_size_request(image->get_width(), image->get_height());
    int result = dia_map->run();
    if(result == Gtk::RESPONSE_APPLY) {
      fcd_world->set_transient_for(*window);
      int result = fcd_world->run();
      if(result == Gtk::RESPONSE_OK) {
        if(fcd_world->get_filename() != "") {
          std::ofstream worldfile;
          worldfile.open(fcd_world->get_filename().c_str(), std::ios_base::out | std::ios_base::trunc);
          if(!worldfile.fail()) {
            worldfile << "<gazebo version=\"1.2\">\n";
            worldfile << "    <world name=\"reconstruction_world\">\n";
            worldfile << "\n";
            worldfile << "        <!-- setup the physics to have a realtime factor of ~1 -->\n";
            worldfile << "        <physics type=\"ode\">\n";
            worldfile << "            <ode>\n";
            worldfile << "                <solver>\n";
            worldfile << "                    <type>world</type>\n";
            worldfile << "                    <dt>0.01</dt>\n";
            worldfile << "                    <iters>100</iters>\n";
            worldfile << "                    <sor>1.3</sor>\n";
            worldfile << "                </solver>\n";
            worldfile << "            </ode>\n";
            worldfile << "            <gravity>0 0 -9.81</gravity>\n";
            worldfile << "            <update_rate>100</update_rate>\n";
            worldfile << "        </physics>\n";
            worldfile << "\n";
            worldfile << "        <!-- setup the scene -->\n";
            worldfile << "        <scene>\n";
            worldfile << "            <ambient>0.9 0.9 0.9 1.0</ambient>\n";
            worldfile << "            <background>0.5 0.5 0.5 1.0</background>\n";
            worldfile << "            <shadows>true</shadows>\n";
            worldfile << "            <fog>\n";
            worldfile << "                <color>0.2 0.2 0.2 0.1</color>\n";
            worldfile << "                <type>linear</type>\n";
            worldfile << "                <start>0.0</start>\n";
            worldfile << "                <end>500.0</end>\n";
            worldfile << "                <density>1</density>\n";
            worldfile << "            </fog>\n";
            worldfile << "        </scene>\n";
            worldfile << "\n";
            worldfile << "        <!-- a simple ground plane -->\n";
            worldfile << "        <model name=\"groundplane\">\n";
            worldfile << "            <link name=\"plane\">\n";
            worldfile << "                <visual name=\"plane\">\n";
            worldfile << "                    <geometry>\n";
            worldfile << "                        <plane>\n";
            worldfile << "                            <normal>0 0 1</normal>\n";
            worldfile << "                            <size>" << image->get_width()*spn_resolution->get_value() << " " << image->get_height()*spn_resolution->get_value() << "</size>\n";
            worldfile << "                        </plane>\n";
            worldfile << "                    </geometry>\n";
            worldfile << "                    <material>\n";
            worldfile << "                        <ambient>" << clb_color->get_rgba().get_red() << " " << clb_color->get_rgba().get_green() << " " << clb_color->get_rgba().get_blue() << " 1</ambient>\n";
            worldfile << "                    </material>\n";
            worldfile << "                </visual>\n";
            worldfile << "                <collision name=\"plane\">\n";
            worldfile << "                    <geometry>\n";
            worldfile << "                        <plane>\n";
            worldfile << "                            <normal>0 0 1</normal>\n";
            worldfile << "                            <size>" << image->get_width()*spn_resolution->get_value() << " " << image->get_height()*spn_resolution->get_value() << "</size>\n";
            worldfile << "                        </plane>\n";
            worldfile << "                    </geometry>\n";
            worldfile << "                </collision>\n";
            worldfile << "            </link>\n";
            worldfile << "            <link name=\"walls\">\n";
            // generate world sdf from image

            Gtk::TreeModel::Children rows = trv_map->get_model()->children();
            int i = 0;
            for(Gtk::TreeModel::Children::iterator l = rows.begin(); l != rows.end(); l++) {
              double _x1, _y1, _x2, _y2, _width;
              l->get_value(0, _x1);
              l->get_value(1, _y1);
              l->get_value(2, _x2);
              l->get_value(3, _y2);
              l->get_value(4, _width);

              double x, y, z, rot, width, length, height;
              height = spn_height->get_value();

              x = (image->get_width()/2.0 - (_x1 + _x2)/2.0)*spn_resolution->get_value()*-1.0;
              y = (image->get_height()/2.0 - (_y1 + _y2)/2.0)*spn_resolution->get_value();
              z = height/2.0;

              width = _width*spn_resolution->get_value();
              length = sqrt((_x1 - _x2)*(_x1 - _x2) + (_y1 - _y2)*(_y1 - _y2))*spn_resolution->get_value();
              rot = atan2((_x2-_x1), (_y2-_y1));
              
              worldfile << "                <visual name=\"wall_" << i << "\">\n";
              worldfile << "                    <pose>" << x << " " << y << " " << z << " 0 0 " << rot << "</pose>\n";
              worldfile << "                    <geometry>\n";
              worldfile << "                        <box>\n";
              worldfile << "                            <size>" << width << " " << length << " " << height << "</size>\n";
              worldfile << "                        </box>\n";
              worldfile << "                    </geometry>\n";
              worldfile << "                    <material>\n";
              worldfile << "                        <ambient>" << clb_color->get_rgba().get_red() << " " << clb_color->get_rgba().get_green() << " " << clb_color->get_rgba().get_blue() << " 1</ambient>\n";
              worldfile << "                    </material>\n";
              worldfile << "                </visual>\n";
              worldfile << "                <collision name=\"wall_" << i << "\">\n";
              worldfile << "                    <pose>" << x << " " << y << " " << z << " 0 0 " << rot << "</pose>\n";
              worldfile << "                    <geometry>\n";
              worldfile << "                        <box>\n";
              worldfile << "                            <size>" << width << " " << length << " " << height << "</size>\n";
              worldfile << "                        </box>\n";
              worldfile << "                    </geometry>\n";
              worldfile << "                </collision>\n";
              i++;
            }

            worldfile << "            </link>\n";
            worldfile << "            <static>true</static>\n";
            worldfile << "        </model>\n";
            worldfile << "\n";
            worldfile << "        <!-- models for the object repository -->\n";

            // generate object sdf from lst_objects
            rows = trv_objects->get_model()->children();
            for(Gtk::TreeModel::Children::iterator l = rows.begin(); l != rows.end(); l++) {
              std::string sdf;
              // add model
              l->get_value(2, sdf);
              worldfile << "\n";
              worldfile << sdf;
              // add clone
              l->get_value(3, sdf);
              worldfile << "\n";
              worldfile << sdf;
            }

            // add and configure objectinstantiatorplugin
            worldfile << "\n";
            worldfile << "        <!-- the objectinstantiator plugin -->\n";
            worldfile << "        <plugin name=\"ObjectInstantiatorPlugin\" filename=\"libObjectInstantiatorPlugin.so\">\n";
            worldfile << "\n";
            worldfile << "            <!-- Position Offset to adjust different map origin -->\n";
            worldfile << "            <settings_position_x_offset>" << ent_offsetx->get_text() << "</settings_position_x_offset>\n";
            worldfile << "            <settings_position_y_offset>" << ent_offsety->get_text() << "</settings_position_y_offset>\n";
            worldfile << "            <settings_position_z_offset>" << ent_offsetz->get_text() << "</settings_position_z_offset>\n";

            i = 0;
            for(Gtk::TreeModel::Children::iterator l = rows.begin(); l != rows.end(); l++) {
              std::string modelname, interface;
              l->get_value(0, modelname);
              l->get_value(1, interface);
              worldfile << "\n";
              worldfile << "            <!-- object " << i << ": " << modelname << " -->\n";
              worldfile << "            <object>" << i << "</object>\n";
              worldfile << "            <name_" << i << ">" << interface << "</name_" << i << ">\n";
              worldfile << "            <model_" << i << ">" << modelname << "</model_" << i << ">\n";
              i++;
            }
            worldfile << "        </plugin>\n";

            // check and include robot file
            if(fcb_robot->get_filename() != "") {
              worldfile << "\n";
              worldfile << "        <!-- the robot -->\n";
              worldfile << "        <include filename=\"" << fcb_robot->get_filename() << "\"></include>\n";
            }

            worldfile << "    </world>\n";
            worldfile << "</gazebo>\n";
          }
          else {
            Gtk::MessageDialog md(*window, "Could not open file: "+fcd_world->get_filename(),
		            /* markup */ false, Gtk::MESSAGE_ERROR,
		            Gtk::BUTTONS_OK, /* modal */ true);
            md.set_title("Could not open file");
            md.run();
          }
        }
        else {
          Gtk::MessageDialog md(*window, "Invalid file name",
		          /* markup */ false, Gtk::MESSAGE_ERROR,
		          Gtk::BUTTONS_OK, /* modal */ true);
          md.set_title("Invalid File Name");
          md.run();
        }
      }

      fcd_world->unselect_all();
      fcd_world->hide();
    }
    dia_map->hide();
  }
  else {
    Gtk::MessageDialog md(*window, "Invalid image file",
		    /* markup */ false, Gtk::MESSAGE_ERROR,
		    Gtk::BUTTONS_OK, /* modal */ true);
    md.set_title("Invalid Image File");
    md.run();
  }
}

bool WorldfileGenerator::on_map_button_press(GdkEventButton* b) {
  // start line
  if(b->button == 1) {
    new_line = new Line();
    new_line->p1.x  = b->x;
    new_line->p1.y  = b->y;
    new_line->width = spn_map->get_value();
  }

  return false;
}

bool WorldfileGenerator::on_map_button_release(GdkEventButton* b) {
  // end line
  if(b->button == 1) {
    new_line->p2.x  = b->x;
    new_line->p2.y  = b->y;

    Gtk::TreeModel::Row row;
    row = *(lst_map->append());
    row.set_value(0, new_line->p1.x);
    row.set_value(1, new_line->p1.y);
    row.set_value(2, new_line->p2.x);
    row.set_value(3, new_line->p2.y);
    row.set_value(4, new_line->width);

    new_line = 0;
  }

  img_map->queue_draw();
  return false;
}

bool WorldfileGenerator::on_map_motion_notify(GdkEventMotion* m) {
  // move line end
  if(new_line) {
    new_line->p2.x  = m->x;
    new_line->p2.y  = m->y;
  }

  img_map->queue_draw();
  return false;
}

bool WorldfileGenerator::on_map_draw(Cairo::RefPtr<Cairo::Context> cr) {
  // draw all lines
  cr->save();

  Gtk::TreeModel::Children rows = trv_map->get_model()->children();
  for(Gtk::TreeModel::Children::iterator l = rows.begin(); l != rows.end(); l++) {
    double x1, y1, x2, y2, width;
    l->get_value(0, x1);
    l->get_value(1, y1);
    l->get_value(2, x2);
    l->get_value(3, y2);
    l->get_value(4, width);

    cr->set_line_width(width);
    cr->set_source_rgb(1.0, 0.0, 0.0);
    cr->move_to(x1, y1);
    cr->line_to(x2, y2);
    cr->stroke();
  }

  // draw temporary line
  if(new_line) {
    cr->set_line_width(new_line->width);
    cr->set_source_rgb(1.0, 0.0, 0.0);
    cr->move_to(new_line->p1.x, new_line->p1.y);
    cr->line_to(new_line->p2.x, new_line->p2.y);
    cr->stroke();
  }

  cr->restore();
  return true;
}

void WorldfileGenerator::on_map_x1_edited(const Glib::ustring &path, const Glib::ustring &new_text) {
  Gtk::TreeIter it = trv_map->get_model()->get_iter(path);
  double value = ustring_to_double(new_text);
  it->set_value(0, value); 
  img_map->queue_draw();
}

void WorldfileGenerator::on_map_y1_edited(const Glib::ustring &path, const Glib::ustring &new_text) {
  Gtk::TreeIter it = trv_map->get_model()->get_iter(path);
  double value = ustring_to_double(new_text);
  it->set_value(1, value); 
  img_map->queue_draw();
}

void WorldfileGenerator::on_map_x2_edited(const Glib::ustring &path, const Glib::ustring &new_text) {
  Gtk::TreeIter it = trv_map->get_model()->get_iter(path);
  double value = ustring_to_double(new_text);
  it->set_value(2, value); 
  img_map->queue_draw();
}

void WorldfileGenerator::on_map_y2_edited(const Glib::ustring &path, const Glib::ustring &new_text) {
  Gtk::TreeIter it = trv_map->get_model()->get_iter(path);
  double value = ustring_to_double(new_text);
  it->set_value(3, value); 
  img_map->queue_draw();
}

void WorldfileGenerator::on_map_width_edited(const Glib::ustring &path, const Glib::ustring &new_text) {
  Gtk::TreeIter it = trv_map->get_model()->get_iter(path);
  double value = ustring_to_double(new_text);
  it->set_value(4, value); 
  img_map->queue_draw();
}

double WorldfileGenerator::ustring_to_double(Glib::ustring val) {
  char *ret;
  double def = strtod(val.c_str(), &ret);
  while(*ret != 0) {
    size_t pos = val.find(*ret);
    if(*ret == ',')
      val = val.replace(pos, 1, ".");
    else if(*ret == '.')
      val = val.replace(pos, 1, ",");
    else
      break;

    def = strtod(val.c_str(), &ret);
  }
  return def;
}

void WorldfileGenerator::treeviewcolumn_x1_cell_data(Gtk::CellRenderer* renderer, const Gtk::TreeModel::iterator& iter) {
  if(iter)
  {
    double value;
    iter->get_value(0, value);

    char buffer[32];
    sprintf(buffer, "%.2f", value); 

    Glib::ustring text = buffer;
    dynamic_cast<Gtk::CellRendererText*>(renderer)->property_text() = text;
  }
}

void WorldfileGenerator::treeviewcolumn_y1_cell_data(Gtk::CellRenderer* renderer, const Gtk::TreeModel::iterator& iter) {
  if(iter)
  {
    double value;
    iter->get_value(1, value);

    char buffer[32];
    sprintf(buffer, "%.2f", value); 

    Glib::ustring text = buffer;
    dynamic_cast<Gtk::CellRendererText*>(renderer)->property_text() = text;
  }
}

void WorldfileGenerator::treeviewcolumn_x2_cell_data(Gtk::CellRenderer* renderer, const Gtk::TreeModel::iterator& iter) {
  if(iter)
  {
    double value;
    iter->get_value(2, value);

    char buffer[32];
    sprintf(buffer, "%.2f", value); 

    Glib::ustring text = buffer;
    dynamic_cast<Gtk::CellRendererText*>(renderer)->property_text() = text;
  }
}

void WorldfileGenerator::treeviewcolumn_y2_cell_data(Gtk::CellRenderer* renderer, const Gtk::TreeModel::iterator& iter) {
  if(iter)
  {
    double value;
    iter->get_value(3, value);

    char buffer[32];
    sprintf(buffer, "%.2f", value); 

    Glib::ustring text = buffer;
    dynamic_cast<Gtk::CellRendererText*>(renderer)->property_text() = text;
  }
}

void WorldfileGenerator::treeviewcolumn_width_cell_data(Gtk::CellRenderer* renderer, const Gtk::TreeModel::iterator& iter) {
  if(iter)
  {
    double value;
    iter->get_value(4, value);

    char buffer[32];
    sprintf(buffer, "%.2f", value); 

    Glib::ustring text = buffer;
    dynamic_cast<Gtk::CellRendererText*>(renderer)->property_text() = text;
  }
}

int main(int argc, char **argv)
{
    Gtk::Main main(argc,argv);
    WorldfileGenerator mapgen;

    main.run(*(mapgen.window));
    return 0;
}
