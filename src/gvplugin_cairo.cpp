
/***************************************************************************
 *  gvplugin_cairo.cpp - Graphviz plugin using cairo
 *
 *  Created: Fri Dec 19 12:01:39 2008
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

#include "gvplugin_cairo.h"

#include <gvplugin_device.h>
#include <gvplugin_render.h>

#include <algorithm>
#include <cstdio>

#define NOEXPORT __attribute__ ((visibility("hidden")))

NOEXPORT CairoRenderInstructor *__cri = NULL;

#if CAIROMM_MAJOR_VERSION > 1 || (CAIROMM_MAJOR_VERSION == 1 && CAIROMM_MINO_VERSION > 8)
NOEXPORT std::vector<double> __cairo_render_dashed;
NOEXPORT std::vector<double> __cairo_render_dotted;
#else
NOEXPORT std::valarray<double> __cairo_render_dashed(1);
NOEXPORT std::valarray<double> __cairo_render_dotted(2);
#endif

/** @class CairoRenderInstructor
 * Graphviz Cairo render plugin instructor.
 * @author Tim Niemueller
 *
 * @fn Cairo::RefPtr<Cairo::Context> CairoRenderInstructor::get_cairo()
 * Get Cairo context.
 * @return cairo context to use for drawing
 *
 * @fn bool   CairoRenderInstructor::scale_override()
 * Check if scale override is enabled.
 * @return true if the instructor determines the scaling, false to have the
 * plugin do this.
 *
 * @fn void   CairoRenderInstructor::get_dimensions(double &width, double &height)
 * Get available space dimensions.
 * @param width upon return contains the available width
 * @param height upon return contains the available height
 *
 * @fn double CairoRenderInstructor::get_scale()
 * Get scale factor.
 * If scale_override() returns true, shall return the requested scale value.
 * @return scale factor
 *
 * @fn void   CairoRenderInstructor::set_scale(double scale)
 * Set scale.
 * Set the scale value that the plugin determined.
 * @param scale scale determined by plugin
 *
 * @fn void   CairoRenderInstructor::get_translation(double &tx, double &ty)
 * Get translation values.
 * If scale_override() returns true, shall return the requested translation values.
 * @param tx upon return contains translation in x
 * @param ty upon return contains translation in y
 *
 * @fn void   CairoRenderInstructor::set_translation(double tx, double ty)
 * Set translation.
 * Set the translation values the plugin determined.
 * @param tx translation in x
 * @param ty translation in y
 *
 * @fn void   CairoRenderInstructor::set_bb(double bbw, double bbh)
 * Set the bounding box.
 * Set by the plugin before calling any other function.
 * @param bbw bounding box width
 * @param bbh bounding box height
 *
 * @fn void   CairoRenderInstructor::set_pad(double pad_x, double pad_y)
 * Set padding.
 * Set by the plugin immediately after set_bb() is called.
 * @param pad_x padding in x
 * @param pad_y padding in y
 *
 * @fn void   CairoRenderInstructor::get_pad(double &pad_x, double &pad_y)
 * Get padding.
 * If scale_override() returns true, shall return the requested padding values.
 * @param pad_x upon return contains padding in x
 * @param pad_y upon return contains padding in y
 */

static void
cairo_device_init(GVJ_t* /* firstjob */)
{
}

static void
cairo_device_finalize(GVJ_t *firstjob)
{
  firstjob->context = (void *)__cri;
  firstjob->external_context = TRUE;

  // Render!
  (firstjob->callbacks->refresh)(firstjob);
}

static inline void
cairo_set_color(Cairo::RefPtr<Cairo::Context> cairo, gvcolor_t * color)
{
  cairo->set_source_rgba(color->u.RGBA[0], color->u.RGBA[1],
			 color->u.RGBA[2], color->u.RGBA[3]);
}

static inline void
cairo_set_penstyle(Cairo::RefPtr<Cairo::Context> cairo, GVJ_t *job)
{
  obj_state_t *obj = job->obj;

  if (obj->pen == PEN_DASHED) {
    cairo->set_dash(__cairo_render_dashed, 0.0);
  } else if (obj->pen == PEN_DOTTED) {
    cairo->set_dash(__cairo_render_dotted, 0.0);
  } else {
#if CAIROMM_MAJOR_VERSION > 1 || (CAIROMM_MAJOR_VERSION == 1 && CAIROMM_MINO_VERSION > 8)
    std::vector<double> empty;
#else
    std::valarray<double> empty;
#endif
    cairo->set_dash(empty, 0.0);
  }
  cairo->set_line_width(obj->penwidth);
}


