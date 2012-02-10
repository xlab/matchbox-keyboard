/*
 *  Matchbox Keyboard - A lightweight software keyboard.
 *
 *  Authored By Matthew Allum <mallum@o-hand.com>
 *              Tomas Frydrych <tomas@sleepfive.com>
 *
 *  Copyright (c) 2005-2012 Intel Corp
 *  Copyright (c) 2012 Vernier Software & Technology
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

#include "matchbox-keyboard.h"
#include <math.h>

#define PAD 1
#define RAD 2
#define R(x) (M_PI * (double)x / 180.)

typedef struct MBKeyboardUIBackendCario
{
  MBKeyboardUIBackend backend;

  cairo_surface_t    *surface;
  cairo_t            *cr;

  Pixmap              foo_pxm;

} MBKeyboardUIBackendCairo;

static void
mb_kbd_ui_cairo_text_extents (MBKeyboardUI  *ui,
			      const  char   *str,
			      int           *width,
			      int           *height)
{
  MBKeyboardUIBackendCairo *cairo_backend = NULL;
  cairo_font_extents_t      font_extents;
  cairo_text_extents_t      text_extents;
  int                       w_t, h_t, h_f;

  cairo_backend = (MBKeyboardUIBackendCairo*)mb_kbd_ui_backend(ui);

  /*
   * This function is used to calculate nominal key sizes -- never allow the
   * reported height to be smaller than the font height, otherwise keys that
   * stretch stretch below the base line will end up drawn partially, or even
   * completely, of the key.
   */
  cairo_text_extents (cairo_backend->cr, str, &text_extents);
  cairo_font_extents (cairo_backend->cr, &font_extents);

  w_t = round (text_extents.width  + 2 * PAD);
  h_t = round (text_extents.height + 2 * PAD);
  h_f = round (font_extents.ascent + font_extents.descent + 2 * PAD);

  *width  = w_t;
  *height = h_t > h_f ? h_t : h_f;
}

static int
mb_kbd_ui_cairo_load_font(MBKeyboardUI *ui)
{
  MBKeyboard *kb = NULL;
  MBKeyboardUIBackendCairo *cairo_backend = NULL;
  double                    mm_per_pixel,  pixel_size;

  cairo_backend = (MBKeyboardUIBackendCairo*)mb_kbd_ui_backend(ui);
  kb          = mb_kbd_ui_kbd(ui);

  /* FIXME: font weights from  kb->font_variant */
  cairo_select_font_face (cairo_backend->cr,
			  kb->font_family,
			  CAIRO_FONT_SLANT_NORMAL,
			  CAIRO_FONT_WEIGHT_NORMAL);

  mm_per_pixel = (double)DisplayHeightMM(mb_kbd_ui_x_display(ui),
					 mb_kbd_ui_x_screen(ui))
                   / DisplayHeight(mb_kbd_ui_x_display(ui),
					 mb_kbd_ui_x_screen(ui));

  /* 1 millimeter = 0.0393700787 inches */

  /* 1 inch = 72 PostScript points */

  pixel_size = (double)kb->font_pt_size / ( (double)mm_per_pixel * 0.039 * 72 );

  cairo_set_font_size (cairo_backend->cr, pixel_size);

  return 1;
}

