MAINTAINERCLEANFILES = $(srcdir)/Makefile.in
ACLOCAL_AMFLAGS = -I m4

AM_CPPFLAGS = -I$(top_srcdir)/include

bin_PROGRAMS = ted

ted_CFLAGS = @LIBWEBSOCKETS_CFLAGS@ @JSON_C_CFLAGS@
ted_SOURCES = ted.c ted_args.c jsonrpc.c http.c tlv_json.c
ted_LDADD = @LIBWEBSOCKETS_LIBS@ @JSON_C_LIBS@ ../libtlv/libtlv.la

noinst_HEADERS = ted.h
