/*******************************************************************************
+
+  LEDA 5.0.1  
+
+
+  max_flow_with_lcaps.t
+
+
+  Copyright (c) 1995-2005
+  by Algorithmic Solutions Software GmbH
+  All rights reserved.
+ 
*******************************************************************************/

// $Revision: 1.3 $  $Date: 2005/04/14 10:45:16 $

#if !defined(_LEDA_MAX_FLOW_WITH_LCAPS_T) && !defined(LEDA_NO_TEMPL_FCTS_FOR_STD_GRAPH)
#define _LEDA_MAX_FLOW_WITH_LCAPS_T

#if !defined(LEDA_ROOT_INCL_ID)
#define LEDA_ROOT_INCL_ID 500843
#include <LEDA/internal/PREAMBLE.h>
#endif

#ifndef _LEDA_MAX_FLOW_T
#include <LEDA/graph/templates/max_flow.h>
#endif

#include <LEDA/core/slist.h>
#include <LEDA/core/h_array.h>

LEDA_BEGIN_NAMESPACE

template<class NT>
NT MAX_FLOW_T(graph& G, node s, node t, 
              const edge_array<NT>& lcap, const edge_array<NT>& ucap, 
              edge_array<NT>& flow)
{
	// setup auxiliary graph
	node ss = G.new_node(), tt = G.new_node(); // add new source and target

	node_array<NT> excess(G, 0);
	edge_map<NT> _ucap(G);
	edge e;
	forall_edges(e, G) {
		excess[source(e)] -= lcap[e];
		excess[target(e)] += lcap[e];
		_ucap[e] = ucap[e] - lcap[e];
	}

	NT total = 0;
	node n;
	forall_nodes(n, G) {
		if (n == ss || n == tt) continue;

		if (excess[n] > 0) {
			_ucap[ G.new_edge(ss, n) ] = excess[n];
			// excess is simulated by ss -> ss tries to push excess[n] units into the network
			total += excess[n];
		}
		else {
			_ucap[ G.new_edge(n, tt) ] = -excess[n];
			// demand is simulated by tt -> tt tries to withdraw -excess[n] from the network
		}
	}

	edge e_ts = G.new_edge(t, s); _ucap[e_ts] = total; // infinity loop from t -> s

	// compute max flow in auxiliary graph
	edge_array<NT> ff_offset(G);
	max_flow<NT, graph> mf;
	mf.run(G, ss, tt, _ucap, ff_offset);

	// check feasibility (i.e. ss succeeds to push everything, and tt is able to withdraw all it wants to)
	bool feasible = true;
	forall_out_edges(e, ss) 
		if (_ucap[e] != ff_offset[e]) { feasible = false; break; }
	if (feasible) forall_in_edges(e, tt)
		if (_ucap[e] != ff_offset[e]) { feasible = false; break; }
	G.del_node(ss); G.del_node(tt); G.del_edge(e_ts);
	if (!feasible) return -1;

	// feasible flow: ff[e] = ff_offset[e] + lcap[e]

	// prepare final max flow computation
	h_array<edge,edge> reversal(nil);
	forall_edges(e, G) {
		if (ff_offset[e] > 0) {
			edge e_rev = G.new_edge(target(e), source(e));
			_ucap[e] -= ff_offset[e]; _ucap[e_rev] = ff_offset[e];
			reversal[e_rev] = e;
		}
	}
	edge_array<NT> _flow(G);
	mf.run(G, s, t, _ucap, _flow);

	// remove the reversal edges and update flow
	edge e_rev;
	forall_defined(e_rev, reversal) {
		_flow[reversal[e_rev]] -= _flow[e_rev];
		G.del_edge(e_rev);
	}

	// compute the final flow
	forall_edges(e, G) {
		flow[e] = lcap[e] + ff_offset[e] + _flow[e];
	}

	// and its value
	NT flow_val = 0;
	forall_out_edges(e, s) flow_val += flow[e];
	forall_in_edges(e, s) flow_val -= flow[e];

	return flow_val;
}

LEDA_END_NAMESPACE

#if LEDA_ROOT_INCL_ID == 500843
#undef LEDA_ROOT_INCL_ID
#include <LEDA/internal/POSTAMBLE.h>
#endif

#endif