void
mb_kbd_ui_cairo_redraw_key(MBKeyboardUI  *ui, MBKeyboardKey *key)
{
  MBKeyboardUIBackendCairo *cairo_backend = NULL;

  MBKeyboardKeyStateType state;
  MBKeyboard            *kbd;
  cairo_pattern_t       *pat;
  double                 x, y, w, h;
  double                 x1p, x2p, y1p, y2p;
  boolean                caps;

  if (mb_kbd_key_is_blank(key)) /* spacer */
    return;

  cairo_backend = (MBKeyboardUIBackendCairo*)mb_kbd_ui_backend(ui);

  kbd = mb_kbd_ui_kbd(ui);

  x = mb_kbd_key_abs_x(key);
  y = mb_kbd_key_abs_y(key);
  w = mb_kbd_key_width(key);
  h = mb_kbd_key_height(key);

  x1p = x + PAD;
  y1p = y + PAD;
  x2p = x + w - 2*PAD;
  y2p = y + h - 2*PAD;

  /* clear it */
  cairo_set_line_width (cairo_backend->cr, 0.04);
  cairo_set_source_rgb(cairo_backend->cr, 0.7686, 0.8314, 0.8549);

  cairo_stroke( cairo_backend->cr );

  pat = cairo_pattern_create_linear (0, y, 0, y + h);

  /* cairo_pattern_add_color_stop_rgb (pat, 1, 0.9, 0.9, 0.9); */
  cairo_pattern_add_color_stop_rgb (pat, 1, 0.7686, 0.8314, 0.8549);
  cairo_pattern_add_color_stop_rgb (pat, 0, 0.7686, 0.8314, 0.8549);
  cairo_set_source (cairo_backend->cr, pat);

  cairo_arc (cairo_backend->cr, x1p + RAD, y1p + RAD, RAD, R(180), R(270));
  cairo_arc (cairo_backend->cr, x2p - RAD, y1p + RAD, RAD, R(270), R(360));
  cairo_arc (cairo_backend->cr, x2p - RAD, y2p - RAD, RAD, R(0), R(90));
  cairo_arc (cairo_backend->cr, x1p + RAD, y2p - RAD, RAD, R(90), R(180));
  cairo_close_path (cairo_backend->cr);

  cairo_fill (cairo_backend->cr);
  cairo_pattern_destroy (pat);

  /* border */
  /* bottom - right */
  cairo_set_line_width (cairo_backend->cr, 1);
  cairo_set_source_rgba (cairo_backend->cr, 0.0, 0.0, 0.0, 0.2);
  cairo_move_to (cairo_backend->cr, x1p + 1 + RAD, y2p - 1);
  cairo_line_to (cairo_backend->cr, x2p - 1 - RAD, y2p - 1);
  cairo_move_to (cairo_backend->cr, x2p - 1, y2p - 1 - RAD);
  cairo_line_to(cairo_backend->cr, x2p - 1, y1p + 1 + RAD);
  cairo_stroke ( cairo_backend->cr );

  cairo_arc (cairo_backend->cr, x2p - RAD - 1, y2p - RAD - 1, RAD, R(0), R(90));
  cairo_stroke ( cairo_backend->cr );

  /* letf - top */
  cairo_set_source_rgba(cairo_backend->cr, 1.0, 1.0, 1.0, 0.5);
  cairo_move_to(cairo_backend->cr, x1p + 1, y2p - 1 - RAD);
  cairo_line_to(cairo_backend->cr, x1p + 1, y1p + 1 + RAD);
  cairo_move_to(cairo_backend->cr, x1p + 1 + RAD, y1p + 1);
  cairo_line_to(cairo_backend->cr, x2p - 1 - RAD, y1p + 1);
  cairo_stroke ( cairo_backend->cr );

  cairo_arc (cairo_backend->cr, x1p + RAD + 1, y1p + RAD + 1, RAD, R(180), R(270));
  cairo_stroke ( cairo_backend->cr );

  /* Handle state related painting */

  state = mb_kbd_keys_current_state(kbd);
  caps = mb_kbd_has_state(kbd, MBKeyboardStateCaps);

  if (caps)
    {
      if (mb_kdb_key_has_state (key, MBKeyboardKeyStateCaps))
        state = MBKeyboardKeyStateCaps;
      else if (mb_kbd_key_get_obey_caps(key))
        state = MBKeyboardKeyStateShifted;
    }

  if (!mb_kdb_key_has_state(key, state))
    {
      if (state == MBKeyboardKeyStateNormal)
	return;  /* keys should at least have a normal state */
      else
        state = MBKeyboardKeyStateNormal;
    }

  cairo_set_source_rgb(cairo_backend->cr, 0, 0, 0);

  if (mb_kbd_key_get_face_type(key, state) == MBKeyboardKeyFaceGlyph)
  {
    const char *face_str = mb_kbd_key_get_glyph_face(key, state);

    if (face_str)
      {
        double x1, y1;
	cairo_font_extents_t font_extents;
        cairo_text_extents_t text_extents;

        cairo_text_extents (cairo_backend->cr, face_str, &text_extents);
	cairo_font_extents (cairo_backend->cr, &font_extents);

        x1 = x + round (((w - text_extents.width) / 2.0) -
                        text_extents.x_bearing) - PAD;

        y1 = y + round ((h - font_extents.ascent -
                         font_extents.descent) / 2.0  + font_extents.ascent) -
          PAD;

        cairo_move_to(cairo_backend->cr, x1, y1);
        cairo_show_text (cairo_backend->cr, face_str);
      }
  }
  else if (mb_kbd_key_get_face_type(key, state) == MBKeyboardKeyFaceImage)
    {
      double x1, y1, w1, h1;
      cairo_surface_t *img;

      img = mb_kbd_key_get_image_face(key, state);

      w1 = cairo_image_surface_get_width (img);
      h1 = cairo_image_surface_get_height (img);

      x1 = mb_kbd_key_abs_x(key) + ((mb_kbd_key_width(key) - w1) / 2.0);
      y1 = mb_kbd_key_abs_y(key) + ((mb_kbd_key_height(key) - h1 ) / 2.0);

      cairo_set_source_surface (cairo_backend->cr, img, x1, y1);
      cairo_rectangle (cairo_backend->cr, x1, y1, w1, h1);
      cairo_fill (cairo_backend->cr);
    }

  if ( mb_kbd_key_is_held(kbd, key) )
    {
      cairo_set_source_rgba(cairo_backend->cr,
			    0,
			    0,
			    0,
			    0.2);

      cairo_rectangle (cairo_backend->cr,
                       x + 1 + PAD, y + 1 + PAD,
                       w - 2 - 2*PAD, h - 2 - 2*PAD);

      cairo_arc (cairo_backend->cr, x1p+1+RAD, y1p+1+RAD, RAD, R(180), R(270));
      cairo_arc (cairo_backend->cr, x2p-1-RAD, y1p+1+RAD, RAD, R(270), R(360));
      cairo_arc (cairo_backend->cr, x2p-1-RAD, y2p-1-RAD, RAD, R(0), R(90));
      cairo_arc (cairo_backend->cr, x1p+1+RAD, y2p-1-RAD, RAD, R(90), R(180));
      cairo_close_path (cairo_backend->cr);

      cairo_fill (cairo_backend->cr);
    }

  // cairo_show_page(cairo_backend->cr);
  // cairo_destroy (cairo_backend->cr);

}


