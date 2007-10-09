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
