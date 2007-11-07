/* 
 *  Matchbox Keyboard - A lightweight software keyboard.
 *
 *  Author: Ross Burton <ross@o-hand.com>
 *
 *  Copyright (c) 2007 OpenedHand Ltd - http://o-hand.com
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

#include <string.h>
#include <X11/Xlib.h>

#include <gdk/gdk.h>
#include <gdk/gdkx.h>

#include "im-protocol.h"

void
protocol_send_event (MBKeyboardRemoteOperation op)
{
  XEvent event;
  int xerror;

  memset (&event, 0, sizeof (XEvent));

  event.xclient.type = ClientMessage;
  event.xclient.window = gdk_x11_get_default_root_xwindow ();
  event.xclient.message_type = gdk_x11_get_xatom_by_name ("_MB_IM_INVOKER_COMMAND");
  event.xclient.format = 32;
  event.xclient.data.l[0] = op;

  gdk_error_trap_push ();

  XSendEvent (GDK_DISPLAY (), 
	      gdk_x11_get_default_root_xwindow (), 
	      False,
	      SubstructureRedirectMask | SubstructureNotifyMask,
	      &event);

  XSync (GDK_DISPLAY(), False);
  
  if ((xerror = gdk_error_trap_pop ())) {
    g_warning ("X error %d", xerror);
  }
}
