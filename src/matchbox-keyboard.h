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
  MBKeyboardKeyFaceImage,

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


struct MBKeyboard
{
  MBKeyboardUI          *ui;
  unsigned char         *font_family;
  int                    font_pt_size;
  unsigned char         *font_variant;

  unsigned char         *config_file;

  List                  *layouts;
  MBKeyboardLayout      *selected_layout;

  int                    key_border, key_pad, key_margin;
  int                    row_spacing, col_spacing;

  MBKeyboardKey         *held_key;
  MBKeyboardStateType    keys_state;
};

/**** UI ***********/

int
mb_kbd_ui_init(MBKeyboard *kbd);

int
mb_kbd_ui_realize(MBKeyboardUI  *ui);

void
mb_kbd_ui_redraw_key(MBKeyboardUI  *ui, MBKeyboardKey *key);

void
mb_kbd_ui_redraw(MBKeyboardUI  *ui);

void
mb_kbd_ui_swap_buffers(MBKeyboardUI  *ui);

void
mb_kbd_ui_send_press(MBKeyboardUI  *ui,
		     unsigned char *utf8_char_in,
		     int            modifiers);

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
			  const unsigned char     *glyph);

const unsigned char*
mb_kbd_key_get_glyph_face(MBKeyboardKey           *key,
			  MBKeyboardKeyStateType   state);

void
mb_kbd_key_set_image_face(MBKeyboardKey           *key,
			  MBKeyboardKeyStateType   state,
			  void                    *image);

MBKeyboardKeyFaceType
mb_kbd_key_get_face_type(MBKeyboardKey           *key,
			 MBKeyboardKeyStateType   state);

void
mb_kbd_key_set_char_action(MBKeyboardKey           *key,
			   MBKeyboardKeyStateType   state,
			   const unsigned char     *glyphs);

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

void*
util_malloc0(int size);

void
util_fatal_error(char *msg);

int
util_utf8_char_cnt(const unsigned char *str);

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

List*
util_list_foreach(List *list, ListForEachCB func, void *userdata);


#endif
