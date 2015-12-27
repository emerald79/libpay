MAINTAINERCLEANFILES = $(srcdir)/Makefile.in
ACLOCAL_AMFLAGS = -I m4

AM_CPPFLAGS = -I$(top_srcdir)/include

TESTS = libtlv_test

check_PROGRAMS = libtlv_test

libtlv_test_SOURCES = libtlv_test.c
libtlv_test_CFLAGS = $(AM_CFLAGS) @CHECK_CFLAGS@ @LOG4C_CFLAGS@
libtlv_test_LDADD = $(AM_LDADD) @CHECK_LIBS@ @LOG4C_LIBS@ -ldl $(top_builddir)/src/libtlv/libtlv.la
