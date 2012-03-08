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

#ifndef _LIBMATCHBOX_KEYBOARD_H
#define _LIBMATCHBOX_KEYBOARD_H

#include <X11/Xlib.h>

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#if WANT_GTK_WIDGET
#include <gdk/gdkx.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

typedef struct MBKeyboard MBKeyboard;


#if WANT_GTK_WIDGET
MBKeyboard *mb_keyboard_new (Display *xdpy, GdkWindow *parent,
                             int x, int y,
                             int width, int height,
                             int argc, char **argv);
#else
MBKeyboard *mb_keyboard_new (Display *xdpy, Window parent,
                             int x, int y,
                             int width, int height,
                             int argc, char **argv);
#endif

void        mb_keyboard_destroy (MBKeyboard *kb);
Window      mb_keyboard_get_xwindow (MBKeyboard *kb);
void        mb_keyboard_handle_xevent (MBKeyboard *kb, XEvent *xev);

#ifdef __cplusplus
}
#endif

#endif
