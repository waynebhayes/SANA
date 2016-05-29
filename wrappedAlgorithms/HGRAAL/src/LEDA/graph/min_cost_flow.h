/*******************************************************************************
+
+  LEDA 5.0.1  
+
+
+  min_cost_flow.h
+
+
+  Copyright (c) 1995-2005
+  by Algorithmic Solutions Software GmbH
+  All rights reserved.
+ 
*******************************************************************************/

// $Revision: 1.4 $  $Date: 2005/04/14 10:45:09 $

#ifndef LEDA_MINCOSTFLOW_H
#define LEDA_MINCOSTFLOW_H


#if !defined(LEDA_ROOT_INCL_ID)
#define LEDA_ROOT_INCL_ID 500173
#include <LEDA/internal/PREAMBLE.h>
#endif

#include <LEDA/graph/graph.h>

LEDA_BEGIN_NAMESPACE


/*{\Manpage{min_cost_flow}{}{Min Cost Flow Algorithms}
}*/


extern __exportF bool FEASIBLE_FLOW(graph& G, const node_array<int>& supply,
                                              const edge_array<int>& lcap,
                                              const edge_array<int>& ucap,
                                              edge_array<int>& flow);


extern __exportF bool FEASIBLE_FLOW(graph& G, const node_array<int>& supply,
                                              const edge_array<int>& cap,
                                              edge_array<int>& flow);


extern __exportF bool MCF_PRIMAL_DUAL(graph& G, const edge_array<int>& ucap,
                                                const edge_array<int>& cost,
                                                const node_array<int>& supply,
                                                edge_array<int>& flow);

extern __exportF bool MCF_PRIMAL_DUAL1(graph& G, node s, node t, int supply,
                                                 const edge_array<int>& ucap,
                                                 const edge_array<int>& cost,
                                                 edge_array<int>& flow);

// capacity scaling

extern __exportF bool MCF_CAPACITY_SCALING_OLD(graph& G,
                                                 const edge_array<int>& lcap,
                                                 const edge_array<int>& ucap,
                                                 const edge_array<int>& cost,
                                                 const node_array<int>& supply,
                                                 edge_array<int>& flow);

extern __exportF bool MCF_CAPACITY_SCALING(const graph& G,
                                                 const edge_array<int>& lcap,
                                                 const edge_array<int>& ucap,
                                                 const edge_array<int>& cost,
                                                 const node_array<int>& supply,
                                                 edge_array<int>& flow);

extern __exportF bool MCF_CAPACITY_SCALING(const graph& G,
                                                 const edge_array<int>& cap,
                                                 const edge_array<int>& cost,
                                                 const node_array<int>& supply,
                                                 edge_array<int>& flow);

extern __exportF int MCMF_CAPACITY_SCALING(const graph& G, node s, node t, 
                                                    const edge_array<int>& cap,
                                                    const edge_array<int>& cost,
                                                    edge_array<int>& flow);



// cost scaling

extern __exportF bool MCF_COST_SCALING(const graph& G, 
                                                const edge_array<int>& lcap,
                                                const edge_array<int>& ucap,
                                                const edge_array<int>& cost,
                                                const node_array<int>& supply,
                                                edge_array<int>& flow);

extern __exportF int MCMF_COST_SCALING(const graph& G,node s,node t, 
                                                const edge_array<int>& cap, 
                                                const edge_array<int>& cost, 
                                                edge_array<int>& flow);



extern __exportF bool MCF_COST_SCALING_OLD(graph& G, 
                                                 const edge_array<int>& lcap,
                                                 const edge_array<int>& ucap,
                                                 const edge_array<int>& cost,
                                                 const node_array<int>& supply,
                                                 edge_array<int>& flow);

extern __exportF int MCMF_COST_SCALING_OLD(graph& G,node s,node t, 
                                                const edge_array<int>& cap, 
                                                const edge_array<int>& cost, 
                                                edge_array<int>& flow);







inline bool MIN_COST_FLOW(graph& G, const edge_array<int>& lcap,
                                    const edge_array<int>& ucap,
                                    const edge_array<int>& cost,
                                    const node_array<int>& supply,
                                          edge_array<int>& flow)
{ return MCF_COST_SCALING(G,lcap,ucap,cost,supply,flow); }

/*{\Mfuncl
MIN\_COST\_FLOW takes as arguments a directed graph $G(V,E)$, an edge\_array
$lcap$ ($ucap$) giving for each edge a lower (upper) capacity bound,
an edge\_array $cost$ specifying for each edge an integer cost and a
node\_array $supply$ defining for each node $v$ a supply or demand 
(if $supply[v] < 0$). If a feasible flow (fulfilling the capacity
and mass balance conditions) exists it computes such a $flow$ of minimal cost
and returns |true|, otherwise |false| is returned.
The algorithm is based on capacity scaling and successive shortest path
computation (cf. \cite{EK72} and \cite{AMO93}) and has running time 
$O(\Labs{E}\log U (\Labs{E} + \Labs{V} \log\Labs{V}))$.
}*/



inline bool MIN_COST_FLOW(graph& G, const edge_array<int>& cap,
                                    const edge_array<int>& cost,
                                    const node_array<int>& supply,
                                          edge_array<int>& flow)
{ 
  edge_array<int> lcap(G);
  edge e;
  forall_edges(e,G) lcap[e] = 0;
  return MCF_COST_SCALING(G,cap,cap,cost,supply,flow); 
 }
/*{\Mfuncl
This variant of MIN\_COST\_FLOW assumes that $lcap[e] = 0$ for every
edge $e \in E$. }*/


inline int MIN_COST_MAX_FLOW(graph& G, node s, node t, 
                                       const edge_array<int>& cap, 
                                       const edge_array<int>& cost, 
                                       edge_array<int>& flow)
{ return MCMF_COST_SCALING(G,s,t,cap,cost,flow); }

/*{\Mfuncl
MIN\_COST\_MAX\_FLOW takes as arguments a directed graph $G(V,E)$, a source 
node $s$, a sink node $t$, an edge\_array $cap$ giving for each edge in $G$ a 
capacity, and an edge\_array $cost$ specifying for each edge an integer cost. 
It computes for every edge $e$ in $G$ a flow $flow[e]$ such that the total 
flow from $s$ to $t$ is maximal, the total cost of the flow is minimal,
and $0 \le flow[e] \le cap[e]$ for all edges $e$. 
MIN\_COST\_MAX\_FLOW returns the total flow from $s$ to $t$.
\bigskip
}*/

#if LEDA_ROOT_INCL_ID == 500173
#undef LEDA_ROOT_INCL_ID
#include <LEDA/internal/POSTAMBLE.h>
#endif


LEDA_END_NAMESPACE

#endif
