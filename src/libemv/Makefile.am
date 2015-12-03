MAINTAINERCLEANFILES = $(srcdir)/Makefile.in

AM_CPPFLAGS = -I$(top_srcdir)/include

lib_LTLIBRARIES = libemv.la

libemv_la_SOURCES = emv_ep.c emv_tag.c

libemv_la_CFLAGS = -fPIC $(AM_CFLAGS) @LOG4C_CFLAGS@

libemv_la_LIBADD = -ldl ../libtlv/libtlv.la @JSON_C_LIBS@ @LOG4C_LIBS@

libemv_la_LDFLAGS = $(AM_LDFLAGS) \
	-export-symbols "$(srcdir)/libemv.exports" \
	-shared -avoid-version -noundefined -pthread

EXTRA_DIST = $(srcdir)/libemv.exports

pcdatadir = $(libdir)/pkgconfig
pcdata_DATA = libemv.pc
