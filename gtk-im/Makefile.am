AM_CFLAGS = -Wall

imdir = @libdir@/gtk-2.0/@GTK_VERSION@/immodules/
im_LTLIBRARIES = libmb-im-invoker.la
# TODO: remove lib prefix

libmb_im_invoker_la_CPPFLAGS = @GTK2_CFLAGS@ -I${top_srcdir}/src
libmb_im_invoker_la_LDFLAGS = -avoid-version
libmb_im_invoker_la_LIBADD = @GTK2_LIBS@

libmb_im_invoker_la_SOURCES = \
	im-module.c \
	im-context.h im-context.c \
	im-protocol.h im-protocol.c
