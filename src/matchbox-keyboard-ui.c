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

  int           dpy_width;
  int           dpy_height;

  XftFont      *font;
  XftColor      font_col; 

  int           xwin_width;
  int           xwin_height;

  XftDraw      *xft_backbuffer;  
  Pixmap        backbuffer;
  GC            xgc;

  /* Crusty temp theme stuff */

  XColor xcol_c5c5c5, xcol_d3d3d3, xcol_f0f0f0, xcol_f8f8f5, 
    xcol_f4f4f4, xcol_a4a4a4;

  /************************* */

  int           key_uwidth, key_uheight;

  int           base_alloc_width, base_alloc_height;
  int           base_font_pt_size;

  FakeKey      *fakekey;

  MBKeyboard   *kbd;
};

static void
mb_kbd_ui_resize(MBKeyboardUI *ui, int width, int height);

static int
mb_kbd_ui_load_font(MBKeyboardUI *ui);

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

      XAllocColor(ui->xdpy, DefaultColormap(ui->xdpy, ui->xscreen), xcol);
    }
  
  return;
}

static unsigned char*
get_current_window_manager_name (MBKeyboardUI  *ui)
{
  Atom           atom_utf8_string, atom_wm_name, atom_check, type;
  int            result, format;
  unsigned char *val, *retval;
  long           nitems, bytes_after;
  Window        *support_xwin = NULL;

  atom_check = XInternAtom (ui->xdpy, "_NET_SUPPORTING_WM_CHECK", False);

  XGetWindowProperty (ui->xdpy, 
		      RootWindow(ui->xdpy, ui->xscreen),
		      atom_check,
		      0, 16L, False, XA_WINDOW, &type, &format,
		      &nitems, &bytes_after, (unsigned char **)&support_xwin);

  if (support_xwin == NULL)
      return NULL;

  atom_utf8_string = XInternAtom (ui->xdpy, "UTF8_STRING", False);
  atom_wm_name     = XInternAtom (ui->xdpy, "_NET_WM_NAME", False);

  result = XGetWindowProperty (ui->xdpy, *support_xwin, atom_wm_name,
			       0, 1000L,False, atom_utf8_string,
			       &type, &format, &nitems,
			       &bytes_after, (unsigned char **)&val);
  if (result != Success)
    return NULL;

  if (type != atom_utf8_string || format !=8 || nitems == 0)
    {
      if (val) XFree (val);
      return NULL;
    }

  retval = strdup (val);

  XFree (val);

  return retval;
}

static boolean
get_desktop_area(MBKeyboardUI *ui, int *x, int *y, int *width, int *height)
{
  Atom           atom_area, type;
  int            result, format;
  long           nitems, bytes_after;
  int           *geometry = NULL;

  atom_area = XInternAtom (ui->xdpy, "_NET_WORKAREA", False);

  result = XGetWindowProperty (ui->xdpy, 
			       RootWindow(ui->xdpy, ui->xscreen),
			       atom_area,
			       0, 16L, False, XA_CARDINAL, &type, &format,
			       &nitems, &bytes_after, 
			       (unsigned char **)&geometry);

  if (result != Success || nitems < 4 || geometry == NULL)
    {
      if (geometry) XFree(geometry);
      return False;
    }

  if (x) *x           = geometry[0];
  if (y) *y           = geometry[1];
  if (width)  *width  = geometry[2];
  if (height) *height = geometry[3];
  
  XFree(geometry);

  return True;
}

void
mb_kbd_ui_send_press(MBKeyboardUI        *ui,
		     const unsigned char *utf8_char_in,
		     int                  modifiers)
{
  DBG("Sending '%s'", utf8_char_in);
  fakekey_press(ui->fakekey, utf8_char_in, -1, modifiers);
}

void
mb_kbd_ui_send_keysym_press(MBKeyboardUI  *ui,
			    KeySym         ks,
			    int            modifiers)
{
  fakekey_press_keysym(ui->fakekey, ks, modifiers);
}

