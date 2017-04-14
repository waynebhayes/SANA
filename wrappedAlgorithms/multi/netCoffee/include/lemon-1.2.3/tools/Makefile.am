EXTRA_DIST += \
	tools/CMakeLists.txt

if WANT_TOOLS

bin_PROGRAMS += \
	tools/dimacs-solver \
	tools/dimacs-to-lgf \
	tools/lgf-gen

dist_bin_SCRIPTS += tools/lemon-0.x-to-1.x.sh

endif WANT_TOOLS

tools_dimacs_solver_SOURCES = tools/dimacs-solver.cc
tools_dimacs_to_lgf_SOURCES = tools/dimacs-to-lgf.cc
tools_lgf_gen_SOURCES = tools/lgf-gen.cc