void 	 /* FIXME: rename to clear backbuffer ? */
mb_kbd_ui_cairo_pre_redraw(MBKeyboardUI  *ui)
{
  MBKeyboardUIBackendCairo *cairo_backend = NULL;

  cairo_backend = (MBKeyboardUIBackendCairo*)mb_kbd_ui_backend(ui);

  cairo_set_source_rgb (cairo_backend->cr, 0.3255, 0.3255, 0.3255);

  cairo_rectangle( cairo_backend->cr,
                   0,
                   0,
		   mb_kbd_ui_x_win_width(ui),
		   mb_kbd_ui_x_win_height(ui));

  cairo_fill( cairo_backend->cr );

  // cairo_destroy( cairo_backend->cr );
}

static int
mb_kbd_ui_cairo_resources_create(MBKeyboardUI  *ui)
{
  MBKeyboardUIBackendCairo *cairo_backend = NULL;

  cairo_backend = (MBKeyboardUIBackendCairo*)mb_kbd_ui_backend(ui);

  /* switch now to 'real' drawable */
  cairo_xlib_surface_set_drawable (cairo_backend->surface,
				   mb_kbd_ui_backbuffer(ui),
				   mb_kbd_ui_x_win_width(ui),
				   mb_kbd_ui_x_win_height(ui));

  XFreePixmap(mb_kbd_ui_x_display(ui),  cairo_backend->foo_pxm);
  cairo_backend->foo_pxm = None;

  cairo_xlib_surface_set_size (cairo_backend->surface,
			       mb_kbd_ui_x_win_width(ui),
			       mb_kbd_ui_x_win_height(ui));

  return True;
}

