
/***************************************************************************
 *  gvplugin_cairo.h - Graphviz plugin using cairo
 *
 *  Created: Fri Dec 19 12:01:03 2008
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

#pragma once

#include <cairomm/cairomm.h>
#include <gvc.h>
#include <map>

class CairoRenderInstructor
{
  public:
    /** Empty virtual destructor. */
    virtual ~CairoRenderInstructor() {}
  
    virtual Cairo::RefPtr<Cairo::Context> get_cairo() = 0;
  
    virtual bool   scale_override() = 0;
    virtual void   get_dimensions(double &width, double &height) = 0;
    virtual double get_scale() = 0;
    virtual void   set_scale(double scale) = 0;
    virtual void   get_translation(double &tx, double &ty) = 0;
    virtual void   set_translation(double tx, double ty) = 0;
    virtual void   set_bb(double bbw, double bbh) = 0;
    virtual void   set_pad(double pad_x, double pad_y) = 0;
    virtual void   get_pad(double &pad_x, double &pad_y) = 0;

    /** bounding box for labels */
    struct box {
        /** X coordinate for the top left corner of the bounding box */
        double x1,
        /** Y coordinate for the top left corner of the bounding box */
               y1;
        /** X coordinate for the bottom right corner of the bounding box */
        double x2,
        /** Y coordinate for the bottom right corner of the bounding box */
               y2;
    };

    /** list of boundingboxes for the node labels */
    std::map<std::string, box>  clickable_nodes;
};

extern void gvplugin_cairo_setup(GVC_t *gvc,
					  CairoRenderInstructor *cri);
