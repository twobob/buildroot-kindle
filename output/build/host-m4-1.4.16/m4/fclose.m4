# fclose.m4 serial 2
dnl Copyright (C) 2008-2011 Free Software Foundation, Inc.
dnl This file is free software; the Free Software Foundation
dnl gives unlimited permission to copy and/or distribute it,
dnl with or without modifications, as long as this notice is preserved.

AC_DEFUN([gl_FUNC_FCLOSE],
[
])

AC_DEFUN([gl_REPLACE_FCLOSE],
[
  AC_REQUIRE([gl_STDIO_H_DEFAULTS])
  REPLACE_FCLOSE=1
  AC_LIBOBJ([fclose])
])
