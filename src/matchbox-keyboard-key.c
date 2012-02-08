/*
 *  Matchbox Keyboard - A lightweight software keyboard.
 *
 *  Authored By Matthew Allum <mallum@o-hand.com>
 *
 *  Copyright (c) 2005-2012 Intel Corp
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

#define MBKB_N_KEY_STATES 5

typedef struct MBKeyboardKeyFace
{
  MBKeyboardKeyFaceType  type;

  union
  {
    MBKeyboardImage *image;
    char            *str;
  } u;
} 
MBKeyboardKeyFace;

typedef struct MBKeyboardKeyAction
{
  MBKeyboardKeyActionType  type;
  union 
  {
    char                  *glyph;
    KeySym                 keysym;
    MBKeyboardKeyModType   type;
  } u;
} 
MBKeyboardKeyAction;


/* A key can have 5 different 'states' */

typedef struct MBKeyboardKeyState
{
  MBKeyboardKeyAction action;
  MBKeyboardKeyFace   face;

} MBKeyboardKeyState;

struct MBKeyboardKey
{
  MBKeyboard            *kbd;
  MBKeyboardKeyState    *states[N_MBKeyboardKeyStateTypes];
  MBKeyboardRow         *row;

  int                    alloc_x, alloc_y, alloc_width, alloc_height;

  int                    extra_width_pad;  /* via win resizes */
  int                    extra_height_pad;

  boolean                obeys_caps;
  boolean                fill;	     /* width fills avialble space */
  int                    req_uwidth; /* unit width in 1/1000's */
  boolean                is_blank;   /* 'blank' keys are spacers */
  boolean                extended;   /* only show in landscape */

  MBKeyboardStateType    sets_kbdstate; /* needed */
};

static void
_mb_kbd_key_init_state(MBKeyboardKey           *key,
		       MBKeyboardKeyStateType   state)
{
  key->states[state] = util_malloc0(sizeof(MBKeyboardKeyState));
}

MBKeyboardKey*
mb_kbd_key_new(MBKeyboard *kbd)
{
  MBKeyboardKey *key = NULL;
  int            i;

  key      = util_malloc0(sizeof(MBKeyboardKey));
  key->kbd = kbd;

  for (i=0; i<N_MBKeyboardKeyStateTypes; i++)
    key->states[i] = NULL;

  return key;
}

void
mb_kbd_key_set_obey_caps(MBKeyboardKey  *key, boolean obey)
{
  key->obeys_caps = obey;
}

boolean
mb_kbd_key_get_obey_caps(MBKeyboardKey  *key)
{
  return key->obeys_caps;
}

void
mb_kbd_key_set_req_uwidth(MBKeyboardKey  *key, int uwidth)
{
  key->req_uwidth = uwidth;
}

int
mb_kbd_key_get_req_uwidth(MBKeyboardKey  *key)
{
  return key->req_uwidth;
}

void
mb_kbd_key_set_fill(MBKeyboardKey  *key, boolean fill)
{
  MARK();
  key->fill = fill;
}

boolean
mb_kbd_key_get_fill(MBKeyboardKey  *key)
{
  return key->fill;
}

void
mb_kbd_key_set_blank(MBKeyboardKey  *key, boolean blank)
{
  key->is_blank = blank;
}

boolean
mb_kbd_key_is_blank(MBKeyboardKey  *key)
{
  return key->is_blank;
}


void
mb_kbd_key_set_geometry(MBKeyboardKey  *key,
			int x,
			int y,
			int width,
			int height)
{
  if (x != -1)
    key->alloc_x = x;

  if (y != -1)
    key->alloc_y = y;

  if (width != -1)
    key->alloc_width = width;

  if (height != -1)
    key->alloc_height = height;
}

int 
mb_kbd_key_abs_x(MBKeyboardKey *key) 
{ 
  return mb_kbd_row_x(key->row) + key->alloc_x;
}

int 
mb_kbd_key_abs_y(MBKeyboardKey *key) 
{ 
  return mb_kbd_row_y(key->row) + key->alloc_y;
}

int 
mb_kbd_key_x(MBKeyboardKey *key) 
{ 
  return key->alloc_x;
}

int 
mb_kbd_key_y(MBKeyboardKey *key) 
{ 
  return key->alloc_y;
}

int 
mb_kbd_key_width(MBKeyboardKey *key) 
{ 
  return key->alloc_width;
}

int 
mb_kbd_key_height(MBKeyboardKey *key) 
{ 
  return key->alloc_height;
}