static void
cairo_render_begin_page(GVJ_t *job)
{
  CairoRenderInstructor *cri = (CairoRenderInstructor *)job->context;
  cri->clickable_nodes.clear();

  float bbwidth  = job->bb.UR.x - job->bb.LL.x;
  float bbheight = job->bb.UR.y - job->bb.LL.y;

  cri->set_bb(bbwidth, bbheight);
  cri->set_pad(job->pad.x, job->pad.y);
  Cairo::RefPtr<Cairo::Context> cairo = cri->get_cairo();

  double pad_x, pad_y;
  cri->get_pad(pad_x, pad_y);

  // For internal calculations we need to care about the padding
  //bbwidth  += 2 * pad_x;
  //bbheight += 2 * pad_y;

  double avwidth, avheight;
  cri->get_dimensions(avwidth, avheight);
  float translate_x = 0;
  float translate_y = 0;

  if ( cri->scale_override() ) {
    float zoom = cri->get_scale();
    float zwidth  = bbwidth * zoom;
    float zheight = bbheight * zoom;
    translate_x += (avwidth  - zwidth ) / 2.;
    translate_y += (avheight - zheight) / 2.;

    double translate_x, translate_y;
    cri->get_translation(translate_x, translate_y);

    cairo->translate(translate_x, translate_y);
    cairo->scale(zoom, zoom);

  } else {
    float zoom_w = avwidth  / bbwidth;
    float zoom_h = avheight / bbheight;
    float zoom   = std::min(zoom_w, zoom_h);

    if (bbwidth > avwidth || bbheight > avheight) {
      float zwidth  = bbwidth * zoom;
      float zheight = bbheight * zoom;
      translate_x += (avwidth  - zwidth ) / 2.;
      translate_y += (avheight - zheight) / 2. + zheight;
    } else {
      zoom = 1.0;
      translate_x += (avwidth  - bbwidth)  / 2.;
      translate_y += (avheight - bbheight) / 2. + bbheight;
    }

    cri->set_scale(zoom);
    cri->set_translation(translate_x, translate_y);

    cairo->translate(translate_x + pad_x * zoom, translate_y - pad_y * zoom);
    cairo->scale(zoom, zoom);
  }
}

static void
cairo_render_end_page(GVJ_t* /* job */)
{
  //CairoRenderInstructor *cri = (CairoRenderInstructor *)job->context;
  //cri->queue_draw();  
}

static void
cairo_render_textpara(GVJ_t *job, pointf p, textpara_t *para)
{
  CairoRenderInstructor *cri = (CairoRenderInstructor *)job->context;
  Cairo::RefPtr<Cairo::Context> cairo = cri->get_cairo();
  obj_state_t *obj = job->obj;

  Cairo::FontWeight weight = Cairo::FONT_WEIGHT_NORMAL;
  Cairo::FontSlant slant   = Cairo::FONT_SLANT_NORMAL;
  char *fontweight = NULL;
  if (obj->type == CLUSTER_OBJTYPE) {
    fontweight = agget(obj->u.sg, (char *)"fontweight");
  } else if (obj->type == ROOTGRAPH_OBJTYPE) {
    fontweight = agget(obj->u.g, (char *)"fontweight");
  } else if (obj->type == NODE_OBJTYPE) {
    fontweight = agget(obj->u.n, (char *)"fontweight");
  } else if (obj->type == EDGE_OBJTYPE) {
    fontweight = agget(obj->u.e, (char *)"fontweight");
  }
  if (fontweight && (strcmp(fontweight, "bold") == 0)) {
    weight = Cairo::FONT_WEIGHT_BOLD;
    p.x -= 8;
  }
  char *fontslant = NULL;
  if (obj->type == CLUSTER_OBJTYPE) {
    fontslant = agget(obj->u.sg, (char *)"fontslant");
  } else if (obj->type == ROOTGRAPH_OBJTYPE) {
    fontslant = agget(obj->u.g, (char *)"fontslant");
  } else if (obj->type == NODE_OBJTYPE) {
    fontslant = agget(obj->u.n, (char *)"fontslant");
  } else if (obj->type == EDGE_OBJTYPE) {
    fontslant = agget(obj->u.e, (char *)"fontslant");
  }
  if (fontslant && (strcmp(fontslant, "italic") == 0)) {
    slant = Cairo::FONT_SLANT_ITALIC;
  }

  double offsetx = 0.0;
  double offsety = 0.0;
  double rotate  = 0.0;

  if ( (obj->type == EDGE_OBJTYPE) && (strcmp(para->str, obj->headlabel) == 0) ) {
    char *labelrotate = agget(obj->u.e, (char *)"labelrotate");
    if (labelrotate && (strlen(labelrotate) > 0)) {
      rotate = atof(labelrotate)*(M_PI/180);
    }
    char *labeloffsetx = agget(obj->u.e, (char *)"labeloffsetx");
    if (labeloffsetx && (strlen(labeloffsetx) > 0)) {
      offsetx = atof(labeloffsetx);
    }
    char *labeloffsety = agget(obj->u.e, (char *)"labeloffsety");
    if (labeloffsety && (strlen(labeloffsety) > 0)) {
      offsety = atof(labeloffsety);
    }
  }
  Cairo::Matrix old_matrix;
  cairo->get_matrix(old_matrix);

  cairo->select_font_face(para->fontname, slant, weight);
  cairo->set_font_size(para->fontsize);
  //cairo->set_font_options ( Cairo::FontOptions() );
  //cairo->set_line_width(1.0);

  Cairo::TextExtents extents;
  cairo->get_text_extents(para->str, extents);

  if (para->just == 'r') {
    p.x -= extents.width;
  } else if (para->just != 'l') {
    p.x -= extents.width / 2.0;
  }

  cairo->move_to(p.x + offsetx, -p.y + offsety);
  cairo->rotate(rotate);
  cairo_set_color(cairo, &(obj->pencolor));
  cairo->text_path( para->str );

  //save text bounding box to allow clicking
  if (obj->type == NODE_OBJTYPE) {
    double translate_x, translate_y;
    cri->get_translation(translate_x, translate_y);
    double x1, y1, x2, y2;
    cairo->get_path_extents(x1, y1, x2, y2);
    double scale;
    scale = cri->get_scale();
    // slightly increase text bounding box to cover a bit more of the actual node
    x1 = (x1 + offsetx - 10)*scale + translate_x;
    y1 = (y1 + offsety - 10)*scale + translate_y;
    x2 = (x2 + offsetx + 10)*scale + translate_x;
    y2 = (y2 + offsety + 10)*scale + translate_y;

    cri->clickable_nodes[para->str].x1 = x1;
    cri->clickable_nodes[para->str].y1 = y1;
    cri->clickable_nodes[para->str].x2 = x2;
    cri->clickable_nodes[para->str].y2 = y2;
  }

  cairo->fill();

  cairo->set_matrix(old_matrix);
}

