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

struct MBKeyboardLayout
{
  MBKeyboard       *kbd;  
  char             *id;
  List             *rows;
};


MBKeyboardLayout*
mb_kbd_layout_new(MBKeyboard *kbd, const char *id)
{
  MBKeyboardLayout *layout = NULL;

  layout = util_malloc0(sizeof(MBKeyboardLayout));

  layout->kbd = kbd;
  layout->id  = strdup(id);

  return layout;
}

void
mb_kbd_layout_append_row(MBKeyboardLayout *layout,
			 MBKeyboardRow    *row)
{
  layout->rows = util_list_append(layout->rows, (pointer)row);
}

List*
mb_kbd_layout_rows(MBKeyboardLayout *layout)
{
  return util_list_get_first(layout->rows);
}

