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

#include "matchbox-keyboard.h"

/* 
    <keyboard>

    <options>
       <font prefered-size=''>
       <size fixed='100x100'>
       <padding>
    </options>

    <layout id="name">
      <row>
        <key id="optional-id" obey-caps='true|false'
	     width="1000"   // 1/1000's of a unit key size
	     fill="true"    // Set width to available space
	     
             >
	  <default
	     display="a"                
	     display="image:" 
	     action="utf8char"     // optional, action defaults to this    
	     action="string"       // from lookup below
	     action="modifier:Shift|Alt|ctrl|mod1|mod2|mod3|caps"
	     action="xkeysym:XK_BLAH"
	  <shifted 
	     ...... >
	  <mod1
	     ...... >
	     
	/>
        <key ... />
	<key ... />
	<space width="1000"
      </row>
    </layout>


    </keyboard>
*/

struct _keysymlookup
{
  KeySym keysym;   char *name;  
} 
MBKeyboardKeysymLookup[] =
{
 { XK_BackSpace,   "backspace" },
 { XK_Tab,	   "tab"       },
 { XK_Linefeed,    "linefeed"  },
 { XK_Clear,       "clear"     },	
 { XK_Return,      "return"    },
 { XK_Pause,       "pause" },	
 { XK_Scroll_Lock, "scrolllock" },	
 { XK_Sys_Req,     "sysreq" },
 { XK_Escape,      "escape" },	
 { XK_Delete,      "delete" },	
 { XK_Home,        "home" },
 { XK_Left,        "left" },
 { XK_Up,          "up"   },
 { XK_Right,       "right" },
 { XK_Down,        "down"  },
 { XK_Prior,       "prior" },		
 { XK_Page_Up,     "pageup" },	
 { XK_Next,        "next"   },
 { XK_Page_Down,   "pagedown" },
 { XK_End,         "end" },
 { XK_Begin,	   "begin" },
 { XK_space,        "space" },
 { XK_F1,          "f1" },
 { XK_F2,          "f2" },
 { XK_F3,          "f3" },
 { XK_F4,          "f4" },
 { XK_F5,          "f5" },
 { XK_F6,          "f6" },
 { XK_F7,          "f7" },
 { XK_F8,          "f8" },
 { XK_F9,          "f9" },
 { XK_F10,         "f10" },
 { XK_F11,         "f11" },
 { XK_F12,         "f12" }
};

struct _modlookup
{
  char *name; MBKeyboardKeyModType type;
}
ModLookup[] =
{
  { "shift",   MBKeyboardKeyModShift },
  { "alt",     MBKeyboardKeyModAlt },
  { "ctrl",    MBKeyboardKeyModControl },
  { "control", MBKeyboardKeyModControl },
  { "mod1",    MBKeyboardKeyModMod1 },
  { "mod2",    MBKeyboardKeyModMod2 },
  { "mod3",    MBKeyboardKeyModMod3 },
  { "caps",    MBKeyboardKeyModCaps }
};

typedef struct MBKeyboardConfigState
{
  MBKeyboard       *keyboard;
  MBKeyboardLayout *current_layout;
  MBKeyboardRow    *current_row;
  MBKeyboardKey    *current_key;
  Bool              error;
  char             *error_msg;
  int               error_lineno;
  XML_Parser        parser;
}
MBKeyboardConfigState;

void 
set_error(MBKeyboardConfigState *state, char *msg)
{
  state->error = True;
  state->error_lineno = XML_GetCurrentLineNumber(state->parser);
  state->error_msg = msg;
}

KeySym
config_str_to_keysym(const char* str)
{
  int i;

  DBG("checking %s", str);

  for (i=0; i<sizeof(MBKeyboardKeysymLookup)/sizeof(struct _keysymlookup); i++)
    if (streq(str, MBKeyboardKeysymLookup[i].name))
      return MBKeyboardKeysymLookup[i].keysym;

  DBG("didnt find it %s", str);

  return 0;
}