static void
cairo_render_ellipse(GVJ_t *job, pointf *A, int filled)
{
  //printf("Render ellipse\n");
  CairoRenderInstructor *cri = (CairoRenderInstructor *)job->context;
  Cairo::RefPtr<Cairo::Context> cairo = cri->get_cairo();
  obj_state_t *obj = job->obj;

  Cairo::Matrix old_matrix;
  cairo->get_matrix(old_matrix);

  cairo_set_penstyle(cairo, job);

  cairo->translate(A[0].x, -A[0].y);

  double rx = A[1].x - A[0].x;
  double ry = A[1].y - A[0].y;
  cairo->scale(1, ry / rx);
  cairo->move_to(rx, 0);
  cairo->arc(0, 0, rx, 0, 2 * M_PI);
  cairo->close_path();

  cairo->set_matrix(old_matrix);

  if (filled) {
    cairo_set_color(cairo, &(obj->fillcolor));
    cairo->fill_preserve();
  }
  cairo_set_color(cairo, &(obj->pencolor));
  cairo->stroke();
}

static void
cairo_render_polygon(GVJ_t *job, pointf *A, int n, int filled)
{
  //printf("Polygon\n");
  CairoRenderInstructor *cri = (CairoRenderInstructor *)job->context;
  Cairo::RefPtr<Cairo::Context> cairo = cri->get_cairo();
  obj_state_t *obj = job->obj;

  cairo_set_penstyle(cairo, job);

  cairo->move_to(A[0].x, -A[0].y);
  for (int i = 1; i < n; ++i) {
    cairo->line_to(A[i].x, -A[i].y);
  }
  cairo->close_path();

  if (filled) {
    cairo_set_color(cairo, &(obj->fillcolor));
    cairo->fill_preserve();
  }

  // HACK to workaround graphviz bug any get the Tim style...
  if ( obj->type == CLUSTER_OBJTYPE ) {
    obj->pencolor.u.RGBA[0] = 0.666;
    obj->pencolor.u.RGBA[1] = 0.666;
    obj->pencolor.u.RGBA[2] = 1.0;
    obj->pencolor.u.RGBA[3] = 1.0;
  }
  cairo_set_color(cairo, &(obj->pencolor));
  cairo->stroke();
}

static void
cairo_render_bezier(GVJ_t * job, pointf * A, int n, int /* arrow_at_start */,
		int /* arrow_at_end */, int filled)
{
  //printf("Bezier\n");
  CairoRenderInstructor *cri = (CairoRenderInstructor *)job->context;
  Cairo::RefPtr<Cairo::Context> cairo = cri->get_cairo();
  obj_state_t *obj = job->obj;

  cairo_set_penstyle(cairo, job);

  cairo->move_to(A[0].x, -A[0].y);
  for (int i = 1; i < n; i += 3)
    cairo->curve_to(A[i].x, -A[i].y, A[i + 1].x, -A[i + 1].y,
		    A[i + 2].x, -A[i + 2].y);
  if (filled) {
    cairo_set_color(cairo, &(obj->fillcolor));
    cairo->fill_preserve();
  }
  cairo_set_color(cairo, &(obj->pencolor));
  cairo->stroke();
}

