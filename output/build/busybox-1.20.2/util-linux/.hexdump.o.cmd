cmd_util-linux/hexdump.o := /home/simon/GIT/buildroot-k3-current/output/host/usr/bin/arm-none-linux-gnueabi-gcc -Wp,-MD,util-linux/.hexdump.o.d   -std=gnu99 -Iinclude -Ilibbb  -include include/autoconf.h -D_GNU_SOURCE -DNDEBUG -D_LARGEFILE_SOURCE -D_LARGEFILE64_SOURCE -D_FILE_OFFSET_BITS=64 -D"BB_VER=KBUILD_STR(1.20.2)" -DBB_BT=AUTOCONF_TIMESTAMP -D_LARGEFILE_SOURCE -D_LARGEFILE64_SOURCE -D_FILE_OFFSET_BITS=64  -pipe -U_FORTIFY_SOURCE -fno-stack-protector -fomit-frame-pointer -fPIC -O2  -I/home/simon/GIT/buildroot-k3-current/output/toolchain/linux/include -Wall -Wshadow -Wwrite-strings -Wundef -Wstrict-prototypes -Wunused -Wunused-parameter -Wunused-function -Wunused-value -Wmissing-prototypes -Wmissing-declarations -Wdeclaration-after-statement -Wold-style-definition -fno-builtin-strlen -finline-limit=0 -fomit-frame-pointer -ffunction-sections -fdata-sections -fno-guess-branch-probability -funsigned-char -static-libgcc -falign-functions=1 -falign-jumps=1 -falign-labels=1 -falign-loops=1 -Os     -D"KBUILD_STR(s)=\#s" -D"KBUILD_BASENAME=KBUILD_STR(hexdump)"  -D"KBUILD_MODNAME=KBUILD_STR(hexdump)" -c -o util-linux/hexdump.o util-linux/hexdump.c