void
mb_kbd_key_set_extra_width_pad(MBKeyboardKey  *key, int pad)
{
  key->alloc_width -= key->extra_width_pad;
  key->extra_width_pad = pad;
  key->alloc_width += key->extra_width_pad;
}

void
mb_kbd_key_set_extra_height_pad(MBKeyboardKey  *key, int pad)
{
  key->alloc_height -= key->extra_height_pad;
  key->extra_height_pad = pad;
  key->alloc_height += key->extra_height_pad;
}

int
mb_kbd_key_get_extra_height_pad(MBKeyboardKey  *key)
{
  return key->extra_height_pad;
}

int
mb_kbd_key_get_extra_width_pad(MBKeyboardKey  *key)
{
  return key->extra_width_pad;
}

void
mb_kbd_key_set_extended(MBKeyboardKey  *key, boolean extend)
{
  key->extended = extend;
}

boolean
mb_kbd_key_get_extended(MBKeyboardKey  *key)
{
  return key->extended;
}


/* URG Nasty - some stuf should be public-ish */
void 
mb_kbd_key_set_row(MBKeyboardKey *key, MBKeyboardRow *row) 
{ 
  key->row = row;
}


boolean
mb_kdb_key_has_state(MBKeyboardKey           *key,
		     MBKeyboardKeyStateType   state)
{
  return (key->states[state] != NULL);
}

void
mb_kbd_key_set_glyph_face(MBKeyboardKey           *key,
			  MBKeyboardKeyStateType   state,
			  const char              *glyph)
{
  if (key->states[state] == NULL)
    _mb_kbd_key_init_state(key, state);

  key->states[state]->face.type    = MBKeyboardKeyFaceGlyph;
  key->states[state]->face.u.str   = strdup(glyph);
}

const char*
mb_kbd_key_get_glyph_face(MBKeyboardKey           *key,
			  MBKeyboardKeyStateType   state)
{
  if (key->states[state] 
      && key->states[state]->face.type == MBKeyboardKeyFaceGlyph)
    {
      return key->states[state]->face.u.str;
    }
  return NULL;
}

void
mb_kbd_key_set_image_face(MBKeyboardKey           *key,
			  MBKeyboardKeyStateType   state,
			  MBKeyboardImage         *image)
{

  if (key->states[state] == NULL)
    _mb_kbd_key_init_state(key, state);

  key->states[state]->face.type    = MBKeyboardKeyFaceImage;
  key->states[state]->face.u.image = image;
}

MBKeyboardImage*
mb_kbd_key_get_image_face(MBKeyboardKey           *key,
			  MBKeyboardKeyStateType   state)
{
  if (key->states[state] 
      && key->states[state]->face.type == MBKeyboardKeyFaceImage)
    {
      return key->states[state]->face.u.image;
    }
  return NULL;
}

void
mb_kbd_key_set_char_action(MBKeyboardKey           *key,
			   MBKeyboardKeyStateType   state,
			   const char              *glyphs)
{
  if (key->states[state] == NULL)
    _mb_kbd_key_init_state(key, state);
  
  key->states[state]->action.type = MBKeyboardKeyActionGlyph;
  key->states[state]->action.u.glyph = strdup(glyphs);
}

const char*
mb_kbd_key_get_char_action(MBKeyboardKey           *key,
			   MBKeyboardKeyStateType   state)
{
  if (key->states[state] 
      && key->states[state]->action.type == MBKeyboardKeyActionGlyph)
    return key->states[state]->action.u.glyph;

  return NULL;
}

void
mb_kbd_key_set_keysym_action(MBKeyboardKey           *key,
			     MBKeyboardKeyStateType   state,
			     KeySym                   keysym)
{
  if (key->states[state] == NULL)
    _mb_kbd_key_init_state(key, state);

  key->states[state]->action.type = MBKeyboardKeyActionXKeySym;
  key->states[state]->action.u.keysym = keysym;
}

KeySym
mb_kbd_key_get_keysym_action(MBKeyboardKey           *key,
			     MBKeyboardKeyStateType   state)
{
  if (key->states[state] 
      && key->states[state]->action.type == MBKeyboardKeyActionXKeySym)
    return key->states[state]->action.u.keysym;

  return None;
}


void
mb_kbd_key_set_modifer_action(MBKeyboardKey          *key,
			      MBKeyboardKeyStateType  state,
			      MBKeyboardKeyModType    type)
{
  if (key->states[state] == NULL)
    _mb_kbd_key_init_state(key, state);

  key->states[state]->action.type = MBKeyboardKeyActionModifier;
  key->states[state]->action.u.type   = type;
}

