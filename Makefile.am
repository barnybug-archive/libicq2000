##
## Process with automake to produce Makefile.in
##

SUBDIRS = src libicq2000 examples

EXTRA_DIST = THANKS NEWS doc \
             libicq2000.m4

bin_SCRIPTS = libicq2000-config

m4datadir = $(datadir)/aclocal
m4data_DATA = libicq2000.m4

docs: src/*.cpp src/*.h
	doxygen doxyfile.conf


