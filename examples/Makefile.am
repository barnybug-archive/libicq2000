##
## Process with automake to produce Makefile.in
##

noinst_PROGRAMS = ickle-shell

INCLUDES = -I$(top_srcdir)/src/

ickle_shell_SOURCES = shell.cpp shell.h PipeExec.cpp PipeExec.h Select.cpp Select.h
ickle_shell_LDADD = $(top_srcdir)/src/libicq2000.la
ickle_shell_DEPENDENCIES = $(top_srcdir)/src/libicq2000.la

