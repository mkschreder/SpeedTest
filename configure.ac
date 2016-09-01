dnl Process this file with autoconf to produce a configure script
AC_INIT(speedtest,[0.1.0],mkschreder.uk@gmail.com)
AC_CONFIG_AUX_DIR(config)
AC_CONFIG_SRCDIR(src/)

m4_pattern_allow([AM_DEFAULT_VERBOSITY])

AM_INIT_AUTOMAKE
AX_CODE_COVERAGE

LT_INIT 
AC_PROG_CXX

PKG_CHECK_MODULES(EXTRA, libxml-2.0 libcurl openssl)
AC_SUBST(EXTRA_CFLAGS)
AC_SUBST(EXTRA_LIBS)

AC_OUTPUT(Makefile src/Makefile)

