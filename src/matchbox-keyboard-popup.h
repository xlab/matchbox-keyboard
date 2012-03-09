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

#ifndef HAVE_MB_KEYBOARD_POPUP_H
#define HAVE_MB_KEYBOARD_POPUP_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef WANT_CAIRO
#include <cairo.h>
#else
#error "Popups are only available with Cairo backend"
#endif

#include "matchbox-keyboard.h"

MBKeyboardPopup *mb_kbd_popup_new (MBKeyboardUI *ui);
void             mb_kbd_popup_destroy (MBKeyboardPopup *popup);
void             mb_kbd_popup_show (MBKeyboardPopup *popup,
                                    MBKeyboardKey   *key,
                                    int              x_root,
                                    int              y_root);
void             mb_kbd_popup_hide (MBKeyboardPopup *popup);
void             mb_kbd_popup_load_font (MBKeyboardPopup *popup);
void             mb_kbd_popup_resize (MBKeyboardPopup *popup);

#endif
