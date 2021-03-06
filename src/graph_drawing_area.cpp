
/***************************************************************************
 *  graph_drawing_area.cpp - Graph drawing area derived from Gtk::DrawingArea
 *
 *  Created: Wed Mar 18 10:40:00 2009
 *  Copyright  2008-2009  Tim Niemueller [www.niemueller.de]
 *  Edited:  Mon Sep 24 11:23:02 2012
 *  Bastian Klingen
 *
 ****************************************************************************/

/*  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Library General Public License for more details.
 *
 *  Read the full text in the LICENSE.GPL file in the doc directory.
 */

#include "graph_drawing_area.h"
#include "gvplugin_cairo.h"

#include <cmath>
#include <libgen.h>
#include <sys/time.h>

using namespace SceneReconstruction;
/** @class GraphDrawingArea "graph_drawing_area.h"
 * Graph drawing area.
 * Derived version of Gtk::DrawingArea that renders a graph via Graphviz.
 * @author Tim Niemueller
 */

/** Constructor. */
GraphDrawingArea::GraphDrawingArea()
{
  add_events(Gdk::SCROLL_MASK | Gdk::BUTTON_MOTION_MASK);

  __gvc = gvContext();

  __graph_fsm = "";
  __graph = "";

  __bbw = __bbh = __pad_x = __pad_y = 0.0;
  __translation_x = __translation_y = 0.0;
  __scale = 1.0;
  __scale_override = false;
  __update_graph = true;

  gvplugin_cairo_setup(__gvc, this);

  __fcd_save = new Gtk::FileChooserDialog("Save Graph",
					  Gtk::FILE_CHOOSER_ACTION_SAVE);
  __fcd_open = new Gtk::FileChooserDialog("Load Graph",
					  Gtk::FILE_CHOOSER_ACTION_OPEN);

  //Add response buttons the the dialog:
  __fcd_save->add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
  __fcd_save->add_button(Gtk::Stock::SAVE, Gtk::RESPONSE_OK);
  __fcd_open->add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
  __fcd_open->add_button(Gtk::Stock::SAVE, Gtk::RESPONSE_OK);

  __filter_pdf = Gtk::FileFilter::create();
  __filter_svg = Gtk::FileFilter::create();
  __filter_png = Gtk::FileFilter::create();
  __filter_dot = Gtk::FileFilter::create();
  __filter_pdf->set_name("Portable Document Format (PDF)");
  __filter_pdf->add_pattern("*.pdf");
  __filter_svg->set_name("Scalable Vector Graphic (SVG)");
  __filter_svg->add_pattern("*.svg");
  __filter_png->set_name("Portable Network Graphic (PNG)");
  __filter_png->add_pattern("*.png");
  __filter_dot->set_name("DOT Graph");
  __filter_dot->add_pattern("*.dot");
  __fcd_save->add_filter(__filter_pdf);
  __fcd_save->add_filter(__filter_svg);
  __fcd_save->add_filter(__filter_png);
  __fcd_save->add_filter(__filter_dot);
  __fcd_save->set_filter(__filter_pdf);

  __fcd_open->add_filter(__filter_dot);
  __fcd_open->set_filter(__filter_dot);

  add_events(Gdk::SCROLL_MASK | Gdk::BUTTON_MOTION_MASK |
	     Gdk::BUTTON_PRESS_MASK | Gdk::BUTTON_RELEASE_MASK );

  signal_button_press_event().connect(sigc::mem_fun(*this, &GraphDrawingArea::on_button_press_event));
  signal_motion_notify_event().connect(sigc::mem_fun(*this, &GraphDrawingArea::on_motion_notify_event));
}

GraphDrawingArea::~GraphDrawingArea()
{
  gvFreeContext(__gvc);
  //delete __fcd;
  delete __fcd_save;
  delete __fcd_open;
  __filter_pdf.reset();
  __filter_svg.reset();
  __filter_png.reset();
  __filter_dot.reset();
}


/** Get "update disabled" signal.
 * @return "update disabled" signal
 */
