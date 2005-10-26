SUBDIRS = src layouts

if WANT_EXAMPLES
SUBDIRS += examples
endif

EXTRA_DIST = matchbox-keyboard.desktop matchbox-keyboard.png

desktopdir = $(datadir)/applications/inputmethods
desktop_DATA = matchbox-keyboard.desktop

pixmapsdir = $(datadir)/pixmaps/
pixmaps_DATA = matchbox-keyboard.png


snapshot:
	$(MAKE) dist distdir=$(PACKAGE)-snap`date +"%Y%m%d"`