MBKeyboardKeyModType 
mb_kbd_key_get_modifer_action(MBKeyboardKey          *key,
			      MBKeyboardKeyStateType  state)
{
  if (key->states[state] 
      && key->states[state]->action.type == MBKeyboardKeyActionModifier)
    return key->states[state]->action.u.type;

  return 0;
}


MBKeyboardKeyFaceType
mb_kbd_key_get_face_type(MBKeyboardKey           *key,
			 MBKeyboardKeyStateType   state)
{
  if (key->states[state]) 
    return key->states[state]->face.type;

  return 0;
}


MBKeyboardKeyActionType
mb_kbd_key_get_action_type(MBKeyboardKey           *key,
			   MBKeyboardKeyStateType   state)
{
  if (key->states[state]) 
    return key->states[state]->action.type;

  return 0;
}


void
mb_kbd_key_press(MBKeyboardKey *key)
{
  /* XXX what about state handling XXX */
  MBKeyboardKeyStateType state;
  int                    flags = 0;
  boolean                queue_full_kbd_redraw = False;

  if (mb_kbd_key_is_blank(key))
    return;

  state = mb_kbd_keys_current_state(key->kbd);

  if (mb_kbd_has_state(key->kbd, MBKeyboardStateCaps)
      && mb_kbd_key_get_obey_caps(key))
    state = MBKeyboardKeyStateShifted;

  /* XXX below fakekey mods probably better in ui */

  if (mb_kbd_has_state(key->kbd, MBKeyboardStateControl))
    flags |= FAKEKEYMOD_CONTROL;

  if (mb_kbd_has_state(key->kbd, MBKeyboardStateAlt))
    flags |= FAKEKEYMOD_ALT;

  if (!mb_kdb_key_has_state(key, state))
    {
      if (state == MBKeyboardKeyStateNormal)
	return;  /* keys should at least have a normal state */
      else
        state = MBKeyboardKeyStateNormal;
    }

  switch (mb_kbd_key_get_action_type(key, state))
    {
    case MBKeyboardKeyActionGlyph:
      {
	const char *key_char;

	if ((key_char = mb_kbd_key_get_char_action(key, state)) != NULL)
	  {
	    mb_kbd_ui_send_press(key->kbd->ui, key_char, flags);
	    mb_kbd_set_held_key(key->kbd, key);
	  }
	break;


      }
    case MBKeyboardKeyActionXKeySym:
      {
	KeySym ks;
	if ((ks = mb_kbd_key_get_keysym_action(key, state)) != None)
	  {
	    mb_kbd_ui_send_keysym_press(key->kbd->ui, ks, flags);
	    mb_kbd_set_held_key(key->kbd, key);
	  }
	break;
      }
    case MBKeyboardKeyActionModifier:
      {
	
	switch ( mb_kbd_key_get_modifer_action(key, state) )
	  {
	  case MBKeyboardKeyModShift:
	    mb_kbd_toggle_state(key->kbd, MBKeyboardStateShifted);
	    queue_full_kbd_redraw = True;
	    break;
	  case MBKeyboardKeyModMod1:
	    mb_kbd_toggle_state(key->kbd, MBKeyboardStateMod1);
	    queue_full_kbd_redraw = True;
	    break;
	  case MBKeyboardKeyModMod2:
	    mb_kbd_toggle_state(key->kbd, MBKeyboardStateMod2);
	    queue_full_kbd_redraw = True;
	    break;
	  case MBKeyboardKeyModMod3:
	    mb_kbd_toggle_state(key->kbd, MBKeyboardStateMod3);
	    queue_full_kbd_redraw = True;
	    break;
	  case MBKeyboardKeyModCaps:
	    mb_kbd_toggle_state(key->kbd, MBKeyboardStateCaps);
	    queue_full_kbd_redraw = True;
	    break;
          case MBKeyboardKeyModControl:
	    mb_kbd_toggle_state(key->kbd, MBKeyboardStateControl);
	    break;
	  case MBKeyboardKeyModAlt:
	    mb_kbd_toggle_state(key->kbd, MBKeyboardStateAlt);
	    break;
	  default:
	    DBG("unknown modifier action");
	    break;
	  }

	/* we dont actually have to send a key sym here - but should we ? 
         *
         * Also we dont set a held key, as we've changed the keyboard 
         * state instead.
	*/
	break;
      }

    default:
      break;
    }  
  
  if (queue_full_kbd_redraw)
    mb_kbd_redraw(key->kbd);
  else
    mb_kbd_redraw_key(key->kbd, key);
}

