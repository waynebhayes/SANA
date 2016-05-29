/*******************************************************************************
+
+  LEDA 5.0.1  
+
+
+  plane_graph_alg.h
+
+
+  Copyright (c) 1995-2005
+  by Algorithmic Solutions Software GmbH
+  All rights reserved.
+ 
*******************************************************************************/

// $Revision: 1.2 $  $Date: 2005/04/14 10:45:13 $

#ifndef LEDA_PLANE_GRAPH_ALG_H
#define LEDA_PLANE_GRAPH_ALG_H


/*
#if !defined(LEDA_ROOT_INCL_ID)
#define LEDA_ROOT_INCL_ID 500212
#include <LEDA/internal/PREAMBLE.h>
#endif
*/

#include <LEDA/graph/graph.h>
#include <LEDA/core/list.h>

LEDA_BEGIN_NAMESPACE

/*{\Manpage{plane_graph_alg}{}{Algorithms for Planar Graphs}}*/

extern __exportF node ST_NUMBERING(const graph& G, 
                                   node_array<int>& stnum, 
                                   list<node>& stlist, edge e_st = nil); 
/*{\Mfuncl
ST\_NUMBERING computes an $st$-numbering of $G$. If |e_st| is nil then
$t$ is set to some arbitrary node of $G$. The node $s$ is set to a 
neighbor of $t$ and is returned. If |e_st| is not nil then $s$ is set
to the source of |e_st| and $t$ is set to its target. 
The nodes of $G$ are numbered such
that $s$ has number 1, $t$ has number $n$, and every node $v$ different
from $s$ and $t$ has a smaller and a larger numbered neighbor. 
The ordered list of nodes is returned in |stlist|. If $G$ has no nodes
then |nil| is returned and if $G$ has exactly one node then this node is returned and given number one. \\
\precond  $G$ is biconnected. }*/
       


extern __exportF bool HT_PLANAR(graph&, bool embed=false);
extern __exportF bool BL_PLANAR(graph&, bool embed=false);
extern __exportF bool PLANAR2(graph&, bool embed=false);

extern __exportF bool PLANAR(graph&, bool embed=false);

/*{\Mfuncl
PLANAR takes as input a directed graph $G(V,E)$ and performs a planarity test
for it. $G$ must not contain selfloops. If the second argument $embed$ has 
value $true$ and $G$ is a planar 
graph it is transformed into a planar map (a combinatorial embedding such that
the edges in all adjacency lists are in clockwise ordering). PLANAR returns 
true if $G$ is planar and false otherwise. 
The algorithm (\cite{HT74}) has running time $O(\Labs{V}+\Labs{E})$.
\bigskip
}*/


extern __exportF bool HT_PLANAR(graph&, list<edge>&, bool embed=false);
extern __exportF bool BL_PLANAR(graph&, list<edge>&, bool embed=false);

//__exportF bool BL_PLANAR(graph&, list<edge>&, bool embed,int);

extern __exportF bool PLANAR(graph& G, list<edge>& el, bool embed=false);
/*{\Mfuncl
PLANAR takes as input a directed graph $G(V,E)$ and performs a planarity test
for $G$. PLANAR returns true if $G$ is planar and false otherwise.
If $G$ is not planar a Kuratowsky-Subgraph is computed and returned in $el$.  
\bigskip
}*/

extern __exportF bool CHECK_KURATOWSKI(const graph& G, const list<edge>& el);
/*{\Mfuncl
returns |true| if all edges in |el| are edges of |G| and if the edges in |el| form a Kuratowski subgraph of $G$, returns |false| otherwise. Writes diagnostic output to |cerr|.\bigskip
}*/
 

