/*
 *  Matchbox Keyboard - A lightweight software keyboard.
 *
 *  Authored By Tomas Frydrych <tomas@sleepfive.com>
 *
 *  Copyright (c) 2005-2012 Intel Corp
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
#include <math.h>

#include "matchbox-keyboard.h"
#include "matchbox-keyboard-popup.h"

#define PAD 1      /* padding between key and window     */
#define RAD 2      /* popup corner radius                */
#define Y_OFFSET 4 /* how far above the key to put popup */
#define MIN_SIZE 60
#define MAX_SIZE 80

#define R(x) (M_PI * (double)x / 180.)

struct MBKeyboardPopup
{
  MBKeyboardUI       *ui;

  int                 width;
  int                 height;

  Window              xwin;
  Pixmap              backbuffer;

  cairo_surface_t    *surface;
  cairo_t            *cr;
};

void
mb_kbd_popup_load_font (MBKeyboardPopup *popup)
{
  double      mm_per_pixel, pixel_size;
  MBKeyboard *kb = mb_kbd_ui_kbd (popup->ui);

  cairo_select_font_face (popup->cr,
			  kb->font_family,
			  CAIRO_FONT_SLANT_NORMAL,
			  CAIRO_FONT_WEIGHT_NORMAL);

  mm_per_pixel = (double)DisplayHeightMM (mb_kbd_ui_x_display (popup->ui),
                                          mb_kbd_ui_x_screen (popup->ui))
    / DisplayHeight (mb_kbd_ui_x_display (popup->ui),
                     mb_kbd_ui_x_screen (popup->ui));

  /* 1 millimeter = 0.0393700787 inches */
  /* 1 inch = 72 PostScript points */

  pixel_size = 1.5 * (double)kb->font_pt_size /
    ( (double)mm_per_pixel * 0.039 * 72 );

  cairo_set_font_size (popup->cr, pixel_size);
}

static void
mb_kbd_popup_calc_size (MBKeyboardPopup *popup, int *width, int *height)
{
  MBKeyboardUI           *ui = popup->ui;
  MBKeyboard             *kbd = mb_kbd_ui_kbd (ui);
  MBKeyboardLayout       *layout;
  List                   *row_item, *key_item;

  *width  = MIN_SIZE;
  *height = MIN_SIZE;

  layout   = mb_kbd_get_selected_layout (kbd);
  row_item = mb_kbd_layout_rows (layout);

  while (row_item)
    {
      mb_kbd_row_for_each_key (row_item->data, key_item)
        {
          MBKeyboardKey *key = key_item->data;
          int            w, h;

          if (!mb_kbd_is_extended (kbd)
              && mb_kbd_key_get_extended(key))
            continue;

          /* Ignore keys whose width is forced */
          if (mb_kbd_key_get_req_uwidth (key))
            continue;


          w = (3 * mb_kbd_key_width (key)) / 2;
          h = (3 * mb_kbd_key_height (key)) / 2;

          if (w > *width && w <= MAX_SIZE)
            *width = w;

          if (h > *height && h <= MAX_SIZE)
            *height = h;

          return;
        }

      row_item = util_list_next(row_item);
    }
}

MBKeyboardPopup *
mb_kbd_popup_new (MBKeyboardUI *ui)
{
  MBKeyboardPopup      *popup = util_malloc0 (sizeof (MBKeyboardPopup));
  Display              *xdpy  = mb_kbd_ui_x_display (ui);
  XSetWindowAttributes  attrs;
  int                   width, height;

  popup->ui = ui;

  mb_kbd_popup_calc_size (popup, &width, &height);

  popup->width  = width;
  popup->height = height;

  attrs.override_redirect = True;

  popup->xwin = XCreateWindow (xdpy,
                               mb_kbd_ui_x_win_root (ui),
                               0, 0,
                               width, height,
                               0,
                               CopyFromParent, CopyFromParent, CopyFromParent,
                               CWOverrideRedirect,
                               &attrs);

  popup->backbuffer = XCreatePixmap (xdpy,
                                     popup->xwin,
                                     width,
                                     height,
                                     DefaultDepth (xdpy,
                                                   mb_kbd_ui_x_screen (ui)));

  XSetWindowBackgroundPixmap(xdpy,
                             popup->xwin,
                             popup->backbuffer);

  popup->surface
    = cairo_xlib_surface_create (xdpy,
				 popup->backbuffer,
				 DefaultVisual (xdpy, mb_kbd_ui_x_screen(ui)),
				 width, height);

  popup->cr = cairo_create (popup->surface);
  cairo_reference (popup->cr);

  mb_kbd_popup_load_font (popup);

  return popup;
}