MBKeyboardKeyModType
config_str_to_modtype(const char* str)
{
  int i;

  for (i=0; i<sizeof(ModLookup)/sizeof(struct _modlookup); i++)
    {
      DBG("checking '%s' vs '%s'", str, ModLookup[i].name);
      if (streq(str, ModLookup[i].name))
	return ModLookup[i].type;
    }

  return 0;
}


static char* 
config_load_file(MBKeyboard *kbd, char *variant_in)
{
  struct stat    stat_info;
  FILE*          fp;
  char          *result;
  char          *country  = NULL;  
  char          *variant  = NULL;
  char          *lang     = NULL;
  int            n = 0, i = 0;
  char           path[1024]; 	/* XXX MAXPATHLEN */

  /* keyboard[-country][-variant].xml */

  /* This is an overide mainly for people developing keyboard layouts  */

  if (getenv("MB_KBD_CONFIG"))
    {
      snprintf(path, 1024, "%s", getenv("MB_KBD_CONFIG"));

      DBG("checking %s\n", path);

      if (util_file_readable(path))
	goto load;

      return NULL;
    }

  lang = getenv("MB_KBD_LANG");

  if (lang == NULL)
    lang = getenv("LANG");

  if (lang)
    {
      n = strlen(lang) + 2;

      country = alloca(n);

      snprintf(country, n, "-%s", lang);

      /* strip anything after first '.' */
      while(country[i] != '\0')
	if (country[i] == '.')
	  country[i] = '\0';
	else
	  i++;
    }

  if (variant_in)
    {
      n = strlen(variant_in) + 2;
      variant = alloca(n);
      snprintf(variant, n, "-%s", variant_in);
    }

  if (getenv("HOME"))
    {
      snprintf(path, 1024, "%s/.matchbox/keyboard.xml", getenv("HOME"));

      DBG("checking %s\n", path);

      if (util_file_readable(path))
	goto load;
    }

  /* Hmmm :/ */
 
  snprintf(path, 1024, PKGDATADIR "/keyboard%s%s.xml",
	   country == NULL ? "" : country,
	   variant == NULL ? "" : variant);

  DBG("checking %s\n", path);
  
  if (util_file_readable(path))
    goto load;

  snprintf(path, 1024, PKGDATADIR "/keyboard%s.xml",
	   variant == NULL ? "" : variant);

  DBG("checking %s\n", path);

  if (util_file_readable(path))
    goto load;

  snprintf(path, 1024, PKGDATADIR "/keyboard%s.xml",
	   country == NULL ? "" : country);

  DBG("checking %s\n", path);

  if (util_file_readable(path))
    goto load;

  snprintf(path, 1024, PKGDATADIR "/keyboard.xml");
  
  DBG("checking %s\n", path);

  if (!util_file_readable(path))
    return NULL;

 load:

  if (stat(path, &stat_info)) 
    return NULL;

  if ((fp = fopen(path, "rb")) == NULL) 
    return NULL;

  DBG("loading %s\n", path);

  kbd->config_file = strdup(path);

  result = malloc(stat_info.st_size + 1);

  n = fread(result, 1, stat_info.st_size, fp);

  if (n >= 0) result[n] = '\0';
  
  fclose(fp);

  return result;
}

static const char *
attr_get_val (char *key, const char **attr)
{
  int i = 0;
  
  while (attr[i] != NULL)
    {
      if (!strcmp(attr[i], key))
	return attr[i+1];
      i += 2;
    }
  
  return NULL;
}


