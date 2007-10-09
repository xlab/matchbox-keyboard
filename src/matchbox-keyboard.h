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

#ifndef HAVE_MB_KEYBOARD_H
#define HAVE_MB_KEYBOARD_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>

#include <png.h>

#include <locale.h>

#include <expat.h>

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>
#include <X11/Xresource.h>
#include <X11/keysym.h>

#include <fakekey/fakekey.h>

#if HAVE_CONFIG_H
#include "config.h"
#endif

#include "matchbox-keyboard-remote.h"

#if (WANT_DEBUG)
#define DBG(x, a...) \
 fprintf (stderr,  __FILE__ ":%d,%s() " x "\n", __LINE__, __func__, ##a)
#else
#define DBG(x, a...) do {} while (0)
#endif

#define MARK() DBG("mark")

typedef void*         pointer;
typedef unsigned char uchar ;
typedef Bool          boolean ;

typedef struct Pixbuf Pixbuf;
typedef struct List List;

typedef void (*ListForEachCB) (void *data, void *userdata);

struct List 
{
  List *next, *prev;
  void *data;
};

typedef struct MBKeyboard       MBKeyboard;
typedef struct MBKeyboardLayout MBKeyboardLayout;
typedef struct MBKeyboardRow    MBKeyboardRow;
typedef struct MBKeyboardKey    MBKeyboardKey;
typedef struct MBKeyboardUI     MBKeyboardUI;
typedef struct MBKeyboardUIBackend MBKeyboardUIBackend;
typedef struct MBKeyboardImage  MBKeyboardImage;

typedef enum 
{
  MBKeyboardKeyActionNone  = 0,
  MBKeyboardKeyActionGlyph,
  MBKeyboardKeyActionXKeySym, 	/* 'specials' be converted into this */
  MBKeyboardKeyActionModifier,

} MBKeyboardKeyActionType;

typedef enum 
{
  MBKeyboardKeyModUnknown,
  MBKeyboardKeyModShift,
  MBKeyboardKeyModMod1,
  MBKeyboardKeyModMod2,
  MBKeyboardKeyModMod3,
  MBKeyboardKeyModCaps,
  MBKeyboardKeyModControl,
  MBKeyboardKeyModAlt,
  MBKeyboardKeyModLayout

} MBKeyboardKeyModType;

typedef enum 
{
  MBKeyboardKeyFaceNone  = 0,
  MBKeyboardKeyFaceGlyph = 1,
  MBKeyboardKeyFaceImage = 2,

} MBKeyboardKeyFaceType;

typedef enum 
{
  MBKeyboardKeyStateNormal = 0,
  MBKeyboardKeyStateShifted,
  MBKeyboardKeyStateMod1,
  MBKeyboardKeyStateMod2,
  MBKeyboardKeyStateMod3,
  N_MBKeyboardKeyStateTypes
} 
MBKeyboardKeyStateType;

typedef enum 
{
  MBKeyboardStateNormal = 0,
  MBKeyboardStateShifted = (1<<1),
  MBKeyboardStateMod1    = (1<<2),
  MBKeyboardStateMod2    = (1<<3),
  MBKeyboardStateMod3    = (1<<4),
  MBKeyboardStateCaps    = (1<<5),
  MBKeyboardStateControl = (1<<6),
  MBKeyboardStateAlt     = (1<<7),
  N_MBKeyboardStateTypes
} 
MBKeyboardStateType;

typedef enum 
{
  MBKeyboardDisplayAny      = 0,
  MBKeyboardDisplayPortrait,
  MBKeyboardDisplayLandscape
} 
MBKeyboardDisplayOrientation;

struct MBKeyboard
{
  MBKeyboardUI          *ui;
  char                  *font_family;
  int                    font_pt_size;
  char                  *font_variant;
  char                  *config_file;
  List                  *layouts;
  MBKeyboardLayout      *selected_layout;
  int                    key_border, key_pad, key_margin;
  int                    row_spacing, col_spacing;
  boolean                extended; /* are we showing extended keys ? */
  MBKeyboardKey         *held_key;
  MBKeyboardStateType    keys_state;
};

/**** UI ***********/

struct MBKeyboardUIBackend
{
  MBKeyboardUIBackend*  (*init) (MBKeyboardUI *ui);
  int  (*font_load) (MBKeyboardUI  *ui);
  void (*redraw_key) (MBKeyboardUI  *ui, MBKeyboardKey *key);
  void (*pre_redraw) (MBKeyboardUI  *ui);
  int  (*resources_create) (MBKeyboardUI  *ui);
  int  (*resize) (MBKeyboardUI  *ui, int width, int height);
  void  (*text_extents) (MBKeyboardUI  *ui, 
			 const char    *str, 
			 int           *width, 
			 int           *height);
};

int
mb_kbd_ui_init(MBKeyboard *kbd);

void
mb_kbd_ui_limit_orientation (MBKeyboardUI                *ui, 
			     MBKeyboardDisplayOrientation orientation);

int
mb_kbd_ui_realize(MBKeyboardUI  *ui);

void
mb_kbd_ui_show(MBKeyboardUI  *ui);

void
mb_kbd_ui_hide(MBKeyboardUI  *ui);

void
mb_kbd_ui_redraw_key(MBKeyboardUI  *ui, MBKeyboardKey *key);

void
mb_kbd_ui_redraw(MBKeyboardUI  *ui);

void
mb_kbd_ui_swap_buffers(MBKeyboardUI  *ui);

void
mb_kbd_ui_send_press(MBKeyboardUI        *ui,
		     const char          *utf8_char_in,
		     int                  modifiers);

void
mb_kbd_ui_send_keysym_press(MBKeyboardUI  *ui,
			    KeySym         ks,
			    int            modifiers);

void
mb_kbd_ui_send_release(MBKeyboardUI  *ui);

int
mb_kbd_ui_display_width(MBKeyboardUI *ui);

int
mb_kbd_ui_display_height(MBKeyboardUI *ui);

MBKeyboardUIBackend*
mb_kbd_ui_backend(MBKeyboardUI *ui);

Display*
mb_kbd_ui_x_display(MBKeyboardUI *ui);

int
mb_kbd_ui_x_screen(MBKeyboardUI *ui);

Window
mb_kbd_ui_x_win(MBKeyboardUI *ui);

int
mb_kbd_ui_x_win_height(MBKeyboardUI *ui);

int
mb_kbd_ui_x_win_width(MBKeyboardUI *ui);

Window
mb_kbd_ui_x_win_root(MBKeyboardUI *ui);

Pixmap
mb_kbd_ui_backbuffer(MBKeyboardUI *ui);

MBKeyboard*
mb_kbd_ui_kbd(MBKeyboardUI *ui);

void
mb_kbd_ui_event_loop(MBKeyboardUI *ui);

void
mb_kbd_ui_set_embeded (MBKeyboardUI *ui, int embed);

void
mb_kbd_ui_set_daemon (MBKeyboardUI *ui, int value);
 
int
mb_kbd_ui_embeded (MBKeyboardUI *ui);

void
mb_kbd_ui_print_window (MBKeyboardUI *ui);

/*** Images ***/

MBKeyboardImage*
mb_kbd_image_new (MBKeyboard *kbd, const char *filename);

int
mb_kbd_image_width (MBKeyboardImage *img);

int
mb_kbd_image_height (MBKeyboardImage *img);

void
mb_kbd_image_destroy (MBKeyboardImage *img);

/*** XEmbed ***/

void
mb_kbd_xembed_init (MBKeyboardUI *ui);

void
mb_kbd_xembed_process_xevents (MBKeyboardUI *ui, XEvent *xevent);

/*** Remote ***/

void
mb_kbd_remote_init (MBKeyboardUI *ui);

MBKeyboardRemoteOperation
mb_kbd_remote_process_xevents (MBKeyboardUI *ui, XEvent *xevent);

/**** Keyboard ****/

int
mb_kbd_row_spacing(MBKeyboard *kb);

int
mb_kbd_col_spacing(MBKeyboard *kb);

int
mb_kbd_keys_border(MBKeyboard *kb);

int
mb_kbd_keys_pad(MBKeyboard *kb);

int
mb_kbd_keys_margin(MBKeyboard *kb);

void
mb_kbd_add_state(MBKeyboard *kbd, MBKeyboardStateType state);

void
mb_kbd_toggle_state(MBKeyboard *kbd, MBKeyboardStateType state);

boolean
mb_kbd_has_state(MBKeyboard *kbd, MBKeyboardStateType state);

boolean
mb_kbd_has_any_state(MBKeyboard *kbd);

void
mb_kbd_remove_state(MBKeyboard *kbd, MBKeyboardStateType state);

MBKeyboardKeyStateType
mb_kbd_keys_current_state(MBKeyboard *kbd);

void
mb_kbd_set_extended(MBKeyboard *kb, boolean extend);

boolean
mb_kbd_is_extended(MBKeyboard *kb);

void
mb_kbd_add_layout(MBKeyboard *kb, MBKeyboardLayout *layout);

MBKeyboardLayout*
mb_kbd_get_selected_layout(MBKeyboard *kb);

MBKeyboardKey*
mb_kbd_locate_key(MBKeyboard *kb, int x, int y);

void
mb_kbd_set_held_key(MBKeyboard *kb, MBKeyboardKey *key);

MBKeyboardKey *
mb_kbd_get_held_key(MBKeyboard *kb);

void
mb_kbd_redraw(MBKeyboard *kb);

void
mb_kbd_redraw_key(MBKeyboard *kb, MBKeyboardKey *key);


/**** Layout ****/

MBKeyboardLayout*
mb_kbd_layout_new(MBKeyboard *kbd, const char *id);

void
mb_kbd_layout_append_row(MBKeyboardLayout *layout,
			 MBKeyboardRow    *row);

List*
mb_kbd_layout_rows(MBKeyboardLayout *layout);


/**** Rows ******/

MBKeyboardRow*
mb_kbd_row_new(MBKeyboard *kbd);

void
mb_kbd_row_set_x(MBKeyboardRow *row, int x);

void
mb_kbd_row_set_y(MBKeyboardRow *row, int y);

int 
mb_kbd_row_x (MBKeyboardRow *row) ;

int 
mb_kbd_row_y(MBKeyboardRow *row) ;

int 
mb_kbd_row_height(MBKeyboardRow *row);

int 
mb_kbd_row_width(MBKeyboardRow *row);

int 
mb_kbd_row_base_width(MBKeyboardRow *row);

void
mb_kbd_row_append_key(MBKeyboardRow *row, MBKeyboardKey *key);

List*
mb_kdb_row_keys(MBKeyboardRow *row);

#define mb_kbd_row_for_each_key(r,k)            \
      for ((k) = mb_kdb_row_keys((r));          \
	   (k) != NULL;                         \
	   (k) = util_list_next((k))) 


/**** Keys ******/

MBKeyboardKey*
mb_kbd_key_new(MBKeyboard *kbd);

void
mb_kbd_key_set_obey_caps(MBKeyboardKey  *key, boolean obey);

boolean
mb_kbd_key_get_obey_caps(MBKeyboardKey  *key);

void
mb_kbd_key_set_req_uwidth(MBKeyboardKey  *key, int uwidth);

int
mb_kbd_key_get_req_uwidth(MBKeyboardKey  *key);

void
mb_kbd_key_set_fill(MBKeyboardKey  *key, boolean fill);

boolean
mb_kbd_key_get_fill(MBKeyboardKey  *key);

void
mb_kbd_key_set_blank(MBKeyboardKey  *key, boolean blank);

boolean
mb_kbd_key_is_blank(MBKeyboardKey  *key);

void 
mb_kbd_key_set_row(MBKeyboardKey *key, MBKeyboardRow *row);

void
mb_kbd_key_set_geometry(MBKeyboardKey  *key,
			int x,
			int y,
			int width,
			int height);
int 
mb_kbd_key_abs_x(MBKeyboardKey *key) ;

int 
mb_kbd_key_abs_y(MBKeyboardKey *key) ;

int 
mb_kbd_key_x(MBKeyboardKey *key) ;

int 
mb_kbd_key_y(MBKeyboardKey *key);

int 
mb_kbd_key_width(MBKeyboardKey *key) ;

int 
mb_kbd_key_height(MBKeyboardKey *key);

void
mb_kbd_key_set_extended(MBKeyboardKey  *key, boolean extend);

boolean
mb_kbd_key_get_extended(MBKeyboardKey  *key);

void
mb_kbd_key_set_extra_width_pad(MBKeyboardKey  *key, int pad);

void
mb_kbd_key_set_extra_height_pad(MBKeyboardKey  *key, int pad);

int
mb_kbd_key_get_extra_height_pad(MBKeyboardKey  *key);

int
mb_kbd_key_get_extra_width_pad(MBKeyboardKey  *key);

Bool
mb_kdb_key_has_state(MBKeyboardKey           *key,
		     MBKeyboardKeyStateType   state);

void
mb_kbd_key_set_glyph_face(MBKeyboardKey           *key,
			  MBKeyboardKeyStateType   state,
			  const  char             *glyph);

const char*
mb_kbd_key_get_glyph_face(MBKeyboardKey           *key,
			  MBKeyboardKeyStateType   state);

void
mb_kbd_key_set_image_face(MBKeyboardKey           *key,
			  MBKeyboardKeyStateType   state,
			  MBKeyboardImage         *image);

MBKeyboardImage*
mb_kbd_key_get_image_face(MBKeyboardKey           *key,
			  MBKeyboardKeyStateType   state);


MBKeyboardKeyFaceType
mb_kbd_key_get_face_type(MBKeyboardKey           *key,
			 MBKeyboardKeyStateType   state);

void
mb_kbd_key_set_char_action(MBKeyboardKey           *key,
			   MBKeyboardKeyStateType   state,
			   const char              *glyphs);

void
mb_kbd_key_set_keysym_action(MBKeyboardKey           *key,
			     MBKeyboardKeyStateType   state,
			     KeySym                   keysym);

KeySym
mb_kbd_key_get_keysym_action(MBKeyboardKey           *key,
			     MBKeyboardKeyStateType   state);

void
mb_kbd_key_set_modifer_action(MBKeyboardKey          *key,
			      MBKeyboardKeyStateType  state,
			      MBKeyboardKeyModType    type);

MBKeyboardKeyModType 
mb_kbd_key_get_modifer_action(MBKeyboardKey          *key,
			      MBKeyboardKeyStateType  state);

boolean 
mb_kbd_key_is_held(MBKeyboard *kbd, MBKeyboardKey *key);

void
mb_kbd_key_press(MBKeyboardKey *key);

void
mb_kbd_key_release(MBKeyboard *kbd);

void
mb_kbd_key_dump_key(MBKeyboardKey *key);

#define mb_kdb_key_foreach_state(k,s)                     \
       for((s)=0; (s) < N_MBKeyboardKeyStateTypes; (s)++) \
            if (mb_kdb_key_has_state((k), (s)))

/*** Config *****/

int
mb_kbd_config_load(MBKeyboard *kbd, char *varient);


/**** Util *****/

#define streq(a,b)      (strcmp(a,b) == 0)
#define strcaseeq(a,b)  (strcasecmp(a,b) == 0)
#define unless(x)       if (!(x))
#define util_abs(x)     ((x) > 0) ? (x) : -1*(x)

void
util_trap_x_errors(void);

int
util_untrap_x_errors(void);

void*
util_malloc0(int size);

void
util_fatal_error(char *msg);

int
util_utf8_char_cnt(const char *str);

boolean 
util_file_readable(char *path);

/* Util list */

#define util_list_next(l) (l)->next
#define util_list_previous(l) (l)->prev

List*
util_list_alloc_item(void);

int
util_list_length(List *list);

List*
util_list_get_last(List *list);

List*
util_list_get_first(List *list);

void*
util_list_get_nth_data(List *list, int n);

List*
util_list_append(List *list, void *data);

void
util_list_foreach(List *list, ListForEachCB func, void *userdata);

/* Backends */

#if WANT_CAIRO
#include "matchbox-keyboard-ui-cairo-backend.h"
#else
#include "matchbox-keyboard-ui-xft-backend.h"

Picture
mb_kbd_image_render_picture (MBKeyboardImage *img);

#endif

#endif
