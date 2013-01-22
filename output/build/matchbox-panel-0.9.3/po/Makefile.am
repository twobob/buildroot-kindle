CATALOGS = @MOFILES@

CLEANFILES = $(CATALOGS) matchbox.pot

# keep this sorted
EXTRA_DIST = en_GB.po es_ES.po fi_FI.po de.po cs.po fr_FR.po zh_TW.po

POTFILES  = \
	$(top_builddir)/src/panel_menu.c   \
	$(top_builddir)/applets/mb-applet-system-monitor.c \
	$(top_builddir)/applets/mb-applet-wireless.c \
	$(top_builddir)/applets/mb-applet-clock.c \
	$(top_builddir)/applets/mb-applet-battery.c 


SUFFIXES = .po .mo

.po.mo: 
	$(MSGFMT) -o $@ $<

matchbox.pot: $(POTFILES)
	 $(XGETTEXT) --default-domain=matchbox \
	--add-comments --keyword=_ $(POTFILES)
	if cmp -s matchbox.po matchbox.pot; then \
	    rm -f matchbox.po; \
	else \
	    mv -f matchbox.po matchbox.pot; \
	fi

update.po: matchbox.pot
	for n in $(CATALOGS) __DuMmY ; do \
		if test "$$n" -a "$$n" != "__DuMmY" ; then \
		l=`basename $$n .mo`; \
		$(MSGMERGE) --update $$l.po matchbox.pot; \
		fi; \
	done

install-data-local:  $(CATALOGS)
	$(mkinstalldirs) $(DESTDIR)$(datadir)/locale
	chmod 755 $(DESTDIR)$(datadir)/locale 
	for n in $(CATALOGS) __DuMmY ; do \
		if test "$$n" -a "$$n" != "__DuMmY" ; then \
			l=`basename $$n .mo`; \
			$(mkinstalldirs) $(DESTDIR)$(datadir)/locale/$$l/LC_MESSAGES; \
			chmod 755 $(DESTDIR)$(datadir)/locale/$$l; \
			chmod 755 $(DESTDIR)$(datadir)/locale/$$l/LC_MESSAGES; \
			$(INSTALL_DATA) -m 644 $$n $(DESTDIR)$(datadir)/locale/$$l/LC_MESSAGES/matchbox-panel.mo; \
		fi; \
	done
