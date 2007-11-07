/* 
 *  Matchbox Keyboard - A lightweight software keyboard.
 *
 *  Author: Ross Burton <ross@o-hand.com>
 *
 *  Copyright (c) 2007 OpenedHand Ltd - http://o-hand.com
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2, or (at your option)
 *  any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
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