void
mb_kbd_ui_send_release(MBKeyboardUI  *ui)
{
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

  if (mb_kbd_key_get_req_uwidth(key) || mb_kbd_key_is_blank(key))
    {
      *width = (ui->key_uwidth * mb_kbd_key_get_req_uwidth(key)) / 1000 ;
      *height = ui->key_uheight;
      return;
    }

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
  int               key_y = 0, key_x = 0; 
  int               row_y, max_row_key_height, max_row_width;

  layout = mb_kbd_get_selected_layout(ui->kbd);

  /* Do an initial run to figure out a 'base' size for 
   * single glyph keys 
  */
  mb_kdb_ui_unit_key_size(ui, &ui->key_uwidth, &ui->key_uheight);

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

	  if (!mb_kbd_key_get_req_uwidth(key)
	      && key_w < ui->key_uwidth)
	    key_w = ui->key_uwidth;

	  if (key_h < ui->key_uheight) 
	    key_h = ui->key_uheight;

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

  DBG("Handling fillers");

  row_item = mb_kbd_layout_rows(layout);

  /* handle any fillers */

  while (row_item != NULL)
    {
      MBKeyboardRow *row        = row_item->data;
      int            n_fillers  = 0, free_space = 0, new_w = 0;

      key_item = mb_kdb_row_keys(row);

      while (key_item != NULL)
	{
	  if (mb_kbd_key_get_fill(key_item->data))
	      n_fillers++;

	  key_item = util_list_next(key_item);
	}

      if (!n_fillers)
	goto next_row;

      key_item = mb_kdb_row_keys(row);

      free_space = max_row_width - mb_kbd_row_width(row);

      while (key_item != NULL)
	{
	  if (mb_kbd_key_get_fill(key_item->data))
	    {
	      int   old_w;
	      List *nudge_key_item = util_list_next(key_item);

	      old_w = mb_kbd_key_width(key_item->data);
	      new_w = old_w + (free_space/n_fillers);

	      mb_kbd_key_set_geometry(key_item->data, -1, -1, new_w, -1);

	      /* nudge next keys forward */
	      while (nudge_key_item)
		{
		  mb_kbd_key_set_geometry(nudge_key_item->data,
					  mb_kbd_key_x(nudge_key_item->data) + (new_w - old_w ), -1, -1, -1);
		  nudge_key_item = util_list_next(nudge_key_item);
		}
	    }
	  key_item = util_list_next(key_item);
	}

    next_row:

      row_item = util_list_next(row_item);
    }


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

/* 

  XColor        xcol;
#if defined (USE_XFT) || defined (USE_PANGO)
  XftColor      xftcol;


 xcol_c5c5c5
 xcol_d3d3d3
 xcol_f0f0f0

 xcol_f8f8f5 - key bg ( 2px white border )
 xcol_f4f4f4 - line on bottom of key

 #636262 - text

 */

void
mb_kbd_ui_redraw_key(MBKeyboardUI  *ui, MBKeyboardKey *key)
{
  XRectangle             rect;
  MBKeyboardKeyStateType state;
  int                    side_pad;

  if (mb_kbd_key_is_blank(key)) /* spacer */
    return;

  rect.x      = mb_kbd_key_abs_x(key); 
  rect.y      = mb_kbd_key_abs_y(key); 
  rect.width  = mb_kbd_key_width(key);       
  rect.height = mb_kbd_key_height(key);       

  /* clear it */

  XSetForeground(ui->xdpy, ui->xgc, WhitePixel(ui->xdpy, ui->xscreen ));

  XFillRectangles(ui->xdpy, ui->backbuffer, ui->xgc, &rect, 1);

  /* draw 'main border' */

  XSetForeground(ui->xdpy, ui->xgc, ui->xcol_c5c5c5.pixel);

  XDrawRectangles(ui->xdpy, ui->backbuffer, ui->xgc, &rect, 1);

  /* shaded bottom line */

  XSetForeground(ui->xdpy, ui->xgc, ui->xcol_f4f4f4.pixel);
  XDrawLine(ui->xdpy, ui->backbuffer, ui->xgc,
	    rect.x + 1,
	    rect.y + rect.height - 1,
	    rect.x + rect.width -2 ,
	    rect.y + rect.height - 1);

  /* Corners - XXX should really use drawpoints */

  XSetForeground(ui->xdpy, ui->xgc, ui->xcol_f0f0f0.pixel);

  XDrawPoint(ui->xdpy, ui->backbuffer, ui->xgc, rect.x, rect.y);
  XDrawPoint(ui->xdpy, ui->backbuffer, ui->xgc, rect.x+rect.width, rect.y);
  XDrawPoint(ui->xdpy, ui->backbuffer, ui->xgc, rect.x+rect.width, rect.y+rect.height);
  XDrawPoint(ui->xdpy, ui->backbuffer, ui->xgc, rect.x, rect.y+rect.height);

  /* soften them more */

  XSetForeground(ui->xdpy, ui->xgc, ui->xcol_d3d3d3.pixel);

  XDrawPoint(ui->xdpy, ui->backbuffer, ui->xgc, rect.x+1, rect.y);
  XDrawPoint(ui->xdpy, ui->backbuffer, ui->xgc, rect.x, rect.y+1);

  XDrawPoint(ui->xdpy, ui->backbuffer, ui->xgc, rect.x+rect.width-1, rect.y);
  XDrawPoint(ui->xdpy, ui->backbuffer, ui->xgc, rect.x+rect.width, rect.y+1);

  XDrawPoint(ui->xdpy, ui->backbuffer, ui->xgc, rect.x+rect.width-1, rect.y+rect.height);
  XDrawPoint(ui->xdpy, ui->backbuffer, ui->xgc, rect.x+rect.width, rect.y+rect.height-1);

  XDrawPoint(ui->xdpy, ui->backbuffer, ui->xgc, rect.x, rect.y+rect.height-1);
  XDrawPoint(ui->xdpy, ui->backbuffer, ui->xgc, rect.x+1, rect.y+rect.height);

  /* background */

  if (mb_kbd_key_is_held(ui->kbd, key))
    XSetForeground(ui->xdpy, ui->xgc, ui->xcol_a4a4a4.pixel);
  else
    XSetForeground(ui->xdpy, ui->xgc, ui->xcol_f8f8f5.pixel);

  side_pad = 
    mb_kbd_keys_border(ui->kbd)
    + mb_kbd_keys_margin(ui->kbd)
    + mb_kbd_keys_pad(ui->kbd);

  /* Why does below need +1's ? */
  XFillRectangle(ui->xdpy, ui->backbuffer, ui->xgc, 
		 rect.x + side_pad,
		 rect.y + side_pad,
		 rect.width  - (side_pad * 2) + 1,
		 rect.height - (side_pad * 2) + 1);

  /* real code is here */

  state = mb_kbd_keys_current_state(ui->kbd); 

  if (mb_kbd_has_state(ui->kbd, MBKeyboardStateCaps)
      && mb_kbd_key_get_obey_caps(key))
    state = MBKeyboardKeyStateShifted;

  if (!mb_kdb_key_has_state(key, state))
    {
      if (state == MBKeyboardKeyStateNormal)
	return;  /* keys should at least have a normal state */
      else
        state = MBKeyboardKeyStateNormal;
    }

  if (mb_kbd_key_get_face_type(key, state) == MBKeyboardKeyFaceGlyph)
  {
    const unsigned char *face_str = mb_kbd_key_get_glyph_face(key, state);
    int                  face_str_w, face_str_h;

    if (face_str)
      {
	int x, y;

	text_extents(ui, face_str, &face_str_w, &face_str_h);

	x = mb_kbd_key_abs_x(key) + ((mb_kbd_key_width(key) - face_str_w)/2);

	y = mb_kbd_key_abs_y(key) + 
	  ( (mb_kbd_key_height(key) - (ui->font->ascent + ui->font->descent))
	                                / 2 );

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

void
mb_kbd_ui_swap_buffers(MBKeyboardUI  *ui)
{
  XClearWindow(ui->xdpy, ui->xwin);
  XSync(ui->xdpy, False);
}

void
mb_kbd_ui_redraw(MBKeyboardUI  *ui)
{
  List             *row_item;
  MBKeyboardLayout *layout;

  /* Background */
  XSetForeground(ui->xdpy, ui->xgc, ui->xcol_f4f4f4.pixel);
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
  } PropMotifWmHints ;

  Atom /* atom_wm_protocols[3], */ 
    atom_NET_WM_WINDOW_TYPE, 
    atom_NET_WM_WINDOW_TYPE_TOOLBAR,
    atom_NET_WM_WINDOW_TYPE_DOCK,
    atom_NET_WM_STRUT_PARTIAL,
    atom_NET_WM_STATE_SKIP_TASKBAR,
    atom_NET_WM_STATE,
    atom_MOTIF_WM_HINTS;
  

  PropMotifWmHints    *mwm_hints;
  XSizeHints           size_hints;
  XWMHints            *wm_hints;
  XSetWindowAttributes win_attr;
  XRenderColor         coltmp;

  unsigned char       *wm_name;
  boolean              have_matchbox_wm = False;             
  boolean              have_ewmh_wm     = False;             

  /*
  atom_wm_protocols = { 
    XInternAtom(ui->xdpy, "WM_DELETE_WINDOW",False),
    XInternAtom(ui->xdpy, "WM_PROTOCOLS",False),
    XInternAtom(ui->xdpy, "WM_NORMAL_HINTS", False),
  };
  */
  atom_NET_WM_WINDOW_TYPE =
    XInternAtom(ui->xdpy, "_NET_WM_WINDOW_TYPE" , False);

  atom_NET_WM_WINDOW_TYPE_TOOLBAR =
    XInternAtom(ui->xdpy, "_NET_WM_WINDOW_TYPE_TOOLBAR", False);

  atom_NET_WM_WINDOW_TYPE_DOCK = 
    XInternAtom(ui->xdpy, "_NET_WM_WINDOW_TYPE_DOCK", False);

  atom_MOTIF_WM_HINTS =
    XInternAtom(ui->xdpy, "_MOTIF_WM_HINTS", False);

  atom_NET_WM_STRUT_PARTIAL = 
    XInternAtom(ui->xdpy, "_NET_WM_STRUT_PARTIAL", False);

  atom_NET_WM_STATE_SKIP_TASKBAR =
    XInternAtom(ui->xdpy, "_NET_WM_STATE_SKIP_TASKBAR", False);

  atom_NET_WM_STATE =
    XInternAtom(ui->xdpy, "_NET_WM_STATE", False);

  if ((wm_name = get_current_window_manager_name(ui)) != NULL)
    {
      have_ewmh_wm = True; 	/* basically assumed to be Metacity
				   or at least only tested with mcity */

      if (streq(wm_name, "matchbox"))
	have_matchbox_wm = True;
    }

  win_attr.override_redirect = False; /* Set to true for extreme case */
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
      DBG("setting no focus hint");
      wm_hints->input = False;
      wm_hints->flags = InputHint;
      XSetWMHints(ui->xdpy, ui->xwin, wm_hints );
      XFree(wm_hints);
    }

  size_hints.flags = PPosition | PSize | PMinSize;
  size_hints.x = 0;
  size_hints.y = 0;
  size_hints.width      =  ui->xwin_width; 
  size_hints.height     =  ui->xwin_height;
  size_hints.min_width  =  ui->xwin_width;
  size_hints.min_height =  ui->xwin_height;
    
  XSetStandardProperties(ui->xdpy, ui->xwin, "Keyboard", 
			 NULL, 0, NULL, 0, &size_hints);

  mwm_hints = util_malloc0(sizeof(PropMotifWmHints));
  
  if (mwm_hints)
    {
      mwm_hints->flags = MWM_HINTS_DECORATIONS;
      mwm_hints->decorations = 0;

      XChangeProperty(ui->xdpy, ui->xwin, atom_MOTIF_WM_HINTS, 
		      XA_ATOM, 32, PropModeReplace, 
		      (unsigned char *)mwm_hints, 
		      PROP_MOTIF_WM_HINTS_ELEMENTS);

      free(mwm_hints);
    }

  if (have_ewmh_wm)
    {
      /* XXX Fix this for display size */
      int wm_struct_vals[] = { 0, /* left */			
			     0, /* right */ 
			     0, /* top */
			     50, /* bottom */
			     0, /* left_start_y */
			     0, /* left_end_y */
			     0, /* right_start_y */
			     0, /* right_end_y */
			     0, /* top_start_x */
			     0, /* top_end_x */
			     0, /* bottom_start_x */
			     1399 }; /* bottom_end_x */

      Atom states[] = { atom_NET_WM_STATE_SKIP_TASKBAR };
      int  desk_width = 0;

      XChangeProperty(ui->xdpy, ui->xwin, 
		      atom_NET_WM_STRUT_PARTIAL, XA_ATOM, 32, 
		      PropModeReplace, 
		      (unsigned char *)wm_struct_vals , 12);

      XChangeProperty(ui->xdpy, ui->xwin, 
		      atom_NET_WM_STATE_SKIP_TASKBAR, XA_ATOM, 32, 
		      PropModeReplace, 
		      (unsigned char *)states, 1);

      if (get_desktop_area(ui, NULL, NULL, &desk_width, NULL))
	{
	  /* Assuming we take up all available display width 
           * ( at least true with matchbox wm ). we resize
	   * the base ui width to this ( and height as a factor ) 
           * to avoid the case of mapping and then the wm resizing
           * us, causing an ugly repaint. 
	  */
	  if (desk_width > ui->xwin_width)
	    {
	      mb_kbd_ui_resize(ui, 
			       desk_width, 
			       ( desk_width * ui->xwin_height ) / ui->xwin_width);
	    }
	}

      if (have_matchbox_wm)
	{
	  XChangeProperty(ui->xdpy, ui->xwin, 
			  atom_NET_WM_WINDOW_TYPE, XA_ATOM, 32, 
			  PropModeReplace, 
			  (unsigned char *) &atom_NET_WM_WINDOW_TYPE_TOOLBAR, 1);

	 
	}
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


  /* #636262 - for text maybe */
  coltmp.red   = coltmp.green = coltmp.blue  = 0x0000; coltmp.alpha = 0xcccc;

  XftColorAllocValue(ui->xdpy,
		     DefaultVisual(ui->xdpy, ui->xscreen), 
		     DefaultColormap(ui->xdpy, ui->xscreen),
		     &coltmp,
		     &ui->font_col);


  ui->xgc = XCreateGC(ui->xdpy, ui->xwin, 0, NULL);

  XSetForeground(ui->xdpy, ui->xgc, BlackPixel(ui->xdpy, ui->xscreen ));
  XSetBackground(ui->xdpy, ui->xgc, WhitePixel(ui->xdpy, ui->xscreen ));

  XSetWindowBackgroundPixmap(ui->xdpy, ui->xwin, ui->backbuffer);

  /* Crusty theme stuff  */

  alloc_color(ui, &ui->xcol_c5c5c5, "#c5c5c5");
  alloc_color(ui, &ui->xcol_d3d3d3, "#d3d3d3");
  alloc_color(ui, &ui->xcol_f0f0f0, "#f0f0f0");
  alloc_color(ui, &ui->xcol_f8f8f5, "#f8f8f5");
  alloc_color(ui, &ui->xcol_f4f4f4, "#f4f4f4");
  alloc_color(ui, &ui->xcol_a4a4a4, "#a4a4a4");
  return 1;
}

static void
mb_kbd_ui_resize(MBKeyboardUI *ui, int width, int height) 
{
  MBKeyboard       *kbd = ui->kbd;
  MBKeyboardLayout *layout;
  List              *row_item, *key_item;
  int               width_diff, height_diff;
  int               height_font_pt_size, width_font_pt_size;
  int               next_row_y,  n_rows, extra_key_height;

  MARK();

  width_diff  = width  - ui->base_alloc_width; 
  height_diff = height - ui->base_alloc_height; 

  if (width_diff < 0 || height_diff < 0)
    return;  /* dont go smaller than our int request - get clipped */

  layout   = mb_kbd_get_selected_layout(ui->kbd);
  row_item = mb_kbd_layout_rows(layout);

  /* load a bigger font ? 
   * Only load if height and width have changed
   */

  width_font_pt_size = ( (ui->base_font_pt_size * width) 
		               / ui->base_alloc_width );
  
  if (util_abs(width_font_pt_size - kbd->font_pt_size) > 2)
    {
      height_font_pt_size = ( (ui->base_font_pt_size * height) 
			         / ui->base_alloc_height );

      if (util_abs(height_font_pt_size - kbd->font_pt_size) > 2)
	{
	  ui->kbd->font_pt_size = (height_font_pt_size > width_font_pt_size) 
                 	       ? width_font_pt_size : height_font_pt_size;

	  mb_kbd_ui_load_font(ui);
	}
    }

  n_rows = util_list_length(row_item);

  extra_key_height = (height_diff / n_rows);

  DBG("****** extra height is %i ******", extra_key_height);

  next_row_y = mb_kbd_row_spacing(ui->kbd);

  /* allocate some extra width padding to keys */

  while (row_item != NULL)
    {
      int row_base_width, new_row_base_width, row_width_diff;
      int  next_key_x = 0,  n_fillers  = 0, free_space = 0, new_w = 0;

      row_base_width = mb_kbd_row_base_width(row_item->data);

      new_row_base_width = ( row_base_width * width ) / ui->base_alloc_width;

      row_width_diff = new_row_base_width - row_base_width;

      DBG("row_width_diff = %i", row_width_diff);

      next_key_x = mb_kbd_col_spacing(ui->kbd);


      /* 
       * row_base_width       
       * --------------  X  new_width  = new_base_width
       *   base_width      
       *
       * key_extra_pad  = key_base_width X base_width_diff 
       *                  --------------------------------
       *                          row_base_width
      */

      key_item = mb_kdb_row_keys(row_item->data);

      MARK();

      while (key_item != NULL)
	{
	  MBKeyboardKey *key = key_item->data;
	  int            key_base_width, key_new_pad;

	  key_base_width =( mb_kbd_key_width(key) 
			    - mb_kbd_key_get_extra_width_pad(key));

	  key_new_pad= ( (key_base_width + mb_kbd_col_spacing(kbd)) * row_width_diff) / row_base_width; 
                             
	  mb_kbd_key_set_extra_width_pad (key, key_new_pad );

	  /* Height */

	  mb_kbd_key_set_extra_height_pad (key, extra_key_height);

	  mb_kbd_key_set_geometry(key, next_key_x, -1, -1, -1);

	  next_key_x += (mb_kbd_key_width(key) + mb_kbd_col_spacing(ui->kbd)); 

	  if (mb_kbd_key_get_fill(key))
	      n_fillers++;

	  key_item = util_list_next(key_item);
	}

      key_item = mb_kdb_row_keys(row_item->data);

      /* The above ( likely due to rounding ) leaves a few pixels free. 
       * This can be critical on a small handheld display. Therefore 
       * we do a second parse deviding up any left over space between
       * keys marked as fill. 
      */

      if (n_fillers)
	{
	  key_item = mb_kdb_row_keys(row_item->data);

	  free_space = width - mb_kbd_row_width(row_item->data);

	  while (key_item != NULL)
	    {
	      if (mb_kbd_key_get_fill(key_item->data))
		{
		  int   old_w;
		  List *nudge_key_item = util_list_next(key_item);
		  
		  old_w = mb_kbd_key_width(key_item->data);
		  new_w = old_w + (free_space/n_fillers);
		  
		  mb_kbd_key_set_geometry(key_item->data, -1, -1, new_w, -1);
		  
		  /* nudge next keys forward */
		  while (nudge_key_item)
		    {
		      mb_kbd_key_set_geometry(nudge_key_item->data,
					      mb_kbd_key_x(nudge_key_item->data) + (new_w - old_w ), -1, -1, -1);
		      nudge_key_item = util_list_next(nudge_key_item);
		    }
		}
	      key_item = util_list_next(key_item);
	    }
	}


      /* re-center row */

      mb_kbd_row_set_x(row_item->data, 
		       (width - mb_kbd_row_width(row_item->data))/2);

      /* and position down */

      mb_kbd_row_set_y(row_item->data, next_row_y);

      DBG("************ setting row y to %i ***********", next_row_y);

      next_row_y  += (mb_kbd_row_height(row_item->data) 
		      + mb_kbd_row_spacing(ui->kbd));

      row_item = util_list_next(row_item);
    }

  XResizeWindow(ui->xdpy, ui->xwin, width, height);

  ui->xwin_width  = width;
  ui->xwin_height = height;

  if (ui->backbuffer) /* may get called before initialised */
    {
      XFreePixmap(ui->xdpy, ui->backbuffer);
      ui->backbuffer = XCreatePixmap(ui->xdpy,
				     ui->xwin,
				     ui->xwin_width, 
				     ui->xwin_height,
				     DefaultDepth(ui->xdpy, ui->xscreen));

      XftDrawChange (ui->xft_backbuffer, ui->backbuffer);

      XSetWindowBackgroundPixmap(ui->xdpy, ui->xwin, ui->backbuffer);

      mb_kbd_ui_redraw(ui);
    }
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
	    mb_kbd_key_release(ui->kbd);	    
	    break;

	  case ConfigureNotify:
	    if (xev.xconfigure.width != ui->xwin_width
		|| xev.xconfigure.height != ui->xwin_height)
	      mb_kbd_ui_resize(ui, 
			       xev.xconfigure.width, 
			       xev.xconfigure.height);
	    break;
	  default:
	    break;
	  }
      }
}

static int
mb_kbd_ui_load_font(MBKeyboardUI *ui)
{
  MBKeyboard *kb = ui->kbd;
  char desc[512];

  snprintf(desc, 512, "%s-%i:%s", 
	   kb->font_family, kb->font_pt_size, kb->font_variant);

  if (ui->font != NULL)
    XftFontClose(ui->xdpy, ui->font);

  if ((ui->font = XftFontOpenName(ui->xdpy, ui->xscreen, desc)) == NULL)
    return 0;
  
  return 1;
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

  ui->dpy_width  = DisplayWidth(ui->xdpy, ui->xscreen);
  ui->dpy_height = DisplayHeight(ui->xdpy, ui->xscreen);


  return 1;
}

int
mb_kbd_ui_display_width(MBKeyboardUI *ui)
{
  return ui->dpy_width;
}

int
mb_kbd_ui_display_height(MBKeyboardUI *ui)
{
  return ui->dpy_height;
}

int
mb_kbd_ui_realize(MBKeyboardUI *ui)
{
  ui->base_font_pt_size = ui->kbd->font_pt_size;

  if (!mb_kbd_ui_load_font(ui))
    return 0;

  mb_kbd_ui_allocate_ui_layout(ui, 
			       &ui->base_alloc_width, &ui->base_alloc_height);

  ui->xwin_width  = ui->base_alloc_width;
  ui->xwin_height = ui->base_alloc_height;

  mb_kbd_ui_resources_create(ui);

  mb_kbd_ui_redraw(ui);

  mb_kbd_ui_show(ui);

  mb_kbd_ui_events_iteration(ui);

  return 1;
}
