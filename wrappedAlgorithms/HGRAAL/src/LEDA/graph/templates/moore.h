/*******************************************************************************
+
+  LEDA 5.0.1  
+
+
+  moore.t
+
+
+  Copyright (c) 1995-2005
+  by Algorithmic Solutions Software GmbH
+  All rights reserved.
+ 
*******************************************************************************/

// $Revision: 1.2 $  $Date: 2005/04/14 10:45:17 $

#if !defined(LEDA_ROOT_INCL_ID)
#define LEDA_ROOT_INCL_ID 500362
#include <LEDA/internal/PREAMBLE.h>
#endif

//------------------------------------------------------------------------------
// MOORE  (single source shortest paths)
//
// last modified: 1997)
//------------------------------------------------------------------------------
 
#include <LEDA/graph/graph_alg.h>

LEDA_BEGIN_NAMESPACE

template <class NT>
__temp_func_inline
void  MOORE_T(const graph& g, node s, const edge_array<NT>& cost,
                                            node_array<NT>& dist, node t) 
{
  // use a double ended queue of candidate nodes (node_list)
  // and a node_array<NT> for dist values
  // stop when t reached (if t != nil)

  int max_num;
  Max_Value(max_num);

  //dist.init(g,max_num);

  node v;
  forall_nodes(v,g) dist[v] = max_num;
  dist[s] = 0;

  node_list labeled;  // deque of candidate nodes
  labeled.append(s);

  while (! labeled.empty()) 
  { 
    node v = labeled.pop();
    NT dv = dist[v];

    if (t && dv > dist[t]) continue;

    edge e;
    forall_adj_edges(e,v)
    { node w = g.opposite(v,e);
      NT d = dv + cost[e];
      if (d < dist[w]) 
      { if ( ! labeled(w) ) 
        { if (dist[w] == max_num)
	       labeled.append(w);
	    else
	       labeled.push(w);
	   }
	  dist[w] = d;
       }
     }
  }
}

#if LEDA_ROOT_INCL_ID == 500362
#undef LEDA_ROOT_INCL_ID
#include <LEDA/internal/POSTAMBLE.h>
#endif

LEDA_END_NAMESPACE

