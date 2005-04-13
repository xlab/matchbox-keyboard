#include "matchbox-keyboard.h"

MBKeyboard*
mb_kbd_new(int argc, char **argv)
{
  MBKeyboard *kb = NULL;

  kb = util_malloc0(sizeof(MBKeyboard));

  kb->key_border = 1;
  kb->key_pad    = 2;
  kb->key_margin = 0;

  kb->col_spacing = 5;
  kb->row_spacing = 5;

  if (!mb_kbd_config_load(kb, "config.xml"))
    return NULL;

  kb->selected_layout 
    = (MBKeyboardLayout *)util_list_get_nth_data(kb->layouts, 0);

  if (!mb_kbd_ui_init(kb))
    return NULL;

  return kb;
}

int
mb_kbd_row_spacing(MBKeyboard *kb)
{
  return kb->row_spacing;
}

int
mb_kbd_col_spacing(MBKeyboard *kb)
{
  return kb->col_spacing;
}

int
mb_kbd_keys_border(MBKeyboard *kb)
{
  return kb->key_border;
}

int
mb_kbd_keys_pad(MBKeyboard *kb)
{
  return kb->key_pad;
}

int
mb_kbd_keys_margin(MBKeyboard *kb)
{
  return kb->key_margin;
}

MBKeyboardKey*
mb_kbd_locate_key(MBKeyboard *kb, int x, int y)
{
  MBKeyboardLayout *layout;
  List             *row_item, *key_item;

  layout = mb_kbd_get_selected_layout(kb);

  row_item = mb_kbd_layout_rows(layout);

  while (row_item != NULL)
    {
      MBKeyboardRow *row = row_item->data;

      if (x >= mb_kbd_row_x(row) 
	  && x <= mb_kbd_row_x(row) + mb_kbd_row_width(row) 
	  && y >= mb_kbd_row_y(row)
	  && y <= mb_kbd_row_y(row) + mb_kbd_row_height(row) )
	{
	  key_item = mb_kdb_row_keys(row);

	  while (key_item != NULL)
	    {
	      MBKeyboardKey *key = key_item->data;

	      if (x >= mb_kbd_key_x(key)
		  && x <= mb_kbd_key_x(key) + mb_kbd_key_width(key))
		return key;

	      key_item = util_list_next(key_item);
	    }
	 
	  return NULL;
	}

      row_item = util_list_next(row_item);
    }
  return NULL;
}

void
mb_kbd_add_layout(MBKeyboard *kb, MBKeyboardLayout *layout)
{
  kb->layouts = util_list_append(kb->layouts, (pointer)layout);
}

MBKeyboardLayout*
mb_kbd_get_selected_layout(MBKeyboard *kb)
{
  return kb->selected_layout;
}


void
mb_kbd_run(MBKeyboard *kb)
{
  /* mb_kbb_ui_process_events() */


}


int 
main(int argc, char **argv)
{
  MBKeyboard *kb;

  kb = mb_kbd_new(argc, argv);

  if (kb) mb_kbd_run(kb);

  return 0;
}
