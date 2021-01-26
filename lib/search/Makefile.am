noinst_LTLIBRARIES = libsearch.la

libsearch_la_SOURCES = \
	main-search.c \
	internal.h \
	srch-lib.c \
	normal.c \
	regex.c \
	glob.c \
	srch-hex.c

AM_CPPFLAGS = -I$(top_srcdir) $(GLIB_CFLAGS) $(PCRE_CPPFLAGS)
