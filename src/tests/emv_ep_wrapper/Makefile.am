MAINTAINERCLEANFILES = $(srcdir)/Makefile.in

AM_CPPFLAGS = -I$(top_srcdir)/include

lib_LTLIBRARIES = libemv_ep_wrapper.la

libemv_ep_wrapper_la_SOURCES = emv_ep_wrapper.c

libemv_ep_wrapper_la_CFLAGS = -fPIC $(AM_CFLAGS) @LOG4C_CFLAGS@ @GCOV_CFLAGS@

libemv_ep_wrapper_la_LIBADD = -ldl $(top_builddir)/src/libtlv/libtlv.la	       \
	$(top_builddir)/src/libemv/libemv.la @JSON_C_LIBS@ @LOG4C_LIBS@

libemv_ep_wrapper_la_LDFLAGS = $(AM_LDFLAGS)				       \
	-export-symbols "$(srcdir)/libemv_ep_wrapper.exports"		       \
	-shared -avoid-version -noundefined -pthread
