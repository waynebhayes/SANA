/*******************************************************************************
+
+  LEDA 5.0.1  
+
+
+  basic_graph_alg.h
+
+
+  Copyright (c) 1995-2005
+  by Algorithmic Solutions Software GmbH
+  All rights reserved.
+ 
*******************************************************************************/

// $Revision: 1.2 $  $Date: 2005/04/14 10:45:04 $

#ifndef LEDA_BASIC_GRAPHALG_H
#define LEDA_BASIC_GRAPHALG_H

#if !defined(LEDA_ROOT_INCL_ID)
#define LEDA_ROOT_INCL_ID 500011
#include <LEDA/internal/PREAMBLE.h>
#endif


#include <LEDA/graph/graph.h>
#include <LEDA/graph/node_matrix.h>

LEDA_BEGIN_NAMESPACE

/*{\Manpage{basic_graph_alg}{}{Basic Graph Algorithms}}*/
/*{\Mtext
\setopdims{1.9cm}{1cm}
}*/


extern __exportF bool TOPSORT(const graph& G, node_array<int>& ord);

/*{\Mfuncl
TOPSORT takes as argument a directed graph $G(V,E)$. It sorts $G$ topologically 
(if $G$ is acyclic) by computing for every node $v \in V$ an integer $ord[v]$ 
such that $1\le ord[v]\le \Labs{V}$ and $ord[v] < ord[w]$ for all edges 
$(v,w) \in E$. TOPSORT returns true if $G$ is acyclic and false otherwise.
The algorithm (\cite{Ka62}) has running time $O(\Labs{V}+\Labs{E})$.
\bigskip
}*/


extern __exportF bool TOPSORT(const graph& G, list<node>& L);

/*{\Mfuncl
a variant of TOPSORT that computes a list $L$ of nodes in topological
order (if $G$ is acyclic). It returns true if $G$ is acyclic and false
otherwise.
}*/



extern __exportF bool TOPSORT1(graph& G);


extern __exportF list<node> DFS(const graph& G, node s, node_array<bool>& reached) ;

/*{\Mfuncl
DFS takes as argument a directed graph $G(V,E)$, a node $s$ of $G$ and a 
node\_array $reached$ of boolean values. It performs a depth first search 
starting at $s$ visiting all reachable nodes $v$ with $reached[v]$ = false. 
For every visited node $v$ $reached[v]$ is changed to true. DFS returns the 
list of all reached nodes.
The algorithm (\cite{T72}) has running time $O(\Labs{V}+\Labs{E})$.
\bigskip
}*/

extern __exportF list<edge> DFS_NUM(const graph& G, node_array<int>& dfsnum, node_array<int>& compnum);
/*{\Mfuncl
DFS\_NUM takes as argument a directed graph $G(V,E)$. It performs a 
depth first search of $G$ numbering the nodes of $G$ in two different ways. 
$dfsnum$ is a numbering with respect to the calling time and $compnum$ a 
numbering with respect to the completion time of the recursive calls. DFS\_NUM 
returns a depth first search forest of $G$ (list of tree edges).
The algorithm (\cite{T72}) has running time $O(\Labs{V}+\Labs{E})$.
\bigskip
}*/


extern __exportF list<node> BFS(const graph& G, node s, node_array<int>& dist);

/*{\Mfuncl
BFS takes as argument a directed graph $G(V,E)$,a node $s$ of $G$ and
a node array $dist$ of integers. It performs a breadth first search starting 
at $s$ visiting all nodes $v$ with $dist[v] = -1$  reachable from $s$.
The $dist$ value of every visited node is replaced by its distance to $s$.  
BFS returns the list of all visited nodes.
The algorithm (\cite{M84}) has running time $O(\Labs{V}+\Labs{E})$.
\bigskip
}*/


extern __exportF list<node> BFS(const graph& G, node s, node_array<int>& dist,
                                                        node_array<edge>& pred);

/*{\Mfuncl
performs a bread first search as described above and computes for every node 
$v$ the predecessor edge $pred[v]$ in the bfs shortest path tree.
(You can use the function COMPUTE\_SHORTEST\_PATH to extract paths from the 
tree (cf. Section~\ref{shortest_path}).)
}*/



extern __exportF int COMPONENTS(const graph& G, node_array<int>& compnum);

