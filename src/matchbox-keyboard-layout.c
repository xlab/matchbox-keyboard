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
  return layout->rows;
}

