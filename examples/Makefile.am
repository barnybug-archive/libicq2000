##
## Process with automake to produce Makefile.in
##

noinst_PROGRAMS = ickle-shell

LDFLAGS = @SIGC_LIBS@
INCLUDES = -I$(top_srcdir)/src/ \
           @SIGC_CFLAGS@

ickle_shell_SOURCES = shell.cpp
ickle_shell_LDADD = $(top_srcdir)/src/libicq2000.la
ickle_shell_DEPENDENCIES = $(top_srcdir)/src/libicq2000.la