void
mb_kbd_popup_destroy (MBKeyboardPopup *popup)
{
  cairo_destroy (popup->cr);

  free (popup);
}

void
mb_kbd_popup_redraw (MBKeyboardPopup *popup, MBKeyboardKey *key)
{
  MBKeyboardKeyStateType state;
  MBKeyboard            *kbd;
  cairo_pattern_t       *pat;
  double                 x, y, w, h;
  double                 x1p, x2p, y1p, y2p;

  kbd = mb_kbd_ui_kbd (popup->ui);

  x = 0;
  y = 0;
  w = popup->width;
  h = popup->height;

  x1p = x + PAD + 1;
  y1p = y + PAD + 1;
  x2p = x + w - 2*PAD;
  y2p = y + h - 2*PAD;

  /* clear it */
  cairo_set_line_width (popup->cr, 0.04);
  cairo_set_source_rgb (popup->cr, 0.7686, 0.8314, 0.8549);

  cairo_stroke (popup->cr);

  pat = cairo_pattern_create_linear (0, y, 0, y + h);

  /* cairo_pattern_add_color_stop_rgb (pat, 1, 0.9, 0.9, 0.9); */
  cairo_pattern_add_color_stop_rgb (pat, 1, 0.7686, 0.8314, 0.8549);
  cairo_pattern_add_color_stop_rgb (pat, 0, 0.7686, 0.8314, 0.8549);
  cairo_set_source (popup->cr, pat);

  cairo_arc (popup->cr, x1p + RAD, y1p + RAD, RAD, R(180), R(270));
  cairo_arc (popup->cr, x2p - RAD, y1p + RAD, RAD, R(270), R(360));
  cairo_arc (popup->cr, x2p - RAD, y2p - RAD, RAD, R(0), R(90));
  cairo_arc (popup->cr, x1p + RAD, y2p - RAD, RAD, R(90), R(180));
  cairo_close_path (popup->cr);

  cairo_fill (popup->cr);
  cairo_pattern_destroy (pat);

  /* border */
  /* bottom - right */
  cairo_set_line_width (popup->cr, 1);
  cairo_set_source_rgba (popup->cr, 0.0, 0.0, 0.0, 0.2);
  cairo_move_to (popup->cr, x1p + 1 + RAD, y2p - 1);
  cairo_line_to (popup->cr, x2p - 1 - RAD, y2p - 1);
  cairo_move_to (popup->cr, x2p - 1, y2p - 1 - RAD);
  cairo_line_to (popup->cr, x2p - 1, y1p + 1 + RAD);
  cairo_stroke (popup->cr);

  cairo_arc (popup->cr, x2p - RAD - 1, y2p - RAD - 1, RAD, R(0), R(90));
  cairo_stroke (popup->cr);

  /* letf - top */
  cairo_set_source_rgba (popup->cr, 1.0, 1.0, 1.0, 0.5);
  cairo_move_to (popup->cr, x1p + 1, y2p - 1 - RAD);
  cairo_line_to (popup->cr, x1p + 1, y1p + 1 + RAD);
  cairo_move_to (popup->cr, x1p + 1 + RAD, y1p + 1);
  cairo_line_to (popup->cr, x2p - 1 - RAD, y1p + 1);
  cairo_stroke (popup->cr );

  cairo_arc (popup->cr, x1p + RAD + 1, y1p + RAD + 1, RAD, R(180), R(270));
  cairo_stroke (popup->cr);

  /* Handle state related painting */

  state = mb_kbd_keys_current_state (kbd);

  if (mb_kbd_has_state (kbd, MBKeyboardStateCaps)
      && mb_kbd_key_get_obey_caps(key))
    state = MBKeyboardKeyStateShifted;

  if (!mb_kdb_key_has_state (key, state))
    {
      if (state == MBKeyboardKeyStateNormal)
	return;  /* keys should at least have a normal state */
      else
        state = MBKeyboardKeyStateNormal;
    }

  cairo_set_source_rgb (popup->cr, 0, 0, 0);

  if (mb_kbd_key_get_face_type (key, state) == MBKeyboardKeyFaceGlyph)
  {
    const char *face_str = mb_kbd_key_get_glyph_face(key, state);

    if (face_str)
      {
        double x1, y1;
	cairo_font_extents_t font_extents;
        cairo_text_extents_t text_extents;

        cairo_text_extents (popup->cr, face_str, &text_extents);
	cairo_font_extents (popup->cr, &font_extents);

        x1 = x + round (((w - text_extents.width) / 2.0) -
                        text_extents.x_bearing);

        y1 = y + round ((h - font_extents.ascent -
                         font_extents.descent) / 2.0  + font_extents.ascent);

        cairo_move_to(popup->cr, x1, y1);
        cairo_show_text (popup->cr, face_str);
      }
  }
  else if (mb_kbd_key_get_face_type(key, state) == MBKeyboardKeyFaceImage)
    {
      double x1, y1, w1, h1;
      cairo_surface_t *img;

      img = mb_kbd_key_get_image_face(key, state);

      w1 = cairo_image_surface_get_width (img);
      h1 = cairo_image_surface_get_height (img);

      x1 = ((popup->width - w1) / 2.0);
      y1 = ((popup->height - h1 ) / 2.0);

      cairo_set_source_surface (popup->cr, img, x1, y1);
      cairo_rectangle (popup->cr, x1, y1, w1, h1);
      cairo_fill (popup->cr);
    }
}


