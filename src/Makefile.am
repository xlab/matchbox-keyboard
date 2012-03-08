PREFIXDIR  = $(prefix)
PKGDATADIR = $(datadir)/matchbox-keyboard
DATADIR    = $(datadir)

pkgconfig_in_files = libmatchbox-keyboard.pc.in
pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = $(pkgconfig_in_files:.pc.in=.pc)

BUILT_SOURCES =

if WANT_CAIRO
CAIRO_BACKEND_C =                             \
	matchbox-keyboard-ui-cairo-backend.c  \
        matchbox-keyboard-ui-cairo-backend.h  \
	matchbox-keyboard-popup.c	      \
	matchbox-keyboard-popup.h
else
XFT_BACKEND_C =                                                       \
	matchbox-keyboard-ui-xft-backend.c                            \
        matchbox-keyboard-ui-xft-backend.h			      \
	matchbox-keyboard-image.c
endif

INCLUDES = -DDATADIR=\"$(DATADIR)\" -DPKGDATADIR=\"$(PKGDATADIR)\" -DPREFIX=\"$(PREFIXDIR)\" $(FAKEKEY_CFLAGS) $(XFT_CFLAGS) $(EXPAT_CFLAGS) $(CAIRO_CFLAGS) $(PNG_CFLAGS)

if WANT_GTK_WIDGET
INCLUDES += $(GTK2_CFLAGS)
endif

bin_PROGRAMS = matchbox-keyboard
lib_LTLIBRARIES = libmatchbox-keyboard.la

libmatchbox_keyboard_includedir = $(includedir)/libmatchbox-keyboard
libmatchbox_keyboard_include_HEADERS = $(lib_headers)

lib_headers = libmatchbox-keyboard.h mb-gtk-keyboard.h

libmatchbox_keyboard_la_SOURCES =				\
	libmatchbox-keyboard.c libmatchbox-keyboard.h		\
	matchbox-keyboard.c matchbox-keyboard.h         	\
	matchbox-keyboard-layout.c                      	\
	matchbox-keyboard-row.c                         	\
	matchbox-keyboard-key.c                         	\
	matchbox-keyboard-ui.c                          	\
	config-parser.c                                 	\
	util-list.c                                     	\
	util.c                                          	\
	$(XFT_BACKEND_C) $(CAIRO_BACKEND_C)			\
	$(NULL)

libmatchbox_keyboard_la_CFLAGS =	\
	-I$(top_srcdir)			\
	-I$(top_builddir)		\
	$(NULL)

libmatchbox_keyboard_la_LIBADD = \
	$(FAKEKEY_LIBS) $(XFT_LIBS) $(EXPAT_LIBS) $(CAIRO_LIBS) $(PNG_LIBS)


if WANT_GTK_WIDGET
libmatchbox_keyboard_la_SOURCES +=	\
	mb-gtk-keyboard.c		\
	mb-gtk-keyboard.h		\
	$(NULL)

libmatchbox_keyboard_la_CFLAGS += $(GTK2_CFLAGS)
libmatchbox_keyboard_la_LIBADD += $(GTK2_LIBS)
endif

matchbox_keyboard_LDADD = \
	$(FAKEKEY_LIBS) $(XFT_LIBS) $(EXPAT_LIBS) $(CAIRO_LIBS) $(PNG_LIBS) \
	libmatchbox-keyboard.la

matchbox_keyboard_SOURCES = 				\
	main.c						\
	matchbox-keyboard.h         			\
	matchbox-keyboard-layout.c                      \
        matchbox-keyboard-xembed.c                      \
        matchbox-keyboard-remote.c                      \
        matchbox-keyboard-remote.h                     	\
	$(NULL)

CLEANFILES = $(BUILT_SOURCES) libmatchbox-keyboard.pc
EXTRA_DIST = $(pkgconfig_DATA)
