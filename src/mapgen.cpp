#include "mapgen.h"
#include <iostream>

using namespace SceneReconstruction;

/** @class SceneGui "mapgen.h"
 *  Main Class that creates the MapGenerator
 *  @author Bastian Klingen
 */

MapGenerator::MapGenerator()
{
  // Setup the GUI
  ui_builder = Gtk::Builder::create_from_file("res/mapgen.glade");
  ui_builder->get_widget("window", window);

  ui_builder->get_widget("filechooserbutton_image", fcb_image);
  ui_builder->get_widget("entry_objectinstantiator_offset_x", ent_offsetx);
  ui_builder->get_widget("entry_objectinstantiator_offset_y", ent_offsety);
  ui_builder->get_widget("entry_objectinstantiator_offset_z", ent_offsetz);
  ui_builder->get_widget("button_objectinstantiator_add_object", btn_addobject);
  btn_addobject->signal_clicked().connect(sigc::mem_fun(*this,&MapGenerator::on_add_object_clicked));
  ui_builder->get_widget("button_objectinstantiator_remove_object", btn_removeobject);
  btn_removeobject->signal_clicked().connect(sigc::mem_fun(*this,&MapGenerator::on_remove_object_clicked));
  ui_builder->get_widget("treeview_objectinstantiator", trv_objects);
  lst_objects = Glib::RefPtr<Gtk::ListStore>::cast_dynamic(ui_builder->get_object("liststore_objectinstantiator_objects"));

  ui_builder->get_widget("dialog_add_object", dia_addobject);
  ui_builder->get_widget("dialog_button_cancel", btn_diacancel);
//  btn_diacancle->signal_clicked().connect(sigc::mem_fun(*this,&MapGenerator::on_dialog_cancel_clicked));
  ui_builder->get_widget("dialog_button_add", btn_diaadd);
//  btn_diaadd->signal_clicked().connect(sigc::mem_fun(*this,&MapGenerator::on_dialog_add_clicked));
  ui_builder->get_widget("dialog_entry_modelname", ent_modelname);
  ent_modelname->signal_key_release_event().connect(sigc::mem_fun(*this,&MapGenerator::on_dialog_name_changed));
  ui_builder->get_widget("dialog_entry_interfacename", ent_interfacename);
  ui_builder->get_widget("dialog_filechooserbutton_model_file", fcb_model);
  fcb_model->signal_file_set().connect(sigc::mem_fun(*this,&MapGenerator::on_dialog_file_set));
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

  ui_builder->get_widget("button_make_world", btn_makeworld);
  btn_makeworld->signal_clicked().connect(sigc::mem_fun(*this,&MapGenerator::on_make_world_clicked));

}

MapGenerator::~MapGenerator() {
}

void MapGenerator::on_add_object_clicked() {
  dia_addobject->set_transient_for(*window);
  int result = dia_addobject->run();
  if(result == Gtk::RESPONSE_APPLY) {
    // TODO: add object to liststore
    Gtk::TreeModel::Row row;
    row = *(lst_objects->append());
    row.set_value(0, ent_modelname->get_text());    
    row.set_value(1, ent_interfacename->get_text());    
    row.set_value(2, buf_preview->get_text());    
  }

  ent_modelname->set_text("");
  ent_interfacename->set_text("");
  fcb_model->unselect_all();
  buf_preview->set_text("");

  dia_addobject->hide();
}

void MapGenerator::on_remove_object_clicked() {
  Glib::RefPtr<Gtk::TreeView::Selection> sel = trv_objects->get_selection();
  if(sel->count_selected_rows() == 1)
    lst_objects->erase(sel->get_selected());
}

bool MapGenerator::on_dialog_name_changed(GdkEventKey* /*event*/) {
  // TODO: replace name in textbuffer by new modelname if file already loaded
  std::string modelsdf = buf_preview->get_text();
  size_t model_pos = modelsdf.find("<model");
  if(model_pos != std::string::npos) {
    size_t name_start_pos = modelsdf.find("name=\"", model_pos)+6;
    size_t name_end_pos = modelsdf.find("\"", name_start_pos);
    buf_preview->set_text(modelsdf.replace(name_start_pos, name_end_pos-name_start_pos, ent_modelname->get_text()));
  }

  return false;
}

void MapGenerator::on_dialog_file_set() {  
  std::cout << "file set\n";
  // TODO: load file to textbuffer and replace name by modelname
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

void MapGenerator::on_make_world_clicked() {
  fcd_world->set_transient_for(*window);
  int result = fcd_world->run();
  if(result == Gtk::RESPONSE_OK) {
    // TODO: generate world file
      // TODO: check and load image file
      // TODO: generate world sdf from image
      // TODO: generate object sdf from lst_objects
      // TODO: add and configure objectinstantiatorplugin
      // TODO: check and include robot file

    fcb_image->unselect_all();
    lst_objects->clear();
    ent_offsetx->set_text("");
    ent_offsety->set_text("");
    ent_offsetz->set_text("");
    fcb_robot->unselect_all();
  }

  fcd_world->unselect_all();
  fcd_world->hide();
}


int main(int argc, char **argv)
{
    Gtk::Main main(argc,argv);
    MapGenerator mapgen;

    main.run(*(mapgen.window));
    return 0;
}
