if USE_VALGRIND
TESTS_ENVIRONMENT=$(top_srcdir)/scripts/valgrind-wrapper.sh
endif

EXTRA_DIST += \
	test/CMakeLists.txt

noinst_HEADERS += \
	test/graph_test.h \
	test/test_tools.h

check_PROGRAMS += \
	test/adaptors_test \
	test/bellman_ford_test \
	test/bfs_test \
	test/circulation_test \
	test/connectivity_test \
	test/counter_test \
	test/dfs_test \
	test/digraph_test \
	test/dijkstra_test \
	test/dim_test \
	test/edge_set_test \
	test/error_test \
	test/euler_test \
	test/fractional_matching_test \
	test/gomory_hu_test \
	test/graph_copy_test \
	test/graph_test \
	test/graph_utils_test \
	test/hao_orlin_test \
	test/heap_test \
	test/kruskal_test \
	test/lgf_test \
	test/maps_test \
	test/matching_test \
	test/min_cost_arborescence_test \
	test/min_cost_flow_test \
	test/min_mean_cycle_test \
	test/path_test \
	test/planarity_test \
	test/preflow_test \
	test/radix_sort_test \
	test/random_test \
	test/suurballe_test \
	test/test_tools_fail \
	test/test_tools_pass \
	test/time_measure_test \
	test/unionfind_test

test_test_tools_pass_DEPENDENCIES = demo

if HAVE_LP
check_PROGRAMS += test/lp_test
endif HAVE_LP
if HAVE_MIP
check_PROGRAMS += test/mip_test
endif HAVE_MIP

TESTS += $(check_PROGRAMS)
XFAIL_TESTS += test/test_tools_fail$(EXEEXT)

test_adaptors_test_SOURCES = test/adaptors_test.cc
test_bellman_ford_test_SOURCES = test/bellman_ford_test.cc
test_bfs_test_SOURCES = test/bfs_test.cc
test_circulation_test_SOURCES = test/circulation_test.cc
test_counter_test_SOURCES = test/counter_test.cc
test_connectivity_test_SOURCES = test/connectivity_test.cc
test_dfs_test_SOURCES = test/dfs_test.cc
test_digraph_test_SOURCES = test/digraph_test.cc
test_dijkstra_test_SOURCES = test/dijkstra_test.cc
test_dim_test_SOURCES = test/dim_test.cc
test_edge_set_test_SOURCES = test/edge_set_test.cc
test_error_test_SOURCES = test/error_test.cc
test_euler_test_SOURCES = test/euler_test.cc
test_fractional_matching_test_SOURCES = test/fractional_matching_test.cc
test_gomory_hu_test_SOURCES = test/gomory_hu_test.cc
test_graph_copy_test_SOURCES = test/graph_copy_test.cc
test_graph_test_SOURCES = test/graph_test.cc
test_graph_utils_test_SOURCES = test/graph_utils_test.cc
test_hao_orlin_test_SOURCES = test/hao_orlin_test.cc
test_heap_test_SOURCES = test/heap_test.cc
test_kruskal_test_SOURCES = test/kruskal_test.cc
test_lgf_test_SOURCES = test/lgf_test.cc
test_lp_test_SOURCES = test/lp_test.cc
test_maps_test_SOURCES = test/maps_test.cc
test_mip_test_SOURCES = test/mip_test.cc
test_matching_test_SOURCES = test/matching_test.cc
test_min_cost_arborescence_test_SOURCES = test/min_cost_arborescence_test.cc
test_min_cost_flow_test_SOURCES = test/min_cost_flow_test.cc
test_min_mean_cycle_test_SOURCES = test/min_mean_cycle_test.cc
test_path_test_SOURCES = test/path_test.cc
test_planarity_test_SOURCES = test/planarity_test.cc
test_preflow_test_SOURCES = test/preflow_test.cc
test_radix_sort_test_SOURCES = test/radix_sort_test.cc
test_suurballe_test_SOURCES = test/suurballe_test.cc
test_random_test_SOURCES = test/random_test.cc
test_test_tools_fail_SOURCES = test/test_tools_fail.cc
test_test_tools_pass_SOURCES = test/test_tools_pass.cc
test_time_measure_test_SOURCES = test/time_measure_test.cc
test_unionfind_test_SOURCES = test/unionfind_test.cc
