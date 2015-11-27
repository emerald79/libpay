MAINTAINERCLEANFILES = $(srcdir)/Makefile.in
ACLOCAL_AMFLAGS = -I m4

AM_CPPFLAGS = -I$(top_srcdir)/include

TESTS = libtlv_test

check_PROGRAMS = libtlv_test

libtlv_test_SOURCES = libtlv_test.c
libtlv_test_LDADD = @CHECK_LIBS@ -ldl $(top_builddir)/src/libtlv/libtlv.la
