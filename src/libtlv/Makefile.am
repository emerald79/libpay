MAINTAINERCLEANFILES = $(srcdir)/Makefile.in

AM_CPPFLAGS = -I$(top_srcdir)/include 

lib_LTLIBRARIES = libtlv.la

libtlv_la_SOURCES = tlv.c

libtlv_la_CFLAGS = -fPIC $(AM_CFLAGS)

libtlv_la_LDFLAGS = $(AM_LDFLAGS) \
	-export-symbols "$(srcdir)/libtlv.exports" \
	-shared -avoid-version -noundefined -pthread

EXTRA_DIST = $(srcdir)/libtlv.exports

pcdatadir = $(libdir)/pkgconfig
pcdata_DATA = libtlv.pc
