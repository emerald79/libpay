AC_INIT(libemv, 0.1)
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
	src/include/Makefile
	src/libtlv/Makefile
	src/libtlv/libtlv.pc
	src/libemv/Makefile
	src/libemv/libemv.pc
	src/tlvdump/Makefile
	src/tests/Makefile
	src/tests/libtlv_test/Makefile
	src/tests/emvco_ep_ta/Makefile
])

PKG_CHECK_MODULES([CHECK], [check >= 0.9.4])
PKG_CHECK_MODULES([JSON_C], [json-c >= 0.11.0])
PKG_CHECK_MODULES([LOG4C], [log4c >= 1.2.4])

AC_OUTPUT
