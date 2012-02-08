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
  Display            *xdpy;
  int                 xscreen;
  Window              xwin_root, xwin;
  Pixmap              backbuffer;

  int                 dpy_width, dpy_height;
  int                 xwin_width, xwin_height;

  int                 key_uwidth, key_uheight;

  int                 base_alloc_width, base_alloc_height;
  int                 base_font_pt_size;

  Bool                want_embedding;
  Bool                is_daemon;
  Bool                visible;
  FakeKey             *fakekey;
  MBKeyboardUIBackend *backend; 
  MBKeyboard          *kbd;

  MBKeyboardDisplayOrientation dpy_orientation;
  MBKeyboardDisplayOrientation valid_orientation;
};

static void
mb_kbd_ui_resize(MBKeyboardUI *ui, int width, int height);

static int
mb_kbd_ui_load_font(MBKeyboardUI *ui);


static char*
get_current_window_manager_name (MBKeyboardUI  *ui)
{
  Atom           atom_utf8_string, atom_wm_name, atom_check, type;
  int            result, format;
  char          *val, *retval;
  unsigned long  nitems, bytes_after;
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
  unsigned long  nitems, bytes_after;
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

static void
update_display_size(MBKeyboardUI *ui)
{
  XWindowAttributes winattr;

  MARK();

  XGetWindowAttributes(ui->xdpy, ui->xwin_root, &winattr);

  /* XXX should actually trap an X error here */

  ui->dpy_width  = winattr.width;
  ui->dpy_height = winattr.height;

  if (ui->dpy_width > ui->dpy_height)
    ui->dpy_orientation = MBKeyboardDisplayLandscape;
  else
    ui->dpy_orientation = MBKeyboardDisplayPortrait;

  if (ui->valid_orientation 
      && ui->dpy_orientation != ui->valid_orientation)
    mb_kbd_ui_hide (ui);

  DBG("#### Orientation know '%i'", ui->dpy_orientation);

  return;
}

static boolean
want_extended(MBKeyboardUI *ui)
{
  /* Are we in portrait ? */
  if (ui->dpy_width > ui->dpy_height)
    return True;

  return False;
}

static boolean
get_xevent_timed(Display        *dpy,
		 XEvent         *event_return, 
		 struct timeval *tv)
{
  if (tv->tv_usec == 0 && tv->tv_sec == 0)
    {
      XNextEvent(dpy, event_return);
      return True;
    }

  XFlush(dpy);

  if (XPending(dpy) == 0) 
    {
      int fd = ConnectionNumber(dpy);

      fd_set readset;
      FD_ZERO(&readset);
      FD_SET(fd, &readset);

      if (select(fd+1, &readset, NULL, NULL, tv) == 0) 
	return False;
      else 
	{
	  XNextEvent(dpy, event_return);
	  return True;
	}
    } else {
      XNextEvent(dpy, event_return);
      return True;
    }
}


void
mb_kbd_ui_send_press(MBKeyboardUI        *ui,
		     const char          *utf8_char_in,
		     int                  modifiers)
{
  DBG("Sending '%s'", utf8_char_in);
  fakekey_press(ui->fakekey, (unsigned char*)utf8_char_in, -1, modifiers);
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
  const char             *face_str;

  *width = 0; *height = 0;

  layout   = mb_kbd_get_selected_layout(ui->kbd);
  row_item = mb_kbd_layout_rows(layout);

  /*
   * Figure out the base size of a 'regular' single glyph key.    
  */

  while (row_item != NULL)
    {
      mb_kbd_row_for_each_key(row_item->data, key_item)
	{
	  MBKeyboardKey *key = key_item->data;

	  if (!mb_kbd_is_extended(ui->kbd) 
	      && mb_kbd_key_get_extended(key))
	    continue;

	  /* Ignore keys whose width is forced */
	  if (mb_kbd_key_get_req_uwidth(key))
	    continue;

	  mb_kdb_key_foreach_state(key, state)
	    {
	      if (mb_kbd_key_get_face_type(key, state) == MBKeyboardKeyFaceGlyph)
		{
		  face_str = mb_kbd_key_get_glyph_face(key, state);

		  if (util_utf8_char_cnt(face_str) == 1)
		    {
		      int str_w =0, str_h = 0;

		      ui->backend->text_extents(ui, face_str, &str_w, &str_h);
		      
		      if (str_w > *width) *width = str_w;
		      if (str_h > *height) *height = str_h;

		    }
		}
	      else if (mb_kbd_key_get_face_type(key, state) == MBKeyboardKeyFaceImage)
		{
		  MBKeyboardImage *img;

		  img = mb_kbd_key_get_image_face(key, state);

		  if (mb_kbd_image_width (img) > *width) 
		    *width = mb_kbd_image_width (img);

		  if (mb_kbd_image_height (img) > *height) 
		    *height = mb_kbd_image_height (img);
		}
	    }
	}
      row_item = util_list_next(row_item);
    }

  /* FIXME: hack for small displays */
  if (mb_kbd_ui_display_height(ui) <= 320)
    {
      *height += 4;
    }

}

static void
mb_kbd_ui_min_key_size(MBKeyboardUI  *ui,
		       MBKeyboardKey *key,
		       int           *width,
		       int           *height)
{
  const char *face_str = NULL;
  int         max_w = 0, max_h = 0, state, kw, kh;

  /* 
   * Figure out how small a key can really be UI wise.
  */

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
	  face_str = mb_kbd_key_get_glyph_face(key, state);

	  ui->backend->text_extents(ui, face_str, &kw, &kh);

	  if (kw > max_w) max_w = kw;
	  if (kh > max_h) max_h = kh;
	}
      else if (mb_kbd_key_get_face_type(key, state) == MBKeyboardKeyFaceImage)
	{
	  MBKeyboardImage *img;

	  img = mb_kbd_key_get_image_face(key, state);

	  if (mb_kbd_image_width (img) > max_w) 
	    max_w = mb_kbd_image_width (img);

	  if (mb_kbd_image_height (img) > max_h) 
	    max_h = mb_kbd_image_height (img);
	}
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

  /* Do an initial run to figure out a 'base' size for single glyph keys */
  mb_kdb_ui_unit_key_size(ui, &ui->key_uwidth, &ui->key_uheight);

  row_item = mb_kbd_layout_rows(layout);

  row_y = mb_kbd_row_spacing(ui->kbd); 

  max_row_width = 0;

  /* 
   * First of entire keyboard, basically get the minimum space needed
  */
  while (row_item != NULL)
    {
      MBKeyboardRow *row = row_item->data;
      
      key_x = mb_kbd_col_spacing(ui->kbd);

      max_row_key_height = 0;

      mb_kbd_row_for_each_key(row, key_item)
	{
	  int            key_w = 0, key_h = 0;          
	  MBKeyboardKey *key = key_item->data;

	  mb_kbd_key_set_extra_height_pad(key, 0);
	  mb_kbd_key_set_extra_width_pad(key, 0);
	  mb_kbd_key_set_geometry(key, 0, 0, 0, 0);

	  if (!mb_kbd_is_extended(ui->kbd) && mb_kbd_key_get_extended(key))
	    continue;

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
	  
	  key_x += (mb_kbd_col_spacing(ui->kbd) + key_w);
	}

      if (key_x > max_row_width) /* key_x now represents row width */
	max_row_width = key_x;

      mb_kbd_row_set_y(row, row_y);

      row_y += max_row_key_height + mb_kbd_row_spacing(ui->kbd);

      row_item = util_list_next(row_item);
    }

  *height = row_y; 

  row_item = mb_kbd_layout_rows(layout);

  /* Now pass again allocating any extra space with have left over */

  while (row_item != NULL)
    {
      MBKeyboardRow *row        = row_item->data;
      int            n_fillers  = 0, free_space = 0, new_w = 0;

      mb_kbd_row_for_each_key(row,key_item)
	{
	  if (!mb_kbd_is_extended(ui->kbd) 
	      && mb_kbd_key_get_extended(key_item->data))
	    continue;

	  if (mb_kbd_key_get_fill(key_item->data)
	      || mb_kbd_ui_display_height(ui) <= 320
	      || mb_kbd_ui_display_width(ui) <= 320 )
	      n_fillers++;
	}

      if (!n_fillers)
	goto next_row;

      free_space = max_row_width - mb_kbd_row_width(row);

      mb_kbd_row_for_each_key(row, key_item)
	{
	  if (!mb_kbd_is_extended(ui->kbd) 
	      && mb_kbd_key_get_extended(key_item->data))
	    continue;

	  if (mb_kbd_key_get_fill(key_item->data)
	      || mb_kbd_ui_display_height(ui) <= 320
	      || mb_kbd_ui_display_width(ui) <= 320 )
	    {
	      int   old_w;
	      List *nudge_key_item = util_list_next(key_item);

	      old_w = mb_kbd_key_width(key_item->data);
	      new_w = old_w + (free_space/n_fillers);

	      mb_kbd_key_set_geometry(key_item->data, -1, -1, new_w, -1);

	      /* nudge next keys forward */

	      for (; 
		   nudge_key_item != NULL; 
		   nudge_key_item = util_list_next(nudge_key_item)) 
		{
		  if (!mb_kbd_is_extended(ui->kbd) 
		      && mb_kbd_key_get_extended(nudge_key_item->data))
		    continue;

		  mb_kbd_key_set_geometry(nudge_key_item->data,
					  mb_kbd_key_x(nudge_key_item->data) + (new_w - old_w ), -1, -1, -1);
		  
		}
	    }

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

void
mb_kbd_ui_redraw_key(MBKeyboardUI  *ui, MBKeyboardKey *key)
{
  ui->backend->redraw_key(ui, key);
}


static void
mb_kbd_ui_redraw_row(MBKeyboardUI  *ui, MBKeyboardRow *row)
{
  List *key_item;

  mb_kbd_row_for_each_key(row, key_item)
    {
      if (!mb_kbd_is_extended(ui->kbd) 
	  && mb_kbd_key_get_extended(key_item->data))
	continue;

      mb_kbd_ui_redraw_key(ui, key_item->data);
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

  MARK();

  /* gives backend a chance to clear everything */
  ui->backend->pre_redraw(ui);

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

void
mb_kbd_ui_show(MBKeyboardUI  *ui)
{
  if (ui->visible)
    return;

  if (ui->valid_orientation 
      && ui->dpy_orientation != ui->valid_orientation)
    return;

  XMapWindow(ui->xdpy, ui->xwin);
  mb_kbd_ui_redraw (ui);

  ui->visible = True;
}

void
mb_kbd_ui_hide(MBKeyboardUI  *ui)
{
  if (!ui->visible)
    return;

  XUnmapWindow(ui->xdpy, ui->xwin);

  ui->visible = False;
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
     atom_NET_WM_STATE_SKIP_PAGER,
     atom_NET_WM_STATE_SKIP_TASKBAR,
     atom_NET_WM_STATE,
     atom_MOTIF_WM_HINTS;
  

  PropMotifWmHints    *mwm_hints;
  XSizeHints           size_hints;
  XWMHints            *wm_hints;
  XSetWindowAttributes win_attr;


  char                *wm_name;
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

  atom_NET_WM_STATE_SKIP_PAGER = 
    XInternAtom(ui->xdpy, "_NET_WM_STATE_SKIP_PAGER", False);

  atom_NET_WM_STATE =
    XInternAtom(ui->xdpy, "_NET_WM_STATE", False);

  if ((wm_name = get_current_window_manager_name(ui)) != NULL)
    {
      have_ewmh_wm = True; 	/* basically assumed to be Metacity
				   or at least only tested with mcity */
    }
  else
    {
      if (ui->is_daemon)
	{
	  /* Hack to avoid starting before the WM. Needed only in daemon mode
	  */
	  while (wm_name == NULL)
	    {
	      sleep(1);
	      wm_name = get_current_window_manager_name(ui);
	    }

	  have_ewmh_wm = True;
	}
    }

  if (wm_name && streq(wm_name, "matchbox"))
    have_matchbox_wm = True;

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

  XSelectInput (ui->xdpy,  ui->xwin_root, 
		SubstructureNotifyMask|StructureNotifyMask);

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

  if (!ui->want_embedding)
    {
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
				   0, /* bottom */
				   0, /* left_start_y */
				   0, /* left_end_y */
				   0, /* right_start_y */
				   0, /* right_end_y */
				   0, /* top_start_x */
				   0, /* top_end_x */
				   0, /* bottom_start_x */
				   1399 }; /* bottom_end_x */
	  
	  Atom states[] = { atom_NET_WM_STATE_SKIP_TASKBAR, atom_NET_WM_STATE_SKIP_PAGER };
	  int  desk_width = 0, desk_height = 0, desk_y = 0;
	  
	  XChangeProperty(ui->xdpy, ui->xwin, 
			  atom_NET_WM_STATE, XA_ATOM, 32, 
			  PropModeReplace, 
			  (unsigned char *)states, 2);
	  
	  if (get_desktop_area(ui, NULL, &desk_y, &desk_width, &desk_height))
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
	      
	      wm_struct_vals[2]  = desk_y + desk_height - ui->xwin_height;
	      wm_struct_vals[11] = desk_width;
	      
	      XChangeProperty(ui->xdpy, ui->xwin, 
			      atom_NET_WM_STRUT_PARTIAL, XA_CARDINAL, 32, 
			      PropModeReplace, 
			      (unsigned char *)wm_struct_vals , 12);
	      
	      DBG("desk width: %i, desk height: %i xwin_height :%i",
		  desk_width, desk_height, ui->xwin_height);
	      
	    }
	  
	  if (have_matchbox_wm)
	    {
	      XChangeProperty(ui->xdpy, ui->xwin, 
			      atom_NET_WM_WINDOW_TYPE, XA_ATOM, 32, 
			      PropModeReplace, 
			      (unsigned char *) &atom_NET_WM_WINDOW_TYPE_TOOLBAR, 1);
	    }
	  else
	    {
	      /*
		XChangeProperty(ui->xdpy, ui->xwin, 
		atom_NET_WM_WINDOW_TYPE, XA_ATOM, 32, 
		PropModeReplace, 
		(unsigned char *) &atom_NET_WM_WINDOW_TYPE_DOCK, 1);
	      */
	      
	    }
	}
    }

  ui->backbuffer = XCreatePixmap(ui->xdpy,
				 ui->xwin,
				 ui->xwin_width, 
				 ui->xwin_height,
				 DefaultDepth(ui->xdpy, ui->xscreen));


  XSetWindowBackgroundPixmap(ui->xdpy,
			     ui->xwin, 
			     ui->backbuffer);

  ui->backend->resources_create(ui);


  /* Get root size change events for rotation */

  /* XSelectInput(ui->xdpy, ui->xwin_root, StructureNotifyMask); */

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

  /* Don't scale beyond a sensible height on wide screens */
  if (height > (ui->dpy_height / 3))
    height = ui->dpy_height / 3;

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

  /* allocate the extra width we have as padding to keys */

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

      mb_kbd_row_for_each_key(row_item->data, key_item)
	{
	  MBKeyboardKey *key = key_item->data;
	  int            key_base_width, key_new_pad;

	  if (!mb_kbd_is_extended(kbd) && mb_kbd_key_get_extended(key))
	    continue;

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
	}

      /* The above ( likely due to rounding ) leaves a few pixels free. 
       * This can be critical on a small handheld display. Therefore 
       * we do a second parse deviding up any left over space between
       * keys marked as fill. 
      */

      if (n_fillers)
	{
	  free_space = width - mb_kbd_row_width(row_item->data);

	  mb_kbd_row_for_each_key(row_item->data, key_item)
	    {
	      if (!mb_kbd_is_extended(kbd) 
		  && mb_kbd_key_get_extended(key_item->data))
		continue;

	      if (mb_kbd_key_get_fill(key_item->data))
		{
		  int   old_w;
		  List *nudge_key_item = util_list_next(key_item);
		  
		  old_w = mb_kbd_key_width(key_item->data);
		  new_w = old_w + (free_space/n_fillers);
		  
		  mb_kbd_key_set_geometry(key_item->data, -1, -1, new_w, -1);
		  
		  /* nudge next keys forward */

		  for (; 
		       nudge_key_item != NULL; 
		       nudge_key_item = util_list_next(nudge_key_item)) 
		    {
		      if (!mb_kbd_is_extended(ui->kbd) 
			  && mb_kbd_key_get_extended(nudge_key_item->data))
			continue;

		      mb_kbd_key_set_geometry(nudge_key_item->data,
					      mb_kbd_key_x(nudge_key_item->data) + (new_w - old_w ), -1, -1, -1);

		    }
		}

	    }
	}


      /* re-center row */

      mb_kbd_row_set_x(row_item->data, 
		       (width - mb_kbd_row_width(row_item->data))/2);

      /* and position down */

      mb_kbd_row_set_y(row_item->data, next_row_y);

      next_row_y  += (mb_kbd_row_height(row_item->data) 
		      + mb_kbd_row_spacing(ui->kbd));

      row_item = util_list_next(row_item);
    }

  /* center entire layout vertically if space left */

  if (next_row_y < height)
    {
      int vspace = ( height - next_row_y ) / 2;

      row_item = mb_kbd_layout_rows(layout);

      while (row_item != NULL)
	{
	  mb_kbd_row_set_y(row_item->data, 
			   mb_kbd_row_y(row_item->data) + vspace + 1);

	  row_item = util_list_next(row_item);
	}
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

      ui->backend->resize(ui, width, height);

      XSetWindowBackgroundPixmap(ui->xdpy, ui->xwin, ui->backbuffer);

      mb_kbd_ui_redraw(ui);
    }


}

void
mb_kbd_ui_handle_configure(MBKeyboardUI *ui,
			   int           width,
			   int           height)
{
  boolean old_state, new_state;

  MARK();

  /* Figure out if screen size has changed - does a round trip - bad */

  update_display_size(ui);

  old_state = mb_kbd_is_extended(ui->kbd);
  new_state = want_extended(ui);
   
  if (new_state == old_state) 	/* Not a rotation */
    {
      mb_kbd_ui_resize(ui, width, height); 
      return;
    }

  mb_kbd_set_extended(ui->kbd, new_state);

  /* realocate the layout */

  mb_kbd_ui_allocate_ui_layout(ui, 
			       &ui->base_alloc_width, &ui->base_alloc_height);

  mb_kbd_ui_resize(ui, width, height); 


}

void
mb_kbd_ui_event_loop(MBKeyboardUI *ui)
{
  MBKeyboardKey *key = NULL;
  struct timeval tvt;

  /* Key repeat - values for standard xorg install ( xset q) */
  int repeat_delay = 100 * 10000;
  int repeat_rate  = 30  * 1000;
  int hide_delay = 100 * 1000;
  int to_hide = 0;

  int press_x = 0, press_y = 0; 

  tvt.tv_sec  = 0;
  tvt.tv_usec = repeat_delay;

  while (True)
      {
	XEvent xev;

	if (get_xevent_timed(ui->xdpy, &xev, &tvt))
	  {
	    switch (xev.type) 
	      {
	      case ButtonPress:
		press_x = xev.xbutton.x; press_y = xev.xbutton.y;
		DBG("got button bress at %i,%i", xev.xbutton.x, xev.xbutton.y);
		key = mb_kbd_locate_key(ui->kbd, xev.xbutton.x, xev.xbutton.y);
		if (key)
		  {
		    /* Hack if we never get a release event */
		    if (key != mb_kbd_get_held_key(ui->kbd))
		      {
			mb_kbd_key_release(ui->kbd);
			tvt.tv_usec = repeat_delay;   
		      }
		    else
		      tvt.tv_usec = repeat_rate;

		    DBG("found key for press");
		    mb_kbd_key_press(key);

		  }
		break;
	      case ButtonRelease:
		if (mb_kbd_get_held_key(ui->kbd) != NULL)
		  {
		    mb_kbd_key_release(ui->kbd);	 
		    tvt.tv_usec = repeat_delay;

		    /* Gestures */
#if 0
		    /* FIXME: check time first */
		    if ( (press_x - xev.xbutton.x) > ui->key_uwidth )
		      {
			/* <-- slide back ...backspace */
			fakekey_press_keysym(ui->fakekey, XK_BackSpace, 0);
			fakekey_repeat(ui->fakekey);
			fakekey_release(ui->fakekey);
			/* FIXME: add <-- --> <-- --> support */
		      }
		    else if ( (xev.xbutton.y - press_y) > ui->key_uheight )
		      {
			/* V slide down ...return  */
			fakekey_press_keysym(ui->fakekey, XK_BackSpace, 0);
			fakekey_release(ui->fakekey);
			fakekey_press_keysym(ui->fakekey, XK_Return, 0);
			fakekey_release(ui->fakekey);
		      }
#endif
		    /* TODO ^ caps support */

		  }
		break;
	      case ConfigureNotify:
		if (xev.xconfigure.window == ui->xwin 
		    &&  (xev.xconfigure.width != ui->xwin_width
			 || xev.xconfigure.height != ui->xwin_height))
		  {
		    mb_kbd_ui_handle_configure(ui,
					       xev.xconfigure.width,
					       xev.xconfigure.height);
		  }
		if (xev.xconfigure.window == ui->xwin_root)		    
		    update_display_size(ui);
		break;
	      case MappingNotify: 
		fakekey_reload_keysyms(ui->fakekey);
		XRefreshKeyboardMapping(&xev.xmapping);
		break;
	      default:
		break;
	      }
	    if (ui->want_embedding)
	      mb_kbd_xembed_process_xevents (ui, &xev);

	    if (ui->is_daemon)
      {
	      switch (mb_kbd_remote_process_xevents (ui, &xev))
        {
        case MBKeyboardRemoteHide:
          if (to_hide == 1) {
            mb_kbd_ui_hide(ui);
          }
          tvt.tv_usec = hide_delay;
          to_hide = 1;
          break;
        case MBKeyboardRemoteShow:
          mb_kbd_ui_show(ui);
          tvt.tv_usec = repeat_delay;
          to_hide = 0;
          break;
        case MBKeyboardRemoteToggle:
          to_hide = 0;
          tvt.tv_usec = repeat_delay;
          if (ui->visible)
            mb_kbd_ui_hide(ui);
          else
            mb_kbd_ui_show(ui);
          break;
        case MBKeyboardRemoteNone:
          if (to_hide == 1) {
            mb_kbd_ui_hide(ui);
            tvt.tv_usec = repeat_delay;
            to_hide = 0;
          }
          break;
        }
      }
          }
	else
	  {
      /* Hide timed out */
      if (to_hide)
      {
        DBG("Hide timed out, calling mb_kbd_ui_hide");
        mb_kbd_ui_hide(ui);
        tvt.tv_usec = repeat_delay;
        to_hide = 0;
      }

	    /* Keyrepeat */
	    if (mb_kbd_get_held_key(ui->kbd) != NULL)
	      {
		fakekey_repeat(ui->fakekey);
		tvt.tv_usec = repeat_rate;
	      }
	  }
      }
}

static int
mb_kbd_ui_load_font(MBKeyboardUI *ui)
{
  return ui->backend->font_load(ui);
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

MBKeyboardUIBackend*
mb_kbd_ui_backend(MBKeyboardUI *ui)
{
  return ui->backend;
}

Display*
mb_kbd_ui_x_display(MBKeyboardUI *ui)
{
  return ui->xdpy;
}

int
mb_kbd_ui_x_screen(MBKeyboardUI *ui)
{
  return ui->xscreen;
}

Window
mb_kbd_ui_x_win(MBKeyboardUI *ui)
{
  return ui->xwin;
}

Window
mb_kbd_ui_x_win_root(MBKeyboardUI *ui)
{
  return ui->xwin_root;
}

int
mb_kbd_ui_x_win_height(MBKeyboardUI *ui)
{
  return ui->xwin_height;
}

int
mb_kbd_ui_x_win_width(MBKeyboardUI *ui)
{
  return ui->xwin_width;
}


Pixmap
mb_kbd_ui_backbuffer(MBKeyboardUI *ui)
{
  return ui->backbuffer;
}

MBKeyboard*
mb_kbd_ui_kbd(MBKeyboardUI *ui)
{
  return ui->kbd;
}


int
mb_kbd_ui_realize(MBKeyboardUI *ui)
{
  ui->base_font_pt_size = ui->kbd->font_pt_size;

  if (!mb_kbd_ui_load_font(ui))
    return 0;

  /* potrait or landscape */
  if (want_extended(ui))
    mb_kbd_set_extended(ui->kbd, True);

  /* 
   * figure out how small this keyboard can be..
  */
  mb_kbd_ui_allocate_ui_layout(ui, 
			       &ui->base_alloc_width, &ui->base_alloc_height);

  ui->xwin_width  = ui->base_alloc_width;
  ui->xwin_height = ui->base_alloc_height;

  mb_kbd_ui_resources_create(ui);

  unless (mb_kbd_ui_embeded(ui))
    {
      if (ui->is_daemon)
	{
	  /* Dont map daemon to begin with */
	  mb_kbd_remote_init (ui);
	}
      else
	{
	  mb_kbd_ui_show(ui);
	  mb_kbd_ui_redraw(ui);
	}
    }
  else
    mb_kbd_xembed_init (ui);

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

  ui->backend = MB_KBD_UI_BACKEND_INIT_FUNC(ui);

  update_display_size(ui);

  return 1;
}

/* Embedding */

void
mb_kbd_ui_set_embeded (MBKeyboardUI *ui, int embed)
{
  ui->want_embedding = embed;
}
 
int
mb_kbd_ui_embeded (MBKeyboardUI *ui)
{
  return ui->want_embedding;
}

void
mb_kbd_ui_print_window (MBKeyboardUI *ui)
{
  fprintf(stdout, "%li\n", mb_kbd_ui_x_win(ui));
  fflush(stdout);
}

/* Remote */

void
mb_kbd_ui_set_daemon (MBKeyboardUI *ui, int value)
{
  ui->is_daemon = value;
}

void
mb_kbd_ui_limit_orientation (MBKeyboardUI                *ui, 
			     MBKeyboardDisplayOrientation orientation)
{
  ui->valid_orientation = orientation;
}
