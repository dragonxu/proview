/* 
 * Proview   Open Source Process Control.
 * Copyright (C) 2005-2017 SSAB EMEA AB.
 *
 * This file is part of Proview.
 *
 * This program is free software; you can redistribute it and/or 
 * modify it under the terms of the GNU General Public License as 
 * published by the Free Software Foundation, either version 2 of 
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful 
 * but WITHOUT ANY WARRANTY; without even the implied warranty of 
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the 
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License 
 * along with Proview. If not, see <http://www.gnu.org/licenses/>
 *
 * Linking Proview statically or dynamically with other modules is
 * making a combined work based on Proview. Thus, the terms and 
 * conditions of the GNU General Public License cover the whole 
 * combination.
 *
 * In addition, as a special exception, the copyright holders of
 * Proview give you permission to, from the build function in the
 * Proview Configurator, combine Proview with modules generated by the
 * Proview PLC Editor to a PLC program, regardless of the license
 * terms of these modules. You may copy and distribute the resulting
 * combined work under the terms of your choice, provided that every 
 * copy of the combined work is accompanied by a complete copy of 
 * the source code of Proview (the version used to produce the 
 * combined work), being distributed under the terms of the GNU 
 * General Public License plus this exception.
 **/

#include <stdlib.h>
#include <math.h>
#include "glow_std.h"

#include "glow_growwidget_qt.h"
#include "glow.h"
#include "glow_ctx.h"
#include "glow_growctx.h"
#include "glow_draw.h"
#include "glow_draw_qt.h"

#include "glow_scroll_widget_qt.h"

#include "cow_qt_helpers.h"

static void scroll_callback(glow_sScroll *data)
{
  widget_sScroll *scroll_data = (widget_sScroll *) data->scroll_data;
  QtScrollWidgetGlow *grow = ((QtScrollWidgetGlow *) scroll_data->parent);

  if (grow->scroll_timerid) {
    delete grow->scroll_timerid;
  }

  grow->scroll_timerid = new QTimer();
  grow->scroll_timerid->setSingleShot(true);
  QObject::connect(grow->scroll_timerid, SIGNAL(timeout()), grow,
                   SLOT(scroll_callback_cb()));
  grow->scroll_timerid->start(200);
  grow->scroll_data = *data;
}

static int grow_init_proc(QWidget *w, GlowCtx *fctx, void *client_data)
{
  QtScrollWidgetGlow *grow = ((QtScrollWidgetGlow *) w);
  GrowCtx *ctx = (GrowCtx *) grow->parent_ctx;

  if (grow->scroll_h) {
    widget_sScroll *scroll_data = new widget_sScroll();
    scroll_data->parent = w;
    scroll_data->scroll_h = grow->scroll_h;
    scroll_data->scroll_v = grow->scroll_v;
    scroll_data->scroll_h_managed = 1;
    scroll_data->scroll_v_managed = 1;

    ctx->register_scroll_callback((void *) scroll_data, scroll_callback);
  }
  return (grow->init_proc)(ctx, client_data);
}

QWidget *growwidgetqt_new(int (*init_proc)(GlowCtx *ctx, void *client_data),
                          void *client_data)
{
  QtScrollWidgetGlow *w = new QtScrollWidgetGlow();
  debug_print("creating a new glow scroll widget\n");
  w->init(glow_eCtxType_Grow, init_proc, client_data, grow_init_proc);
  return (QWidget *) w;
}

QWidget *scrolledgrowwidgetqt_new(
    int (*init_proc)(GlowCtx *ctx, void *client_data), void *client_data,
    QWidget **growwidget)
{
  QtScrollWidgetGlow *w = new QtScrollWidgetGlow();
  *growwidget = w;
  debug_print("creating a new glow scroll widget\n");
  return (QWidget *) w
      ->initScroll(glow_eCtxType_Grow, init_proc, client_data, grow_init_proc);;
}

QWidget *grownavwidgetqt_new(QWidget *main_grow)
{
  QtScrollWidgetGlow *w = new QtScrollWidgetGlow();
  debug_print("creating a new glow scroll widget\n");
  w->init(glow_eCtxType_Grow, main_grow);
  return (QWidget *) w;
}