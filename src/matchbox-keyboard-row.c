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
  row->keys = util_list_append(row->keys, (pointer)key);
}

List*
mb_kdb_row_keys(MBKeyboardRow *row)
{
  return row->keys;
}
