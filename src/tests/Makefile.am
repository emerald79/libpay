MAINTAINERCLEANFILES = $(srcdir)/Makefile.in
ACLOCAL_AMFLAGS = -I m4

AM_CPPFLAGS = -I$(top_srcdir)/include

TESTS = tlv_test

check_PROGRAMS = tlv_test
tlv_test_SOURCES = tlv_test.c
tlv_test_LDADD = @CHECK_LIBS@ -ldl $(top_builddir)/src/libtlv/libtlv.la

