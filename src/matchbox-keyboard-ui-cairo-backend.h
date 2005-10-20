#ifndef HAVE_MB_KEYBOARD_CAIRO_BACKEND_XFT_H
#define HAVE_MB_KEYBOARD_CAIRO_BACKEND_XFT_H

#include "matchbox-keyboard.h"

#include <cairo/cairo.h>
#include <cairo/cairo-xlib.h>

MBKeyboardUIBackend*
mb_kbd_ui_cairo_init(MBKeyboardUI *ui);

#define MB_KBD_UI_BACKEND_INIT_FUNC(ui)  mb_kbd_ui_cairo_init((ui))

#endif
