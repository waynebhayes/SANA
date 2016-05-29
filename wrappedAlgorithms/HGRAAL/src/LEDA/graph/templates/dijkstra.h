/*******************************************************************************
+
+  LEDA 5.0.1  
+
+
+  dijkstra.t
+
+
+  Copyright (c) 1995-2005
+  by Algorithmic Solutions Software GmbH
+  All rights reserved.
+ 
*******************************************************************************/

// $Revision: 1.3 $  $Date: 2005/04/14 10:45:14 $

#include <LEDA/graph/graph.h>
#include <LEDA/graph/node_pq22.h>

#include <limits>


LEDA_BEGIN_NAMESPACE

template <class NT, class graph_t   = graph,
                    class node_pq_t = node_pq22<NT,graph_t> >
class dijkstra
{
  typedef typename graph_t::node node;
  typedef typename graph_t::edge edge;
  
  float T;

  node opposite(const graph_t& G, edge e, node s)
  { if (graph::category() == bidirectional_graph_category || 
        graph::category() == directed_graph_category)
      return G.target(e);
    else
      return G.opposite(e,s);
  }
  
  public:
        
  template <class cost_array, class dist_array, class pred_array>
  void run(const graph_t& G, node s, node t, const cost_array& cost,
                                             dist_array& dist,
                                             pred_array& pred)
  {
    T = used_time();

    int n = G.number_of_nodes();
    int m = G.number_of_edges();

    node_pq_t PQ(n+m); 

#if defined(LEDA_STD_HEADERS) || defined(__mipspro__) || defined(_MSC_VER) || defined(__DMC__)
    NT max_dist = std::numeric_limits<NT>::max();
#else
    NT max_dist = numeric_limits<NT>::max();
#endif

    node v;
    forall_nodes(v,G) 
    { dist[v] = max_dist;
      pred[v] = 0;
     }

    dist[s] = 0;
    PQ.insert(s,0);

    while (!PQ.empty())
    { NT du;
      node u = PQ.del_min(du,dist);  

      if (du != dist[u]) continue;

      if (t && du >= dist[t]) continue;
  
      edge e;
      forall_adj_edges(e,u)
      { node v = opposite(G,e,u);
        NT c = du + cost[e]; 
        if (c < dist[v]) 
        { PQ.insert(v,c); 
          dist[v] = c; 
          pred[v] = e;                           
         }
      }                                                                  
    }
    
    T = used_time(T);
  }  


        
  template <class cost_array, class dist_array>
  void run(const graph_t& G, node s, const cost_array& cost, dist_array& dist)
  {
    T = used_time();

    int n = G.number_of_nodes();
    int m = G.number_of_edges();

    node_pq_t PQ(n+m); 

#if defined(LEDA_STD_HEADERS) || defined(__mipspro__) || defined(_MSC_VER) || defined(__DMC__)
    NT max_dist = std::numeric_limits<NT>::max();
#else
    NT max_dist = numeric_limits<NT>::max();
#endif
    
    node v;
    forall_nodes(v,G) dist[v] = max_dist;

    dist[s] = 0;
    PQ.insert(s,0);

    while (!PQ.empty())
    { NT du;
      node u = PQ.del_min(du,dist);  

      if (du != dist[u]) continue;
  
      edge e;
      forall_adj_edges(e,u)
      { node v = opposite(G,e,u);
        NT c = du + cost[e]; 
        if (c < dist[v]) 
        { PQ.insert(v,c); 
          dist[v] = c; 
         }
      }                                                                  
    }
    
    T = used_time(T);
  }  


  template <class cost_array, class dist_array, class pred_array>
  void run(const graph_t& G, node s, const cost_array& cost,
                                     dist_array& dist,
                                     pred_array& pred)
  { run(G,s,0,cost,dist,pred); }



  
  template <class dist_array, class pred_array>
  double sum_of_distances(const graph_t& G, dist_array& dist, 
                                            pred_array& pred)
  {
    double D = 0;
   
    node v;
    forall_nodes(v,G)
    {
      edge e;
      forall_adj_edges(e,v)
      {
        node w = opposite(G,e,v);
          
        if (e == pred[w]) D += dist[w]; 
      }
    }
    
    return D;    
  }



  template <class cost_array, class dist_array, class pred_array>
  bool check(const graph_t& G, node s, const cost_array& cost,
                                       const dist_array& dist,
                                       const pred_array& pred,
                                       string& msg)
  { return true; }

  float cpu_time() const { return T; }                                       

};

template <class NT>
__temp_func_inline
NT DIJKSTRA_T(const graph& G, node s, node t,
                                         const edge_array<NT>& cost,
                                         node_array<NT>& dist, 
                                         node_array<edge>& pred)
{ dijkstra<NT,graph> D;
  D.run(G,s,t,cost,dist,pred);
  return dist[t];
}



template <class NT>
__temp_func_inline
void DIJKSTRA_T(const graph& G, node s, const edge_array<NT>& cost,
                                         node_array<NT>& dist, 
                                         node_array<edge>& pred)
{ dijkstra<NT,graph> D;
  D.run(G,s,cost,dist,pred);
}


template <class NT>
__temp_func_inline
void DIJKSTRA_T(const graph& G, node s, const edge_array<NT>& cost,
                                         node_array<NT>& dist)
{ dijkstra<NT,graph> D;
  D.run(G,s,cost,dist);
  
}


LEDA_END_NAMESPACE
