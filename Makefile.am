##
## Process with automake to produce Makefile.in
##

SUBDIRS = src libicq2000 examples

EXTRA_DIST = TODO THANKS NEWS doc libicq2000.m4 doxyfile.conf	\
libicq2000-config.1

bin_SCRIPTS = libicq2000-config

m4datadir = $(datadir)/aclocal
m4data_DATA = libicq2000.m4

man_MANS = libicq2000-config.1

docs: src/*.cpp src/*.h
	doxygen doxyfile.conf
	touch docs

dist: docs

install-docs: docs
	if [ ! -d $(DESTDIR)$(docdir) ]; then \
	  mkdir -p $(DESTDIR)$(docdir) ; \
	fi; \
	cp -r $(top_srcdir)/doc/* $(DESTDIR)$(docdir)

