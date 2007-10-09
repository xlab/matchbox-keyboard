PREFIXDIR  = $(prefix)
PKGDATADIR = $(datadir)/matchbox-keyboard
DATADIR    = $(datadir)

if WANT_CAIRO
CAIRO_BACKEND_C =                                                     \
	matchbox-keyboard-ui-cairo-backend.c                          \
        matchbox-keyboard-ui-cairo-backend.h
else
XFT_BACKEND_C =                                                       \
	matchbox-keyboard-ui-xft-backend.c                            \
        matchbox-keyboard-ui-xft-backend.h
endif

INCLUDES = -DDATADIR=\"$(DATADIR)\" -DPKGDATADIR=\"$(PKGDATADIR)\" -DPREFIX=\"$(PREFIXDIR)\" $(FAKEKEY_CFLAGS) $(XFT_CFLAGS) $(EXPAT_CFLAGS) $(CAIRO_CFLAGS) $(PNG_CFLAGS)

bin_PROGRAMS = matchbox-keyboard

matchbox_keyboard_LDADD = $(FAKEKEY_LIBS) $(XFT_LIBS) $(EXPAT_LIBS) $(CAIRO_LIBS) $(PNG_LIBS)

matchbox_keyboard_SOURCES =                                          \
	matchbox-keyboard.c matchbox-keyboard.h                      \
	matchbox-keyboard-image.c                                    \
	matchbox-keyboard-layout.c                                   \
        matchbox-keyboard-row.c                                      \
        matchbox-keyboard-key.c                                      \
        matchbox-keyboard-ui.c                                       \
        matchbox-keyboard-xembed.c                                   \
        matchbox-keyboard-remote.c                                   \
        matchbox-keyboard-remote.h                                   \
        config-parser.c                                              \
	util-list.c                                                  \
        util.c                                                       \
	$(XFT_BACKEND_C) $(CAIRO_BACKEND_C)

