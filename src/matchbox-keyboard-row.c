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
mb_kbd_row_destroy (MBKeyboardRow *row)
{
  List *l;

  l = row->keys;

  while (l)
    {
      List *n = l->next;
      MBKeyboardKey *k = l->data;

      mb_kbd_key_destroy (k);
      free (l);
      l = n;
    }

  free (row);
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
  List          *key_item;

  /* XX this is a little crazed
   * We avoid keys with 0 height - spacers or non allocated extended ones
  */

  mb_kbd_row_for_each_key(row, key_item)
    {
      if (!mb_kbd_is_extended(row->kbd)
          && mb_kbd_key_get_extended(key_item->data))
        continue;


      if (mb_kbd_key_height(key_item->data) > 0)
	return mb_kbd_key_height(key_item->data);
    }

  return 0;
}

int
mb_kbd_row_width(MBKeyboardRow *row)
{
  List *key_item;
  int   result;

  /* XXX we should cache this result somehow as locate_key calls this */

  result = mb_kbd_col_spacing(row->kbd);


  mb_kbd_row_for_each_key(row, key_item)
    {
      MBKeyboardKey *key = key_item->data;

      if (!mb_kbd_is_extended(row->kbd)
          && mb_kbd_key_get_extended(key))
        continue;

      result += (mb_kbd_key_width(key) + mb_kbd_col_spacing(row->kbd));
    }

  return result;
}

int
mb_kbd_row_base_width(MBKeyboardRow *row)
{
  List *key_item;
  int   result;

  result = mb_kbd_col_spacing(row->kbd);

  mb_kbd_row_for_each_key(row, key_item)
    {
      MBKeyboardKey *key = key_item->data;

      if (!mb_kbd_is_extended(row->kbd)
          && mb_kbd_key_get_extended(key))
        continue;

      result += (mb_kbd_key_width(key)
                 + mb_kbd_col_spacing(row->kbd)
                 - mb_kbd_key_get_extra_width_pad(key));

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
