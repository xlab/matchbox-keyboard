/* -*- mode: C; c-file-style: "gnu"; indent-tabs-mode: nil; -*- */

/*
 *  Matchbox Keyboard - A lightweight software keyboard.
 *
 *  Authored By Tomas Frydrych <tomas@sleepfive.com>
 *
 *  Copyright (c) 2012 Vernier Software and Technology
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

#include "libmatchbox-keyboard.h"
#include "matchbox-keyboard.h"

extern Display *mb_xdpy;
extern int      mb_xscreen;
extern Window   mb_xroot;

#if WANT_GTK_WIDGET
MBKeyboard *
mb_keyboard_new (Display *xdpy, GdkWindow *parent,
                 int x, int y,
                 int width, int height,
                 int argc, char **argv)
#else
MBKeyboard *
mb_keyboard_new (Display *xdpy, Window parent,
                 int x, int y,
                 int width, int height,
                 int argc, char **argv)
#endif
{
  MBKeyboard *kb;

  mb_xdpy    = xdpy;
  mb_xscreen = DefaultScreen (mb_xdpy);
  mb_xroot   = RootWindow (mb_xdpy, mb_xscreen);

  kb = mb_kbd_new (argc, argv, True, parent, x, y, width, height);

  if (!mb_kbd_ui_realize(kb->ui))
    return NULL;

#if 0
  mb_kbd_ui_show(kb->ui);
  mb_kbd_ui_redraw(kb->ui);
#endif

  return kb;
}

void
mb_keyboard_destroy (MBKeyboard *kb)
{
  mb_kbd_destroy (kb);
}

Window
mb_keyboard_get_xwindow (MBKeyboard *kb)
{
  return mb_kbd_ui_x_win (kb->ui);
}

void
mb_keyboard_handle_xevent (MBKeyboard *kb, XEvent *xev)
{
  mb_kbd_ui_handle_widget_xevent (kb->ui, xev);
}
