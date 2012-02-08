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

typedef struct _MbIMContext MbIMContext;
typedef struct _MbIMContextClass MbIMContextClass;

struct _MbIMContext
{
  GtkIMContextSimple context;
};

struct _MbIMContextClass
{
  GtkIMContextSimpleClass parent_class;
};

void mb_im_context_register_type (GTypeModule *module);

GtkIMContext *mb_im_context_new (void);
