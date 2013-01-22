cmd_libbb/ptr_to_globals.o := /home/simon/GIT/buildroot-k3-current/output/host/usr/bin/arm-none-linux-gnueabi-gcc -Wp,-MD,libbb/.ptr_to_globals.o.d   -std=gnu99 -Iinclude -Ilibbb  -include include/autoconf.h -D_GNU_SOURCE -DNDEBUG -D_LARGEFILE_SOURCE -D_LARGEFILE64_SOURCE -D_FILE_OFFSET_BITS=64 -D"BB_VER=KBUILD_STR(1.20.2)" -DBB_BT=AUTOCONF_TIMESTAMP -D_LARGEFILE_SOURCE -D_LARGEFILE64_SOURCE -D_FILE_OFFSET_BITS=64  -pipe -U_FORTIFY_SOURCE -fno-stack-protector -fomit-frame-pointer -fPIC -O2  -I/home/simon/GIT/buildroot-k3-current/output/toolchain/linux/include -Wall -Wshadow -Wwrite-strings -Wundef -Wstrict-prototypes -Wunused -Wunused-parameter -Wunused-function -Wunused-value -Wmissing-prototypes -Wmissing-declarations -Wdeclaration-after-statement -Wold-style-definition -fno-builtin-strlen -finline-limit=0 -fomit-frame-pointer -ffunction-sections -fdata-sections -fno-guess-branch-probability -funsigned-char -static-libgcc -falign-functions=1 -falign-jumps=1 -falign-labels=1 -falign-loops=1 -Os     -D"KBUILD_STR(s)=\#s" -D"KBUILD_BASENAME=KBUILD_STR(ptr_to_globals)"  -D"KBUILD_MODNAME=KBUILD_STR(ptr_to_globals)" -c -o libbb/ptr_to_globals.o libbb/ptr_to_globals.c

deps_libbb/ptr_to_globals.o := \
  libbb/ptr_to_globals.c \
  /home/simon/GIT/buildroot-k3-current/output/host/usr/arm-buildroot-linux-gnueabi/sysroot/usr/include/errno.h \
  /home/simon/GIT/buildroot-k3-current/output/host/usr/arm-buildroot-linux-gnueabi/sysroot/usr/include/features.h \
  /home/simon/GIT/buildroot-k3-current/output/host/usr/arm-buildroot-linux-gnueabi/sysroot/usr/include/bits/predefs.h \
  /home/simon/GIT/buildroot-k3-current/output/host/usr/arm-buildroot-linux-gnueabi/sysroot/usr/include/sys/cdefs.h \
  /home/simon/GIT/buildroot-k3-current/output/host/usr/arm-buildroot-linux-gnueabi/sysroot/usr/include/bits/wordsize.h \
  /home/simon/GIT/buildroot-k3-current/output/host/usr/arm-buildroot-linux-gnueabi/sysroot/usr/include/gnu/stubs.h \
  /home/simon/GIT/buildroot-k3-current/output/host/usr/arm-buildroot-linux-gnueabi/sysroot/usr/include/bits/errno.h \
  /home/simon/GIT/buildroot-k3-current/output/host/usr/arm-buildroot-linux-gnueabi/sysroot/usr/include/linux/errno.h \
  /home/simon/GIT/buildroot-k3-current/output/host/usr/arm-buildroot-linux-gnueabi/sysroot/usr/include/asm/errno.h \
  /home/simon/GIT/buildroot-k3-current/output/host/usr/arm-buildroot-linux-gnueabi/sysroot/usr/include/asm-generic/errno.h \
  /home/simon/GIT/buildroot-k3-current/output/host/usr/arm-buildroot-linux-gnueabi/sysroot/usr/include/asm-generic/errno-base.h \

libbb/ptr_to_globals.o: $(deps_libbb/ptr_to_globals.o)

$(deps_libbb/ptr_to_globals.o):