static void
config_handle_key_subtag(MBKeyboardConfigState *state,
			 const char            *tag,
			 const char           **attr)
{
  MBKeyboardKeyStateType keystate;
  const char            *val;
  KeySym                 found_keysym;

  /* TODO: Fix below with a lookup table 
   */
  if (streq(tag, "normal") || streq(tag, "default"))
    {
      keystate = MBKeyboardKeyStateNormal;
    }
  else if (streq(tag, "shifted"))
    {
      keystate = MBKeyboardKeyStateShifted;
    }
  else if (streq(tag, "mod1"))
    {
      keystate = MBKeyboardKeyStateMod1;
    }
  else if (streq(tag, "mod2"))
    {
      keystate = MBKeyboardKeyStateMod2;
    }
  else if (streq(tag, "mod3"))
    {
      keystate = MBKeyboardKeyStateMod3;
    }
  else
    {
      set_error(state, "Unknown key subtag");
      return;
    }

  if ((val = attr_get_val("display", attr)) == NULL)
    {
      set_error(state, "Attribute 'display' is required");
      return;
    }

  if (!strncmp(val, "image:", 6))
    {
      MBKeyboardImage *img;


      if (val[6] != '/')
	{
	  /* Relative, rather than absolute path, try pkddatadir and home */
	  char buf[512];
	  snprintf(buf, 512, "%s/%s", PKGDATADIR, &val[6]);

	  if (!util_file_readable(buf))
	    snprintf(buf, 512, "%s/.matchbox/%s", getenv("HOME"), &val[6]);

	  img = mb_kbd_image_new (state->keyboard, buf);
	}
      else
	img = mb_kbd_image_new (state->keyboard, &val[6]);

      if (!img)
	{
	  fprintf(stderr, "matchbox-keyboard: Failed to load '%s'\n", &val[6]);
	  state->error = True;
	  return;
	}
      mb_kbd_key_set_image_face(state->current_key, keystate, img);
    }
  else
    {
      mb_kbd_key_set_glyph_face(state->current_key, keystate, 
				attr_get_val("display", attr));
    }

  if ((val = attr_get_val("action", attr)) != NULL)
    {
      /*
	     action="utf8char"     // optional, action defulats to this    
	     action="modifier:Shift|Alt|ctrl|mod1|mod2|mod3|caps"
	     action="xkeysym:XK_BLAH"
	     action="control:">    // return etc - not needed use lookup 
      */

      if (!strncmp(val, "modifier:", 9))
	{
	  MBKeyboardKeyModType found_type;

	  DBG("checking '%s'", &val[9]);

	  found_type = config_str_to_modtype(&val[9]);

	  if (found_type)
	    {
	      mb_kbd_key_set_modifer_action(state->current_key,
					    keystate,
					    found_type);
	    }
	  else
	    {
              set_error(state, "Unknown modifier");
	      return;
	    }
	  
	}
      else if (!strncmp(val, "xkeysym:", 8))
	{
	  DBG("Checking %s\n", &val[8]);

	  found_keysym = XStringToKeysym(&val[8]);

	  if (found_keysym)
	    {
	      mb_kbd_key_set_keysym_action(state->current_key, 
					   keystate,
					   found_keysym);
	    }
	  else 
	    {
	      /* Should this error really be terminal */
              set_error(state, "Unknown keysym");
	      return;
	    }
	}
      else
	{
	  /* Its just 'regular' key  */

	  if (strlen(val) > 1  	/* match backspace, return etc */
	      && ((found_keysym  = config_str_to_keysym(val)) != 0))
	    {
	      mb_kbd_key_set_keysym_action(state->current_key, 
					   keystate,
					   found_keysym);
	    }
	  else
	    {
	      /* XXX We should actually check its a single UTF8 Char here */
	      mb_kbd_key_set_char_action(state->current_key, 
					 keystate, val);
	    }
	}
    }
  else /* fallback to reusing whats displayed  */
    {

      /* display could be an image in which case we should throw an error 
       * or summin.
      */

      mb_kbd_key_set_char_action(state->current_key, 
				 keystate, 
				 attr_get_val("display", attr));
    }

}

static void
config_handle_layout_tag(MBKeyboardConfigState *state, const char **attr)
{
  const char            *val;

  if ((val = attr_get_val("id", attr)) == NULL)
    {
      set_error(state, "Attribute 'id' is required");
      return;
    }

  state->current_layout = mb_kbd_layout_new(state->keyboard, val);

  mb_kbd_add_layout(state->keyboard, state->current_layout);
}

