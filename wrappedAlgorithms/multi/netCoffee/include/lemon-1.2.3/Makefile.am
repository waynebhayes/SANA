ACLOCAL_AMFLAGS = -I m4

AM_CXXFLAGS = $(WARNINGCXXFLAGS)

AM_CPPFLAGS = -I$(top_srcdir) -I$(top_builddir)
LDADD = $(top_builddir)/lemon/libemon.la

EXTRA_DIST = \
	AUTHORS \
	LICENSE \
	m4/lx_check_cplex.m4 \
	m4/lx_check_glpk.m4 \
	m4/lx_check_soplex.m4 \
	m4/lx_check_coin.m4 \
	CMakeLists.txt \
	cmake/FindGhostscript.cmake \
	cmake/FindCPLEX.cmake \
	cmake/FindGLPK.cmake \
	cmake/FindCOIN.cmake \
	cmake/LEMONConfig.cmake.in \
	cmake/version.cmake.in \
	cmake/version.cmake \
	cmake/nsis/lemon.ico \
	cmake/nsis/uninstall.ico

pkgconfigdir = $(libdir)/pkgconfig
lemondir = $(pkgincludedir)
bitsdir = $(lemondir)/bits
conceptdir = $(lemondir)/concepts
pkgconfig_DATA =
lib_LTLIBRARIES =
lemon_HEADERS =
bits_HEADERS =
concept_HEADERS =
noinst_HEADERS =
noinst_PROGRAMS =
bin_PROGRAMS =
check_PROGRAMS =
dist_bin_SCRIPTS =
TESTS =
XFAIL_TESTS =

include lemon/Makefile.am
include test/Makefile.am
include doc/Makefile.am
include tools/Makefile.am
include scripts/Makefile.am

DIST_SUBDIRS = demo

demo:
	$(MAKE) $(AM_MAKEFLAGS) -C demo

MRPROPERFILES = \
	aclocal.m4 \
	config.h.in \
	config.h.in~ \
	configure \
	Makefile.in \
	build-aux/config.guess \
	build-aux/config.sub \
	build-aux/depcomp \
	build-aux/install-sh \
	build-aux/ltmain.sh \
	build-aux/missing \
	doc/doxygen.log

mrproper:
	$(MAKE) $(AM_MAKEFLAGS) maintainer-clean
	-rm -f $(MRPROPERFILES)

dist-bz2: dist
	zcat $(PACKAGE)-$(VERSION).tar.gz | \
	bzip2 --best -c > $(PACKAGE)-$(VERSION).tar.bz2

distcheck-bz2: distcheck
	zcat $(PACKAGE)-$(VERSION).tar.gz | \
	bzip2 --best -c > $(PACKAGE)-$(VERSION).tar.bz2

.PHONY: demo mrproper dist-bz2 distcheck-bz2
