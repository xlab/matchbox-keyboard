appletdir = $(libdir)/matchbox-panel
applet_LTLIBRARIES = libkeyboard.la

libkeyboard_la_SOURCES = applet.c $(top_srcdir)/gtk-im/im-protocol.c
libkeyboard_la_CPPFLAGS = @APPLET_CFLAGS@ -I$(top_srcdir)
libkeyboard_la_LDFLAGS = -avoid-version
libkeyboard_la_LIBADD = @APPLET_LIBS@