extern __exportF int KURATOWSKI(graph& G, list<node>& V, list<edge>& E, node_array<int>& deg);
/*{\Mfuncl
KURATOWKI computes a Kuratowski subdivision $K$ of $G$ as follows. $V$ is the 
list of all nodes and subdivision points of $K$. For all $v \in V$ the degree
$deg[v]$ is equal to 2 for subdivision points, 4 for all other nodes if $K$ 
is a $K_5$, and -3 (+3) for the nodes of the left (right) side if $K$ is a 
$K_{3,3}$. $E$ is the list of all edges in the Kuratowski subdivision.
\bigskip }*/



extern __exportF list<edge> TRIANGULATE_PLANAR_MAP(graph& G);
/*{\Mfuncl
TRIANGULATE\_PLANAR\_MAP takes a directed graph $G$ representing a planar map.
It triangulates the faces of $G$ by inserting additional edges. The list of
inserted edges is returned.\\ \precond $G$ must be connected.\\
The algorithm (\cite{HU89}) has running time $O(\Labs{V}+\Labs{E})$.
\bigskip
}*/


extern __exportF void FIVE_COLOR(graph& G, node_array<int>& C);
/*{\Mfuncl colors the nodes of $G$ using 5 colors, more precisely, computes 
           for every node $v$ a color $C[v] \in \{0,\dots,4\}$,
           such that $C[source(e)] != C[target(e)]$  for every edge $e$. 
           \precond  $G$ is planar. {\bf Remark}: works also for many (sparse ?)
           non-planar graph. }*/


extern __exportF void INDEPENDENT_SET(const graph& G, list<node>& I);
/*{\Mfunc determines an independent set of nodes $I$ in $G$.
          Every node in $I$ has degree at most 9. If $G$ is planar and has
          no parallel edges then $I$ contains at least $n/6$ nodes.}*/


extern __exportF bool Is_CCW_Ordered(const graph& G, 
                            const node_array<int>& x,
                            const node_array<int>& y);
/*{\Mfunc checks whether the cyclic adjacency list of any node 
          $v$ agrees with the counter-clockwise ordering of the neighbors 
          of $v$ around $v$ defined by their geometric positions.}*/


extern __exportF bool SORT_EDGES(graph& G, 
                                 const node_array<int>& x,
                                 const node_array<int>& y);
/*{\Mfunc reorders all adjacency lists such the cyclic adjacency list
          of any node $v$ agrees with the counter-clockwise order of $v$'s 
          neighbors around $v$ defined by their geometric positions. 
          The function returns true if 
          $G$ is a plane map after the call.}*/

extern __exportF bool Is_CCW_Ordered(const graph& G, 
                            const edge_array<int>& dx,
                            const edge_array<int>& dy);
/*{\Mfunc checks whether the cyclic adjacency list of any node 
          $v$ agrees with the counter-clockwise ordering of the neighbors 
          of $v$ around $v$. The direction of edge $e$ is given by the vector
          $(dx(e),dy(e))$.}*/


extern __exportF bool SORT_EDGES(graph& G, 
                                 const edge_array<int>& dx,
                                 const edge_array<int>& dy);
/*{\Mfunc reorders all adjacency lists such the cyclic adjacency list
          of any node $v$ agrees with the counter-clockwise order of $v$'s 
          neighbors around $v$. The direction of edge $e$ is given 
          by the vector
          $(dx(e),dy(e))$. The function returns true if 
          $G$ is a plane map after the call.}*/





//-----------------------------------------------------------------------------
// floating point (double) versions 
//-----------------------------------------------------------------------------







extern __exportF bool Is_CCW_Ordered(const graph& G, 
                            const node_array<double>& x,
                            const node_array<double>& y);



extern __exportF bool SORT_EDGES(graph& G, 
                                 const node_array<double>& x,
                                 const node_array<double>& y);

extern __exportF bool Is_CCW_Ordered(const graph& G, 
                            const edge_array<double>& x,
                            const edge_array<double>& y);



extern __exportF bool SORT_EDGES(graph& G, 
                                 const edge_array<double>& x,
                                 const edge_array<double>& y);





LEDA_END_NAMESPACE

#endif