void
mb_kbd_popup_pre_redraw (MBKeyboardPopup  *popup)
{

  cairo_set_source_rgb (popup->cr, 0.3255, 0.3255, 0.3255);

  cairo_rectangle (popup->cr,
                   0,
                   0,
		   popup->width,
		   popup->height);

  cairo_fill (popup->cr);
}


void
mb_kbd_popup_show (MBKeyboardPopup *popup,
                   MBKeyboardKey   *key,
                   int              x_root,
                   int              y_root)
{
  int x, y;
  Display *xdpy = mb_kbd_ui_x_display (popup->ui);
  const char *glyph;

  if (mb_kbd_key_is_blank (key) ||
      mb_kbd_key_get_modifer_action (key, MBKeyboardKeyStateNormal) ||
      ((mb_kbd_key_get_face_type (key, 0) == MBKeyboardKeyFaceGlyph) &&
       (!(glyph = mb_kbd_key_get_glyph_face(key, 0)) || !strcmp (glyph, " "))))
    return;

  mb_kbd_popup_pre_redraw (popup);
  mb_kbd_popup_redraw (popup, key);

  util_trap_x_errors ();

  x = mb_kbd_key_abs_x (key) + x_root +
    (mb_kbd_key_width (key) - popup->width) / 2;
  y = mb_kbd_key_abs_y (key) + y_root - popup->height - Y_OFFSET;

  XMoveWindow (xdpy, popup->xwin, x, y);
  XMapWindow (xdpy, popup->xwin);
  XRaiseWindow (xdpy, popup->xwin);
  XSync (xdpy, False);

  util_untrap_x_errors ();
}

void
mb_kbd_popup_hide (MBKeyboardPopup *popup)
{
  util_trap_x_errors ();
  XUnmapWindow (mb_kbd_ui_x_display (popup->ui), popup->xwin);
  util_untrap_x_errors ();
}

void
mb_kbd_popup_resize (MBKeyboardPopup *popup)
{
  Display *xdpy    = mb_kbd_ui_x_display (popup->ui);
  int      xscreen = mb_kbd_ui_x_screen (popup->ui);
  int      w       = popup->width;
  int      h       = popup->height;

  mb_kbd_popup_calc_size (popup, &w, &h);

  if (w == popup->width && h == popup->height)
    return;

  popup->width  = w;
  popup->height = h;

  util_trap_x_errors ();

  XResizeWindow (xdpy, popup->xwin, w, h);

  if (popup->backbuffer)
    {
      XFreePixmap (xdpy, popup->backbuffer);
      popup->backbuffer = XCreatePixmap (xdpy,
                                         popup->xwin,
                                         w, h,
                                         DefaultDepth (xdpy, xscreen));

      XSetWindowBackgroundPixmap (xdpy, popup->xwin, popup->backbuffer);

      if (popup->cr)
        {
          cairo_xlib_surface_set_size (cairo_get_target (popup->cr), w, h);

          cairo_xlib_surface_set_drawable (popup->surface,
                                           popup->backbuffer, w, h);
        }
    }

  util_untrap_x_errors ();
}
