/*
 *  Matchbox Keyboard - A lightweight software keyboard.
 *
 *  Author: Ross Burton <ross@o-hand.com>
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
