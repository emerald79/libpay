MAINTAINERCLEANFILES = $(srcdir)/Makefile.in $(srcdir)/test-suite-drd.log      \
	     $(srcdir)/test-suite-helgrind.log $(srcdir)/test-suite-memcheck.log

ACLOCAL_AMFLAGS = -I m4

AM_CPPFLAGS = -I$(top_srcdir)/include

@VALGRIND_CHECK_RULES@

TESTS = emvco_ep_ta

check_PROGRAMS = emvco_ep_ta

emvco_ep_ta_SOURCES = emvco_ep_ta.c term.c lt.c tk.c chk.c 
emvco_ep_ta_CFLAGS = $(AM_CFLAGS) @LOG4C_CFLAGS@
emvco_ep_ta_LDADD = @CHECK_LIBS@ @LOG4C_LIBS@ -ldl			       \
					  $(top_builddir)/src/libtlv/libtlv.la \
					  $(top_builddir)/src/libemv/libemv.la \
		   $(top_builddir)/src/tests/emv_ep_wrapper/libemv_ep_wrapper.la
noinst_HEADERS = emvco_ep_ta.h
