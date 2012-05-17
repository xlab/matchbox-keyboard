keyboardsdir = $(datadir)/matchbox-keyboard

IMGS  = arrow.png      arrows.png       \
        return.png      shift.png       \
        shift_2.png     shift_3.png     \
	shift_caps.png			\
        super_0.png     sub_0.png       \
        super_1.png     sub_1.png       \
        super_2.png     sub_2.png       \
        super_3.png     sub_3.png       \
        super_4.png     sub_4.png       \
        super_5.png     sub_5.png       \
        super_6.png     sub_6.png       \
        super_7.png     sub_7.png       \
        super_8.png     sub_8.png       \
        super_9.png     sub_9.png       \
        super_plus.png 	sub_plus.png    \
        super_minus.png	sub_minus.png   \
        delta_cap.png	degree.png

variant_in_files = keyboard.xml.in keyboard-lq1.xml.in keyboard-symbol.xml.in keyboard-numpad-extended.xml.in keyboard-numpad-small.xml.in    \
                   keyboard-en_US.xml.in keyboard-ar_EG.xml.in keyboard-ca_ES.xml.in keyboard-es_ES.xml.in keyboard-ru_RU.xml.in \
                   keyboard-cs_CZ.xml.in keyboard-da_DK.xml.in keyboard-de_DE.xml.in keyboard-el_GR.xml.in keyboard-et_EE.xml.in \
                   keyboard-fi_FI.xml.in keyboard-fr_FR.xml.in keyboard-it_IT.xml.in keyboard-lv_LV.xml.in keyboard-nb_NO.xml.in \
                   keyboard-nl_NL.xml.in keyboard-pl_PL.xml.in keyboard-pt_PT.xml.in keyboard-sk_SK.xml.in keyboard-sl_SI.xml.in \
                   keyboard-sv_SE.xml.in keyboard-tr_TR.xml.in \
                  $(NULL)

%.xml.tmp: %.xml.in
	sed \
        -e 's,@datadir\@,$(pkgdatadir),g' \
        -e 's,@prefix\@,$(prefix),g'      \
        -e 's,@version\@,@VERSION@,g'     \
	$< > $@

%.xml: %.xml.tmp
	cp $< $@

variantdir        = $(pkgdatadir)
variant_tmp_files = $(variant_in_files:.xml.in=.xml.tmp)
variant_DATA      = $(variant_tmp_files:.xml.tmp=.xml)

keyboardsdir      = $(pkgdatadir)
keyboards_DATA    = $(variant_DATA) $(IMGS)

clean-local:
	rm -fr oh-docbook-to-pdf

EXTRA_DIST = $(IMGS) $(variant_in_files)

CLEANFILES = $(variant_DATA) $(variant_tmp_files)