sigc::signal<void>
GraphDrawingArea::signal_update_disabled()
{
  return __signal_update_disabled;
}


/** Set graph's FSM name.
 * @param fsm_name name of FSM the graph belongs to
 */
void
GraphDrawingArea::set_graph_fsm(std::string fsm_name)
{
  if ( __update_graph ) {
    if ( __graph_fsm != fsm_name ) {
      __scale_override = false;
    }
    __graph_fsm = fsm_name;
  } else {
    __nonupd_graph_fsm = fsm_name;
  }
}


/** Set graph.
 * @param graph string representation of the current graph in the dot language.
 */
void
GraphDrawingArea::set_graph(std::string graph)
{
  if ( __update_graph ) {
    __graph = graph;
    queue_draw();
  } else {
    __nonupd_graph = graph;
  }
}

/** Set bounding box.
 * To be called only by the Graphviz plugin.
 * @param bbw bounding box width
 * @param bbh bounding box height
 */
void
GraphDrawingArea::set_bb(double bbw, double bbh)
{
  __bbw = bbw;
  __bbh = bbh;
}


/** Set padding.
 * To be called only by the Graphviz plugin.
 * @param pad_x padding in x
 * @param pad_y padding in y
 */
void
GraphDrawingArea::set_pad(double pad_x, double pad_y)
{
  __pad_x = pad_x;
  __pad_y = pad_y;
}


/** Get padding.
 * To be called only by the Graphviz plugin.
 * @param pad_x upon return contains padding in x
 * @param pad_y upon return contains padding in y
 */
void
GraphDrawingArea::get_pad(double &pad_x, double &pad_y)
{
  if (__scale_override) {
    pad_x = pad_y = 0;
  } else {
    pad_x = __pad_x;
    pad_y = __pad_y;
  }
}


/** Set translation.
 * To be called only by the Graphviz plugin.
 * @param tx translation in x
 * @param ty translation in y
 */
void
GraphDrawingArea::set_translation(double tx, double ty)
{
  __translation_x = tx;
  __translation_y = ty;
}


/** Set scale.
 * To be called only by the Graphviz plugin.
 * @param scale scale value
 */
void
GraphDrawingArea::set_scale(double scale)
{
  __scale = scale;
}

/** Get scale.
 * To be called only by the Graphviz plugin.
 * @return scale value
 */
double
GraphDrawingArea::get_scale()
{
  return __scale;
}

/** Get translation.
 * @param tx upon return contains translation value
 * @param ty upon return contains translation value
 */
void
GraphDrawingArea::get_translation(double &tx, double &ty)
{
  tx = __translation_x;
  ty = __translation_y;
}


/** Get dimensions
 * @param width upon return contains width
 * @param height upon return contains height
 */
void
GraphDrawingArea::get_dimensions(double &width, double &height)
{
  Gtk::Allocation alloc = get_allocation();
  width  = alloc.get_width();
  height = alloc.get_height();
}


/** Zoom in.
 * Increases zoom factor by 20, no upper limit.
 */
void
GraphDrawingArea::zoom_in()
{
  Gtk::Allocation alloc = get_allocation();
  __scale += 0.1;
  __scale_override = true;
  __translation_x = (alloc.get_width()  - __bbw * __scale) / 2.0;
  __translation_y = (alloc.get_height() - __bbh * __scale) / 2.0 + __bbh * __scale;
  queue_draw();
}

/** Zoom out.
 * Decreases zoom factor by 20 with a minimum of 1.
 */
void
GraphDrawingArea::zoom_out()
{
  __scale_override = true;
  if ( __scale > 0.1 ) {
    Gtk::Allocation alloc = get_allocation();
    __scale -= 0.1;
    __translation_x = (alloc.get_width()  - __bbw * __scale) / 2.0;
    __translation_y = (alloc.get_height() - __bbh * __scale) / 2.0 + __bbh * __scale;
    queue_draw();
  }
}


/** Zoom to fit.
 * Disables scale override and draws with values suggested by Graphviz plugin.
 */
