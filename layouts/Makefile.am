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

fragment_in_files = \
		base-fragment.xml.in	   \
		base-fragment-en_US.xml.in \
		base-fragment-ar_EG.xml.in \
		base-fragment-ca_ES.xml.in \
		base-fragment-es_ES.xml.in \
		base-fragment-ru_RU.xml.in \
		base-fragment-cs_CZ.xml.in \
		base-fragment-da_DK.xml.in \
		base-fragment-de_DE.xml.in \
		base-fragment-el_GR.xml.in \
		base-fragment-et_EE.xml.in \
		base-fragment-fi_FI.xml.in \
		base-fragment-fr_FR.xml.in \
		base-fragment-it_IT.xml.in \
		base-fragment-lv_LV.xml.in \
		base-fragment-nb_NO.xml.in \
		base-fragment-nl_NL.xml.in \
		base-fragment-pl_PL.xml.in \
		base-fragment-pt_PT.xml.in \
		base-fragment-sk_SK.xml.in \
		base-fragment-sl_SI.xml.in \
		base-fragment-sv_SE.xml.in \
		base-fragment-tr_TR.xml.in \
		$(NULL)

variant_in_files = $(fragment_in_files) \
		keyboard.xml.in \
		keyboard-lq1.xml.in \
		keyboard-symbol.xml.in \
		keyboard-numpad-extended.xml.in \
		keyboard-numpad-small.xml.in    \
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

EXTRA_DIST = $(IMGS) $(variant_in_files) $(fragment_in_files)

CLEANFILES = $(variant_DATA) $(variant_tmp_files)
