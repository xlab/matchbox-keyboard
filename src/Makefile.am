PREFIXDIR  = $(prefix)
PKGDATADIR = $(datadir)/matchbox-keyboard
DATADIR    = $(datadir)

BACEKND_C =                                                           \
	matchbox-keyboard-ui-xft-backend.c                            \
        matchbox-keyboard-ui-xft-backend.h


if WANT_CAIRO
BACKEND_C =                                                           \
	matchbox-keyboard-ui-cairo-backend.c                          \
        matchbox-keyboard-ui-cairo-backend.h
endif

INCLUDES = -DDATADIR=\"$(DATADIR)\" -DPKGDATADIR=\"$(PKGDATADIR)\" -DPREFIX=\"$(PREFIXDIR)\" $(FAKEKEY_CFLAGS) $(XFT_CFLAGS) $(EXPAT_CFLAGS) $(CAIRO_CFLAGS)

bin_PROGRAMS = matchbox-keyboard

matchbox_keyboard_LDADD = $(FAKEKEY_LIBS) $(XFT_LIBS) $(EXPAT_LIBS) $(CAIRO_LIBS)

matchbox_keyboard_SOURCES =                                          \
	matchbox-keyboard.c matchbox-keyboard.h                      \
	matchbox-keyboard-layout.c                                   \
        matchbox-keyboard-row.c                                      \
        matchbox-keyboard-key.c                                      \
        matchbox-keyboard-ui.c                                       \
        config-parser.c                                              \
	util-list.c                                                  \
        util.c                                                       \
	$(BACKEND_C)

