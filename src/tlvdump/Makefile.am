MAINTAINERCLEANFILES = $(srcdir)/Makefile.in
ACLOCAL_AMFLAGS = -I m4

AM_CPPFLAGS = -I$(top_srcdir)/include

bin_PROGRAMS = tlvdump

tlvdump_CFLAGS = @LOG4C_CFLAGS@
tlvdump_SOURCES = tlvdump.c
tlvdump_LDADD = $(top_builddir)/src/libtlv/libtlv.la \
		$(top_builddir)/src/libemv/libemv.la \
		@LOG4C_LIBS@