static void
config_handle_row_tag(MBKeyboardConfigState *state, const char **attr)
{
  state->current_row = mb_kbd_row_new(state->keyboard);
  mb_kbd_layout_append_row(state->current_layout, state->current_row);
}

static void
config_handle_key_tag(MBKeyboardConfigState *state, const char **attr)
{
  const char *val;
  DBG("got key");

  state->current_key = mb_kbd_key_new(state->keyboard);

  if ((val = attr_get_val("obey-caps", attr)) != NULL)
    {
      if (strcaseeq(val, "true"))
	mb_kbd_key_set_obey_caps(state->current_key, True);
    }

  if ((val = attr_get_val("extended", attr)) != NULL)
    {
      if (strcaseeq(val, "true"))
	mb_kbd_key_set_extended(state->current_key, True);
    }

  if ((val = attr_get_val("width", attr)) != NULL)
    {
      if (atoi(val) > 0)
	mb_kbd_key_set_req_uwidth(state->current_key, atoi(val));
    }

  if ((val = attr_get_val("fill", attr)) != NULL)
    {
      if (strcaseeq(val, "true"))
	mb_kbd_key_set_fill(state->current_key, True);
    }

  mb_kbd_row_append_key(state->current_row, state->current_key);
}

static void 
config_xml_start_cb(void *data, const char *tag, const char **attr)
{
  MBKeyboardConfigState *state = (MBKeyboardConfigState *)data;

  if (streq(tag, "layout"))
    {
      config_handle_layout_tag(state, attr);
    }
  else if (streq(tag, "row"))
    {
      config_handle_row_tag(state, attr);
    }
  else if (streq(tag, "key"))
    {
      config_handle_key_tag(state, attr);
    }
  else  if (streq(tag, "space"))
    {
      config_handle_key_tag(state, attr);
      mb_kbd_key_set_blank(state->current_key, True);
    }
  else if (streq(tag, "normal") 
	   || streq(tag, "default")
	   || streq(tag, "shifted")
	   || streq(tag, "mod1")
	   || streq(tag, "mod2")
	   || streq(tag, "mod3"))
    {
      config_handle_key_subtag(state, tag, attr);
    }

  if (state->error)
    {
      fprintf(stderr, "matchbox-keyboard:%s:%d: %s\n", state->keyboard->config_file, 
                                              state->error_lineno, state->error_msg);
      util_fatal_error("Error parsing\n");
    }
}


int
mb_kbd_config_load(MBKeyboard *kbd, char *variant)
{
  char                  *data;
  XML_Parser             p;
  MBKeyboardConfigState *state;

  if ((data = config_load_file(kbd, variant)) == NULL)
    util_fatal_error("Couldn't find a keyboard config file\n");

  p = XML_ParserCreate(NULL);

  if (!p) 
    util_fatal_error("Couldn't allocate memory for XML parser\n");

  if (variant && !strstr(kbd->config_file, variant))
    fprintf(stderr, 
	    "matchbox-keyboard: *Warning* Unable to locate variant: %s\n"
	    "                   falling back to %s\n",
	    variant, kbd->config_file);

  state = util_malloc0(sizeof(MBKeyboardConfigState));

  state->keyboard = kbd;
  state->parser = p;

  XML_SetElementHandler(p, config_xml_start_cb, NULL);

  /* XML_SetCharacterDataHandler(p, chars); */

  XML_SetUserData(p, (void *)state);

  if (! XML_Parse(p, data, strlen(data), 1)) {
    fprintf(stderr, 
	    "matchbox-keyboard:%s:%d: XML Parse error:%s\n",
	    kbd->config_file,
	    XML_GetCurrentLineNumber(p),
	    XML_ErrorString(XML_GetErrorCode(p)));
    util_fatal_error("XML Parse failed.\n");
  }

  return 1;
}