static void
cairo_render_polyline(GVJ_t * job, pointf * A, int n)
{
  //printf("Polyline\n");
  CairoRenderInstructor *cri = (CairoRenderInstructor *)job->context;
  Cairo::RefPtr<Cairo::Context> cairo = cri->get_cairo();
  obj_state_t *obj = job->obj;

  cairo_set_penstyle(cairo, job);

  //cairo->set_line_width(obj->penwidth * job->scale.x);
  cairo->move_to(A[0].x, -A[0].y);
  for (int i = 1; i < n; i++) {
    cairo->line_to(A[i].x, -A[i].y);
  }
  cairo_set_color(cairo, &(obj->pencolor));
  cairo->stroke();
}


static gvrender_engine_t cairo_render_engine = {
    0,				/* cairo_render_begin_job */
    0,				/* cairo_render_end_job */
    0,				/* cairo_render_begin_graph */
    0,				/* cairo_render_end_graph */
    0,				/* cairo_render_begin_layer */
    0,				/* cairo_render_end_layer */
    cairo_render_begin_page,
    cairo_render_end_page,
    0,				/* cairo_render_begin_cluster */
    0,				/* cairo_render_end_cluster */
    0,				/* cairo_render_begin_nodes */
    0,				/* cairo_render_end_nodes */
    0,				/* cairo_render_begin_edges */
    0,				/* cairo_render_end_edges */
    0,				/* cairo_render_begin_node */
    0,				/* cairo_render_end_node */
    0,				/* cairo_render_begin_edge */
    0,				/* cairo_render_end_edge */
    0,				/* cairo_render_begin_anchor */
    0,				/* cairo_render_end_anchor */
    0,				/* cairo_begin_label */
    0,				/* cairo_end_label */
    cairo_render_textpara,
    0,				/* cairo_render_resolve_color */
    cairo_render_ellipse,
    cairo_render_polygon,
    cairo_render_bezier,
    cairo_render_polyline,
    0,				/* cairo_render_comment */
    0,				/* cairo_render_library_shape */
};

static gvdevice_engine_t cairo_device_engine = {
    cairo_device_init,
    NULL,			/* cairo_device_format */
    cairo_device_finalize,
};


#ifdef __cplusplus
extern "C" {
#endif


static gvrender_features_t cairo_render_features = {
  GVRENDER_Y_GOES_DOWN |
  GVRENDER_DOES_LABELS |
  GVRENDER_DOES_TRANSFORM |
  GVRENDER_NO_WHITE_BG, 			/* flags */
  8,                         			/* default pad - graph units */
  0,						/* knowncolors */
  0,						/* sizeof knowncolors */
  RGBA_DOUBLE,					/* color_type */
};


static gvdevice_features_t cairo_device_features = {
  GVDEVICE_DOES_TRUECOLOR | GVDEVICE_EVENTS,	/* flags */
  {0.,0.},                    			/* default margin - points */
  {0.,0.},					/* default page width, height - points */
  {96.,96.},					/* dpi */
};

gvplugin_installed_t gvdevice_types_cairo[] = {
  {0, ( char *)"cairo:cairo", 0, &cairo_device_engine, &cairo_device_features},
  {0, NULL, 0, NULL, NULL}
};

gvplugin_installed_t gvrender_types_cairo[] = {
  {0, (char *)"cairo", 10, &cairo_render_engine, &cairo_render_features},
  {0, NULL, 0, NULL, NULL}
};

static gvplugin_api_t apis[] = {
  {API_device, gvdevice_types_cairo},
  {API_render, gvrender_types_cairo},
  {(api_t)0, 0},
};

gvplugin_library_t gvplugin_cairo_LTX_library = { (char *)"cairo", apis };

#ifdef __cplusplus
}
#endif


void
gvplugin_cairo_setup(GVC_t *gvc, CairoRenderInstructor *cri)
{
  __cri = cri;
  gvAddLibrary(gvc, &gvplugin_cairo_LTX_library);

#if CAIROMM_MAJOR_VERSION > 1 || (CAIROMM_MAJOR_VERSION == 1 && CAIROMM_MINO_VERSION > 8)
  __cairo_render_dashed.clear();
  __cairo_render_dashed.push_back(6.0);
  __cairo_render_dotted.clear();
  __cairo_render_dotted.push_back(2.0);
  __cairo_render_dotted.push_back(6.0);
#else
  __cairo_render_dashed[0] = 6.0;
  __cairo_render_dotted[0] = 2.0;
  __cairo_render_dotted[1] = 6.0;
#endif
}
