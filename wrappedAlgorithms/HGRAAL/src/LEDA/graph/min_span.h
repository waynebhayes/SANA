/*******************************************************************************
+
+  LEDA 5.0.1  
+
+
+  min_span.h
+
+
+  Copyright (c) 1995-2005
+  by Algorithmic Solutions Software GmbH
+  All rights reserved.
+ 
*******************************************************************************/

// $Revision: 1.2 $  $Date: 2005/04/14 10:45:10 $

#ifndef LEDA_MINSPAN_H
#define LEDA_MINSPAN_H

/*
#if !defined(LEDA_ROOT_INCL_ID)
#define LEDA_ROOT_INCL_ID 500175
#include <LEDA/internal/PREAMBLE.h>
#endif
*/

#include <LEDA/graph/graph.h>
#include <LEDA/core/list.h>

LEDA_BEGIN_NAMESPACE

/*{\Manpage{min_span}{}{Minimum Spanning Trees}}*/

extern __exportF list<edge> SPANNING_TREE(const graph& G);
/*{\Mfuncl
SPANNING\_TREE takes as argument a graph $G(V,E)$. It computes a spanning
tree $T$ of the underlying undirected graph, SPANNING\_TREE returns the 
list of edges of $T$.
The algorithm (\cite{M84}) has running time $O(\Labs{V}+\Labs{E})$.
\bigskip
}*/

extern __exportF void SPANNING_TREE1(graph& G);
/*{\Mfuncl
SPANNING\_TREE takes as argument a graph $G(V,E)$. It computes a spanning
tree $T$ of the underlying undirected graph by deleting the edges in $G$ that do not
belong to $T$.
The algorithm (\cite{M84}) has running time $O(\Labs{V}+\Labs{E})$.
\bigskip
}*/


extern __exportF list<edge> MIN_SPANNING_TREE(const graph& G, const edge_array<int>& cost);
/*{\Mfuncl
MIN\_SPANNING\_TREE takes as argument an undirected graph $G(V,E)$ and an 
edge\_array $cost$ giving for each edge an integer cost. 
It computes a minimum spanning 
tree $T$ of $G$, i.e., a spanning tree such that the sum of all edge costs
is minimal. MIN\_SPANNING\_TREE returns the list of edges of $T$.
The algorithm (\cite{Kr56}) has running time $O(\Labs{E} \log\Labs{V})$.
\bigskip
}*/


extern __exportF list<edge> MIN_SPANNING_TREE(const graph& G,
                                       const leda_cmp_base<edge>& cmp);
/*{\Mfuncl A variant using a {\em compare object} to compare edge costs. }*/


extern __exportF list<edge> MIN_SPANNING_TREE(const graph& G,
                                       int (*cmp)(const edge&,const edge&));
/*{\Mfuncl A variant using a {\em compare function} to compare edge costs. }*/



// double versions

extern __exportF list<edge> MIN_SPANNING_TREE(const graph& G, 
                                              const edge_array<double>& cost);

extern __exportF list<edge> MIN_SPANNING_TREE(const graph& G, 
                                              const edge_array<double>& cost);




LEDA_END_NAMESPACE

#endif