/*{\Mfuncl
COMPONENTS takes a graph $G(V,E)$ as argument and computes the connected
components of the underlying undirected graph, i.e., for every node $v \in V$ 
an integer $compnum[v]$ from $[0\dots c-1]$ where $c$ is the number of 
connected components of $G$ and $v$ belongs to the $i$-th connected 
component iff $compnum[v] = i$.  COMPONENTS returns $c$.
The algorithm (\cite{M84}) has running time $O(\Labs{V}+\Labs{E})$.
\bigskip
}*/


extern __exportF int COMPONENTS1(const graph& G, node_array<int>& compnum);


extern __exportF int STRONG_COMPONENTS(const graph& G,node_array<int>& compnum);
/*{\Mfuncl
STRONG\_COMPONENTS takes a directed graph $G(V,E)$ as argument and computes for 
every node $v \in V$ an integer $compnum[v]$ from $[0\dots c-1]$ where
$c$ is the number of strongly connected components of $G$ and
$v$ belongs to the $i$-th strongly connected component iff $compnum[v] = i$.
STRONG\_COMPONENTS returns $c$.
The algorithm (\cite{M84}) has running time $O(\Labs{V}+\Labs{E})$.
\bigskip
}*/

extern __exportF int STRONG_COMPONENTS1(const graph&, node_array<int>&);


extern __exportF int BICONNECTED_COMPONENTS(const graph& G, edge_array<int>& compnum);

/*{\Mfuncl BICONNECTED\_COMPONENTS computes the biconnected components
of the undirected version of $G$. A biconnected component of an
undirected graph is a maximal biconnected subgraph and a biconnected
graph is a graph which cannot be disconnected by removing one of its
nodes. A graph having only one node is biconnected.\\ Let $c$ be the
number of biconnected component and let $c'$ be the number of
biconnected components containing at least one edge, $c - c'$ is the
number of isolated nodes in $G$, where a node $v$ is isolated if is
not connected to a node different from $v$ (it may be incident to
self-loops). The function returns $c$ and labels each edge of $G$
(which is not a self-loop) by an integer in $[0\dots c'-1]$. Two edges
receive the same label iff they belong to the same biconnected
component. The edge labels are returned in |compnum|.  Be aware that
self-loops receive no label since self-loops are ignored when
interpreting a graph as an undirected graph.\\ The algorithm
(\cite{Cheriyan-Mehlhorn}) has running time $O(\Labs{V}+\Labs{E})$. 
\bigskip }*/


extern __exportF GRAPH<node,edge> TRANSITIVE_CLOSURE(const graph& G);
/*{\Mfuncl
|TRANSITIVE_CLOSURE| takes a directed graph $G=(V,E)$ as argument and computes 
the transitive closure of $G$. It returns a directed graph $G'=(V',E')$ 
such that |G'.inf(.)| is a bijective mapping from $V'$ to $V$ and
$(v,w) \in E'  \Leftrightarrow$ there is a path from
|G'.inf(v')| to |G'.inf(w')| in $G$. 
(The edge information of $G'$ is undefined.)
The algorithm (\cite{GK79}) has running time $O(\Labs{V} \cdot \Labs{E})$.
\bigskip
}*/


extern __exportF GRAPH<node,edge> TRANSITIVE_REDUCTION(const graph& G);
/*{\Mfuncl
|TRANSITIVE_REDUCTION| takes a directed graph $G=(V,E)$ as argument and 
computes the transitive reduction of $G$. It returns a directed graph 
$G'=(V',E')$. The function |G'.inf(.)| is a bijective mapping from $V'$ to $V$. 
The graph $G$ and $G'$ have the same reachability relation, i.e. there is a 
path from $v'$ to $w'$ in $G'$ $\Leftrightarrow$ there is a path from
|G'.inf(v')| to |G'.inf(w')| in $G$. And there is no graph with the previous 
property and less edges than $G'$.
(The edge information of $G'$ is undefined.)
The algorithm (\cite{GK79}) has running time $O(\Labs{V} \cdot \Labs{E})$.
\bigskip
}*/

extern __exportF void MAKE_TRANSITIVELY_CLOSED(graph& G);
/*{\Mfuncl
|MAKE_TRANSITIVELY_CLOSED| transforms $G$ into its transitive closure
by adding edges.
}*/

extern __exportF void MAKE_TRANSITIVELY_REDUCED(graph& G);
/*{\Mfuncl
|MAKE_TRANSITIVELY_REDUCED| transforms $G$ into its transitive reduction
by removing edges.
}*/

#if LEDA_ROOT_INCL_ID == 500011
#undef LEDA_ROOT_INCL_ID
#include <LEDA/internal/POSTAMBLE.h>
#endif


LEDA_END_NAMESPACE

#endif