static int
mb_kbd_ui_cairo_resize(MBKeyboardUI  *ui, int width, int height)
{
  MBKeyboardUIBackendCairo *cairo_backend = NULL;

  cairo_backend = (MBKeyboardUIBackendCairo*)mb_kbd_ui_backend(ui);

  if (cairo_backend->cr != NULL) /* may get called before initialised */
    {
      cairo_xlib_surface_set_size (cairo_get_target(cairo_backend->cr),
				   mb_kbd_ui_x_win_width(ui),
				   mb_kbd_ui_x_win_height(ui));

      cairo_xlib_surface_set_drawable (cairo_backend->surface,
				       mb_kbd_ui_backbuffer(ui),
				       mb_kbd_ui_x_win_width(ui),
				       mb_kbd_ui_x_win_height(ui));
      /*
      cairo_scale (cairo_get_target(cairo_backend->cr),
		   mb_kbd_ui_x_win_width(ui),
		   mb_kbd_ui_x_win_height(ui));
      */
    }

  return True;
}

MBKeyboardUIBackend*
mb_kbd_ui_cairo_init(MBKeyboardUI *ui)
{
  MBKeyboardUIBackendCairo *cairo_backend = NULL;

  cairo_backend = util_malloc0(sizeof(MBKeyboardUIBackendCairo));

  cairo_backend->backend.init             = mb_kbd_ui_cairo_init;
  cairo_backend->backend.font_load        = mb_kbd_ui_cairo_load_font;
  cairo_backend->backend.text_extents     = mb_kbd_ui_cairo_text_extents;
  cairo_backend->backend.redraw_key       = mb_kbd_ui_cairo_redraw_key;
  cairo_backend->backend.pre_redraw       = mb_kbd_ui_cairo_pre_redraw;
  cairo_backend->backend.resources_create = mb_kbd_ui_cairo_resources_create;
  cairo_backend->backend.resize           = mb_kbd_ui_cairo_resize;

  /* We need to make a temp surface so we can make cairo
   * cairo font calls ok with a context before ui knows
   * its window and therefore backbuffer sizes etc.
  */
  cairo_backend->foo_pxm = XCreatePixmap(mb_kbd_ui_x_display(ui),
					 mb_kbd_ui_x_win_root(ui),
					 10,
					 10,
					 DefaultDepth(mb_kbd_ui_x_display(ui),
						      mb_kbd_ui_x_screen(ui)));

  cairo_backend->surface
    = cairo_xlib_surface_create (mb_kbd_ui_x_display(ui),
				 cairo_backend->foo_pxm,
				 DefaultVisual(mb_kbd_ui_x_display(ui),
					       mb_kbd_ui_x_screen(ui)),
				 10, 10);

  cairo_backend->cr = cairo_create (cairo_backend->surface);

  cairo_reference(cairo_backend->cr);

  return (MBKeyboardUIBackend*)cairo_backend;
}

void
mb_kbd_ui_cairo_destroy (MBKeyboardUI *ui)
{
  MBKeyboardUIBackend *backend = mb_kbd_ui_backend (ui);
  MBKeyboardUIBackendCairo *cairo_backend = (MBKeyboardUIBackendCairo*)backend;

  if (cairo_backend->foo_pxm)
    XFreePixmap (mb_kbd_ui_x_display (ui), cairo_backend->foo_pxm);

  cairo_destroy (cairo_backend->cr);

  free (cairo_backend);
}
