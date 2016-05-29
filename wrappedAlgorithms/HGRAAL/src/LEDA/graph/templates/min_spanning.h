/*******************************************************************************
+
+  LEDA 5.0.1  
+
+
+  min_spanning.t
+
+
+  Copyright (c) 1995-2005
+  by Algorithmic Solutions Software GmbH
+  All rights reserved.
+ 
*******************************************************************************/

// $Revision: 1.2 $  $Date: 2005/04/14 10:45:17 $

#if !defined(LEDA_ROOT_INCL_ID)
#define LEDA_ROOT_INCL_ID 500361
#include <LEDA/internal/PREAMBLE.h>
#endif


/*******************************************************************************
 *
 *                          MIN_SPANNING_TREE
 *
 *      Kruskal's Algorithm for computing a minimum spanning tree
 *
 * In order to avoid sorting the entire edge set we proceed as follows:
 * We first run Kruskal's Algorithm with the 3*n cheapest edges of the graph. 
 * In general, this gives already a good approximation. Then we collect all 
 * edges still connecting different components and run the algorithm for them.
 * 
 *       worst case running time:  m * log n
 * ????? expected running time:    m * alpha(n) + n * log n  ?????
 *
 * last modified:  April 1998  (leda_cmp_base)
 *
 ******************************************************************************/

#include <LEDA/graph/graph_alg.h>
#include <LEDA/graph/node_partition.h>
#include <LEDA/graph/node_pq.h>
#include <LEDA/core/basic_alg.h>

LEDA_BEGIN_NAMESPACE


template<class NT> 
class cmp_edges : public leda_cmp_base<edge> {
   const edge_array<NT>* edge_cost;
public:

   cmp_edges(const edge_array<NT>& cost) : edge_cost(&cost) {}

   int operator()(const edge& x, const edge& y) const
   { return compare((*edge_cost)[x],(*edge_cost)[y]); }
};


template<class NT>
__temp_func_inline
void KRUSKAL(list<edge>& L, const edge_array<NT>& cost, node_partition& P, 
                                                        list<edge>& T,
                                                        NT /*dummy*/)
{ 
  cmp_edges<NT> cmp(cost);
  L.sort(cmp);

  edge e;
  forall(e,L)
  { node v = source(e);
    node w = target(e);
    if (! P.same_block(v,w))
    { T.append(e);
      P.union_blocks(v,w);
     }
   }
}



template <class NT>
__temp_func_inline
list<edge> MIN_SPANNING_TREE_T(const graph& G, const edge_array<NT>& cost)
{ 
  list<edge> T;
  list<edge> L;
  node_partition P(G);
  edge e;

  int n = G.number_of_nodes();
  int m = G.number_of_edges();

  if (m == 0) return T;


/* Compute 3n-th biggest edge cost x by SELECT (from basic_alg.h)
 * and sort all edges with cost smaller than x in a list L
 */

  if (m > 3*n)
   { NT* c = new NT[m];
     NT* p = c;
     forall_edges(e,G) *p++ = cost[e];

     NT x = SELECT(c,p-1,3*n);

     delete[] c;

     forall_edges(e,G) 
        if (cost[e] < x) L.append(e);
    }
  else
    L = G.all_edges();


  KRUSKAL(L,cost,P,T,NT(0));


// collect and sort edges still connecting different trees into L
// and run the algorithm on L

  L.clear();

  forall_edges(e,G) 
     if (!P.same_block(source(e),target(e))) L.append(e);

  KRUSKAL(L,cost,P,T,NT(0));

  return T;
}




/*******************************************************************************
 *
 *                          MIN_SPANNING_TREE1
 *
 * priority-queue-based algorithm  for undirected graph
 *
 * for details see Mehlhorn Vol. II, Section IV.8, Theorem 2
 *
 * Running time with Fibonnaci heap:  O(m + n * log n)   
 *                                    ( m * decrease_p + n * del_min)
 *
 * S.N. (1992)
 *
 ******************************************************************************/



template <class NT>
__temp_func_inline
list<edge> MIN_SPANNING_TREE1_T(const graph& G, const edge_array<NT>& cost)
{
  // computes a minimum spanning tree of the underlying undirected graph

  list<edge> result;

  node_pq<NT> PQ(G); 

  node_array<NT> dist(G,MAXINT); 
                                 // dist[v] = current distance value of v
                                 // MAXINT: no edge adjacent to v visited
                                 //-MAXINT: v already in a tree

  node_array<edge> T(G,nil);      // T[v] = e with cost[e] = dist[v]


  node v;
  forall_nodes(v,G)  PQ.insert(v,MAXINT);

  while (! PQ.empty())
  { 
    node u = PQ.del_min();

    if (dist[u] != MAXINT) result.append(T[u]);

    dist[u] = -MAXINT;

    edge e;
    forall_inout_edges(e,u)
    { v = G.opposite(u,e);
      NT c = cost[e];
      if (c < dist[v])
      { PQ.decrease_p(v,c);
        dist[v] = c;
        T[v] = e;
       }
     }
   }

 return result;
}

#if LEDA_ROOT_INCL_ID == 500361
#undef LEDA_ROOT_INCL_ID
#include <LEDA/internal/POSTAMBLE.h>
#endif


LEDA_END_NAMESPACE

