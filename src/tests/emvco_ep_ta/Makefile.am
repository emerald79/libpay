MAINTAINERCLEANFILES = $(srcdir)/Makefile.in
ACLOCAL_AMFLAGS = -I m4

AM_CPPFLAGS = -I$(top_srcdir)/include

TESTS = emvco_ep_ta

check_PROGRAMS = emvco_ep_ta

emvco_ep_ta_SOURCES = emvco_ep_ta.c emvco_ep_ta_termsettings.c lower_tester.c
emvco_ep_ta_LDADD = @CHECK_LIBS@ -ldl $(top_builddir)/src/libtlv/libtlv.la $(top_builddir)/src/libemv/libemv.la
