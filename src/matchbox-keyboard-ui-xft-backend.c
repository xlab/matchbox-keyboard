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

typedef struct MBKeyboardUIBackendXft
{
  MBKeyboardUIBackend backend;

  XftFont            *font;
  XftColor            font_col;
  XftDraw            *xft_backbuffer;
  GC                  xgc;

  /* Our theme */

  XColor xcol_c5c5c5, xcol_d3d3d3, xcol_f0f0f0, xcol_f8f8f5,
    xcol_f4f4f4, xcol_a4a4a4;

} MBKeyboardUIBackendXft;

static void
mb_kbd_ui_xft_text_extents (MBKeyboardUI        *ui,
			    const char          *str,
			    int                 *width,
			    int                 *height)
{
  XGlyphInfo  extents;
  MBKeyboardUIBackendXft *xft_backend = NULL;

  xft_backend = (MBKeyboardUIBackendXft*)mb_kbd_ui_backend(ui);

  XftTextExtentsUtf8(mb_kbd_ui_x_display(ui),
		     xft_backend->font,
		     (unsigned char*)str,
		     strlen(str),
		     &extents);

  *width  = extents.width;
  /* *height = extents.height; */
  *height = xft_backend->font->ascent + xft_backend->font->descent;
}

void
alloc_color(MBKeyboardUI  *ui, XColor *xcol, char *spec)
{
  int           result;

  if (spec[0] != '#')
    return;

  if ( sscanf (spec+1, "%x", &result))
    {
      xcol->red   = ((result >> 16) & 0xff) << 8;
      xcol->green = ((result >> 8) & 0xff)  << 8;
      xcol->blue  = (result & 0xff) << 8;
      xcol->flags = DoRed|DoGreen|DoBlue;

      XAllocColor(mb_kbd_ui_x_display(ui),
		  DefaultColormap(mb_kbd_ui_x_display(ui),
				  mb_kbd_ui_x_screen(ui)),
		  xcol);
    }

  return;
}

static int
mb_kbd_ui_xft_load_font(MBKeyboardUI *ui)
{
  MBKeyboard *kb = NULL;
  char desc[512];
  MBKeyboardUIBackendXft *xft_backend = NULL;

  xft_backend = (MBKeyboardUIBackendXft*)mb_kbd_ui_backend(ui);
  kb          = mb_kbd_ui_kbd(ui);

  /* load_font */

  snprintf(desc, 512, "%s-%i:%s",
	   kb->font_family, kb->font_pt_size, kb->font_variant);

  if (xft_backend->font != NULL)
    XftFontClose(mb_kbd_ui_x_display(ui), xft_backend->font);

  if ((xft_backend->font = XftFontOpenName(mb_kbd_ui_x_display(ui),
					   mb_kbd_ui_x_screen(ui),
					   desc)) == NULL)
    return 0;

  return 1;
}


