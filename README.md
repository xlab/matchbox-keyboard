# MATCHBOX KEYBOARD 1.5 FORK

```
Usage:
   matchbox-keyboard [options] [layout]

Supported options are;
   -xid,--xid               Print window ID to stdout ( for embedding )
   -d,--daemon              Run in 'daemon' mode (for remote control)
   -t, --gestures           Enable gestures

------------------------- UI Tweaks & Positioning --------------------

   -p,--key-padding <px>    Key padding
   -c,--col-spacing <px>    Space between columns
   -r,--row-spacing <px>    Space between rows
   -f,--font-family <name>  Font family (ex: sans, droidsans)
   -s,--font-size <pt>      Font size
   -b,--non-bold            Switch to normal weight
   -v,--override            Absolute positioning on the screen
   -i,--invert              Attach keyboard to the top of the screen
   -g,--geometry <HxW.y.x>  Specify keyboard's geometry 
  (ex: -g 200x800; -g 0x800.200.0; -g 0x0.0.50; zeroes mean "by-default")

matchbox-keyboard 1.5 
Copyright (C) 2007 OpenedHand Ltd.
Modifications (C) 2009 Maxim Kouprianov ( http://kouprianov.com )
Special thanks to Paguro ( http://smartqmid.ru )
```

**New features:**
* Gestures x4
* Language switching
* Three great layouts:
  - `keyboard` -- Original layout, EN-RU
  - `keyboard-full` -- Full keyboard layout, by 123456 and SeNS, EN-RU
  - `keyboard-finger` -- Minimalistic keybord for fingers, by 123456, EN-RU
* UI tweaks (you may adjust spacings)
* Improved keyboard positioning in matchbox, openbox, etc
* Keyboard now can shrink neighbour windows - perfect!
* Custom geometry - scale as you want
* Performance tweaks...

