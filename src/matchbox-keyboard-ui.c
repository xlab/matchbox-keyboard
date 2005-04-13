#include "matchbox-keyboard.h"

#include <X11/Xft/Xft.h>

#define PROP_MOTIF_WM_HINTS_ELEMENTS    5
#define MWM_HINTS_DECORATIONS          (1L << 1)
#define MWM_DECOR_BORDER               (1L << 1)

typedef struct
{
  unsigned long       flags;
  unsigned long       functions;
  unsigned long       decorations;
  long                inputMode;
  unsigned long       status;
} 
PropMotifWmHints;

struct MBKeyboardUI
{
  Display      *xdpy;
  int           xscreen;
  Window        xwin_root, xwin;


  XftFont      *font;
  XftColor      font_col; 

  int           xwin_width;
  int           xwin_height;

  XftDraw      *xft_backbuffer;  
  Pixmap        backbuffer;
  GC            xgc;

  FakeKey      *fakekey;

  MBKeyboard   *kbd;
};

static void 
text_extents(MBKeyboardUI        *ui, 
	     const unsigned char *str, 
	     int                 *width, 
	     int                 *height)
{
  XGlyphInfo  extents;
  
  XftTextExtentsUtf8(ui->xdpy, 
		     ui->font,
		     str, 
		     strlen(str),
		     &extents);

  *width  = extents.width;
  /* *height = extents.height; */
  *height = ui->font->ascent + ui->font->descent;

}


void
mb_kbd_ui_send_press(MBKeyboardUI  *ui,
		     unsigned char *utf8_char_in,
		     int            modifiers)
{
  DBG("Sending '%s'", utf8_char_in);
  fakekey_press(ui->fakekey, utf8_char_in, -1, 0);
  fakekey_release(ui->fakekey);
}

static void
mb_kdb_ui_unit_key_size(MBKeyboardUI *ui, int *width, int *height)
{
  MBKeyboardLayout       *layout;
  List                   *row_item, *key_item;
  MBKeyboardKeyStateType  state;
  const unsigned char    *face_str;

  *width = 0; *height = 0;

  layout   = mb_kbd_get_selected_layout(ui->kbd);
  row_item = mb_kbd_layout_rows(layout);

  while (row_item != NULL)
    {
      key_item = mb_kdb_row_keys((MBKeyboardRow *)row_item->data);

      while (key_item != NULL)
	{
	  MBKeyboardKey *key = key_item->data;

	  mb_kdb_key_foreach_state(key, state)
	    {
	      if (mb_kbd_key_get_face_type(key, state) == MBKeyboardKeyFaceGlyph)
		{
		  face_str = mb_kbd_key_get_glyph_face(key, state);

		  if (util_utf8_char_cnt(face_str) == 1)
		    {
		      int str_w =0, str_h = 0;

		      text_extents(ui, face_str, &str_w, &str_h);
		      
		      if (str_w > *width) *width = str_w;
		      if (str_h > *height) *height = str_h;

		    }
		}

	      /* XXX TODO, also need to check height of image keys etc */

	    }
	  key_item = util_list_next(key_item);
	}
      row_item = util_list_next(row_item);
    }
}

static void
mb_kbd_ui_min_key_size(MBKeyboardUI  *ui,
		       MBKeyboardKey *key,
		       int           *width,
		       int           *height)
{
  const unsigned char *face_str = NULL;
  int                  max_w = 0, max_h = 0, state;

  mb_kdb_key_foreach_state(key, state)
    {
      if (mb_kbd_key_get_face_type(key, state) == MBKeyboardKeyFaceGlyph)
	{
	  
	  if (mb_kbd_key_get_face_type(key, state) == MBKeyboardKeyFaceGlyph)
	    {
	      
	      face_str = mb_kbd_key_get_glyph_face(key, state);

	      text_extents(ui, face_str, width, height);

	      if (*width < max_w) *width = max_w;
	      if (*height < max_h) *height = max_h;
	    }
	}
      /* XXX else, images etc */
    }

}

