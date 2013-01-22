cmd_libbb/makedev.o := /home/simon/GIT/buildroot-k3-current/output/host/usr/bin/arm-none-linux-gnueabi-gcc -Wp,-MD,libbb/.makedev.o.d   -std=gnu99 -Iinclude -Ilibbb  -include include/autoconf.h -D_GNU_SOURCE -DNDEBUG -D_LARGEFILE_SOURCE -D_LARGEFILE64_SOURCE -D_FILE_OFFSET_BITS=64 -D"BB_VER=KBUILD_STR(1.20.2)" -DBB_BT=AUTOCONF_TIMESTAMP -D_LARGEFILE_SOURCE -D_LARGEFILE64_SOURCE -D_FILE_OFFSET_BITS=64  -pipe -U_FORTIFY_SOURCE -fno-stack-protector -fomit-frame-pointer -fPIC -O2  -I/home/simon/GIT/buildroot-k3-current/output/toolchain/linux/include -Wall -Wshadow -Wwrite-strings -Wundef -Wstrict-prototypes -Wunused -Wunused-parameter -Wunused-function -Wunused-value -Wmissing-prototypes -Wmissing-declarations -Wdeclaration-after-statement -Wold-style-definition -fno-builtin-strlen -finline-limit=0 -fomit-frame-pointer -ffunction-sections -fdata-sections -fno-guess-branch-probability -funsigned-char -static-libgcc -falign-functions=1 -falign-jumps=1 -falign-labels=1 -falign-loops=1 -Os     -D"KBUILD_STR(s)=\#s" -D"KBUILD_BASENAME=KBUILD_STR(makedev)"  -D"KBUILD_MODNAME=KBUILD_STR(makedev)" -c -o libbb/makedev.o libbb/makedev.c

deps_libbb/makedev.o := \
  libbb/makedev.c \
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
  /home/simon/GIT/buildroot-k3-current/output/host/usr/arm-buildroot-linux-gnueabi/sysroot/usr/include/sys/sysmacros.h \

libbb/makedev.o: $(deps_libbb/makedev.o)

$(deps_libbb/makedev.o):
