MAINTAINERCLEANFILES = $(srcdir)/Makefile.in
ACLOCAL_AMFLAGS = -I m4

AM_CPPFLAGS = -I$(top_srcdir)/include

TESTS = tlv_test #emv_ep_test

check_PROGRAMS = tlv_test #emv_ep_test

tlv_test_SOURCES = tlv_test.c
tlv_test_LDADD = @CHECK_LIBS@ -ldl $(top_builddir)/src/libtlv/libtlv.la

#emv_ep_test_SOURCES = emv_ep_test.c lower_tester.c
#emv_ep_test_LDADD = @CHECK_LIBS@ -ldl $(top_builddir)/src/libtlv/libtlv.la \
#					$(top_builddir)/src/libemv/libemv.la