![mbk2](https://dl.kc.vc/old/matchbox/Screenshot-2.png)

![mbk](https://dl.kc.vc/old/matchbox/mb_2.png)

## Matchbox-Keyboard README

### Introduction

Matchbox-keyboard is an on screen 'virtual' or 'software' keyboard. It
will hopefully work well on various touchscreen devices from mobile
phones to tablet PCs running X Windows.

It aims to 'just work' supporting localised, easy to write XML layout
configuration files.

Its made available under the GPL.


### Rational

I wrote 'xkbd' a few years back which tried to do the same thing. It
was the first xlib app I wrote and the first bit of C Id coded in
quite a few years. It was a mess but basically worked, though with
a few problems.

matchbox-keyboard is my much promised rewrite. The code is cleaner 
and it hopefully addresses much of the previous short comings of xkbd.


### Building

Do the usual autotool jig of ./configure, make, make install. ( If
building from SVN you'll need to run ./autogen.sh before this).

matchbox-keyboard needs xlibs, xft, libfakekey and expat to build -
The configure script will detect these. Also optionally there is
experimental cairo support for rendering the keys and example
embedded code.


### Running

Do;

`matchbox-keyboard [Options..] [optional variant name]`

and start typing. The config file will be selected based on locale
setting and supplied variant name. The only current option is -xid,
used for embedding ( see below ).

The following Environmental Variables are also used, if set;

* MB_KBD_CONFIG
 
   Set to full path of a alternate layout file to use overriding any other
   selection mechanisms.

* MB_KBD_VARIANT

   Same as the first argument to binary. If both set argument overrides.

* LANG, MB_KBD_LANG 

   The value up to the first '.' ( i.e en_GB ) is used to build up 
   the config file name based on locale. MB_KBD_LANG can be used to
   override the systems LANG var ( E.g, for the case of a Dutch person
   wanting a Dutch keyboard but an English locale - or the other way 
   round ).

### Embedding

You can embed matchbox-keyboard into other applications with toolkits
that support the XEMBED protocol ( GTK2 for example ). 

See `examples/matchbox-keyboard-gtk-embed.c` for how its done. 

### Making your own keyboard layouts

Keyboard layout files are UTF8 XML files ( Make sure they are saved
with this encoding! ). They are loaded from the directory
$PREFIX/share/matchbox-keyboard and are named in the format
keyboard[-locale][-variant].xml. This can be overridden by setting
MB_KBD_CONFIG environment variable to a valid config file path or by
creating $HOME/.matchbox/keyboard.xml.

The basic layout of the file looks like;

```xml
    <keyboard>

    <options>
    </options>

    <layout>
      <row>
        <key ...>
	  <default .. >
	  <shifted .. >
	  <mod1 .. >
          ....
	<key>
        .... more keys ...
	<space width="1000" />
      </row>
      <row>
      ...
      </row>
    </layout>

    </keyboard>
```

A number of layouts can be defined ( though currently only 1 is
supported ) each with any number of rows of keys, defining the
keyboard from top to bottom.

The most important tag to know about is the <key> tag and its
children. A key tag can optionally have the following attributes;

* obey-caps=true|false ( defaults to false if not declared )

  Specifies if the key obeys the Caps Lock key - Its shifted state
  is shown when the Caps key is held.

* width=1000th's of a 'base' key width.

  Override the automatically calculated key width in 1000th's of 
  a base key width ( The average width of a key with a single glyph ).

* fill=true|false ( defaults to false )

  If set, the keys width is set to fill all available free space. 

* extended=true|false ( defaults to false )

  Keys set with this extended attribute set to true will *only*
  be shown if the display is landscape, rather than portrait.
  The rational for this is to better adapt to screen rotations.
  ( Note: the <space> tag can also use this. )
  

The <key> then has sub tags specifies the appearance and action for
the five possible key states; <default>,<shifted>,<mod1>,<mod2>,<mod3>.

There are two possible attributes for each of these state tags;

* display=UTF8 String|image:<filename>

  Sets what is displayed on the key face for the particular case.
  If this is not set the key will be blank.

  Prefixing the string with 'image:' and then a filename to a valid
  PNG image will cause that image to be used on that key face. The
  filename can be absolute or relative to
  `$PREFIX/share/matchbox-keyboard` or `~/.matchbox`.

* action=action string.

  The specifies the action of the key. For most (all?) single glyph keys
  the action is deduced automatically. For 'special' function keys, it
  can be set to any of the following. 

  ```
  backspace, tab, linefeed, clear, return, pause, scrolllock,
  sysreq, escape, delete, home, left, up, right, down, prior,
  pageup, next, pagedown, end, begin, space, f1, f2, f3, f4, f5, f6,
  f7, f8, f9, f10, f11, f12
  ```

  By prefixing the value with 'xkeysym:', a a xkeysym can be defined to
  be 'pressed' as the action.
  
  If the key is a 'modifier' key, the action value is prefixed with 
  'modifier:' and then one of the following;

  `Shift, Alt, Ctrl, mod1, mod2, mod3, Caps`


Rows can also contain a <space> tags which denote blank space. They
simply take a width attribute specifying the base in 1000th's of a base
key width.

See the various keyboard.xml files included in the distribution for
example setups.


### Misc Notes

* matchbox-keyboard attempts to detect the window manager and set up its
 window 'hints' based on that. This is experimental, YMMV.

 matchbox-keyboard never wants to get keyboard focus itself, if the
 window manager gives it focus ( matchbox-keyboard requests the 
 w-m doesn't ), it wont work. 

* It shouldn't be too hard to make the keyboard use GTK or another toolkit
 ( possibly even Non X11 ) just by hacking matchbox-keyboard-ui.c . 
 ( If you do either of these, please send patches ). 

* There is an applet and experimental GTK-IM module included in the source
 for launching the keyboard. The IM module allows for the keyboard to
 automatically mapped / unmapped on 'demand'. For this to work;

 - The IM must be added to your gtk setup (See /etc/gtk-2.0/gtk.immodules,
   gtk-query-immodules-2.0 )

 - Matchbox-keyboard must be run with the '--daemon' option. You can also
   use the --orientation switch so it is only mapped when the display is
   in a certain orientation.

### Todo

* Fix layout engine on small on displays.
* Theming.

 Needs thought... 


Matthew Allum 2007.
<mallum@openedhand.com>
