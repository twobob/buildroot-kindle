if BUILD_SHARED
if ENABLE_TRACE

LIBTONM = $(LTLIBRARIES:.la=-$(LT_RELEASE).so.$(LT_BINARY))

install-data-local: install-libLTLIBRARIES
	mkdir -p -- "$(DESTDIR)$(libdir)"
	nm -n "$(DESTDIR)$(libdir)/$(LIBTONM)" > "$(DESTDIR)$(libdir)/nm-n.$(LIBTONM)"

endif
endif
