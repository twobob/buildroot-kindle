## the "name-test" subdir in fact contains a bunch of tests now that need a temporary bus 
## to be running to do stuff with. The directory should be renamed.
## We want to build the current directory first to pick up the testutils lib
SUBDIRS= . name-test 
DIST_SUBDIRS=name-test

INCLUDES=-I$(top_srcdir) $(DBUS_TEST_CFLAGS) 

libdbus_testutils_la_SOURCES = test-utils.h test-utils.c

noinst_LTLIBRARIES = libdbus-testutils.la

if DBUS_BUILD_TESTS
## break-loader removed for now
## most of these binaries are used in tests but are not themselves tests
TEST_BINARIES = \
	shell-test \
	spawn-test \
	test-exit \
	test-names \
	test-segfault \
	test-service \
	test-shell-service \
	test-sleep-forever \
	$(NULL)

## these are the things to run in make check (i.e. they are actual tests)
## (binaries in here must also be in TEST_BINARIES)
TESTS = \
	shell-test \
	$(NULL)

## These are conceptually part of directories that come earlier in SUBDIRS
## order, but we don't want to run them til we arrive in this directory,
## since they depend on stuff from this directory
TESTS += \
	../bus/bus-test$(EXEEXT) \
	../bus/bus-test-system$(EXEEXT) \
	../dbus/dbus-test$(EXEEXT) \
	$(NULL)

if DBUS_UNIX
TESTS += ../bus/bus-test-launch-helper$(EXEEXT)
endif

else !DBUS_BUILD_TESTS

TEST_BINARIES=
TESTS=

endif !DBUS_BUILD_TESTS

noinst_PROGRAMS= $(TEST_BINARIES)

test_service_SOURCES=				\
	test-service.c

test_names_SOURCES=				\
	test-names.c

##break_loader_SOURCES=				\
##	break-loader.c

test_shell_service_SOURCES =			\
	test-shell-service.c

shell_test_SOURCES=                             \
        shell-test.c

spawn_test_SOURCES=				\
	spawn-test.c

test_exit_SOURCES =				\
	test-exit.c

test_segfault_SOURCES =				\
	test-segfault.c

test_sleep_forever_SOURCES =			\
	test-sleep-forever.c

# This assumes that most tests will be linked to libdbus-internal;
# tests linked to only the public libdbus have their own CPPFLAGS.
AM_CPPFLAGS=-DDBUS_STATIC_BUILD
TEST_LIBS=$(top_builddir)/dbus/libdbus-internal.la $(DBUS_TEST_LIBS)

test_service_LDADD=libdbus-testutils.la $(TEST_LIBS)
test_service_LDFLAGS=@R_DYNAMIC_LDFLAG@
test_names_LDADD=libdbus-testutils.la $(TEST_LIBS)
test_names_LDFLAGS=@R_DYNAMIC_LDFLAG@
## break_loader_LDADD= $(TEST_LIBS)
## break_loader_LDFLAGS=@R_DYNAMIC_LDFLAG@
test_shell_service_LDADD=libdbus-testutils.la $(TEST_LIBS)
test_shell_service_LDFLAGS=@R_DYNAMIC_LDFLAG@
shell_test_LDADD=libdbus-testutils.la $(TEST_LIBS)
shell_test_LDFLAGS=@R_DYNAMIC_LDFLAG@
spawn_test_LDADD=$(TEST_LIBS)
spawn_test_LDFLAGS=@R_DYNAMIC_LDFLAG@

test_refs_SOURCES = internals/refs.c
test_refs_CPPFLAGS = -DDBUS_STATIC_BUILD $(GLIB_CFLAGS)
test_refs_LDADD = libdbus-testutils.la $(GLIB_LIBS) $(TEST_LIBS)

test_syslog_SOURCES = internals/syslog.c
test_syslog_CPPFLAGS = -DDBUS_STATIC_BUILD $(GLIB_CFLAGS)
test_syslog_LDADD = libdbus-testutils.la $(GLIB_LIBS) $(TEST_LIBS)

EXTRA_DIST = dbus-test-runner

testexecdir = $(libdir)/dbus-1.0/test

testexec_PROGRAMS =

installable_tests = \
	test-corrupt \
	test-dbus-daemon \
	test-loopback \
	test-marshal \
	test-refs \
	test-relay \
	test-syslog \
	$(NULL)

installcheck_tests =
installcheck_environment = \
	DBUS_TEST_DAEMON=$(DESTDIR)$(DBUS_DAEMONDIR)/dbus-daemon$(EXEEXT) \
	DBUS_TEST_HOMEDIR=@abs_top_builddir@/dbus \
	DBUS_TEST_SYSCONFDIR=$(DESTDIR)$(sysconfdir)

TESTS_ENVIRONMENT = \
	DBUS_BLOCK_ON_ABORT=1 \
	DBUS_FATAL_WARNINGS=1 \
	DBUS_TEST_DAEMON=@abs_top_builddir@/bus/dbus-daemon$(EXEEXT) \
	DBUS_TEST_DATA=@abs_top_builddir@/test/data \
	DBUS_TEST_HOMEDIR=@abs_top_builddir@/dbus \
	$(NULL)

