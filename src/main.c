/*
 *  Matchbox Keyboard - A lightweight software keyboard.
 *
 *  Authored By Matthew Allum <mallum@o-hand.com>
 *
 *  Copyright (c) 2005 OpenedHand Ltd - http://o-hand.com
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
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "matchbox-keyboard.h"

/*
 * These are located in matchbox-keyboard.c
 */
extern Display *mb_xdpy;
extern int      mb_xscreen;
extern Window   mb_xroot;

/*
 * X Event processing.
 */
static boolean
get_xevent_timed (Display *dpy, XEvent *event_return, struct timeval *tv)
{
  if (tv->tv_usec == 0 && tv->tv_sec == 0)
    {
      XNextEvent(dpy, event_return);
      return True;
    }

  XFlush(dpy);

  if (XPending(dpy) == 0)
    {
      int fd = ConnectionNumber(dpy);

      fd_set readset;
      FD_ZERO(&readset);
      FD_SET(fd, &readset);

      if (select(fd+1, &readset, NULL, NULL, tv) == 0)
        return False;
      else
        {
          XNextEvent(dpy, event_return);
          return True;
        }
    } else {
    XNextEvent(dpy, event_return);
    return True;
  }
}

static void
mb_kbd_event_loop (MBKeyboardUI *ui)
{
  MBKeyboardKey *key = NULL;
  struct timeval tvt;
  Display *xdpy = mb_kbd_ui_x_display (ui);
  MBKeyboard* kbd = mb_kbd_ui_kbd (ui);
  Window xroot = mb_kbd_ui_x_win_root (ui);
  Window xwin = mb_kbd_ui_x_win (ui);
  int xwin_height = mb_kbd_ui_x_win_height (ui);
  int xwin_width = mb_kbd_ui_x_win_width (ui);

  /* Key repeat - values for standard xorg install ( xset q) */
  int repeat_delay = 100 * 10000;
  int repeat_rate  = 30  * 1000;
  int hide_delay = 100 * 1000;
  int to_hide = 0;

  int press_x = 0, press_y = 0;

  tvt.tv_sec  = 0;
  tvt.tv_usec = repeat_delay;

  while (True)
    {
      XEvent xev;

      if (get_xevent_timed (xdpy, &xev, &tvt))
        {
          switch (xev.type)
            {
            case ButtonPress:
              press_x = xev.xbutton.x; press_y = xev.xbutton.y;
              DBG("got button press at %i,%i (%i,%i)",
                  xev.xbutton.x, xev.xbutton.y,
                  xev.xbutton.x_root, xev.xbutton.y_root);
              key = mb_kbd_locate_key(kbd, xev.xbutton.x, xev.xbutton.y);
              if (key)
                {
                  /* Hack if we never get a release event */
                  if (key != mb_kbd_get_held_key(kbd))
                    {
                      mb_kbd_key_release(kbd, True);
                      tvt.tv_usec = repeat_delay;
                    }
                  else
                    tvt.tv_usec = repeat_rate;

                  DBG("found key for press");
                  mb_kbd_key_press(key);
                  mb_kbd_show_popup (kbd, key,
                                     xev.xbutton.x_root - xev.xbutton.x,
                                     xev.xbutton.y_root - xev.xbutton.y);
                }
              break;
            case ButtonRelease:
              DBG("got button release at %i,%i (%i,%i)",
                  xev.xbutton.x, xev.xbutton.y,
                  xev.xbutton.x_root, xev.xbutton.y_root);
              if (mb_kbd_get_held_key(kbd) != NULL)
                {
                  Bool cancel = False;

                  key = mb_kbd_locate_key (kbd,
                                           xev.xbutton.x, xev.xbutton.y);
                  if (key != mb_kbd_get_held_key(kbd))
                    cancel = True;

                  mb_kbd_key_release (kbd, cancel);
                  tvt.tv_usec = repeat_delay;

                  /* Gestures */
#if 0
                  /* FIXME: check time first */
                  if ( (press_x - xev.xbutton.x) > ui->key_uwidth )
                    {
                      /* <-- slide back ...backspace */
                      fakekey_press_keysym(ui->fakekey, XK_BackSpace, 0);
                      fakekey_repeat(ui->fakekey);
                      fakekey_release(ui->fakekey);
                      /* FIXME: add <-- --> <-- --> support */
                    }
                  else if ( (xev.xbutton.y - press_y) > ui->key_uheight )
                    {
                      /* V slide down ...return  */
                      fakekey_press_keysym(ui->fakekey, XK_BackSpace, 0);
                      fakekey_release(ui->fakekey);
                      fakekey_press_keysym(ui->fakekey, XK_Return, 0);
                      fakekey_release(ui->fakekey);
                    }
#endif
                  /* TODO ^ caps support */

                }
              break;
            case ConfigureNotify:
              if (xev.xconfigure.window == xwin
                  &&  (xev.xconfigure.width != xwin_width
                       || xev.xconfigure.height != xwin_height))
                {
                  mb_kbd_ui_handle_configure(ui,
                                             xev.xconfigure.width,
                                             xev.xconfigure.height);
                }
              if (xev.xconfigure.window == xroot)
                mb_kbd_ui_update_display_size(ui);
              break;
            case MappingNotify:
              fakekey_reload_keysyms(mb_kbd_ui_get_fakekey (ui));
              XRefreshKeyboardMapping(&xev.xmapping);
              break;
            default:
              break;
            }
          if (mb_kbd_ui_embeded (ui))
            mb_kbd_xembed_process_xevents (ui, &xev);

          if (mb_kbd_ui_is_daemon (ui))
            {
              switch (mb_kbd_remote_process_xevents (ui, &xev))
                {
                case MBKeyboardRemoteHide:
                  if (to_hide == 1) {
                    mb_kbd_ui_hide(ui);
                  }
                  tvt.tv_usec = hide_delay;
                  to_hide = 1;
                  break;
                case MBKeyboardRemoteShow:
                  mb_kbd_ui_show(ui);
                  tvt.tv_usec = repeat_delay;
                  to_hide = 0;
                  break;
                case MBKeyboardRemoteToggle:
                  to_hide = 0;
                  tvt.tv_usec = repeat_delay;
                  if (mb_kbd_ui_is_visible (ui))
                    mb_kbd_ui_hide(ui);
                  else
                    mb_kbd_ui_show(ui);
                  break;
                case MBKeyboardRemoteNone:
                  if (to_hide == 1) {
                    mb_kbd_ui_hide(ui);
                    tvt.tv_usec = repeat_delay;
                    to_hide = 0;
                  }
                  break;
                }
            }
        }
      else
        {
          /* Hide timed out */
          if (to_hide)
            {
              DBG("Hide timed out, calling mb_kbd_ui_hide");
              mb_kbd_ui_hide(ui);
              tvt.tv_usec = repeat_delay;
              to_hide = 0;
            }

          /* Keyrepeat */
          if (mb_kbd_get_held_key(kbd) != NULL)
            {
              fakekey_repeat(mb_kbd_ui_get_fakekey (ui));
              tvt.tv_usec = repeat_rate;
            }
        }
    }
}

int
main(int argc, char **argv)
{
  MBKeyboard *kb;

  if ((mb_xdpy = XOpenDisplay(getenv("DISPLAY"))) == NULL)
    {
      fprintf (stderr, "Cannot open display\n");
      return 1;
    }

  mb_xscreen = DefaultScreen (mb_xdpy);
  mb_xroot = RootWindow (mb_xdpy, mb_xscreen);

  kb = mb_kbd_new (argc, argv, False, None, 0, 0, 0, 0);

  if (!mb_kbd_ui_realize(kb->ui))
    exit (1);

  unless (mb_kbd_ui_embeded(kb->ui))
    {
      if (mb_kbd_ui_is_daemon (kb->ui))
        {
          /* Dont map daemon to begin with */
          mb_kbd_remote_init (kb->ui);
        }
      else
        {
          mb_kbd_ui_show(kb->ui);
          mb_kbd_ui_redraw(kb->ui);
        }
    }
  else
    {
      mb_kbd_xembed_init (kb->ui);
      mb_kbd_ui_print_window (kb->ui);
    }

  if (kb)
    mb_kbd_event_loop (kb->ui);

  mb_kbd_destroy (kb);

  return 0;
}
