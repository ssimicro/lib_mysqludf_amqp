AC_PREREQ(2.59)

AC_INIT([lib_mysqludf_amqp], [2.0.0], [thomasc@ssimicro.com])
AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_AUX_DIR([config])

AM_INIT_AUTOMAKE([-Wall -Werror foreign])
AM_SILENT_RULES([yes])

AC_LANG([C])

AC_PROG_CC
AM_PROG_AR
AC_PROG_LIBTOOL
AC_SUBST(LIBTOOL)

AC_CHECK_LIB([bsd], [arc4random])
AC_CHECK_LIB([dl], [dlopen])

AX_LIB_MYSQL([5.7.0])
if test -n "$MYSQL_PLUGINDIR" && test "$libdir" == '${exec_prefix}/lib' ; then
    AC_MSG_NOTICE(Setting libdir to mysql plugin dir $MYSQL_PLUGINDIR)
    libdir=$MYSQL_PLUGINDIR
    AC_SUBST(libdir)
else
    AC_MSG_NOTICE(No idea where to install this plugin. Will try $libdir)
fi

PKG_CHECK_MODULES([RABBITMQ], [librabbitmq])
AC_SUBST([RABBITMQ_CFLAGS])
AC_SUBST([RABBITMQ_LIBS])

AC_C_CONST
AC_TYPE_SIZE_T
AC_HEADER_STDC

AC_DEFINE([_BSD_SOURCE], [], [for snprintf()])
AC_DEFINE([_DEFAULT_SOURCE], [], [for GNU systems that complain about _BSD_SOURCE being deprecated])

AC_CONFIG_HEADERS([config.h:config.in])

AC_OUTPUT([Makefile])
