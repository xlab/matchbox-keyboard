/* 
 *  Matchbox Keyboard - A lightweight software keyboard.
 *
 *  Authored By Matthew Allum <mallum@o-hand.com>
 *
 *  Copyright (c) 2005 OpenedHand Ltd - http://o-hand.com
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

#include "matchbox-keyboard.h"
#include <X11/extensions/Xrender.h>

struct MBKeyboardImage
{
  MBKeyboard            *kbd;
  int                    width, height;
  Pixmap                 xdraw;
  Picture                xpic;
};

static unsigned char* 
png_file_load (const char *file, 
	       int        *width, 
	       int        *height)
{
  FILE *fd;
  unsigned char *data;
  unsigned char header[8];
  int  bit_depth, color_type;

  png_uint_32  png_width, png_height, i, rowbytes;
  png_structp png_ptr;
  png_infop info_ptr;
  png_bytep *row_pointers;

  if ((fd = fopen( file, "rb" )) == NULL) return NULL;

  fread( header, 1, 8, fd );
  if ( ! png_check_sig( header, 8 ) ) 
    {
      fclose(fd);
      return NULL;
    }

  png_ptr = png_create_read_struct( PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
  if ( ! png_ptr ) {
    fclose(fd);
    return NULL;
  }

  info_ptr = png_create_info_struct(png_ptr);
  if ( ! info_ptr ) {
    png_destroy_read_struct( &png_ptr, (png_infopp)NULL, (png_infopp)NULL);
    fclose(fd);
    return NULL;
  }

  if ( setjmp( png_ptr->jmpbuf ) ) {
    png_destroy_read_struct( &png_ptr, &info_ptr, NULL);
    fclose(fd);
    return NULL;
  }

  png_init_io( png_ptr, fd );
  png_set_sig_bytes( png_ptr, 8);
  png_read_info( png_ptr, info_ptr);
  png_get_IHDR( png_ptr, info_ptr, &png_width, &png_height, &bit_depth, 
		&color_type, NULL, NULL, NULL);
  *width = (int) png_width;
  *height = (int) png_height;

  if ( bit_depth == 16 )
    png_set_strip_16(png_ptr);

  if (bit_depth < 8)
    png_set_packing(png_ptr);

  if (( color_type == PNG_COLOR_TYPE_GRAY ) ||
            ( color_type == PNG_COLOR_TYPE_GRAY_ALPHA ))
    png_set_gray_to_rgb(png_ptr);

  /* Add alpha */
  if (( color_type == PNG_COLOR_TYPE_GRAY ) ||
      ( color_type == PNG_COLOR_TYPE_RGB ))
    png_set_add_alpha(png_ptr, 0xff, PNG_FILLER_AFTER); /* req 1.2.7 */

  if (( color_type == PNG_COLOR_TYPE_PALETTE )||
      ( png_get_valid( png_ptr, info_ptr, PNG_INFO_tRNS )))
    png_set_expand(png_ptr);

  png_read_update_info( png_ptr, info_ptr);

  /* allocate space for data and row pointers */
  rowbytes = png_get_rowbytes( png_ptr, info_ptr);
  data = (unsigned char *) malloc( (rowbytes*(*height + 1)));
  row_pointers = (png_bytep *) malloc( (*height)*sizeof(png_bytep));

  if (( data == NULL )||( row_pointers == NULL )) {
    png_destroy_read_struct( &png_ptr, &info_ptr, NULL);
    free(data);
    free(row_pointers);
    return NULL;
  }

  for ( i = 0;  i < *height; i++ )
    row_pointers[i] = data + i*rowbytes;

  png_read_image( png_ptr, row_pointers );
  png_read_end( png_ptr, NULL);

  free(row_pointers);
  png_destroy_read_struct( &png_ptr, &info_ptr, NULL);
  fclose(fd);

  return data;
}

MBKeyboardImage*
mb_kbd_image_new (MBKeyboard *kbd, const char *filename)
{
  MBKeyboardUI            *ui;
  MBKeyboardImage         *img;
  unsigned char           *data, *p;
  int                      width, height, x, y;
  XRenderPictFormat       *ren_fmt;
  XRenderPictureAttributes ren_attr;
  GC                       gc;
  XImage                  *ximg;

  ui = kbd->ui;
  
  data = png_file_load (filename, &width, &height);

  if (data == NULL || width == 0 || height == 0)
    {
      if (data) free(data);
      return NULL;
    }

  img = util_malloc0(sizeof(MBKeyboardImage));

  img->width  = width;
  img->height = height;

  ren_fmt = XRenderFindStandardFormat(mb_kbd_ui_x_display(ui), 
				      PictStandardARGB32);
  img->xdraw = XCreatePixmap(mb_kbd_ui_x_display(ui), 
			     mb_kbd_ui_x_win_root(ui),
			     width, height, 
			     ren_fmt->depth);

  XSync(mb_kbd_ui_x_display(ui), False);

  ren_attr.dither          = True;
  ren_attr.component_alpha = True;
  ren_attr.repeat          = False;

  img->xpic = XRenderCreatePicture(mb_kbd_ui_x_display(ui), 
				   img->xdraw, 
				   ren_fmt, 
				   CPRepeat|CPDither|CPComponentAlpha, 
				   &ren_attr);

  gc = XCreateGC(mb_kbd_ui_x_display(ui), img->xdraw, 0, NULL);

  ximg = XCreateImage(mb_kbd_ui_x_display(ui), 
		      DefaultVisual(mb_kbd_ui_x_display(ui), 
				    mb_kbd_ui_x_screen(ui)), 
		      ren_fmt->depth, 
		      ZPixmap, 
		      0, 
		      NULL, 
		      width, 
		      height, 
		      32, 
		      0);
  
  ximg->data = malloc(ximg->bytes_per_line * ximg->height);

  p = data;

  for (y = 0; y < height; y++)
    for (x = 0; x < width; x++)
      {
	unsigned char a, r, g, b;
	r = *p++; g = *p++; b = *p++; a = *p++; 
	r = (r * (a + 1)) / 256; /* premult */
	g = (g * (a + 1)) / 256;
	b = (b * (a + 1)) / 256;
	XPutPixel(ximg, x, y, (a << 24) | (r << 16) | (g << 8) | b);
      }

  XPutImage(mb_kbd_ui_x_display(ui), 
	    img->xdraw, 
	    gc, 
	    ximg, 
	    0, 0, 0, 0, width, height);

  free(ximg->data);
  ximg->data = NULL;
  XDestroyImage(ximg);
  XFreeGC (mb_kbd_ui_x_display(ui), gc);

  free(data);

  return img;
}

int
mb_kbd_image_width (MBKeyboardImage *img)
{
  return img->width;
}

int
mb_kbd_image_height (MBKeyboardImage *img)
{
  return img->height;
}

Picture
mb_kbd_image_render_picture (MBKeyboardImage *img)
{
  return img->xpic;
}

void
mb_kbd_image_destroy (MBKeyboardImage *img)
{
  /* ... */
}


