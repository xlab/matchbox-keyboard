#include "matchbox-keyboard.h"

#define MBKB_N_KEY_STATES 5

typedef struct MBKeyboardKeyFace
{
  MBKeyboardKeyFaceType  type;

  union
  {
    void          *image;
    unsigned char *str;
  } u;
} 
MBKeyboardKeyFace;

typedef struct MBKeyboardKeyAction
{
  MBKeyboardKeyActionType  type;
  union 
  {
    unsigned char *glyph;
    KeySym         keysym;
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
  int                    alloc_x, alloc_y, alloc_width, alloc_height;
  MBKeyboardKeyState    *states[N_MBKeyboardKeyStateTypes];
  MBKeyboardRow         *row;
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

Bool
mb_kdb_key_has_state(MBKeyboardKey           *key,
		     MBKeyboardKeyStateType   state)
{
  return (key->states[state] != NULL);
}

void
mb_kbd_key_set_glyph_face(MBKeyboardKey           *key,
			  MBKeyboardKeyStateType   state,
			  const unsigned char     *glyph)
{

  if (key->states[state] == NULL)
    _mb_kbd_key_init_state(key, state);

  key->states[state]->face.type    = MBKeyboardKeyFaceGlyph;
  key->states[state]->face.u.str   = strdup(glyph);
}

void
mb_kbd_key_set_image_face(MBKeyboardKey           *key,
			  MBKeyboardKeyStateType   state,
			  void                    *image)
{

  if (key->states[state] == NULL)
    _mb_kbd_key_init_state(key, state);

  key->states[state]->face.type    = MBKeyboardKeyFaceImage;
  key->states[state]->face.u.image = image;
}


void
mb_kbd_key_set_char_action(MBKeyboardKey           *key,
			   MBKeyboardKeyStateType   state,
			   const unsigned char     *glyphs)
{
  if (key->states[state] == NULL)
    _mb_kbd_key_init_state(key, state);
  
  key->states[state]->action.type = MBKeyboardKeyActionGlyph;
  key->states[state]->action.u.glyph = strdup(glyphs);
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

void
mb_kbd_key_set_modifer_action(MBKeyboardKey           *key,
			      MBKeyboardKeyStateType   state,
			      int                      modifier)
{
  if (key->states[state] == NULL)
    _mb_kbd_key_init_state(key, state);

  key->states[state]->action.type = MBKeyboardKeyActionModifier;
}

MBKeyboardKeyFaceType
mb_kbd_key_get_face_type(MBKeyboardKey           *key,
			 MBKeyboardKeyStateType   state)
{
    return 0;
}


MBKeyboardKeyActionType
mb_kbd_key_get_action_type(MBKeyboardKey           *key,
			   MBKeyboardKeyStateType   state)
{
  return 0;
}


