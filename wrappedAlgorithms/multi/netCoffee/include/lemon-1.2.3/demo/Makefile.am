AM_CXXFLAGS = $(WARNINGCXXFLAGS)

AM_CPPFLAGS = -I$(top_srcdir) -I$(top_builddir)
LDADD = $(top_builddir)/lemon/libemon.la

EXTRA_DIST = \
	CMakeLists.txt \
	digraph.lgf

noinst_PROGRAMS = \
	arg_parser_demo \
	graph_to_eps_demo \
	lgf_demo

arg_parser_demo_SOURCES = arg_parser_demo.cc
graph_to_eps_demo_SOURCES = graph_to_eps_demo.cc
lgf_demo_SOURCES = lgf_demo.cc