void
mb_kbd_ui_allocate_ui_layout(MBKeyboardUI *ui,
			     int          *width,
			     int          *height)
{
  MBKeyboardLayout *layout;
  List             *row_item, *key_item;
  int               ukey_width, ukey_height;
  int               key_y = 0, key_x = 0; 
  int               row_y, max_row_key_height, max_row_width;

  layout = mb_kbd_get_selected_layout(ui->kbd);

  /* Do an initial run to figure out a 'base' size for 
   * single glyph keys 
  */
  mb_kdb_ui_unit_key_size(ui, &ukey_width, &ukey_height);

  row_item = mb_kbd_layout_rows(layout);

  row_y = mb_kbd_row_spacing(ui->kbd); 

  max_row_width = 0;

  while (row_item != NULL)
    {
      MBKeyboardRow *row = row_item->data;
      
      key_item = mb_kdb_row_keys(row);

      key_x = mb_kbd_col_spacing(ui->kbd);

      max_row_key_height = 0;

      while (key_item != NULL)
	{
	  int            key_w, key_h;          
	  MBKeyboardKey *key = key_item->data;

	  mb_kbd_ui_min_key_size(ui, key, &key_w, &key_h);

	  if (key_w < ukey_width)  key_w = ukey_width;
	  if (key_h < ukey_height) key_h = ukey_height;

	  key_y = 0;

	  key_w += 2 * ( mb_kbd_keys_border(ui->kbd) 
			 + mb_kbd_keys_margin(ui->kbd) 
			 + mb_kbd_keys_pad(ui->kbd) );

	  key_h += 2 * ( mb_kbd_keys_border(ui->kbd) 
			 + mb_kbd_keys_margin(ui->kbd) 
			 + mb_kbd_keys_pad(ui->kbd) );
	  
	  if (key_h > max_row_key_height)
	    max_row_key_height = key_h;

	  mb_kbd_key_set_geometry(key, key_x, key_y, key_w, key_h); 

	  key_item = util_list_next(key_item);

	  key_x += (mb_kbd_col_spacing(ui->kbd) + key_w);
	}

      if (key_x > max_row_width) /* key_x now represents row width */
	max_row_width = key_x;

      mb_kbd_row_set_y(row, row_y);

      row_y += max_row_key_height + mb_kbd_row_spacing(ui->kbd);

      row_item = util_list_next(row_item);
    }

  *height = row_y; 

  /* Now center the rows */
  
  row_item = mb_kbd_layout_rows(layout);

  while (row_item != NULL)
    {
      MBKeyboardRow *row = row_item->data;

      mb_kbd_row_set_x(row, (max_row_width - mb_kbd_row_width(row))/2);

      row_item = util_list_next(row_item);
    }
  
  *width = max_row_width;

}


static void
mb_kbd_ui_redraw_key(MBKeyboardUI  *ui, MBKeyboardKey *key)
{
  XRectangle             rect;
  MBKeyboardKeyStateType state;

  rect.x      = mb_kbd_key_abs_x(key); 
  rect.y      = mb_kbd_key_abs_y(key); 
  rect.width  = mb_kbd_key_width(key);       
  rect.height = mb_kbd_key_height(key);       

  XDrawRectangles(ui->xdpy, ui->backbuffer, ui->xgc, &rect, 1);

  state = MBKeyboardKeyStateNormal;

  if (mb_kbd_key_get_face_type(key, state) == MBKeyboardKeyFaceGlyph)
  {
    const unsigned char *face_str = mb_kbd_key_get_glyph_face(key, state);
    int                  face_str_w, face_str_h;

    if (face_str)
      {
	int x, y;

	text_extents(ui, face_str, &face_str_w, &face_str_h);

	x = mb_kbd_key_abs_x(key) + ((mb_kbd_key_width(key) - face_str_w)/2);

	y = mb_kbd_key_abs_y(key) 
	  + mb_kbd_keys_border(ui->kbd) /* this is rect line */
	  + mb_kbd_keys_margin(ui->kbd) 
	  + mb_kbd_keys_pad(ui->kbd);
	
	DBG("painting '%s' to %ix%i", face_str, x, y);

	XftDrawStringUtf8(ui->xft_backbuffer,
			  &ui->font_col,
			  ui->font,
			  x,
			  y + ui->font->ascent,
			  face_str, 
			  strlen(face_str));
      }
  }

}


static void
mb_kbd_ui_redraw_row(MBKeyboardUI  *ui, MBKeyboardRow *row)
{
  List *key_item;

  key_item = mb_kdb_row_keys(row);

  while (key_item != NULL)
    {
      mb_kbd_ui_redraw_key(ui, key_item->data);

      key_item = util_list_next(key_item);
    }
}

static void
mb_kbd_ui_swap_buffers(MBKeyboardUI  *ui)
{
  XSetWindowBackgroundPixmap(ui->xdpy, ui->xwin, ui->backbuffer);
  XClearWindow(ui->xdpy, ui->xwin);
  XFlush(ui->xdpy);
}

static void
mb_kbd_ui_redraw(MBKeyboardUI  *ui)
{
  List             *row_item;
  MBKeyboardLayout *layout;

  XSetForeground(ui->xdpy, ui->xgc, WhitePixel(ui->xdpy, ui->xscreen ));
  XFillRectangle(ui->xdpy, ui->backbuffer, ui->xgc,
		 0, 0, ui->xwin_width, ui->xwin_height);
  XSetForeground(ui->xdpy, ui->xgc, BlackPixel(ui->xdpy, ui->xscreen ));

  layout = mb_kbd_get_selected_layout(ui->kbd);

  row_item = mb_kbd_layout_rows(layout);

  while (row_item != NULL)
    {
      MBKeyboardRow *row = row_item->data;

      mb_kbd_ui_redraw_row(ui, row);

      row_item = util_list_next(row_item);
    }

  mb_kbd_ui_swap_buffers(ui);
}

