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
  XftColor      color_bg; 
  XftColor      color_fg;
  

  XftDraw      *xft_backbuffer;  
  Pixmap        backbuffer;

  FakeKey      *fakekey;

  MBKeyboard   *kbd;
};

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
		      XGlyphInfo  extents;

		      XftTextExtentsUtf8(ui->xdpy, 
					 ui->font,
					 face_str, 
					 strlen(face_str),
					 &extents);
		      
		      if (extents.width > *width)
			*width = extents.width;

		      if (extents.height > *height)
			*height = extents.height;

		    }
		}

	      /* XXX TODO, also need to check height of image keys etc */

	    }
	  


	  key_item = util_list_next(key_item);
	}
      row_item = util_list_next(row_item);
    }





}

void
mb_kbd_ui_allocate_ui_layout(MBKeyboardUI *ui,
			     int           width,
			     int           height)
{
  MBKeyboardLayout *layout;
  List *row_item, *key_item;
  int   uwidth, uheight;

  layout = mb_kbd_get_selected_layout(ui->kbd);

  row_item = mb_kbd_layout_rows(layout);

  while (row_item != NULL)
    {
      MBKeyboardRow *row = row_item->data;
      
      key_item = mb_kdb_row_keys(row);

      printf("Got Row \n");

      while (key_item != NULL)
	{
	  MBKeyboardKey *key = key_item->data;

	  mb_kbd_key_dump_key(key);

	  key_item = util_list_next(key_item);
	}

      row_item = util_list_next(row_item);
    }

  mb_kdb_ui_unit_key_size(ui, &uwidth, &uheight);

  DBG("key unit %ix%i", uwidth, uheight);
}
			  

int
mb_kbd_ui_init(MBKeyboard *kbd)
{
  MBKeyboardUI     *ui = NULL;

  PropMotifWmHints *mwm_hints;
  XSizeHints        size_hints;
  XWMHints         *wm_hints;
  
  ui = kbd->ui = util_malloc0(sizeof(MBKeyboardUI));
  
  ui->kbd = kbd;

  if ((ui->xdpy = XOpenDisplay(getenv("DISPLAY"))) == NULL)
    return 0;

  if ((ui->fakekey = fakekey_init(ui->xdpy)) == NULL)
    return 0;

  ui->xscreen   = DefaultScreen(ui->xdpy);
  ui->xwin_root = RootWindow(ui->xdpy, ui->xscreen);   

  kbd->font_desc = "Sans-20"; 	/* HACK HACK HACK */

  if ((ui->font = XftFontOpenName(ui->xdpy, 
				  ui->xscreen, 
				  kbd->font_desc)) == NULL)
    return 0;

  mb_kbd_ui_allocate_ui_layout(ui, 0, 0);

  return 1;
}
