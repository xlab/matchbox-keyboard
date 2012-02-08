/*
 *  Matchbox Keyboard - A lightweight software keyboard.
 *
 *  Author: Ross Burton <ross@o-hand.com>
 *
 *  Copyright (c) 2007-2012 Intel Corp
 *
 *  This program is free software; you can redistribute it and/or modify it
 *  under the terms and conditions of the GNU Lesser General Public License,
 *  version 2.1, as published by the Free Software Foundation.
 *
 *  This program is distributed in the hope it will be useful, but WITHOUT ANY
 *  WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 *  FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for
 *  more details.
 *
 */

#include <gtk/gtkimcontextsimple.h>

#include "im-context.h"
#include "im-protocol.h"

static GtkIMContextClass *parent_class;
static GType im_context_type = 0;

static void
mb_im_context_focus_in (GtkIMContext *context)
{
  protocol_send_event (MBKeyboardRemoteShow);

  if (GTK_IM_CONTEXT_CLASS (parent_class)->focus_in)
    GTK_IM_CONTEXT_CLASS (parent_class)->focus_in (context);
}

static void
mb_im_context_focus_out (GtkIMContext *context)
{
  protocol_send_event (MBKeyboardRemoteHide);

  if (GTK_IM_CONTEXT_CLASS (parent_class)->focus_out)
    GTK_IM_CONTEXT_CLASS (parent_class)->focus_out (context);
}

static void
mb_im_context_class_init (MbIMContextClass *klass)
{
  GtkIMContextClass *context_class = GTK_IM_CONTEXT_CLASS (klass);

  parent_class = g_type_class_peek_parent (klass);

  context_class->focus_in = mb_im_context_focus_in;
  context_class->focus_out = mb_im_context_focus_out;
}

static void
mb_im_context_init (MbIMContext *self)
{
}

void
mb_im_context_register_type (GTypeModule *module)
{
  if (!im_context_type) {
    static const GTypeInfo im_context_info = {
      sizeof (MbIMContextClass),
      NULL,
      NULL,
      (GClassInitFunc) mb_im_context_class_init,
      NULL,
      NULL,
      sizeof (MbIMContext),
      0,
      (GInstanceInitFunc) mb_im_context_init,
    };
    im_context_type = g_type_module_register_type (module, GTK_TYPE_IM_CONTEXT_SIMPLE,
                                                   "MbIMContext", &im_context_info, 0);
  }
}

GtkIMContext *
mb_im_context_new (void)
{
  return g_object_new (im_context_type, NULL);
}
