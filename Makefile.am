##
## Process with automake to produce Makefile.in
##

SUBDIRS = src libicq2000 examples

EXTRA_DIST = THANKS NEWS doc libicq2000.m4 debian/README	\
debian/changelog debian/control debian/copyright		\
debian/libicq2000-dev.files debian/libicq2000-dev.links		\
debian/libicq2000-doc.links debian/libicq2000.docs		\
debian/libicq2000.files debian/rules                            \
debian/libicq2000-dev.manpages doxyfile.conf libicq2000-config.1

bin_SCRIPTS = libicq2000-config

m4datadir = $(datadir)/aclocal
m4data_DATA = libicq2000.m4

man_MANS = libicq2000-config.1

docs: src/*.cpp src/*.h
	doxygen doxyfile.conf

dist: docs