deps_util-linux/hexdump.o := \
  util-linux/hexdump.c \
    $(wildcard include/config/feature/hexdump/reverse.h) \
    $(wildcard include/config/hd.h) \
  include/libbb.h \
    $(wildcard include/config/feature/shadowpasswds.h) \
    $(wildcard include/config/use/bb/shadow.h) \
    $(wildcard include/config/selinux.h) \
    $(wildcard include/config/feature/utmp.h) \
    $(wildcard include/config/locale/support.h) \
    $(wildcard include/config/use/bb/pwd/grp.h) \
    $(wildcard include/config/lfs.h) \
    $(wildcard include/config/feature/buffers/go/on/stack.h) \
    $(wildcard include/config/feature/buffers/go/in/bss.h) \
    $(wildcard include/config/feature/ipv6.h) \
    $(wildcard include/config/feature/seamless/xz.h) \
    $(wildcard include/config/feature/seamless/lzma.h) \
    $(wildcard include/config/feature/seamless/bz2.h) \
    $(wildcard include/config/feature/seamless/gz.h) \
    $(wildcard include/config/feature/seamless/z.h) \
    $(wildcard include/config/feature/check/names.h) \
    $(wildcard include/config/feature/prefer/applets.h) \
    $(wildcard include/config/long/opts.h) \
    $(wildcard include/config/feature/getopt/long.h) \
    $(wildcard include/config/feature/pidfile.h) \
    $(wildcard include/config/feature/syslog.h) \
    $(wildcard include/config/feature/individual.h) \
    $(wildcard include/config/echo.h) \
    $(wildcard include/config/printf.h) \
    $(wildcard include/config/test.h) \
    $(wildcard include/config/kill.h) \
    $(wildcard include/config/chown.h) \
    $(wildcard include/config/ls.h) \
    $(wildcard include/config/xxx.h) \
    $(wildcard include/config/route.h) \
    $(wildcard include/config/feature/hwib.h) \
    $(wildcard include/config/desktop.h) \
    $(wildcard include/config/feature/crond/d.h) \
    $(wildcard include/config/use/bb/crypt.h) \
    $(wildcard include/config/feature/adduser/to/group.h) \
    $(wildcard include/config/feature/del/user/from/group.h) \
    $(wildcard include/config/ioctl/hex2str/error.h) \
    $(wildcard include/config/feature/editing.h) \
    $(wildcard include/config/feature/editing/history.h) \
    $(wildcard include/config/feature/editing/savehistory.h) \
    $(wildcard include/config/feature/tab/completion.h) \
    $(wildcard include/config/feature/username/completion.h) \
    $(wildcard include/config/feature/editing/vi.h) \
    $(wildcard include/config/feature/editing/save/on/exit.h) \
    $(wildcard include/config/pmap.h) \
    $(wildcard include/config/feature/show/threads.h) \
    $(wildcard include/config/feature/ps/additional/columns.h) \
    $(wildcard include/config/feature/topmem.h) \
    $(wildcard include/config/feature/top/smp/process.h) \
    $(wildcard include/config/killall.h) \
    $(wildcard include/config/pgrep.h) \
    $(wildcard include/config/pkill.h) \
    $(wildcard include/config/pidof.h) \
    $(wildcard include/config/sestatus.h) \
    $(wildcard include/config/unicode/support.h) \
    $(wildcard include/config/feature/mtab/support.h) \
    $(wildcard include/config/feature/devfs.h) \
  include/platform.h \
    $(wildcard include/config/werror.h) \
    $(wildcard include/config/big/endian.h) \
    $(wildcard include/config/little/endian.h) \
    $(wildcard include/config/nommu.h) \
  /opt/arm-2007q3/bin/../lib/gcc/arm-none-linux-gnueabi/4.2.1/include-fixed/limits.h \
  /opt/arm-2007q3/bin/../lib/gcc/arm-none-linux-gnueabi/4.2.1/include-fixed/syslimits.h \
  /home/simon/GIT/buildroot-k3-current/output/host/usr/arm-buildroot-linux-gnueabi/sysroot/usr/include/limits.h \
  /home/simon/GIT/buildroot-k3-current/output/host/usr/arm-buildroot-linux-gnueabi/sysroot/usr/include/features.h \
  /home/simon/GIT/buildroot-k3-current/output/host/usr/arm-buildroot-linux-gnueabi/sysroot/usr/include/bits/predefs.h \
  /home/simon/GIT/buildroot-k3-current/output/host/usr/arm-buildroot-linux-gnueabi/sysroot/usr/include/sys/cdefs.h \
  /home/simon/GIT/buildroot-k3-current/output/host/usr/arm-buildroot-linux-gnueabi/sysroot/usr/include/bits/wordsize.h \
  /home/simon/GIT/buildroot-k3-current/output/host/usr/arm-buildroot-linux-gnueabi/sysroot/usr/include/gnu/stubs.h \
  /home/simon/GIT/buildroot-k3-current/output/host/usr/arm-buildroot-linux-gnueabi/sysroot/usr/include/bits/posix1_lim.h \
  /home/simon/GIT/buildroot-k3-current/output/host/usr/arm-buildroot-linux-gnueabi/sysroot/usr/include/bits/local_lim.h \
  /home/simon/GIT/buildroot-k3-current/output/host/usr/arm-buildroot-linux-gnueabi/sysroot/usr/include/linux/limits.h \
  /home/simon/GIT/buildroot-k3-current/output/host/usr/arm-buildroot-linux-gnueabi/sysroot/usr/include/bits/posix2_lim.h \
  /home/simon/GIT/buildroot-k3-current/output/host/usr/arm-buildroot-linux-gnueabi/sysroot/usr/include/bits/xopen_lim.h \
  /home/simon/GIT/buildroot-k3-current/output/host/usr/arm-buildroot-linux-gnueabi/sysroot/usr/include/bits/stdio_lim.h \
  /home/simon/GIT/buildroot-k3-current/output/host/usr/arm-buildroot-linux-gnueabi/sysroot/usr/include/byteswap.h \
  /home/simon/GIT/buildroot-k3-current/output/host/usr/arm-buildroot-linux-gnueabi/sysroot/usr/include/bits/byteswap.h \
  /home/simon/GIT/buildroot-k3-current/output/host/usr/arm-buildroot-linux-gnueabi/sysroot/usr/include/endian.h \
  /home/simon/GIT/buildroot-k3-current/output/host/usr/arm-buildroot-linux-gnueabi/sysroot/usr/include/bits/endian.h \
  /home/simon/GIT/buildroot-k3-current/output/host/usr/arm-buildroot-linux-gnueabi/sysroot/usr/include/stdint.h \
  /home/simon/GIT/buildroot-k3-current/output/host/usr/arm-buildroot-linux-gnueabi/sysroot/usr/include/bits/wchar.h \
  /opt/arm-2007q3/bin/../lib/gcc/arm-none-linux-gnueabi/4.2.1/include/stdbool.h \
  /home/simon/GIT/buildroot-k3-current/output/host/usr/arm-buildroot-linux-gnueabi/sysroot/usr/include/unistd.h \
  /home/simon/GIT/buildroot-k3-current/output/host/usr/arm-buildroot-linux-gnueabi/sysroot/usr/include/bits/posix_opt.h \
  /home/simon/GIT/buildroot-k3-current/output/host/usr/arm-buildroot-linux-gnueabi/sysroot/usr/include/bits/environments.h \
  /home/simon/GIT/buildroot-k3-current/output/host/usr/arm-buildroot-linux-gnueabi/sysroot/usr/include/bits/types.h \
  /opt/arm-2007q3/bin/../lib/gcc/arm-none-linux-gnueabi/4.2.1/include/stddef.h \
  /home/simon/GIT/buildroot-k3-current/output/host/usr/arm-buildroot-linux-gnueabi/sysroot/usr/include/bits/typesizes.h \
  /home/simon/GIT/buildroot-k3-current/output/host/usr/arm-buildroot-linux-gnueabi/sysroot/usr/include/bits/confname.h \
  /home/simon/GIT/buildroot-k3-current/output/host/usr/arm-buildroot-linux-gnueabi/sysroot/usr/include/getopt.h \
  /home/simon/GIT/buildroot-k3-current/output/host/usr/arm-buildroot-linux-gnueabi/sysroot/usr/include/ctype.h \
  /home/simon/GIT/buildroot-k3-current/output/host/usr/arm-buildroot-linux-gnueabi/sysroot/usr/include/xlocale.h \
  /home/simon/GIT/buildroot-k3-current/output/host/usr/arm-buildroot-linux-gnueabi/sysroot/usr/include/dirent.h \
  /home/simon/GIT/buildroot-k3-current/output/host/usr/arm-buildroot-linux-gnueabi/sysroot/usr/include/bits/dirent.h \
  /home/simon/GIT/buildroot-k3-current/output/host/usr/arm-buildroot-linux-gnueabi/sysroot/usr/include/errno.h \
  /home/simon/GIT/buildroot-k3-current/output/host/usr/arm-buildroot-linux-gnueabi/sysroot/usr/include/bits/errno.h \
  /home/simon/GIT/buildroot-k3-current/output/host/usr/arm-buildroot-linux-gnueabi/sysroot/usr/include/linux/errno.h \
  /home/simon/GIT/buildroot-k3-current/output/host/usr/arm-buildroot-linux-gnueabi/sysroot/usr/include/asm/errno.h \
  /home/simon/GIT/buildroot-k3-current/output/host/usr/arm-buildroot-linux-gnueabi/sysroot/usr/include/asm-generic/errno.h \
  /home/simon/GIT/buildroot-k3-current/output/host/usr/arm-buildroot-linux-gnueabi/sysroot/usr/include/asm-generic/errno-base.h \
  /home/simon/GIT/buildroot-k3-current/output/host/usr/arm-buildroot-linux-gnueabi/sysroot/usr/include/fcntl.h \
  /home/simon/GIT/buildroot-k3-current/output/host/usr/arm-buildroot-linux-gnueabi/sysroot/usr/include/bits/fcntl.h \
  /home/simon/GIT/buildroot-k3-current/output/host/usr/arm-buildroot-linux-gnueabi/sysroot/usr/include/sys/types.h \
  /home/simon/GIT/buildroot-k3-current/output/host/usr/arm-buildroot-linux-gnueabi/sysroot/usr/include/time.h \
  /home/simon/GIT/buildroot-k3-current/output/host/usr/arm-buildroot-linux-gnueabi/sysroot/usr/include/sys/select.h \
  /home/simon/GIT/buildroot-k3-current/output/host/usr/arm-buildroot-linux-gnueabi/sysroot/usr/include/bits/select.h \
  /home/simon/GIT/buildroot-k3-current/output/host/usr/arm-buildroot-linux-gnueabi/sysroot/usr/include/bits/sigset.h \
  /home/simon/GIT/buildroot-k3-current/output/host/usr/arm-buildroot-linux-gnueabi/sysroot/usr/include/bits/time.h \
  /home/simon/GIT/buildroot-k3-current/output/host/usr/arm-buildroot-linux-gnueabi/sysroot/usr/include/sys/sysmacros.h \
  /home/simon/GIT/buildroot-k3-current/output/host/usr/arm-buildroot-linux-gnueabi/sysroot/usr/include/bits/pthreadtypes.h \
  /home/simon/GIT/buildroot-k3-current/output/host/usr/arm-buildroot-linux-gnueabi/sysroot/usr/include/bits/uio.h \
  /home/simon/GIT/buildroot-k3-current/output/host/usr/arm-buildroot-linux-gnueabi/sysroot/usr/include/sys/stat.h \
  /home/simon/GIT/buildroot-k3-current/output/host/usr/arm-buildroot-linux-gnueabi/sysroot/usr/include/bits/stat.h \
  /home/simon/GIT/buildroot-k3-current/output/host/usr/arm-buildroot-linux-gnueabi/sysroot/usr/include/inttypes.h \
  /home/simon/GIT/buildroot-k3-current/output/host/usr/arm-buildroot-linux-gnueabi/sysroot/usr/include/netdb.h \
  /home/simon/GIT/buildroot-k3-current/output/host/usr/arm-buildroot-linux-gnueabi/sysroot/usr/include/netinet/in.h \
  /home/simon/GIT/buildroot-k3-current/output/host/usr/arm-buildroot-linux-gnueabi/sysroot/usr/include/sys/socket.h \
  /home/simon/GIT/buildroot-k3-current/output/host/usr/arm-buildroot-linux-gnueabi/sysroot/usr/include/sys/uio.h \
  /home/simon/GIT/buildroot-k3-current/output/host/usr/arm-buildroot-linux-gnueabi/sysroot/usr/include/bits/socket.h \
  /home/simon/GIT/buildroot-k3-current/output/host/usr/arm-buildroot-linux-gnueabi/sysroot/usr/include/bits/sockaddr.h \
  /home/simon/GIT/buildroot-k3-current/output/host/usr/arm-buildroot-linux-gnueabi/sysroot/usr/include/asm/socket.h \
  /home/simon/GIT/buildroot-k3-current/output/host/usr/arm-buildroot-linux-gnueabi/sysroot/usr/include/asm/sockios.h \
  /home/simon/GIT/buildroot-k3-current/output/host/usr/arm-buildroot-linux-gnueabi/sysroot/usr/include/bits/in.h \
  /home/simon/GIT/buildroot-k3-current/output/host/usr/arm-buildroot-linux-gnueabi/sysroot/usr/include/rpc/netdb.h \
  /home/simon/GIT/buildroot-k3-current/output/host/usr/arm-buildroot-linux-gnueabi/sysroot/usr/include/bits/siginfo.h \
  /home/simon/GIT/buildroot-k3-current/output/host/usr/arm-buildroot-linux-gnueabi/sysroot/usr/include/bits/netdb.h \
  /home/simon/GIT/buildroot-k3-current/output/host/usr/arm-buildroot-linux-gnueabi/sysroot/usr/include/setjmp.h \
  /home/simon/GIT/buildroot-k3-current/output/host/usr/arm-buildroot-linux-gnueabi/sysroot/usr/include/bits/setjmp.h \
  /home/simon/GIT/buildroot-k3-current/output/host/usr/arm-buildroot-linux-gnueabi/sysroot/usr/include/signal.h \
  /home/simon/GIT/buildroot-k3-current/output/host/usr/arm-buildroot-linux-gnueabi/sysroot/usr/include/bits/signum.h \
  /home/simon/GIT/buildroot-k3-current/output/host/usr/arm-buildroot-linux-gnueabi/sysroot/usr/include/bits/sigaction.h \
  /home/simon/GIT/buildroot-k3-current/output/host/usr/arm-buildroot-linux-gnueabi/sysroot/usr/include/bits/sigcontext.h \
  /home/simon/GIT/buildroot-k3-current/output/host/usr/arm-buildroot-linux-gnueabi/sysroot/usr/include/asm/sigcontext.h \
  /home/simon/GIT/buildroot-k3-current/output/host/usr/arm-buildroot-linux-gnueabi/sysroot/usr/include/bits/sigstack.h \
  /home/simon/GIT/buildroot-k3-current/output/host/usr/arm-buildroot-linux-gnueabi/sysroot/usr/include/sys/ucontext.h \
  /home/simon/GIT/buildroot-k3-current/output/host/usr/arm-buildroot-linux-gnueabi/sysroot/usr/include/sys/procfs.h \
  /home/simon/GIT/buildroot-k3-current/output/host/usr/arm-buildroot-linux-gnueabi/sysroot/usr/include/sys/time.h \
  /home/simon/GIT/buildroot-k3-current/output/host/usr/arm-buildroot-linux-gnueabi/sysroot/usr/include/sys/user.h \
  /home/simon/GIT/buildroot-k3-current/output/host/usr/arm-buildroot-linux-gnueabi/sysroot/usr/include/bits/sigthread.h \
  /home/simon/GIT/buildroot-k3-current/output/host/usr/arm-buildroot-linux-gnueabi/sysroot/usr/include/stdio.h \
  /home/simon/GIT/buildroot-k3-current/output/host/usr/arm-buildroot-linux-gnueabi/sysroot/usr/include/libio.h \
  /home/simon/GIT/buildroot-k3-current/output/host/usr/arm-buildroot-linux-gnueabi/sysroot/usr/include/_G_config.h \
  /home/simon/GIT/buildroot-k3-current/output/host/usr/arm-buildroot-linux-gnueabi/sysroot/usr/include/wchar.h \
  /home/simon/GIT/buildroot-k3-current/output/host/usr/arm-buildroot-linux-gnueabi/sysroot/usr/include/gconv.h \
  /opt/arm-2007q3/bin/../lib/gcc/arm-none-linux-gnueabi/4.2.1/include/stdarg.h \
  /home/simon/GIT/buildroot-k3-current/output/host/usr/arm-buildroot-linux-gnueabi/sysroot/usr/include/bits/sys_errlist.h \
  /home/simon/GIT/buildroot-k3-current/output/host/usr/arm-buildroot-linux-gnueabi/sysroot/usr/include/stdlib.h \
  /home/simon/GIT/buildroot-k3-current/output/host/usr/arm-buildroot-linux-gnueabi/sysroot/usr/include/bits/waitflags.h \
  /home/simon/GIT/buildroot-k3-current/output/host/usr/arm-buildroot-linux-gnueabi/sysroot/usr/include/bits/waitstatus.h \
  /home/simon/GIT/buildroot-k3-current/output/host/usr/arm-buildroot-linux-gnueabi/sysroot/usr/include/alloca.h \
  /home/simon/GIT/buildroot-k3-current/output/host/usr/arm-buildroot-linux-gnueabi/sysroot/usr/include/string.h \
  /home/simon/GIT/buildroot-k3-current/output/host/usr/arm-buildroot-linux-gnueabi/sysroot/usr/include/libgen.h \
  /home/simon/GIT/buildroot-k3-current/output/host/usr/arm-buildroot-linux-gnueabi/sysroot/usr/include/sys/poll.h \
  /home/simon/GIT/buildroot-k3-current/output/host/usr/arm-buildroot-linux-gnueabi/sysroot/usr/include/bits/poll.h \
  /home/simon/GIT/buildroot-k3-current/output/host/usr/arm-buildroot-linux-gnueabi/sysroot/usr/include/sys/ioctl.h \
  /home/simon/GIT/buildroot-k3-current/output/host/usr/arm-buildroot-linux-gnueabi/sysroot/usr/include/bits/ioctls.h \
  /home/simon/GIT/buildroot-k3-current/output/host/usr/arm-buildroot-linux-gnueabi/sysroot/usr/include/asm/ioctls.h \
  /home/simon/GIT/buildroot-k3-current/output/host/usr/arm-buildroot-linux-gnueabi/sysroot/usr/include/asm/ioctl.h \
  /home/simon/GIT/buildroot-k3-current/output/host/usr/arm-buildroot-linux-gnueabi/sysroot/usr/include/asm-generic/ioctl.h \
  /home/simon/GIT/buildroot-k3-current/output/host/usr/arm-buildroot-linux-gnueabi/sysroot/usr/include/bits/ioctl-types.h \
  /home/simon/GIT/buildroot-k3-current/output/host/usr/arm-buildroot-linux-gnueabi/sysroot/usr/include/sys/ttydefaults.h \
  /home/simon/GIT/buildroot-k3-current/output/host/usr/arm-buildroot-linux-gnueabi/sysroot/usr/include/sys/mman.h \
  /home/simon/GIT/buildroot-k3-current/output/host/usr/arm-buildroot-linux-gnueabi/sysroot/usr/include/bits/mman.h \
  /home/simon/GIT/buildroot-k3-current/output/host/usr/arm-buildroot-linux-gnueabi/sysroot/usr/include/sys/wait.h \
  /home/simon/GIT/buildroot-k3-current/output/host/usr/arm-buildroot-linux-gnueabi/sysroot/usr/include/sys/resource.h \
  /home/simon/GIT/buildroot-k3-current/output/host/usr/arm-buildroot-linux-gnueabi/sysroot/usr/include/bits/resource.h \
  /home/simon/GIT/buildroot-k3-current/output/host/usr/arm-buildroot-linux-gnueabi/sysroot/usr/include/termios.h \
  /home/simon/GIT/buildroot-k3-current/output/host/usr/arm-buildroot-linux-gnueabi/sysroot/usr/include/bits/termios.h \
  /home/simon/GIT/buildroot-k3-current/output/host/usr/arm-buildroot-linux-gnueabi/sysroot/usr/include/sys/param.h \
  /home/simon/GIT/buildroot-k3-current/output/host/usr/arm-buildroot-linux-gnueabi/sysroot/usr/include/linux/param.h \
  /home/simon/GIT/buildroot-k3-current/output/host/usr/arm-buildroot-linux-gnueabi/sysroot/usr/include/asm/param.h \
  /home/simon/GIT/buildroot-k3-current/output/host/usr/arm-buildroot-linux-gnueabi/sysroot/usr/include/pwd.h \
  /home/simon/GIT/buildroot-k3-current/output/host/usr/arm-buildroot-linux-gnueabi/sysroot/usr/include/grp.h \
  /home/simon/GIT/buildroot-k3-current/output/host/usr/arm-buildroot-linux-gnueabi/sysroot/usr/include/shadow.h \
  /home/simon/GIT/buildroot-k3-current/output/host/usr/arm-buildroot-linux-gnueabi/sysroot/usr/include/paths.h \
  /home/simon/GIT/buildroot-k3-current/output/host/usr/arm-buildroot-linux-gnueabi/sysroot/usr/include/mntent.h \
  /home/simon/GIT/buildroot-k3-current/output/host/usr/arm-buildroot-linux-gnueabi/sysroot/usr/include/sys/statfs.h \
  /home/simon/GIT/buildroot-k3-current/output/host/usr/arm-buildroot-linux-gnueabi/sysroot/usr/include/bits/statfs.h \
  /home/simon/GIT/buildroot-k3-current/output/host/usr/arm-buildroot-linux-gnueabi/sysroot/usr/include/utmp.h \
  /home/simon/GIT/buildroot-k3-current/output/host/usr/arm-buildroot-linux-gnueabi/sysroot/usr/include/bits/utmp.h \
  /home/simon/GIT/buildroot-k3-current/output/host/usr/arm-buildroot-linux-gnueabi/sysroot/usr/include/arpa/inet.h \
  include/xatonum.h \
  include/dump.h \

util-linux/hexdump.o: $(deps_util-linux/hexdump.o)

$(deps_util-linux/hexdump.o):
