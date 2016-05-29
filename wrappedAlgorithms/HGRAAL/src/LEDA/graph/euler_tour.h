/*******************************************************************************
+
+  LEDA 5.0.1  
+
+
+  euler_tour.h
+
+
+  Copyright (c) 1995-2005
+  by Algorithmic Solutions Software GmbH
+  All rights reserved.
+ 
*******************************************************************************/

// $Revision: 1.2 $  $Date: 2005/04/14 10:45:05 $

#ifndef LEDA_EULER_TOUR_H
#define LEDA_EULER_TOUR_H

#include <LEDA/graph/graph.h>
#include <LEDA/core/tuple.h>

LEDA_BEGIN_NAMESPACE

/*{\Manpage{euler_tour}{}{Euler Tours}}*/

/*{\Mtext An Euler tour in an undirected graph $G$ 
is a cycle using every edge of $G$ exactly once. A graph has an Euler tour if 
it is connected and the degree of every vertex is even. }*/


extern __exportF bool Euler_Tour(const graph& G, list<two_tuple<edge,int> >& T);
     /*{\Mfunc The function returns true if the undirected version of 
$G$ has an Euler tour. The Euler
tour is returned in $T$. The items in $T$ are of the form $(e,\pm +1)$,
where the second component indicates the traversal direction $d$ of the edge.
If $d = +1$, the edge is traversed in forward direction, and if $d = -1$, the 
edge is traversed in reverse direction. The running time is $O(n + m)$.}*/

extern __exportF bool Check_Euler_Tour(const graph& G, const list<two_tuple<edge,int> >& T);
     /*{\Mfunc returns true if $T$ is an Euler tour in $G$. 
The running time is $O(n + m)$.}*/

extern __exportF bool Euler_Tour(graph& G, list<edge>& T);
     /*{\Mfunc The function returns true if the undirected verion of 
$G$ has an Euler tour. 
$G$ is reoriented such that every node has indegree equal to its outdegree 
and an Euler tour (of the reoriented graph) is returned in $T$. 
The running time is $O(n + m)$. }*/

bool __exportF Check_Euler_Tour(const graph& G, const list<edge>& T);
/*{\Mfunc returns true if $T$ is an Euler tour in the directed graph $G$. 
The running time is $O(n + m)$.}*/



LEDA_END_NAMESPACE

#endif
