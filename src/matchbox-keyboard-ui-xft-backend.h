#ifndef HAVE_MB_KEYBOARD_UI_BACKEND_XFT_H
#define HAVE_MB_KEYBOARD_UI_BACKEND_XFT_H

#include "matchbox-keyboard.h"
#include <X11/Xft/Xft.h>

MBKeyboardUIBackend*
mb_kbd_ui_xft_init(MBKeyboardUI *ui);

#define MB_KBD_UI_BACKEND_INIT_FUNC(ui)  mb_kbd_ui_xft_init((ui))

#endif