test_corrupt_SOURCES = corrupt.c
test_corrupt_CPPFLAGS = $(GLIB_CFLAGS) $(DBUS_GLIB_CFLAGS)
test_corrupt_LDFLAGS = @R_DYNAMIC_LDFLAG@
test_corrupt_LDADD = $(top_builddir)/dbus/libdbus-1.la \
    $(GLIB_LIBS) \
    $(DBUS_GLIB_LIBS)

test_loopback_SOURCES = loopback.c
test_loopback_CPPFLAGS = $(GLIB_CFLAGS) $(DBUS_GLIB_CFLAGS)
test_loopback_LDFLAGS = @R_DYNAMIC_LDFLAG@
test_loopback_LDADD = $(top_builddir)/dbus/libdbus-1.la \
    $(GLIB_LIBS) \
    $(DBUS_GLIB_LIBS)

test_relay_SOURCES = relay.c
test_relay_CPPFLAGS = $(GLIB_CFLAGS) $(DBUS_GLIB_CFLAGS)
test_relay_LDFLAGS = @R_DYNAMIC_LDFLAG@
test_relay_LDADD = $(top_builddir)/dbus/libdbus-1.la \
    $(GLIB_LIBS) \
    $(DBUS_GLIB_LIBS)

test_dbus_daemon_SOURCES = dbus-daemon.c
test_dbus_daemon_CPPFLAGS = $(GLIB_CFLAGS) $(DBUS_GLIB_CFLAGS)
test_dbus_daemon_LDFLAGS = @R_DYNAMIC_LDFLAG@
test_dbus_daemon_LDADD = $(top_builddir)/dbus/libdbus-1.la \
    $(GLIB_LIBS) \
    $(DBUS_GLIB_LIBS)

test_marshal_SOURCES = marshal.c
test_marshal_CPPFLAGS = $(GLIB_CFLAGS) $(DBUS_GLIB_CFLAGS)
test_marshal_LDFLAGS = @R_DYNAMIC_LDFLAG@
test_marshal_LDADD = $(top_builddir)/dbus/libdbus-1.la \
    $(GLIB_LIBS) \
    $(DBUS_GLIB_LIBS)

if DBUS_ENABLE_MODULAR_TESTS
TESTS += $(installable_tests)
installcheck_tests += $(installable_tests)

if DBUS_ENABLE_INSTALLED_TESTS
  testexec_PROGRAMS += $(installable_tests)
else !DBUS_ENABLE_INSTALLED_TESTS
  noinst_PROGRAMS += $(installable_tests)
endif !DBUS_ENABLE_INSTALLED_TESTS

endif DBUS_ENABLE_MODULAR_TESTS

# If we're installing the tests into a DESTDIR we can't run them
# again using the installed copy, because we don't know how to
# do a portable equivalent of setting LD_LIBRARY_PATH.
installcheck-local:
	$(MAKE) check-TESTS TESTS='$$(installcheck_tests)' \
		TESTS_ENVIRONMENT='$$(installcheck_environment)'
if DBUS_ENABLE_INSTALLED_TESTS
	test -n "$(DESTDIR)" || \
	$(installcheck_environment) \
		$(srcdir)/dbus-test-runner \
		$(testexecdir) \
		$(testexec_PROGRAMS)
endif DBUS_ENABLE_INSTALLED_TESTS

in_data = \
	data/valid-config-files-system/debug-allow-all-fail.conf.in \
	data/valid-config-files-system/debug-allow-all-pass.conf.in \
	data/valid-config-files/debug-allow-all-sha1.conf.in \
	data/valid-config-files/debug-allow-all.conf.in \
	data/invalid-service-files-system/org.freedesktop.DBus.TestSuiteNoExec.service.in \
	data/invalid-service-files-system/org.freedesktop.DBus.TestSuiteNoService.service.in \
	data/invalid-service-files-system/org.freedesktop.DBus.TestSuiteNoUser.service.in \
	data/valid-service-files-system/org.freedesktop.DBus.TestSuiteEchoService.service.in \
	data/valid-service-files-system/org.freedesktop.DBus.TestSuiteSegfaultService.service.in \
	data/valid-service-files-system/org.freedesktop.DBus.TestSuiteShellEchoServiceFail.service.in \
	data/valid-service-files-system/org.freedesktop.DBus.TestSuiteShellEchoServiceSuccess.service.in \
	data/valid-service-files/org.freedesktop.DBus.TestSuite.PrivServer.service.in \
	data/valid-service-files/org.freedesktop.DBus.TestSuiteEchoService.service.in \
	data/valid-service-files/org.freedesktop.DBus.TestSuiteForkingEchoService.service.in \
	data/valid-service-files/org.freedesktop.DBus.TestSuiteSegfaultService.service.in \
	data/valid-service-files/org.freedesktop.DBus.TestSuiteShellEchoServiceFail.service.in \
	data/valid-service-files/org.freedesktop.DBus.TestSuiteShellEchoServiceSuccess.service.in \
	$(NULL)