static void
mb_kbd_ui_show(MBKeyboardUI  *ui)
{
  XMapWindow(ui->xdpy, ui->xwin);
}
			  
static int
mb_kbd_ui_resources_create(MBKeyboardUI  *ui)
{
  PropMotifWmHints    *mwm_hints;
  XSizeHints           size_hints;
  XWMHints            *wm_hints;
  XSetWindowAttributes win_attr;

  XRenderColor      coltmp;

  /*
  ui->xwin = XCreateSimpleWindow(ui->xdpy,
				 ui->xwin_root,
				 0, 0,
				 ui->xwin_width, ui->xwin_height,
				 0, 
				 BlackPixel(ui->xdpy, ui->xscreen),
				 WhitePixel(ui->xdpy, ui->xscreen));
  */

  win_attr.override_redirect = True;
  win_attr.event_mask 
    = ButtonPressMask|ButtonReleaseMask|Button1MotionMask|StructureNotifyMask;

  ui->xwin = XCreateWindow(ui->xdpy,
			   ui->xwin_root,
			   0, 0,
			   ui->xwin_width, ui->xwin_height,
			   0,
			   CopyFromParent, CopyFromParent, CopyFromParent,
			   CWOverrideRedirect|CWEventMask,
			   &win_attr);


  wm_hints = XAllocWMHints();

  if (wm_hints)
    {
      DBG("setting no fous hint");
      wm_hints->input = False;
      wm_hints->flags = InputHint;
      XSetWMHints(ui->xdpy, ui->xwin, wm_hints );
      XFree(wm_hints);
    }

  ui->backbuffer = XCreatePixmap(ui->xdpy,
				 ui->xwin,
				 ui->xwin_width, 
				 ui->xwin_height,
				 DefaultDepth(ui->xdpy, ui->xscreen));

  ui->xft_backbuffer = XftDrawCreate(ui->xdpy,
				     ui->backbuffer,
				     DefaultVisual(ui->xdpy, ui->xscreen),
				     DefaultColormap(ui->xdpy, ui->xscreen));

  coltmp.red   = coltmp.green = coltmp.blue  = 0x0000; coltmp.alpha = 0xffff;

  XftColorAllocValue(ui->xdpy,
		     DefaultVisual(ui->xdpy, ui->xscreen), 
		     DefaultColormap(ui->xdpy, ui->xscreen),
		     &coltmp,
		     &ui->font_col);


  ui->xgc = XCreateGC(ui->xdpy, ui->xwin, 0, NULL);

  XSetForeground(ui->xdpy, ui->xgc, BlackPixel(ui->xdpy, ui->xscreen ));
  XSetBackground(ui->xdpy, ui->xgc, WhitePixel(ui->xdpy, ui->xscreen ));

  return 1;
}

int
mb_kbd_ui_events_iteration(MBKeyboardUI *ui)
{
  MBKeyboardKey *key = NULL;

  /* while ( XPending(ui->xdpy) ) */
  while (True)
      {
	XEvent xev;
	XNextEvent(ui->xdpy, &xev);

	switch (xev.type) 
	  {
	  case ButtonPress:
	    DBG("got button bress at %i,%i", xev.xbutton.x, xev.xbutton.y);
	    key = mb_kbd_locate_key(ui->kbd, xev.xbutton.x, xev.xbutton.y);
	    if (key)
	      {
		DBG("found key for press");
		mb_kbd_key_press(key);
	      }
	    break;
	  case ButtonRelease:
	    
	    break;

	  case ConfigureNotify:
	    /* XXX Handle resize */
	  default:
	    break;
	  }
      }
}


int
mb_kbd_ui_init(MBKeyboard *kbd)
{
  MBKeyboardUI     *ui = NULL;
  
  ui = kbd->ui = util_malloc0(sizeof(MBKeyboardUI));
  
  ui->kbd = kbd;

  if ((ui->xdpy = XOpenDisplay(getenv("DISPLAY"))) == NULL)
    return 0;

  if ((ui->fakekey = fakekey_init(ui->xdpy)) == NULL)
    return 0;

  ui->xscreen   = DefaultScreen(ui->xdpy);
  ui->xwin_root = RootWindow(ui->xdpy, ui->xscreen);   

  kbd->font_desc = "Sans-10"; 	/* HACK HACK HACK */

  if ((ui->font = XftFontOpenName(ui->xdpy, 
				  ui->xscreen, 
				  kbd->font_desc)) == NULL)
    return 0;

  mb_kbd_ui_allocate_ui_layout(ui, &ui->xwin_width, &ui->xwin_height);

  mb_kbd_ui_resources_create(ui);

  mb_kbd_ui_redraw(ui);

  mb_kbd_ui_show(ui);

  mb_kbd_ui_events_iteration(ui);

  return 1;
}
