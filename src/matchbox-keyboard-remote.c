/*
 *  Matchbox Keyboard - A lightweight software keyboard.
 *
 *  Authored By Matthew Allum <mallum@o-hand.com>
 *
 *  Copyright (c) 2007-2012 Intel Corp
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
#include "matchbox-keyboard-remote.h"

static Atom Atom_MB_IM_INVOKER_COMMAND;

void
mb_kbd_remote_init (MBKeyboardUI *ui)
{
  Atom_MB_IM_INVOKER_COMMAND = XInternAtom(mb_kbd_ui_x_display(ui), 
					   "_MB_IM_INVOKER_COMMAND", False);
}

MBKeyboardRemoteOperation
mb_kbd_remote_process_xevents (MBKeyboardUI *ui, XEvent *xevent)
{
  DBG("got a message\n");

  switch (xevent->type)
    {
    case ClientMessage:
      DBG("is a Client Message\n");
      if (xevent->xclient.message_type == Atom_MB_IM_INVOKER_COMMAND)
        {
	  DBG("got a message of type _MB_IM_INVOKER_COMMAND, val %lu\n",
	      xevent->xclient.data.l[0]);
	  return xevent->xclient.data.l[0];
	}
    }
  return MBKeyboardRemoteNone;
}