void
GraphDrawingArea::zoom_fit()
{
  __scale_override = false;
  queue_draw();
}


/** Zoom reset.
 * Reset zoom to 1. Enables scale override.
 */
void
GraphDrawingArea::zoom_reset()
{
  Gtk::Allocation alloc = get_allocation();
  __scale = 1.0;
  __scale_override = true;
  __translation_x = (alloc.get_width()  - __bbw) / 2.0 + __pad_x;
  __translation_y = (alloc.get_height() - __bbh) / 2.0 + __bbh - __pad_y;
  queue_draw();
}


/** Check if scale override is enabled.
 * @return true if scale override is enabled, false otherwise
 */
bool
GraphDrawingArea::scale_override()
{
  return __scale_override;
}


/** Get Cairo context.
 * This is only valid during the expose event and is only meant for the
 * Graphviz plugin.
 * @return Cairo context
 */
Cairo::RefPtr<Cairo::Context>
GraphDrawingArea::get_cairo()
{
  return __cairo;
}



/** Check if graph is being updated.
 * @return true if the graph will be update if new data is received, false otherwise
 */
bool
GraphDrawingArea::get_update_graph()
{
  return __update_graph;
}


/** Set if the graph should be updated on new data.
 * @param update true to update on new data, false to disable update
 */
void
GraphDrawingArea::set_update_graph(bool update)
{
  if (update && ! __update_graph) {
    if ( __graph_fsm != __nonupd_graph_fsm ) {
      __scale_override = false;
    }
    __graph     = __nonupd_graph;
    __graph_fsm = __nonupd_graph_fsm;
    queue_draw();
  }
  __update_graph = update;
}


void
GraphDrawingArea::save_dotfile(const char *filename)
{
  FILE *f = fopen(filename, "w");
  if (f) {
    if (fwrite(__graph.c_str(), __graph.length(), 1, f) != 1) {
      // bang, ignored
      printf("Failed to write dot file '%s'\n", filename);
    }
    fclose(f);
  }
}


/** save current graph. */
void
GraphDrawingArea::save()
{
  Gtk::Window *w = dynamic_cast<Gtk::Window *>(get_toplevel());
  __fcd_save->set_transient_for(*w);

  int result = __fcd_save->run();
  if (result == Gtk::RESPONSE_OK) {

    Glib::RefPtr<Gtk::FileFilter> f = __fcd_save->get_filter();
    std::string filename = __fcd_save->get_filename();
    if (filename != "") {
      if (f == __filter_dot) {
      	save_dotfile(filename.c_str());
      } else {
      	Cairo::RefPtr<Cairo::Surface> surface;

      	bool write_to_png = false;
	      if (f == __filter_pdf) {
	        surface = Cairo::PdfSurface::create(filename, __bbw, __bbh);
	      } else if (f == __filter_svg) {
	        surface = Cairo::SvgSurface::create(filename, __bbw, __bbh);
	      } else if (f == __filter_png) {
	        surface = Cairo::ImageSurface::create(Cairo::FORMAT_ARGB32,
          				(int)ceilf(__bbw),
						      (int)ceilf(__bbh));
	        write_to_png = true;
	      }

	      if (surface) {
	        __cairo = Cairo::Context::create(surface);
	        
	        bool old_scale_override = __scale_override;
	        double old_tx = __translation_x;
	        double old_ty = __translation_y;
	        double old_scale = __scale;
	        __translation_x = __pad_x;
	        __translation_y = __bbh - __pad_y;
	        __scale = 1.0;
	        __scale_override = true;

	        Agraph_t *g = agmemread((char *)__graph.c_str());
	        if (g) {
	          gvLayout(__gvc, g, (char *)"dot");
	          gvRender(__gvc, g, (char *)"cairo", NULL);
	          gvFreeLayout(__gvc, g);
	          agclose(g);
	        }

	        if (write_to_png) {
	          surface->write_to_png(filename);
	        }

	        __cairo.clear();

	        __translation_x = old_tx;
	        __translation_y = old_ty;
	        __scale = old_scale;
	        __scale_override = old_scale_override;
	      }
      }

    } else {
      Gtk::MessageDialog md(*w, "Invalid filename",
			    /* markup */ false, Gtk::MESSAGE_ERROR,
			    Gtk::BUTTONS_OK, /* modal */ true);
      md.set_title("Invalid File Name");
      md.run();
    }
  }

  __fcd_save->hide();
}


