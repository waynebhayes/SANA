
/*******************************************************************************
+
+  LEDA 5.0.1  
+
+
+  st_matching.h
+
+
+  Copyright (c) 1995-2005
+  by Algorithmic Solutions Software GmbH
+  All rights reserved.
+ 
*******************************************************************************/

// $Revision: 1.2 $  $Date: 2005/04/14 10:45:14 $

#ifndef LEDA_ST_MATCHING
#define LEDA_ST_MATCHING

#include <LEDA/core/array.h>
#include <LEDA/graph/graph.h>
#include <LEDA/system/misc.h>
#include <LEDA/core/tuple.h>
#include <LEDA/graph/graph_alg.h>
#include <LEDA/graph/node_map.h>

#if !defined(LEDA_ROOT_INCL_ID)
#define LEDA_ROOT_INCL_ID 500313
#include <LEDA/internal/PREAMBLE.h>
#endif

#if defined(LEDA_NAMESPACE)
LEDA_BEGIN_NAMESPACE
#endif

/*{\Manpage{stable_matching}{}{Stable Matching}}*/

/*{\Mtext We are given a bipartite graph $G = (A \cup B, E)$ in which the
edges incident to every vertex are linearly ordered. The order expresses preferences.
A matching $M$ in $G$ is \emph{stable} if there is no pair $(a,b) \in E \setminus M$
such that (1) $a$ is unmatched or prefers $b$ over its partner in $M$  and (2) $b$ is
unmatched or prefers $a$ over its partner in $M$. In such a situation
$a$ has the intention to switch to $b$ and $b$ has the intention to
switch to $a$, i.e., the
pairing is unstable.\\
We provide a function to compute a correct input graph from the preference data,
a function that computes the stable matching when the graph is given and a function
that checks whether a given matching is stable.

}*/


extern __exportF void StableMatching(const graph& G,
                    const list<node>& A,
                    const list<node>& B,
                    list<edge>& M);
     /*{\Mfunc The function takes a bipartite graph $G$ with sides $A$ and $B$ and
computes a maximal stable matching $M$ which is $A$-optimal. The graph is assumed to be
bidirected, i.e, for each $(a,b) \in E$ we also have $(b,a) \in E$. It is assumed
that adjacency lists record the preferences of the vertices. The running time
is $O(n + m)$.\\
\precond The graph $G$ is bidirected and a map.\\
Sets $A$ and $B$ only contain nodes of graph $G$. In addition they are disjoint
from each other.}*/


extern __exportF bool CheckStableMatching(const graph& G, const list<node>& A,
                    const list<node>& B,
                    const list<edge>& M);
     /*{\Mfunc returns true if $M$ is a stable matching in $G$.
The running time is $O(n + m)$.\\
\precond $A$ and $B$ only contain nodes from $G$.\\
The graph $G$ is bipartite with respect to lists $A$ and
$B$.}*/


extern __exportF void CreateInputGraph(graph& G,
                      list<node>& A,
                      list<node>& B,
                      node_map<int>& nodes_a, // nodes of objects in A
                      node_map<int>& nodes_b, // nodes of objects in B
                      const list<int>& InputA,
                      const list<int>& InputB,
                      const map<int, list<int> >& preferencesA,
                      const map<int, list<int> >& preferencesB);
/*{\Mfunc The function takes a list of objects $InputA$ and a list of objects $InputB$.
The objects are represented bei integer numbers, multiple occurences
of the same number in the same list are ignored. The maps $preferencesA$ and
$preferencesB$ give for each object $i$ the list of partner candidates
with respect to a matching. The lists are decreasingly ordered according to the preferences.
The function computes the input data $G$, $A$ and $B$ for calling
the function $StableMatching(const graph\&, ...)$. The maps $nodes\_a$ and
$nodes\_b$ provide the objects in $A$ and $B$ corresponding to the nodes in the graph.\\
\precond The entries in the lists in the preference maps only contain elements from
$InputB$ resp. $InputA$.\\
There are no multiple occurences of an element in the same such list.}*/


#if LEDA_ROOT_INCL_ID == 500313
#undef LEDA_ROOT_INCL_ID
#include <LEDA/internal/POSTAMBLE.h>
#endif


LEDA_END_NAMESPACE

#endif

