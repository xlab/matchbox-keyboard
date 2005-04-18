#include "matchbox-keyboard.h"

struct MBKeyboardRow
{
  MBKeyboard       *kbd;
  List             *keys;

  int               alloc_x, alloc_y;
};

MBKeyboardRow*
mb_kbd_row_new(MBKeyboard *kbd)
{
  MBKeyboardRow *row = NULL;

  row = util_malloc0(sizeof(MBKeyboardRow));
  row->kbd = kbd;

  return row;
}

void
mb_kbd_row_set_x(MBKeyboardRow *row, int x)
{
  row->alloc_x = x;
}

void
mb_kbd_row_set_y(MBKeyboardRow *row, int y)
{
  row->alloc_y = y;
}

int 
mb_kbd_row_x (MBKeyboardRow *row) 
{ 
  return row->alloc_x;
}

int 
mb_kbd_row_y(MBKeyboardRow *row) 
{ 
  return row->alloc_y;
}

int 
mb_kbd_row_height(MBKeyboardRow *row) 
{
  List          *key_item = mb_kdb_row_keys(row);
  MBKeyboardKey *key;

  /* XX this is a little crazed */

  key = key_item->data;

  return mb_kbd_key_height(key);
}

int 
mb_kbd_row_width(MBKeyboardRow *row) 
{
  List *key_item;
  int   result;

  /* XXX we should cache this result somehow as locate_key calls this */

  result = mb_kbd_col_spacing(row->kbd);

  key_item = mb_kdb_row_keys(row);

  while (key_item != NULL)
    {
      MBKeyboardKey *key = key_item->data;
      
      result += (mb_kbd_key_width(key) + mb_kbd_col_spacing(row->kbd));

      key_item = util_list_next(key_item);
    }

  return result;
}

int 
mb_kbd_row_base_width(MBKeyboardRow *row) 
{
  List *key_item;
  int   result;

  /* XXX we should cache this result somehow as locate_key calls this */

  result = mb_kbd_col_spacing(row->kbd);

  key_item = mb_kdb_row_keys(row);

  while (key_item != NULL)
    {
      MBKeyboardKey *key = key_item->data;
      
      result += (mb_kbd_key_width(key) 
		 + mb_kbd_col_spacing(row->kbd) 
		 - mb_kbd_key_get_extra_width_pad(key));
 
      key_item = util_list_next(key_item);
    }

  return result;
}

void
mb_kbd_row_append_key(MBKeyboardRow *row, MBKeyboardKey *key)
{
  row->keys = util_list_append(row->keys, (pointer)key);

  mb_kbd_key_set_row(key, row);
}

List*
mb_kdb_row_keys(MBKeyboardRow *row)
{
  return util_list_get_first(row->keys);
}