/** Open a dot graph and display it. */
void
GraphDrawingArea::open()
{
  Gtk::Window *w = dynamic_cast<Gtk::Window *>(get_toplevel());
  __fcd_open->set_transient_for(*w);

  int result = __fcd_open->run();
  if (result == Gtk::RESPONSE_OK) {
    __update_graph = false;
    __graph = "";
    char *basec = strdup(__fcd_open->get_filename().c_str());
    char *basen = basename(basec);
    __graph_fsm = basen;
    free(basec);

    FILE *f = fopen(__fcd_open->get_filename().c_str(), "r");
    while (! feof(f)) {
      char tmp[4096];
      size_t s;
      if ((s = fread(tmp, 1, 4096, f)) > 0) {
       	__graph.append(tmp, s);
      }
    }
    fclose(f);
    __signal_update_disabled.emit();
    queue_draw();
  }

  __fcd_open->hide();
}


/** Draw event handler.
 * @param cr cairo context
 * @return true
 */
bool
GraphDrawingArea::on_draw(const Cairo::RefPtr<Cairo::Context> &cr)
{
  // This is where we draw on the window
  Glib::RefPtr<Gdk::Window> window = get_window();
  if(window) {
    clickable_nodes.clear();
    //Gtk::Allocation allocation = get_allocation();
    //const int width = allocation.get_width();
    //const int height = allocation.get_height();
    
    // coordinates for the center of the window
    //int xc, yc;
    //xc = width / 2;
    //yc = height / 2;
    __cairo = cr;
    __cairo->set_source_rgb(1, 1, 1);
    __cairo->paint();

    Agraph_t *g = agmemread((char *)__graph.c_str());
    if (g) {
      gvLayout(__gvc, g, (char *)"dot");
      gvRender(__gvc, g, (char *)"cairo", NULL);
      gvFreeLayout(__gvc, g);
      agclose(g);
    }

    __cairo.clear();
  }    

  return true;
}

/** Scroll event handler.
 * @param event event structure
 * @return signal return value
 */
bool
GraphDrawingArea::on_scroll_event(GdkEventScroll *event)
{
  if (event->direction == GDK_SCROLL_UP) {
    zoom_in();
  } else if (event->direction == GDK_SCROLL_DOWN) {
    zoom_out();
  }
  return true;
}


/** Button press event handler.
 * @param event event data
 * @return true
 */
bool
GraphDrawingArea::on_button_press_event(GdkEventButton *event)
{
  __last_mouse_x = event->x;
  __last_mouse_y = event->y;
  return true;
}


/** Mouse motion notify event handler.
 * @param event event data
 * @return true
 */
bool
GraphDrawingArea::on_motion_notify_event(GdkEventMotion *event)
{
  __scale_override = true;
  __translation_x -= __last_mouse_x - event->x;
  __translation_y -= __last_mouse_y - event->y;
  __last_mouse_x = event->x;
  __last_mouse_y = event->y;
  queue_draw();
  return true;
}


/** getter for the label of the clicked node
 *  @param x x coord of the click
 *  @param y y coord of the click
 *  @return the label of the clicked node if it was a node
 */
std::string
GraphDrawingArea::get_clicked_node(double x, double y) {
  std::map<std::string, box>::iterator bb;
  for(bb = clickable_nodes.begin(); bb != clickable_nodes.end(); bb++) {
    if(bb->second.x1 <= x && bb->second.y1 <= y && bb->second.x2 >= x && bb->second.y2 >= y) {
      return bb->first;
    }
  }
  
  return "";
}