EXTRA_DIST += $(in_data)

static_data = \
	name-test/tmp-session-like-system.conf \
	data/auth/anonymous-client-successful.auth-script \
	data/auth/anonymous-server-successful.auth-script \
	data/auth/cancel.auth-script \
	data/auth/client-out-of-mechanisms.auth-script \
	data/auth/external-failed.auth-script \
	data/auth/external-root.auth-script \
	data/auth/external-silly.auth-script \
	data/auth/external-successful.auth-script \
	data/auth/extra-bytes.auth-script \
	data/auth/fail-after-n-attempts.auth-script \
	data/auth/fallback.auth-script \
	data/auth/invalid-command-client.auth-script \
	data/auth/invalid-command.auth-script \
	data/auth/invalid-hex-encoding.auth-script \
	data/auth/mechanisms.auth-script \
	data/equiv-config-files/basic/basic-1.conf \
	data/equiv-config-files/basic/basic-2.conf \
	data/equiv-config-files/basic/basic.d/basic.conf \
	data/equiv-config-files/entities/basic.d/basic.conf \
	data/equiv-config-files/entities/entities-1.conf \
	data/equiv-config-files/entities/entities-2.conf \
	data/incomplete-messages/missing-body.message \
	data/invalid-config-files/badselinux-1.conf \
	data/invalid-config-files/badselinux-2.conf \
	data/invalid-config-files/circular-1.conf \
	data/invalid-config-files/circular-2.conf \
	data/invalid-config-files/circular-3.conf \
	data/invalid-config-files/not-well-formed.conf \
	data/invalid-config-files/truncated-file.conf \
	data/invalid-messages/array-of-nil.message \
	data/invalid-messages/array-with-mixed-types.message \
	data/invalid-messages/bad-boolean-array.message \
	data/invalid-messages/bad-boolean.message \
	data/invalid-messages/bad-endian.message \
	data/invalid-messages/bad-header-field-alignment.message \
	data/invalid-messages/boolean-has-no-value.message-raw \
	data/invalid-messages/local-namespace.message \
	data/invalid-messages/no-dot-in-name.message \
	data/invalid-messages/not-nul-header-padding.message \
	data/invalid-messages/overlong-name.message \
	data/invalid-messages/too-little-header-padding.message \
	data/invalid-messages/too-much-header-padding-by-far.message \
	data/invalid-messages/too-much-header-padding.message \
	data/invalid-messages/too-short-dict.message \
	data/sha-1/Readme.txt \
	data/sha-1/bit-hashes.sha1 \
	data/sha-1/bit-messages.sha1 \
	data/sha-1/byte-hashes.sha1 \
	data/sha-1/byte-messages.sha1 \
	data/valid-config-files/basic.conf \
	data/valid-config-files/basic.d/basic.conf \
	data/valid-config-files/entities.conf \
	data/valid-config-files/many-rules.conf \
	data/valid-config-files/system.d/test.conf \
	data/valid-messages/array-of-array-of-uint32.message \
	data/valid-messages/dict-simple.message \
	data/valid-messages/dict.message \
	data/valid-messages/emptiness.message \
	data/valid-messages/lots-of-arguments.message \
	data/valid-messages/no-padding.message \
	data/valid-messages/opposite-endian.message \
	data/valid-messages/recursive-types.message \
	data/valid-messages/simplest-manual.message \
	data/valid-messages/simplest.message \
	data/valid-messages/standard-acquire-service.message \
	data/valid-messages/standard-hello.message \
	data/valid-messages/standard-list-services.message \
	data/valid-messages/standard-service-exists.message \
	data/valid-messages/unknown-header-field.message \
	$(NULL)

EXTRA_DIST += $(static_data)

## copy tests to builddir so that generated tests and static tests
## are all in one place.
all-local:
	$(AM_V_at)$(MKDIR_P) data/valid-config-files/session.d
	$(AM_V_at)set -e && \
	if test $(srcdir) = . || test $(srcdir) -ef .; then \
		echo '-- No need to copy test data as srcdir = builddir'; \
	else \
		for F in $(static_data); do \
			$(MKDIR_P) $${F%/*}; \
			cp $(srcdir)/$$F $$F; \
		done; \
	fi

## this doesn't clean most copied test data files when srcdir=builddir
clean-local:
	$(AM_V_at)if test $(srcdir) = . || test $(srcdir) -ef .; then \
		echo '-- No need to clean test data as srcdir = builddir'; \
	else \
		rm -f $(static_data); \
	fi

imported_data = \
	data/valid-config-files/session.conf \
	data/valid-config-files/system.conf \
	$(NULL)

noinst_DATA = $(imported_data)
CLEANFILES = $(noinst_DATA)

data/valid-config-files/session.conf: $(top_builddir)/bus/session.conf
	$(AM_V_at)$(MKDIR_P) data/valid-config-files
	$(AM_V_GEN)cp $< $@

data/valid-config-files/system.conf: $(top_builddir)/bus/system.conf
	$(AM_V_at)$(MKDIR_P) data/valid-config-files
	$(AM_V_GEN)cp $< $@
