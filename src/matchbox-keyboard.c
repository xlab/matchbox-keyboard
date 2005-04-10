#include "matchbox-keyboard.h"

MBKeyboard*
mb_kbd_new(int argc, char **argv)
{
  MBKeyboard *kb = NULL;

  kb = util_malloc0(sizeof(MBKeyboard));

  if (!mb_kbd_config_load(kb, "config.xml"))
    return NULL;

  kb->selected_layout 
    = (MBKeyboardLayout *)util_list_get_nth_data(kb->layouts, 0);

  if (!mb_kbd_ui_init(kb))
    return NULL;

  return kb;
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



}


int 
main(int argc, char **argv)
{
  MBKeyboard *kb;

  kb = mb_kbd_new(argc, argv);

  if (kb) mb_kbd_run(kb);

  return 0;
}
