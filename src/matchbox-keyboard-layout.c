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
mb_kbd_layout_destroy (MBKeyboardLayout *layout)
{
  List *l;

  if (layout->id)
    free (layout->id);

  l = layout->rows;

  while (l)
    {
      List *n = l->next;
      MBKeyboardRow *r = l->data;

      mb_kbd_row_destroy (r);
      free (l);
      l = n;
    }

  free (layout);
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

