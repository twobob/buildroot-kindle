prefix			= /home/simon/GIT/buildroot-k3-current/output/host/usr
exec_prefix		= ${prefix}
bindir			= ${exec_prefix}/bin
libdir			= ${exec_prefix}/lib
datarootdir		= ${prefix}/share
datadir			= ${datarootdir}
includedir		= ${prefix}/include
system_includedir	= ${prefix}/include
system_libdir		= ${exec_prefix}/lib
pkgconfigdir		= ${exec_prefix}/lib/pkgconfig:${datarootdir}/pkgconfig

CC			= /usr/bin/gcc
PROG			= pkgconf
SRCS			= main.c pkg.c bsdstubs.c getopt_long.c fragment.c argvsplit.c fileio.c tuple.c dependency.c queue.c
OBJS			= ${SRCS:.c=.o}
CFLAGS			= -O2 -I/home/simon/GIT/buildroot-k3-current/output/host/usr/include
LDFLAGS			= -L/home/simon/GIT/buildroot-k3-current/output/host/lib -L/home/simon/GIT/buildroot-k3-current/output/host/usr/lib -Wl,-rpath,/home/simon/GIT/buildroot-k3-current/output/host/usr/lib
CFLAGS			+= -DLIBDIR=\"${libdir}\" -DINCLUDEDIR=\"${includedir}\" -DPKG_DEFAULT_PATH=\"${pkgconfigdir}\" -DSYSTEM_INCLUDEDIR=\"${system_includedir}\" -DSYSTEM_LIBDIR=\"${system_libdir}\" -Wall -Wextra -Wformat=2 -std=gnu99 -D_FORTIFY_SOURCE=2

all: build
build: ${PROG}

clean:
	rm *.o
	rm $(PROG)

install:
	mkdir -p $(DESTDIR)$(bindir)
	install -c -m755 $(PROG) $(DESTDIR)$(bindir)/$(PROG)
	mkdir -p $(DESTDIR)$(datarootdir)/aclocal
	install -c -m644 pkg.m4 $(DESTDIR)$(datarootdir)/aclocal/pkg.m4

check: $(PROG)
	$(SHELL) tests/run.sh ./$(PROG)

valgrind-check: $(PROG)
	$(SHELL) tests/run.sh 'valgrind --leak-check=full --show-reachable=yes ./$(PROG)'

$(PROG): $(OBJS)
	${CC} ${LDFLAGS} -o $@ $(OBJS)

.c.o:
	${CC} ${CFLAGS} -c $< -o $@
