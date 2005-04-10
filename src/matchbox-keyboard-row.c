#include "matchbox-keyboard.h"

struct MBKeyboardRow
{
  MBKeyboard       *kbd;
  List             *keys;
};

MBKeyboardRow*
mb_kbd_row_new(MBKeyboard *kbd)
{
  MBKeyboardRow *row = NULL;

  row = util_malloc0(sizeof(MBKeyboardRow));

  return row;
}

void
mb_kbd_row_append_key(MBKeyboardRow *row, MBKeyboardKey *key)
{
  List* l;

  row->keys = util_list_append(row->keys, (pointer)key);

  l = row->keys;

  while (l)
    {
      DBG("got list item %p", l);

      l = util_list_next(l);
    }
}

List*
mb_kdb_row_keys(MBKeyboardRow *row)
{
  return util_list_get_first(row->keys);
}
