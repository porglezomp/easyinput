AC_PREREQ([2.69])
AC_INIT([easyinput], [0.3], [bugs.adventurous@gmail.com])
AC_CONFIG_AUX_DIR([build-aux])
AM_INIT_AUTOMAKE([-Wall -Werror foreign])
AC_CONFIG_SRCDIR([src/])
AC_CONFIG_HEADERS([config.h])

AC_PROG_CC
AC_PROG_RANLIB
AM_PROG_AR

AC_CHECK_HEADER([stdlib.h])
AC_CHECK_HEADER([sys/time.h])
AC_CHECK_HEADER([sys/types.h])
AC_CHECK_HEADER([unistd.h])
AC_CHECK_HEADER([linux/input.h])
AC_CHECK_HEADER([fcntl.h])
AC_CHECK_HEADER([stdio.h])

AC_CONFIG_FILES([Makefile
                 src/Makefile])

AC_OUTPUT