boolean 
mb_kbd_key_is_held(MBKeyboard *kbd, MBKeyboardKey *key)
{
  MBKeyboardKeyStateType  state;

  if (mb_kbd_get_held_key(key->kbd) ==  key)
    return True;

  /* XXX below should probably go into own func */

  state = mb_kbd_keys_current_state(kbd); 

  if (!mb_kdb_key_has_state(key, state))
    {
      if (state == MBKeyboardKeyStateNormal)
	return False;  /* keys should at least have a normal state */
      else
        state = MBKeyboardKeyStateNormal;
    }

  if (mb_kbd_key_get_action_type(key, state) == MBKeyboardKeyActionModifier)
    {
	switch ( mb_kbd_key_get_modifer_action(key, state) )
	  {
	  case MBKeyboardKeyModShift:
	    if (mb_kbd_has_state(kbd, MBKeyboardStateShifted))
	      return True;
	    break;
	  case MBKeyboardKeyModMod1:
	    if (mb_kbd_has_state(kbd, MBKeyboardStateMod1))
	      return True;
	    break;
	  case MBKeyboardKeyModMod2:
	    if (mb_kbd_has_state(kbd, MBKeyboardStateMod2))
	      return True;
	    break;
	  case MBKeyboardKeyModMod3:
	    if (mb_kbd_has_state(kbd, MBKeyboardStateMod3))
	      return True;
	    break;
	  case MBKeyboardKeyModCaps:
	    if (mb_kbd_has_state(kbd, MBKeyboardStateCaps))
	      return True;
	    break;
          case MBKeyboardKeyModControl:
	    if (mb_kbd_has_state(kbd, MBKeyboardStateControl))
	      return True;
	    break;
	  case MBKeyboardKeyModAlt:
	    if (mb_kbd_has_state(kbd, MBKeyboardStateAlt))
	      return True;
	    break;
	  default:
	    DBG("unknown modifier action");
	    break;
	  }
    }

  return False;
}

void
mb_kbd_key_release(MBKeyboard *kbd)
{
  MBKeyboardKey *key = mb_kbd_get_held_key(kbd);

  mb_kbd_set_held_key(kbd, NULL);

  if (key)
    {
      boolean queue_full_kbd_redraw = False;

      if (mb_kbd_key_get_action_type(key, MBKeyboardKeyStateNormal) != MBKeyboardKeyActionModifier)
	{
	  if (mb_kbd_has_any_state(kbd))
	    {
	      mb_kbd_remove_state(kbd, (MBKeyboardStateShifted|
					MBKeyboardStateMod1|
					MBKeyboardStateMod2|
					MBKeyboardStateMod3|
					MBKeyboardStateControl|
					MBKeyboardStateAlt));
	      queue_full_kbd_redraw = True;
	    }
	}

      if (queue_full_kbd_redraw)
	mb_kbd_redraw(key->kbd);
      else
	mb_kbd_redraw_key(key->kbd, key);

      mb_kbd_ui_send_release(kbd->ui);
    }
}

void
mb_kbd_key_dump_key(MBKeyboardKey *key)
{
  int i;

  char state_lookup[][32] =
    {
      /* MBKeyboardKeyStateNormal  */ "Normal",
      /* MBKeyboardKeyStateShifted */ "Shifted" ,
      /* MBKeyboardKeyStateMod1,    */ "Mod1" ,
      /* MBKeyboardKeyStateMod2,    */ "Mod2" ,
      /* MBKeyboardKeyStateMod3,    */ "Mod3" 
    };

  fprintf(stderr, "-------------------------------\n");
  fprintf(stderr, "Dumping info for key at %p\n", key);

  for (i = 0; i < N_MBKeyboardKeyStateTypes; i++)
    {
      if (mb_kdb_key_has_state(key, i))
	{
	  fprintf(stderr, "state : %s\n", state_lookup[i]);
	  fprintf(stderr, "\tface type %i", mb_kbd_key_get_face_type(key, i));

	  if (mb_kbd_key_get_face_type(key, i) == MBKeyboardKeyFaceGlyph)
	    {
	      fprintf(stderr, ", showing '%s'", 
		      mb_kbd_key_get_glyph_face(key , i));
	    }

	  fprintf(stderr, "\n");

	  fprintf(stderr, "\taction type %i\n", 
		  mb_kbd_key_get_action_type(key, i));

	}
    }

  fprintf(stderr, "-------------------------------\n");

}

