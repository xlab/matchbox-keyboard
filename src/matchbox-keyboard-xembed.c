/*
 *	Matchbox Keyboard - A lightweight software keyboard.
 *
 *  Authored By Matthew Allum <mallum@o-hand.com>
 *              Tomas Frydrych <tomas@sleepfive.com>
 *
 *  Copyright (c) 2005-2012 Intel Corp
 *  Copyright (c) 2012 Vernier Software & Technology
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

#define MAX_SUPPORTED_XEMBED_VERSION   1

#define XEMBED_MAPPED          (1 << 0)

/* XEMBED messages */
#define XEMBED_EMBEDDED_NOTIFY          0
#define XEMBED_WINDOW_ACTIVATE          1
#define XEMBED_WINDOW_DEACTIVATE        2
#define XEMBED_REQUEST_FOCUS            3
#define XEMBED_FOCUS_IN                 4
#define XEMBED_FOCUS_OUT                5
#define XEMBED_FOCUS_NEXT               6
#define XEMBED_FOCUS_PREV               7
/* 8-9 were used for XEMBED_GRAB_KEY/XEMBED_UNGRAB_KEY */
#define XEMBED_MODALITY_ON              10
#define XEMBED_MODALITY_OFF             11
#define XEMBED_REGISTER_ACCELERATOR     12
#define XEMBED_UNREGISTER_ACCELERATOR   13
#define XEMBED_ACTIVATE_ACCELERATOR     14

static Atom Atom_XEMBED; /* FIXME: put array of atoms in UI struct	*/
static Atom Atom_WM_DELETE_WINDOW;
static Atom Atom_WM_PROTOCOLS;

static void
mb_kbd_xembed_set_win_info (MBKeyboardUI *ui, int flags)
{
   CARD32 list[2];

   Atom atom_ATOM_XEMBED_INFO;

  atom_ATOM_XEMBED_INFO
    = XInternAtom(mb_kbd_ui_x_display(ui), "_XEMBED_INFO", False);


  list[0] = MAX_SUPPORTED_XEMBED_VERSION;
  list[1] = flags;
  XChangeProperty (mb_kbd_ui_x_display(ui),
                   mb_kbd_ui_x_win(ui),
                   atom_ATOM_XEMBED_INFO,
                   atom_ATOM_XEMBED_INFO, 32,
                   PropModeReplace, (unsigned char *) list, 2);
}

static Bool
mb_kbd_xembed_send_message (MBKeyboardUI *ui,
                            Window        w,
                            long          message,
                            long          detail,
                            long          data1,
                            long          data2)
{
  XEvent ev;

  memset(&ev, 0, sizeof(ev));

  ev.xclient.type = ClientMessage;
  ev.xclient.window = w;
  ev.xclient.message_type = Atom_XEMBED;
  ev.xclient.format = 32;
  ev.xclient.data.l[0] = CurrentTime; /* FIXME: Is this correct */
  ev.xclient.data.l[1] = message;
  ev.xclient.data.l[2] = detail;
  ev.xclient.data.l[3] = data1;
  ev.xclient.data.l[4] = data2;

  util_trap_x_errors();

  XSendEvent(mb_kbd_ui_x_display(ui), w, False, NoEventMask, &ev);
  XSync(mb_kbd_ui_x_display(ui), False);

  if (util_untrap_x_errors())
    return False;

  return True;
}

void
mb_kbd_xembed_init (MBKeyboardUI *ui)
{
  /* FIXME: Urg global  */
  Atom_XEMBED = XInternAtom(mb_kbd_ui_x_display(ui), "_XEMBED", False);
	Atom_WM_DELETE_WINDOW =
          XInternAtom(mb_kbd_ui_x_display(ui), "WM_DELETE_WINDOW", False);
	Atom_WM_PROTOCOLS =
          XInternAtom(mb_kbd_ui_x_display(ui), "WM_PROTOCOLS", False);

  mb_kbd_xembed_set_win_info (ui, 0);
}

void
mb_kbd_xembed_process_xevents (MBKeyboardUI *ui, XEvent *xevent)
{
  switch (xevent->type)
    {
    case MapNotify:
      DBG("### got Mapped ###");
      break;
    case ClientMessage:
      if (xevent->xclient.message_type == Atom_XEMBED)
        {
          switch (xevent->xclient.data.l[1])
            {
            case XEMBED_EMBEDDED_NOTIFY:
              /* We are now reparented. Call the repaint.
               * note, 'data1' ( see spec ) is is embedders window
               */
              DBG("### got XEMBED_EMBEDDED_NOTIFY ###");
              mb_kbd_ui_set_x_embedder (ui, xevent->xclient.data.l[3]);

              /* FIXME: we really want to know what our final
               *        size will be before mapping as this can
               *        look ugly when window is mapped then a
               *        load of resizes.
               *        Maybe fixible in GTK calling code ?
               */

              mb_kbd_ui_redraw(ui);

              XSync(mb_kbd_ui_x_display(ui), False);

              /* And please Map us */
              mb_kbd_xembed_set_win_info (ui, XEMBED_MAPPED);

              /*
               * The above should get us mapped, but there is a bug in GtkSocket
               * where the map is supposed to happen during the widget
               * allocation, but that gets short circuited if the widget size
               * does not change ... this happens to us, because we go into
               * great length in NgiKeyboard to avoid unsightly resizing of the
               * kbd window ... so we take the liberty of mapping ourselves,
               * since now we are already reparented.
               */
              XMapWindow (mb_kbd_ui_x_display(ui), mb_kbd_ui_x_win (ui));
              break;
            case XEMBED_WINDOW_ACTIVATE:
              /* FIXME: What to do here */
              DBG("### got XEMBED_WINDOW_ACTIVATE ###");
              break;
            case XEMBED_WINDOW_DEACTIVATE:
              /* FIXME: What to do here ? unmap or exit */
              DBG("### got XEMBED_WINDOW_DEACTIVATE ###");
              break;
            case XEMBED_FOCUS_OUT:
              DBG("### got XEMBED_FOCUS_OUT ###");
              break;
            case XEMBED_FOCUS_IN:
              DBG("### got XEMBED_FOCUS_IN ###");
              /*
               * Please never give us key focus...
               */
              if (mb_kbd_ui_x_embedder (ui))
                mb_kbd_xembed_send_message (ui,
                                            mb_kbd_ui_x_embedder (ui),
                                            XEMBED_FOCUS_NEXT,
                                            0, 0, 0);
              break; /* TODO: Modility + rest of spec ? */
            }
        }
      else if (xevent->xclient.message_type == Atom_WM_PROTOCOLS)
        {
          if (xevent->xclient.data.l[0] == Atom_WM_DELETE_WINDOW)
            {
              exit (0);
            }
        }
    }

  /* FIXME: Handle case of Embedder dieing ( Xfixes call ) ? */

}