void
mb_kbd_ui_xft_redraw_key(MBKeyboardUI  *ui, MBKeyboardKey *key)
{
  MBKeyboardUIBackendXft *xft_backend = NULL;
  XRectangle             rect;
  MBKeyboardKeyStateType state;
  int                    side_pad;
  Display               *xdpy;
  int                    xscreen;
  Pixmap                 backbuffer;
  MBKeyboard            *kbd;
  boolean                caps;

  if (mb_kbd_key_is_blank(key)) /* spacer */
    return;

  xft_backend = (MBKeyboardUIBackendXft*)mb_kbd_ui_backend(ui);
  xdpy        = mb_kbd_ui_x_display(ui);
  xscreen     = mb_kbd_ui_x_screen(ui);
  backbuffer  = mb_kbd_ui_backbuffer(ui);
  kbd         = mb_kbd_ui_kbd(ui);


  rect.x      = mb_kbd_key_abs_x(key);
  rect.y      = mb_kbd_key_abs_y(key);
  rect.width  = mb_kbd_key_width(key);
  rect.height = mb_kbd_key_height(key);

  /* Hacky clip to work around issues with off by ones in layout code :( */

  if (rect.x + rect.width >= mb_kbd_ui_x_win_width(ui))
    rect.width  = mb_kbd_ui_x_win_width(ui) - rect.x - 1;

  if (rect.y + rect.height >= mb_kbd_ui_x_win_height(ui))
    rect.height  = mb_kbd_ui_x_win_height(ui) - rect.y - 1;

  /* clear it */

  XSetForeground(xdpy, xft_backend->xgc, WhitePixel(xdpy, xscreen));

  XFillRectangles(xdpy, backbuffer, xft_backend->xgc, &rect, 1);

  /* draw 'main border' */

  XSetForeground(xdpy, xft_backend->xgc, xft_backend->xcol_c5c5c5.pixel);

  XDrawRectangles(xdpy, backbuffer, xft_backend->xgc, &rect, 1);

  /* shaded bottom line */

  XSetForeground(xdpy, xft_backend->xgc, xft_backend->xcol_f4f4f4.pixel);
  XDrawLine(xdpy, backbuffer, xft_backend->xgc,
	    rect.x + 1,
	    rect.y + rect.height - 1,
	    rect.x + rect.width -2 ,
	    rect.y + rect.height - 1);

  /* Corners - XXX should really use drawpoints */

  XSetForeground(xdpy, xft_backend->xgc, xft_backend->xcol_f0f0f0.pixel);

  XDrawPoint(xdpy, backbuffer, xft_backend->xgc, rect.x, rect.y);
  XDrawPoint(xdpy, backbuffer, xft_backend->xgc, rect.x+rect.width, rect.y);
  XDrawPoint(xdpy, backbuffer, xft_backend->xgc, rect.x+rect.width, rect.y+rect.height);
  XDrawPoint(xdpy, backbuffer, xft_backend->xgc, rect.x, rect.y+rect.height);

  /* soften them more */

  XSetForeground(xdpy, xft_backend->xgc, xft_backend->xcol_d3d3d3.pixel);

  XDrawPoint(xdpy, backbuffer, xft_backend->xgc, rect.x+1, rect.y);
  XDrawPoint(xdpy, backbuffer, xft_backend->xgc, rect.x, rect.y+1);

  XDrawPoint(xdpy, backbuffer, xft_backend->xgc, rect.x+rect.width-1, rect.y);
  XDrawPoint(xdpy, backbuffer, xft_backend->xgc, rect.x+rect.width, rect.y+1);

  XDrawPoint(xdpy, backbuffer, xft_backend->xgc, rect.x+rect.width-1, rect.y+rect.height);
  XDrawPoint(xdpy, backbuffer, xft_backend->xgc, rect.x+rect.width, rect.y+rect.height-1);

  XDrawPoint(xdpy, backbuffer, xft_backend->xgc, rect.x, rect.y+rect.height-1);
  XDrawPoint(xdpy, backbuffer, xft_backend->xgc, rect.x+1, rect.y+rect.height);

  /* background */

  if (mb_kbd_key_is_held(kbd, key))
    XSetForeground(xdpy, xft_backend->xgc, xft_backend->xcol_a4a4a4.pixel);
  else
    XSetForeground(xdpy, xft_backend->xgc, xft_backend->xcol_f8f8f5.pixel);

  side_pad =
    mb_kbd_keys_border(kbd)
    + mb_kbd_keys_margin(kbd)
    + mb_kbd_keys_pad(kbd);

  /* Why does below need +1's ? */
  XFillRectangle(xdpy, backbuffer, xft_backend->xgc,
		 rect.x + side_pad,
		 rect.y + side_pad,
		 rect.width  - (side_pad * 2) + 1,
		 rect.height - (side_pad * 2) + 1);

  /* real code is here */

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

  if (mb_kbd_key_get_face_type(key, state) == MBKeyboardKeyFaceGlyph)
    {
      const char *face_str = mb_kbd_key_get_glyph_face(key, state);
      int         face_str_w, face_str_h;

      if (face_str)
	{
	  int x, y;

	  mb_kbd_ui_xft_text_extents(ui, face_str, &face_str_w, &face_str_h);

	  x = mb_kbd_key_abs_x(key) + ((mb_kbd_key_width(key) - face_str_w)/2);

	  y = mb_kbd_key_abs_y(key) +
	    ( (mb_kbd_key_height(key)
                 - (xft_backend->font->ascent + xft_backend->font->descent))
	                             / 2 );

	  XftDrawStringUtf8(xft_backend->xft_backbuffer,
			    &xft_backend->font_col,
			    xft_backend->font,
			    x,
			    y + xft_backend->font->ascent,
			    (unsigned char*)face_str,
			    strlen(face_str));
	}
    }
  else if (mb_kbd_key_get_face_type(key, state) == MBKeyboardKeyFaceImage)
    {
      int x, y, w, h;
      MBKeyboardImage *img;

      img = mb_kbd_key_get_image_face(key, state);

      w = mb_kbd_image_width (img);
      h = mb_kbd_image_height (img);

      x = mb_kbd_key_abs_x(key) + ((mb_kbd_key_width(key) - w) / 2);
      y = mb_kbd_key_abs_y(key) + ((mb_kbd_key_height(key) - h ) / 2);


      XRenderComposite(xdpy,
		       PictOpOver,
		       mb_kbd_image_render_picture (img),
		       None,
		       XftDrawPicture (xft_backend->xft_backbuffer),
		       0, 0, 0, 0, x, y, w, h);
    }
}

