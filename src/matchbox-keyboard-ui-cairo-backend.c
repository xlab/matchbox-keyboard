/*
 *  Matchbox Keyboard - A lightweight software keyboard.
 *
 *  Authored By Matthew Allum <mallum@o-hand.com>
 *
 *  Copyright (c) 2005-2012 Intel Corp
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
  cairo_text_extents_t      extents;

  cairo_backend = (MBKeyboardUIBackendCairo*)mb_kbd_ui_backend(ui);

  cairo_text_extents (cairo_backend->cr, str, &extents);

  *width  = extents.width;
  *height = extents.height;
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

  pixel_size = (double)kb->font_pt_size / ( (double)mm_per_pixel * 0.03 * 72 );
  
  cairo_set_font_size (cairo_backend->cr, pixel_size);
  
  return 1;
}


void
mb_kbd_ui_cairo_redraw_key(MBKeyboardUI  *ui, MBKeyboardKey *key)
{
  MBKeyboardUIBackendCairo *cairo_backend = NULL;

  XRectangle             rect;
  MBKeyboardKeyStateType state;
  Display               *xdpy;
  int                    xscreen;
  Pixmap                 backbuffer;
  MBKeyboard            *kbd;
  cairo_pattern_t       *pat;

  if (mb_kbd_key_is_blank(key)) /* spacer */
    return;

  cairo_backend = (MBKeyboardUIBackendCairo*)mb_kbd_ui_backend(ui);

  xdpy        = mb_kbd_ui_x_display(ui);
  xscreen     = mb_kbd_ui_x_screen(ui);
  backbuffer  = mb_kbd_ui_backbuffer(ui);
  kbd         = mb_kbd_ui_kbd(ui);

  rect.x      = mb_kbd_key_abs_x(key); 
  rect.y      = mb_kbd_key_abs_y(key); 
  rect.width  = mb_kbd_key_width(key);       
  rect.height = mb_kbd_key_height(key);       


  /* clear it */

  cairo_set_line_width (cairo_backend->cr, 0.04);
  cairo_set_source_rgb(cairo_backend->cr, 0.9, 0.9, 0.8);

  cairo_stroke( cairo_backend->cr );

  pat = cairo_pattern_create_linear (0, rect.y, 0, rect.y + rect.height);

  cairo_pattern_add_color_stop_rgb (pat, 1, 0.2, 0.2, 0.2 );
  cairo_pattern_add_color_stop_rgb (pat, 0, 0.7, 0.7, 0.7 );

  cairo_set_source (cairo_backend->cr, pat);

  cairo_rectangle( cairo_backend->cr, 
		   rect.x, rect.y, 
		   rect.width, rect.height);

  cairo_fill (cairo_backend->cr);

  cairo_pattern_destroy (pat);


  /* border */

  cairo_move_to(cairo_backend->cr,
                rect.x + 1,
                (mb_kbd_key_abs_y(key) + mb_kbd_key_height(key)) - 1);

  cairo_rel_line_to(cairo_backend->cr,
                    rect.width - 2,
                    0);

  cairo_rel_line_to(cairo_backend->cr,
                    0,
                    -rect.height);

  cairo_set_source_rgba(cairo_backend->cr, 0, 0, 0, 0.2);

  cairo_move_to(cairo_backend->cr,
                rect.x,
                rect.y + rect.height - 1);

  cairo_rel_line_to(cairo_backend->cr,
                     0,
                    - rect.height);

  cairo_rel_line_to(cairo_backend->cr,
                    rect.width - 1,
                    0);

  cairo_stroke ( cairo_backend->cr );

  cairo_set_source_rgb(cairo_backend->cr, 0, 0, 0);

  cairo_set_line_width (cairo_backend->cr, 1);

  cairo_rectangle( cairo_backend->cr,
                   rect.x, rect.y, rect.width, rect.height);

  cairo_stroke(cairo_backend->cr);

  /* Handle state related painting */

  state = mb_kbd_keys_current_state(kbd); 

  if (mb_kbd_has_state(kbd, MBKeyboardStateCaps)
      && mb_kbd_key_get_obey_caps(key))
    state = MBKeyboardKeyStateShifted;

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
        double x, y;
	int    face_width, face_height;
	cairo_font_extents_t font_extents;

	/* FIXME: Below is borked */
	mb_kbd_ui_cairo_text_extents (ui, face_str, &face_width, &face_height);

	cairo_font_extents (cairo_backend->cr, &font_extents);
	
	DBG(" %i - %i = %i", 
	    rect.width, face_width,
	    (rect.width - face_width) );

        x = rect.x + ( (rect.width - face_width) / 2.0);

        y = rect.y + ( (rect.height - (font_extents.ascent + font_extents.descent)) / 2.0 ) + font_extents.ascent;

	/* DBG("draw text to %d,%d", __func__, x, y); */

        cairo_move_to(cairo_backend->cr, x, y);
        cairo_show_text (cairo_backend->cr, face_str);

      }
  }

  if ( mb_kbd_key_is_held(kbd, key) )
    {
      cairo_set_source_rgba(cairo_backend->cr,
			    0,
			    0,
			    0,
			    0.2);
      
      cairo_rectangle(cairo_backend->cr,
		      rect.x,
		      rect.y,
		      rect.width,
		      rect.height );
      
      cairo_fill( cairo_backend->cr );
    }

  // cairo_show_page(cairo_backend->cr);
  // cairo_destroy (cairo_backend->cr);

}


void 	 /* FIXME: rename to clear backbuffer ? */
mb_kbd_ui_cairo_pre_redraw(MBKeyboardUI  *ui)
{
  MBKeyboardUIBackendCairo *cairo_backend = NULL;

  cairo_backend = (MBKeyboardUIBackendCairo*)mb_kbd_ui_backend(ui);

  cairo_set_source_rgb (cairo_backend->cr, 0.95, 0.95, 0.95);

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

