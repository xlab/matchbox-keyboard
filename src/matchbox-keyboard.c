/* 
 *  Matchbox Keyboard - A lightweight software keyboard.
 *
 *  Authored By Matthew Allum <mallum@o-hand.com>
 *  Hacked by Maxim Kouprianov <me@kc.vc>
 *
 *  Copyright (c) 2005 OpenedHand Ltd - http://o-hand.com
 *  Modifications (c) 2009 Maxim Kouprianov - http://kc.vc/projects/matchbox-keyboard.html
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

static void
mb_kbd_usage (char *progname)
{
  fprintf(stderr, "Usage:\n   %s [options] [layout]\n", progname);
  fprintf(stderr, "\nSupported options are;\n"
	  "   -xid,--xid            Print window ID to stdout ( for embedding )\n"
	  "   -d,--daemon      Run in 'daemon' mode (for remote control)\n"
	  "   -t, --gestures	Enable gestures\n"
	  "------------------------- UI Tweaks & Positioning --------------------\n"
	  "   -p,--key-padding <px>  Key padding\n"
	  "   -c,--col-spacing <px>	Space between columns\n"
	  "   -r,--row-spacing <px>	Space between rows\n"
	  "   -f,--font-family <name>	Font family (ex: sans, droidsans)\n"
	  "   -s,--font-size <pt>	Font size\n"
	  "   -b,--non-bold	Switch to normal weight\n"
	  "   -v,--override	Absolute positioning on the screen\n"
	  "   -i,--invert	Attach keyboard to the top instead of bottom\n"
	  "   -g,--geometry <HxW.y.x>	Specify keyboard's geometry \n (ex: -g 200x800; -g 0x800.200.0; -g 0x0.0.50; zeroes mean \"by-default\")\n"
);
  fprintf(stderr, "\nmatchbox-keyboard 1.5 \nCopyright (C) 2007 OpenedHand Ltd.\nModifications (C) 2009 Maxim Kouprianov ( http://me@kc.vc )\nSpecial thanks to Paguro ( http://smartqmid.ru )\n\n");

  exit(-1);
}

MBKeyboard*
mb_kbd_new (int argc, char **argv)
{
  MBKeyboard *kb = NULL;
  char       *variant = NULL; 
  Bool        want_embedding = False, want_daemon = False, override=False,invert=False,gest=False;
  int         i; 
  FILE  *cfg;
  char   cfg_path[1024];
  char   gm[20];
  char   vr[20];
  int		iHeightPercent = MATCHBOX_KBD_DEF_HEIGHT_P;
  char *geometry = "";
  MBKeyboardDisplayOrientation orientation = MBKeyboardDisplayAny;

  kb = util_malloc0(sizeof(MBKeyboard));

  kb->key_border = 0;
  kb->key_pad     = 2;
  kb->key_margin = 0;

  kb->col_spacing = 2;
  kb->row_spacing = 2;

  kb->font_family  = strdup("droidsans");
  kb->font_pt_size = 10;
  kb->font_variant = strdup("bold");

  snprintf(cfg_path, 1024, "%s/.matchbox/kb_config", getenv("HOME"));
 cfg=fopen(cfg_path,"r");
 if(cfg){
 fscanf(cfg,"%d %d %d %d %s %d",&want_daemon,&invert,&gest,&override,&gm,&kb->font_pt_size,&vr);
 }
geometry=gm;
variant=vr;

  for (i = 1; i < argc; i++) 
    {

//------ getting params --------
if (streq ("-f", argv[i]) || streq ("--font-family", argv[i])) 
	{
		 if (++i>=argc) mb_kbd_usage (argv[0]);
		kb->font_family  = strdup(argv[i]);
		continue;
	}
if (streq ("-s", argv[i]) || streq ("--font-size", argv[i])) 
	{
		 if (++i>=argc) mb_kbd_usage (argv[0]);
		kb->font_pt_size = atoi(argv[i]);
		continue;
	}
if (streq ("-b", argv[i]) || streq ("--non-bold", argv[i])) 
	{
		kb->font_variant = strdup("regular");
		continue;
	}
if (streq ("-p", argv[i]) || streq ("--key-padding", argv[i])) 
	{
		 if (++i>=argc) mb_kbd_usage (argv[0]);
		kb->key_pad = atoi(argv[i]);
		continue;
	}
if (streq ("-r", argv[i]) || streq ("--row-spacing", argv[i])) 
	{
		 if (++i>=argc) mb_kbd_usage (argv[0]);
		 kb->row_spacing = atoi(argv[i]);
		continue;
	}
if (streq ("-c", argv[i]) || streq ("--col-spacing", argv[i])) 
	{
		 if (++i>=argc) mb_kbd_usage (argv[0]);
		kb->col_spacing = atoi(argv[i]);
		continue;
	}

      if (streq ("-v", argv[i]) || streq ("--override", argv[i])) 
	{
	  override = True;
	  continue;
	}
      if (streq ("-t", argv[i]) || streq ("--gestures", argv[i])) 
	{
	  gest= True;
	  continue;
	}
      if (streq ("-i", argv[i]) || streq ("--invert", argv[i])) 
	{
	  invert = True;
	  continue;
	}
	 if (streq ("-g", argv[i]) || streq ("--geometry", argv[i])) 
	{
	  if (++i>=argc) mb_kbd_usage (argv[0]);
	  geometry = argv[i]; 
	  continue;
	}
//--------------

      if (streq ("-xid", argv[i]) || streq ("--xid", argv[i])) 
	{
	  want_embedding = True;
	  continue;
	}

      if (streq ("-d", argv[i]) || streq ("--daemon", argv[i])) 
	{
	  want_daemon = True;
	  continue;
	}

	if (streq ("-h", argv[i]) || streq ("--help", argv[i])) 
	{
		 if (++i>=argc) mb_kbd_usage (argv[0]);
		iHeightPercent = atoi(argv[i]);
		continue;
	}

      if (streq ("-o", argv[i]) || streq ("--orientation", argv[i]))
	{
	  if (++i>=argc) mb_kbd_usage (argv[0]);

	  if (streq(argv[i], "portrait"))
	    {
	      orientation = MBKeyboardDisplayPortrait;
	    }
	  else if (streq(argv[i], "landscape"))
	    {
	      orientation = MBKeyboardDisplayLandscape;
	    }
	  else
	    mb_kbd_usage (argv[0]);

	  continue;
	}

      if (i == (argc-1) && argv[i][0] != '-')
	variant = argv[i];
      else
	if(variant==NULL){mb_kbd_usage(argv[0]);}
    }

  if (variant == NULL)
    variant = getenv("MB_KBD_VARIANT");

  if (!mb_kbd_ui_init(kb))
    return NULL;
  
  //MBKeyboardUI* ui = kb->ui;
  
  mb_kbd_ui_set_height_percent(kb->ui, iHeightPercent);
  

  if (!mb_kbd_config_load(kb, variant))
    return NULL;

  kb->selected_layout 
    = (MBKeyboardLayout *)util_list_get_nth_data(kb->layouts, 0);

  if (want_embedding)
    mb_kbd_ui_set_embeded (kb->ui, True);

  if (override)
    mb_kbd_ui_set_override (kb->ui, True);

  if (gest)
    mb_kbd_ui_set_gestures (kb->ui, True);

  if (geometry)
    mb_kbd_ui_set_geometry (kb->ui, geometry);

  if (invert)
    mb_kbd_ui_set_invert (kb->ui, True);

  if (want_daemon)
    {
      mb_kbd_ui_set_daemon (kb->ui, True);
      if (orientation != MBKeyboardDisplayAny)
	mb_kbd_ui_limit_orientation (kb->ui, orientation);
    }

  if (!mb_kbd_ui_realize(kb->ui))
    return NULL;

  if (want_embedding)
    mb_kbd_ui_print_window (kb->ui);



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

void
mb_kbd_add_state(MBKeyboard *kbd, MBKeyboardStateType state)
{
  kbd->keys_state |= state;
}

void
mb_kbd_toggle_state(MBKeyboard *kbd, MBKeyboardStateType state)
{
  kbd->keys_state ^= state;
}

boolean
mb_kbd_has_state(MBKeyboard *kbd, MBKeyboardStateType state)
{
  return (kbd->keys_state & state);
}

boolean
mb_kbd_has_any_state(MBKeyboard *kbd)
{
  return  (kbd->keys_state > 0);
}

void
mb_kbd_remove_state(MBKeyboard *kbd, MBKeyboardStateType state)
{
  kbd->keys_state &= ~(state);
}

MBKeyboardKeyStateType
mb_kbd_keys_current_state(MBKeyboard *kbd)
{
  if (mb_kbd_has_state(kbd, MBKeyboardStateShifted))
    return MBKeyboardKeyStateShifted;

  if (mb_kbd_has_state(kbd, MBKeyboardStateMod1))
    return MBKeyboardKeyStateMod1;

  if (mb_kbd_has_state(kbd, MBKeyboardStateMod2))
    return MBKeyboardKeyStateMod2;
  
  if (mb_kbd_has_state(kbd, MBKeyboardStateMod3))
    return MBKeyboardKeyStateMod3;
  
  return MBKeyboardKeyStateNormal;
}

void
mb_kbd_redraw(MBKeyboard *kb)
{
  mb_kbd_ui_redraw(kb->ui);
}

void
mb_kbd_redraw_key(MBKeyboard *kb, MBKeyboardKey *key)
{
  mb_kbd_ui_redraw_key(kb->ui, key);
  mb_kbd_ui_swap_buffers(kb->ui);
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
	  mb_kbd_row_for_each_key(row, key_item) 
	    {
	      MBKeyboardKey *key = key_item->data;

	      if (!mb_kbd_is_extended(kb) 
		      && mb_kbd_key_get_extended(key))
		continue;

	      if (!mb_kbd_key_is_blank(key)
		  && x >= mb_kbd_key_abs_x(key)
		  && x <= mb_kbd_key_abs_x(key) + mb_kbd_key_width(key))
		return key;

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

/*!
 * Advance to the next keyboard layout. If at the last layout, reset to the first.
 * \param kb Keyboard.
 * \param iIncr Amount of increment. +/-.
 */