void
mb_kbd_ui_xft_pre_redraw(MBKeyboardUI  *ui)
{
  MBKeyboardUIBackendXft *xft_backend = NULL;

  xft_backend = (MBKeyboardUIBackendXft*)mb_kbd_ui_backend(ui);

  /* Background */
  XSetForeground(mb_kbd_ui_x_display(ui),
		 xft_backend->xgc, xft_backend->xcol_f4f4f4.pixel);

  XFillRectangle(mb_kbd_ui_x_display(ui),
		 mb_kbd_ui_backbuffer(ui),
		 xft_backend->xgc,
		 0, 0,
		 mb_kbd_ui_x_win_width(ui),
		 mb_kbd_ui_x_win_height(ui));

  XSetForeground(mb_kbd_ui_x_display(ui),
		 xft_backend->xgc,
		 BlackPixel(mb_kbd_ui_x_display(ui),
			    mb_kbd_ui_x_screen(ui)));


}

static int
mb_kbd_ui_xft_resources_create(MBKeyboardUI  *ui)
{
  MBKeyboardUIBackendXft *xft_backend = NULL;
  XRenderColor            coltmp;

  xft_backend = (MBKeyboardUIBackendXft*)mb_kbd_ui_backend(ui);

  xft_backend->xft_backbuffer = XftDrawCreate(mb_kbd_ui_x_display(ui),
					      mb_kbd_ui_backbuffer(ui),
					      DefaultVisual(mb_kbd_ui_x_display(ui),
							    mb_kbd_ui_x_screen(ui)),
					      DefaultColormap(mb_kbd_ui_x_display(ui),
							      mb_kbd_ui_x_screen(ui)));


  coltmp.red   = coltmp.green = coltmp.blue  = 0x0000; coltmp.alpha = 0xcccc;

  XftColorAllocValue(mb_kbd_ui_x_display(ui),
		     DefaultVisual(mb_kbd_ui_x_display(ui),
				   mb_kbd_ui_x_screen(ui)),
		     DefaultColormap(mb_kbd_ui_x_display(ui),
				     mb_kbd_ui_x_screen(ui)),
		     &coltmp,
		     &xft_backend->font_col);

  xft_backend->xgc = XCreateGC(mb_kbd_ui_x_display(ui),
			       mb_kbd_ui_x_win(ui), 0, NULL);

  XSetForeground(mb_kbd_ui_x_display(ui),
		 xft_backend->xgc,
		 BlackPixel(mb_kbd_ui_x_display(ui), mb_kbd_ui_x_screen(ui)));

  XSetBackground(mb_kbd_ui_x_display(ui),
		 xft_backend->xgc,
		 WhitePixel(mb_kbd_ui_x_display(ui), mb_kbd_ui_x_screen(ui)));

  /* Crusty theme stuff  */

  alloc_color(ui, &xft_backend->xcol_c5c5c5, "#c5c5c5");
  alloc_color(ui, &xft_backend->xcol_d3d3d3, "#d3d3d3");
  alloc_color(ui, &xft_backend->xcol_f0f0f0, "#f0f0f0");
  alloc_color(ui, &xft_backend->xcol_f8f8f5, "#f8f8f5");
  alloc_color(ui, &xft_backend->xcol_f4f4f4, "#f4f4f4");
  alloc_color(ui, &xft_backend->xcol_a4a4a4, "#a4a4a4");

  return True;
}

static int
mb_kbd_ui_xft_resize(MBKeyboardUI  *ui, int width, int height)
{
  MBKeyboardUIBackendXft *xft_backend = NULL;

  xft_backend = (MBKeyboardUIBackendXft*)mb_kbd_ui_backend(ui);

  XftDrawChange (xft_backend->xft_backbuffer, mb_kbd_ui_backbuffer(ui));

  return True;
}

MBKeyboardUIBackend*
mb_kbd_ui_xft_init(MBKeyboardUI *ui)
{
  MBKeyboardUIBackendXft *xft_backend = NULL;

  xft_backend = util_malloc0(sizeof(MBKeyboardUIBackendXft));

  xft_backend->backend.init             = mb_kbd_ui_xft_init;
  xft_backend->backend.font_load        = mb_kbd_ui_xft_load_font;
  xft_backend->backend.text_extents     = mb_kbd_ui_xft_text_extents;
  xft_backend->backend.redraw_key       = mb_kbd_ui_xft_redraw_key;
  xft_backend->backend.pre_redraw       = mb_kbd_ui_xft_pre_redraw;
  xft_backend->backend.resources_create = mb_kbd_ui_xft_resources_create;
  xft_backend->backend.resize           = mb_kbd_ui_xft_resize;

  return (MBKeyboardUIBackend*)xft_backend;
}

void
mb_kbd_ui_xft_destroy (MBKeyboardUI *ui)
{
  MBKeyboardUIBackend *backend = mb_kbd_ui_backend (ui);
  MBKeyboardUIBackendXft *xft_backend = (MBKeyboardUIBackendXft*)backend;

  free (xft_backend);
}
