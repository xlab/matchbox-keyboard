#include "matchbox-keyboard.h"

MBKeyboard*
mb_kbd_new(int argc, char **argv)
{
  MBKeyboard *kb = NULL;

  kb = util_malloc0(sizeof(MBKeyboard));

  if (!mb_kbd_config_load(kb, "config.xml"))
    return NULL;

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
mb_kbd_get_current_layout(MBKeyboard *kb)
{
  return NULL;
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
