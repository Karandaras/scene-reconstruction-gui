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
  /** @class GraphDrawingArea "graph_drawing_area.h"
   * Graph drawing area.
   * Derived version of Gtk::DrawingArea that renders a graph via Graphviz.
   * @author Tim Niemueller
   * @author Bastian Klingen
   */
  class GraphDrawingArea
  : public Gtk::DrawingArea,
    public CairoRenderInstructor
  {
   public:

    /** Constructor. */
    GraphDrawingArea();
    /** Destructor. */
    ~GraphDrawingArea();

    /** save current graph. */
    void save();
    /** Open a dot graph and display it. */
    void open();

    /** Zoom in.
     * Increases zoom factor by 20, no upper limit.
     */
    void zoom_in();
    /** Zoom out.
     * Decreases zoom factor by 20 with a minimum of 1.
     */
    void zoom_out();
    /** Zoom to fit.
     * Disables scale override and draws with values suggested by Graphviz plugin.
     */
    void zoom_fit();
    /** Zoom reset.
     * Reset zoom to 1. Enables scale override.
     */
    void zoom_reset();

    /** Set graph's FSM name.
     * @param fsm_name name of FSM the graph belongs to
     */
    void set_graph_fsm(std::string fsm_name);
    /** Set graph.
     * @param graph string representation of the current graph in the dot language.
     */
    void set_graph(std::string graph);

    /** Set bounding box.
     * To be called only by the Graphviz plugin.
     * @param bbw bounding box width
     * @param bbh bounding box height
     */
    void   set_bb(double bbw, double bbh);
    /** Set padding.
     * To be called only by the Graphviz plugin.
     * @param pad_x padding in x
     * @param pad_y padding in y
     */
    void   set_pad(double pad_x, double pad_y);
    /** Set translation.
     * To be called only by the Graphviz plugin.
     * @param tx translation in x
     * @param ty translation in y
     */
    void   set_translation(double tx, double ty);
    /** Set scale.
     * To be called only by the Graphviz plugin.
     * @param scale scale value
     */
    void   set_scale(double scale);
    /** Check if scale override is enabled.
     * @return true if scale override is enabled, false otherwise
     */
    bool   scale_override();
    /** Get scale.
     * To be called only by the Graphviz plugin.
     * @return scale value
     */
    double get_scale();
    /** Get translation.
     * @param tx upon return contains translation value
     * @param ty upon return contains translation value
     */
    void   get_translation(double &tx, double &ty);
    /** Get dimensions
     * @param width upon return contains width
     * @param height upon return contains height
     */
    void   get_dimensions(double &width, double &height);
    /** Get padding.
     * To be called only by the Graphviz plugin.
     * @param pad_x upon return contains padding in x
     * @param pad_y upon return contains padding in y
     */
    void   get_pad(double &pad_x, double &pad_y);
    /** Get Cairo context.
     * This is only valid during the expose event and is only meant for the
     * Graphviz plugin.
     * @return Cairo context
     */
    Cairo::RefPtr<Cairo::Context> get_cairo();

    /** Check if graph is being updated.
     * @return true if the graph will be update if new data is received, false otherwise
     */
    bool get_update_graph();
    /** Set if the graph should be updated on new data.
     * @param update true to update on new data, false to disable update
     */
    void set_update_graph(bool update);

    /** Get "update disabled" signal.
     * @return "update disabled" signal
     */
    sigc::signal<void> signal_update_disabled();

    /** getter for the label of the clicked node
     *  @param x x coord of the click
     *  @param y y coord of the click
     *  @return the label of the clicked node if it was a node
     */
    std::string get_clicked_node(double x, double y);

   protected:
    /** Draw event handler.
     * @param cr cairo context
     * @return true
     */
    virtual bool on_draw(const Cairo::RefPtr<Cairo::Context> &cr);
    /** Scroll event handler.
     * @param event event structure
     * @return signal return value
     */
    virtual bool on_scroll_event(GdkEventScroll *event);
    /** Button press event handler.
     * @param event event data
     * @return true
     */
    virtual bool on_button_press_event(GdkEventButton *event);
    /** Mouse motion notify event handler.
     * @param event event data
     * @return true
     */
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
