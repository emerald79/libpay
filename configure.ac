AC_INIT(libpay, 0.1.6)
AC_CANONICAL_TARGET
AC_CONFIG_MACRO_DIR([m4])
AM_INIT_AUTOMAKE
LT_INIT
DX_PS_FEATURE(OFF)
DX_INIT_DOXYGEN(LibPAY, doc/Doxyfile, doc/manual)
AX_VALGRIND_CHECK

AC_PROG_CC

AC_SYS_LARGEFILE

AC_CONFIG_FILES([
	Makefile
	doc/Makefile
	include/Makefile
	include/libpay/Makefile
	src/Makefile
	src/include/Makefile
	src/libtlv/Makefile
	src/libtlv/libtlv.pc
	src/libemv/Makefile
	src/libemv/libemv.pc
	src/ted/Makefile
	src/tlvdump/Makefile
	src/tests/Makefile
	src/tests/emv_ep_wrapper/Makefile
	src/tests/libtlv_test/Makefile
	src/tests/emvco_ep_ta/Makefile
])

AC_CONFIG_FILES(
	[src/tests/emvco_ep_ta/emvco_ep_ta.sh],
	[chmod +x src/tests/emvco_ep_ta/emvco_ep_ta.sh]
)

AC_ARG_ENABLE(gcov, AC_HELP_STRING([--enable-gcov],
					[turn on code coverage analysis tools [default=no]]))
AM_CONDITIONAL([ENABLE_GCOV], [test x${enable_gcov} = xyes])

if test "x$enable_gcov" = "xyes";
then
  AC_CHECK_PROGS(LCOV, lcov)
  if test "x$LCOV" == "x" ; then
    AC_MSG_ERROR([Please install lcov])
  fi

  AC_CHECK_PROGS(GENHTML, genhtml)
  if test "x$GENHTML" == "x" ; then
    AC_MSG_ERROR([Please install genhtml])
  fi

  AC_SUBST(GCOV_CFLAGS, "-g -fprofile-arcs -ftest-coverage")
fi

PKG_CHECK_MODULES([CHECK], [check >= 0.9.4])
PKG_CHECK_MODULES([JSON_C], [json-c >= 0.11.0])
AM_PATH_LOG4C(1.2.1)
PKG_CHECK_MODULES([LIBWEBSOCKETS], [libwebsockets >= 1.7])

AC_OUTPUT
