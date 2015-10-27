AC_INIT(libtlv, 0.1)
AC_CANONICAL_TARGET
AC_CONFIG_MACRO_DIR([m4])
AM_INIT_AUTOMAKE
LT_INIT

AC_PROG_CC

AC_SYS_LARGEFILE

AC_CONFIG_FILES([
	Makefile
	include/Makefile
	src/Makefile
	src/libtlv/Makefile
	src/libtlv/libtlv.pc
	src/tests/Makefile
])

PKG_CHECK_MODULES([CHECK], [check >= 0.9.4])

AC_OUTPUT