void mb_kbd_incr_layout(MBKeyboard *kb, int iIncr)
{
	int idx 	= util_list_index_of(kb->layouts, kb->selected_layout);
	int max 	= util_list_length(kb->layouts);
	
	idx += iIncr;					// Advance to next/prev profile.
	if (idx >= max) 	idx = 0;		// Constrain to number of profiles.
	if (idx < 0) 		idx = max - 1;	
	
	kb->selected_layout = util_list_get_nth_data(kb->layouts, idx);
}

/*!
 * Advance to the next keyboard layout. If at the last layout, reset to the first.
 * \param kb Keyboard.
 */
void mb_kbd_set__layout(MBKeyboard *kb)
{
	int idx 	= util_list_index_of(kb->layouts, kb->selected_layout);
	int max 	= util_list_length(kb->layouts);
	
	idx++;					// Advance to next profile.
	if (idx >= max) idx = 0;		// Constrain to number of profiles.
	
	kb->selected_layout = util_list_get_nth_data(kb->layouts, idx);
}

void
mb_kbd_set_held_key(MBKeyboard *kb, MBKeyboardKey *key)
{
  kb->held_key = key;
}

MBKeyboardKey *
mb_kbd_get_held_key(MBKeyboard *kb)
{
  return kb->held_key;
}

void
mb_kbd_set_extended(MBKeyboard *kb, boolean extend)
{
  kb->extended = extend;
}

boolean
mb_kbd_is_extended(MBKeyboard *kb)
{
  return kb->extended;
}


void
mb_kbd_run(MBKeyboard *kb)
{
  mb_kbd_ui_event_loop(kb->ui);
}


int 
main(int argc, char **argv)
{
  MBKeyboard *kb;

  kb = mb_kbd_new(argc, argv);

  if (kb) mb_kbd_run(kb);

  return 0;
}
