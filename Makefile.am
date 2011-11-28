SUBDIRS = src layouts

if WANT_EXAMPLES
SUBDIRS += examples
endif

if WANT_IM
SUBDIRS += gtk-im
endif 

if WANT_APPLET
SUBDIRS += applet
endif 

desktopdir = $(datadir)/applications/inputmethods
dist_desktop_DATA = matchbox-keyboard.desktop

pixmapsdir = $(datadir)/pixmaps/
dist_pixmaps_DATA = matchbox-keyboard.png

DISTCHECK_CONFIGURE_FLAGS = --enable-examples --enable-gtk-im --enable-applet

snapshot:
	$(MAKE) dist distdir=$(PACKAGE)-snap`date +"%Y%m%d"`

CLEANFILES = $(BUILT_SOURCES)
