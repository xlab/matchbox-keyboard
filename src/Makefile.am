PREFIXDIR  = $(prefix)
PKGDATADIR = $(datadir)/matchbox
DATADIR    = $(datadir)

INCLUDES = -DDATADIR=\"$(DATADIR)\" -DPKGDATADIR=\"$(PKGDATADIR)\" -DPREFIX=\"$(PREFIXDIR)\" $(LIBMB_CFLAGS) $(EXPAT_CFLAGS)

bin_PROGRAMS = matchbox-keyboard

matchbox_keyboard_LDADD = $(LIBMB_LIBS) $(EXPAT_LIBS)

matchbox_keyboard_SOURCES =                                          \
	matchbox-keyboard.c matchbox-keyboard.h                      \
	matchbox-keyboard-layout.c                                   \
        matchbox-keyboard-row.c                                      \
        matchbox-keyboard-key.c                                      \
        matchbox-keyboard-ui.c                                       \
        config-parser.c                                              \
	util-list.c                                                  \
        util.c


