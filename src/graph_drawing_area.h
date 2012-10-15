
/***************************************************************************
 *  graph_drawing_area.h - Graph drawing area derived from Gtk::DrawingArea
 *
 *  Created: Wed Mar 18 10:38:07 2009
 *  Copyright  2009  Tim Niemueller [www.niemueller.de]
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

#pragma once

#include <gtkmm.h>

#include <gvc.h>
#include <gvcjob.h>

#include "gvplugin_cairo.h"

namespace SceneReconstruction {
  class GraphDrawingArea
  : public Gtk::DrawingArea,
    public CairoRenderInstructor
  {
   public:

    GraphDrawingArea();
    ~GraphDrawingArea();

    void save();
    void open();

    void zoom_in();
    void zoom_out();
    void zoom_fit();
    void zoom_reset();

    void set_graph_fsm(std::string fsm_name);
    void set_graph(std::string graph);

    void   set_bb(double bbw, double bbh);
    void   set_pad(double pad_x, double pad_y);
    void   set_translation(double tx, double ty);
    void   set_scale(double scale);
    bool   scale_override();
    double get_scale();
    void   get_translation(double &tx, double &ty);
    void   get_dimensions(double &width, double &height);
    void   get_pad(double &pad_x, double &pad_y);
    Cairo::RefPtr<Cairo::Context> get_cairo();

    bool get_update_graph();
    void set_update_graph(bool update);

    sigc::signal<void> signal_update_disabled();

    std::string get_clicked_node(double x, double y);

   protected:
    virtual bool on_draw(const Cairo::RefPtr<Cairo::Context> &cr);
    virtual bool on_scroll_event(GdkEventScroll *event);
    virtual bool on_button_press_event(GdkEventButton *event);
    virtual bool on_motion_notify_event(GdkEventMotion *event);

   private:
    void save_dotfile(const char *filename);

   private:
    Cairo::RefPtr<Cairo::Context> __cairo;
    Gtk::FileChooserDialog *__fcd_save;
    Gtk::FileChooserDialog *__fcd_open;
    Glib::RefPtr<Gtk::FileFilter> __filter_pdf;
    Glib::RefPtr<Gtk::FileFilter> __filter_svg;
    Glib::RefPtr<Gtk::FileFilter> __filter_png;
    Glib::RefPtr<Gtk::FileFilter> __filter_dot;
    sigc::signal<void> __signal_update_disabled;

    GVC_t *__gvc;

    std::string __graph_fsm;
    std::string __graph;
    std::string __nonupd_graph;
    std::string __nonupd_graph_fsm;

    double __bbw;
    double __bbh;
    double __pad_x;
    double __pad_y;
    double __translation_x;
    double __translation_y;
    double __scale;

    double __last_mouse_x;
    double __last_mouse_y;

    bool __scale_override;
    bool __update_graph;
  };
}
