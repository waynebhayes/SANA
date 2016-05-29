/*******************************************************************************
+
+  LEDA 5.0.1  
+
+
+  shortest_path.t
+
+
+  Copyright (c) 1995-2005
+  by Algorithmic Solutions Software GmbH
+  All rights reserved.
+ 
*******************************************************************************/

// $Revision: 1.3 $  $Date: 2005/04/14 10:45:18 $

#if !defined(LEDA_ROOT_INCL_ID)
#define LEDA_ROOT_INCL_ID 500367
#include <LEDA/internal/PREAMBLE.h>
#endif


#include <LEDA/graph/graph_alg.h>
#include <LEDA/core/array.h>
#include <LEDA/core/stack.h>
#include <LEDA/core/b_queue.h>
#include <LEDA/graph/node_pq.h>
#include <LEDA/system/assert.h>

#include <LEDA/graph/templates/dijkstra.h>
#include <LEDA/graph/templates/moore.h>

LEDA_BEGIN_NAMESPACE


template <class T>
inline ostream& operator<<(ostream& o,const node_array<T>&) { return o;}

template <class T>
inline istream& operator>>(istream& i,node_array<T>&) { return i;}


template<class NT>
__temp_func_inline
NT DIJKSTRA_T(const graph& G, node s, node t, 
              const edge_array<NT>& cost, 
              node_array<edge>& pred)
{ 
  
  array<node> terminal(2);
  terminal[0] = s; terminal[1] = t;

  array<node_pq<NT>* >  PQ(2);
  PQ[0] = new node_pq<NT>(G);
  PQ[1] = new node_pq<NT>(G);
  PQ[0]->insert(terminal[0],0); 
  PQ[1]->insert(terminal[1],0);
                                                                                 
  //array<node_array<NT> > dist(2);
  //array<node_array<edge> > Pred(2);

  node_array<NT>*   dist = new node_array<NT>[2];
  node_array<edge>* Pred = new node_array<edge>[2];

  dist[0] = dist[1] = node_array<NT>(G,0);
  Pred[0] = Pred[1] = node_array<edge>(G,nil); 

  bool D_equals_infinity = (s != t? true : false);
  NT D = 0;   


  while ( !PQ[0]->empty() || !PQ[1]->empty() )
  { for (int i = 0; i < 2; i++)
    { if ( PQ[i]->empty() ) continue;
      node u = PQ[i]->del_min();

      
      if ( (u == terminal[1-i] || Pred[1-i][u] != nil) && 
           !PQ[1-i]->member(u) && 
           dist[0][u] + dist[1][u] == D )
      { // have found shortest path from s to t.
        node z = u;
        while ( z != s ) z = G.source(pred[z] = Pred[0][z]);
              
        z = u;
              
        edge e; 
        while ( (e = Pred[1][z] ) != nil) 
        { pred[z = G.target(e)] = e; }
              
        delete PQ[0];
        delete PQ[1];
        delete[] dist;
        delete[] Pred;
        return D;
      }
                

      
      for ( edge e = (i == 0? G.first_adj_edge(u):
                              G.first_in_edge(u));
            e != nil; 
            e = (i == 0? G.adj_succ(e): G.in_succ(e)) )    
      { node v = (i == 0? G.target(e) : G.source(e) ); 
        NT c = dist[i][u] + cost[e]; 
        if ( Pred[i][v] == nil && v != terminal[i] ) 
           PQ[i]->insert(v,c); // first path to v   
        else if (c < dist[i][v]) PQ[i]->decrease_p(v,c); // better path
             else continue;
        dist[i][v] = c; 
        Pred[i][v] = e;
        
        if ( ( v == terminal[1-i] || Pred[1-i][v] != nil ) 
             // dist[1-i][v] is defined iff true
           && ( D_equals_infinity || dist[0][v] + dist[1][v] < D ))
        { D_equals_infinity = false;
          D = dist[0][v] + dist[1][v];
        }
      }
 
    }
  }

  // no path from s to t
  pred[t] = nil;

  delete PQ[0];
  delete PQ[1];

  delete[] dist;
  delete[] Pred;

  return D; // no path from s to t
}




#if defined(__hpuxcc__)
// no local enums in function templates
enum{ NEG_CYCLE = -2,  ATT_TO_CYCLE = -1, FINITE = 0, PLUS = 1, 
      CYCLE = 2, ON_CUR_PATH = 3, UNKNOWN = 4  };
#endif

template <class NT>
__temp_func_inline
node_array<int> CHECK_SP_T(const graph& G, node s, 
                           const edge_array<NT>& c, 
                           const node_array<NT>& dist,
                           const node_array<edge>& pred)
{ 
#if !defined(__hpuxcc__)
  enum{ NEG_CYCLE = -2,  ATT_TO_CYCLE = -1, FINITE = 0, PLUS = 1, 
        CYCLE = 2, ON_CUR_PATH = 3, UNKNOWN = 4  };
#endif

  node_array<int> label(G,UNKNOWN);
  node_array<bool> reachable(G,false);
  DFS(G,s,reachable);

  node v;
  forall_nodes(v,G)
  { if (v != s)
    { assert( (pred[v] == nil) == (reachable[v] == false));
      if (reachable[v] == false) label[v] = PLUS;
    }
  }

  
  if (pred[s] == nil) label[s] = FINITE;
  forall_nodes(v,G)
  { if ( label[v] == UNKNOWN )
    { stack<node> S;
      node w = v;
      while ( label[w] == UNKNOWN )
      { label[w] = ON_CUR_PATH;
        S.push(w);
        w = G.source(pred[w]);
      }
      
      int t = label[w];
      if ( t == ON_CUR_PATH )
      { node z;
        do { z = S.pop();
             label[z] = CYCLE; 
           }
        while ( z != w );
        while ( !S.empty() ) label[S.pop()] = ATT_TO_CYCLE;
      }
      else // t is CYCLE, ATT_TO_CYCLE, or FINITE
      { if ( t == CYCLE ) t = ATT_TO_CYCLE;
        while ( !S.empty() ) label[S.pop()] = t;  
      }

    }
  }

  /*
  forall_nodes(v,G)
  { node w = v;
    if (v == s && pred[s] == nil) 
      {assert(label[s] == FINITE);
        continue;
      }
    if (pred[v] == nil) 
    { assert(label[v] = PLUS);
      continue;
    }
    bool reached= false;
    int n = G.number_of_nodes();
    for(int i = 0; i<n; i++)
        { if ( pred[w] != nil) 
          { w = G.source(pred[w]);
            if (w == v) reached = true;
          }
        }
    if (pred[w] == nil) assert(label[v] == FINITE);
    else { if (reached == true) assert(label[v] == CYCLE);
          else assert(label[v] == ATT_TO_CYCLE);
    }
  }
  */

  

  forall_nodes(v,G)
  { if ( label[v] == CYCLE )
    { node w = v;
      NT cycle_length = 0; 
      do
      { cycle_length += c[pred[w]];
        label[w] = NEG_CYCLE;
        w = G.source(pred[w]);
      } while (w != v);
      assert(cycle_length < 0);
    }
  }

  
  if ( label[s] == FINITE ) assert(dist[s] == 0);

  edge e;
  forall_edges(e,G)
  { node v = G.source(e);
    node w = G.target(e);
    if ( label[w] == FINITE ) 
    { assert( label[v] == FINITE || label[v] == PLUS);
      if ( label[v] == FINITE )
      { assert( dist[v] + c[e] >= dist[w] );
        if ( e == pred[w] ) assert( dist[v] + c[e] == dist[w] );
      }
    }
  }

  return label;
}
 


template <class NT>
__temp_func_inline
void ACYCLIC_SHORTEST_PATH_T(const graph& G, node s, 
                             const edge_array<NT>& c, 
                             node_array<NT>& dist,
                             node_array<edge>& pred)
{ 
  
  node_array<int> top_ord(G); 
  TOPSORT(G,top_ord); 
                // top_ord is now a topological ordering of G

  int n = G.number_of_nodes(); 

  array<node> v(1,n); 

  node w; 
  forall_nodes(w,G) v[top_ord[w]] = w; 
                               // top_ord[v[i]] == i for all i 

 
  forall_nodes(w,G) pred[w] = nil; 
  dist[s] = 0; 

  for(int i = 1; i <= n; i++)
  { node u = v[i];
    if ( pred[u] != nil || u == s )   
    { edge e;
      NT du = dist[u];
      forall_adj_edges(e,u)
      { node w = G.target(e); 
        if ( pred[w] == nil || du + c[e] < dist[w]) 
        { pred[w] = e; 
          dist[w] = du + c[e]; 
        }
      }
    }
  }
}
 




template <class tt>
__temp_func_inline
void Update_pred(const graph& G, node v, 
                 const node_array<bool>& in_R,
                 node_array<bool>& reached_from_Q,                 
                 node_array<edge>& pred, tt)
{ reached_from_Q[v] = true;
  edge e;
  forall_adj_edges(e,v)
    { node w = G.target(e);
      if ( !reached_from_Q[w] )
        { if ( in_R[w] ) pred[w] = e;
            Update_pred(G,w,in_R,reached_from_Q,pred,0);
        }
    }
}
  

template <class NT>
__temp_func_inline
bool BELLMAN_FORD_B_T(const graph& G, node s, 
                         const edge_array<NT>& c, 
                         node_array<NT>& dist, 
                         node_array<edge>& pred ) 
{ int n = G.number_of_nodes();
  int phase_count = 0;

  b_queue<node> Q(n+1);
  node_array<bool> in_Q(G,false);
  node u,v;
  edge e;

  forall_nodes(v,G) pred[v] = 0;

  dist[s] = 0;
  Q.append(s); in_Q[s] = true;
  Q.append((node) nil); // end marker

  while( phase_count < n )
  { u = Q.pop();
    if ( u == nil) 
    { phase_count++;
      if ( Q.empty() ) return true;
      Q.append((node) nil);
      continue;
    }
    else in_Q[u] = false;

    NT du = dist[u];

    forall_adj_edges(e,u) 
    { v = G.opposite(u,e);    
      NT d = du + c[e];
      if ( (pred[v] == nil && v != s) || d < dist[v] )
      { dist[v] = d; pred[v] = e;

        if ( !in_Q[v] ) { Q.append(v); in_Q[v] = true; }
      }
    } 
  }
  
  if (pred[s] != nil) return false;

  node_array<bool> in_R(G,false);

  forall_edges(e,G)
    if (e != pred[G.target(e)]) ((graph*) &G)->hide_edge(e);
   
  DFS(G,s,in_R);

  ((graph*) &G)->restore_all_edges();

  node_array<bool> reached_from_Q(G,false);

  forall_nodes(v,G)
    if (in_Q[v] && !reached_from_Q[v]) 
      Update_pred(G,v,in_R,reached_from_Q,pred,0);
 
  return false;
}
 



template <class tt>
__temp_func_inline 
list_item BF_delete_subtree(list_item w_item, list<node>& Q, 
          list<node>& T, node_array<int>& t_degree, 
          node_array<list_item>& pos_in_Q,
          node_array<list_item>& pos_in_T, tt)
{ list_item child = T.succ(w_item);
  node w = T[w_item];
  while (t_degree[w] > 0)
  { t_degree[w]--;       
    child = BF_delete_subtree(child,Q,T,t_degree,
                                pos_in_Q,pos_in_T,0);
  }
  pos_in_T[w] = nil; 
  T.del_item(w_item);
  if ( pos_in_Q[w] ) 
  { Q.del_item(pos_in_Q[w]); 
    pos_in_Q[w] = nil;
  }
  return child;
}


template <class tt>
__temp_func_inline 
void BF_add_to_Vm(const graph& G, node z, 
                  node_array<bool>& in_Vm,                 
                  node_array<edge>& pred,
                  list<node>& Q, list<node>& T,
                  node_array<int>& t_degree,
                  node_array<list_item>& pos_in_Q,
                  node_array<list_item>& pos_in_T, tt)
{ edge e;
  forall_adj_edges(e,z)
    { node w = G.target(e);
      if ( !in_Vm[w] )
        { if (pos_in_T[w]) 
          { BF_delete_subtree(pos_in_T[w],Q,T,t_degree,
                                      pos_in_Q,pos_in_T, 0);
            if (pred[w] != nil) t_degree[G.source(pred[w])]--;
          }
          pred[w] = e;
          in_Vm[w] = true;
          BF_add_to_Vm(G,w,in_Vm,pred,
                         Q,T,t_degree,pos_in_Q,pos_in_T,0);
        }
    }
}


template <class NT>
__temp_func_inline
bool BELLMAN_FORD_T(const graph& G, node s, 
                       const edge_array<NT> & c,
                       node_array<NT> & dist,
                       node_array<edge>& pred)
{ 
  node_array<list_item> pos_in_Q(G,nil);
  node_array<int>       t_degree(G,0);
  node_array<list_item> pos_in_T(G,nil);

  node v;
  forall_nodes(v,G) pred[v] = nil; 
  dist[s] = 0;

  list<node> Q;  pos_in_Q[s] = Q.append(s); 
  list<node> T;  pos_in_T[s] = T.append(s);


  node_array<bool> in_Vm(G,false); // for V_minus
  bool no_negative_cycle = true;

  while (!Q.empty())
  { // select a node v from Q
   
    node v = Q.pop(); pos_in_Q[v] = nil;
   
    edge e;
    forall_adj_edges(e,v)
    { node w = G.target(e);
      if ( in_Vm[w] ) continue;
      NT d = dist[v] + c[e];
      if ( ( pred[w] == nil && w != s ) || d < dist[w])
      { dist[w] = d;
        // remove the subtree rooted at w from T and Q
        // if w has a parent, decrease its degree

        if (pos_in_T[w]) 
        { BF_delete_subtree(pos_in_T[w],Q,T,t_degree,
                                    pos_in_Q,pos_in_T,0);
          if (pred[w] != nil) t_degree[G.source(pred[w])]--;
        }
 
        pred[w] = e;

        if (pos_in_T[v] == nil) 
        { no_negative_cycle = false;
          
          node z = v;
          do 
          { in_Vm[z] = true; 
            z = G.source(pred[z]);
          } while (z != v);
          do 
          { BF_add_to_Vm(G,z,in_Vm,pred,Q,T,t_degree,pos_in_Q,pos_in_T,0);
            z = G.source(pred[z]);
          } while (z != v);
 
        }
        else
        { // make w a child of v and add w to Q
          pos_in_T[w] = T.insert(w,pos_in_T[v],leda::behind);
          t_degree[v]++;
          pos_in_Q[w] = Q.append(w);
        }
      }
    }
  }
#ifndef LEDA_CHECKING_OFF
CHECK_SP_T(G,s,c,dist,pred);
#endif
  return no_negative_cycle;
}
 



template <class NT>
__temp_func_inline
bool ALL_PAIRS_SHORTEST_PATHS_T(graph&G, 
                                const edge_array<NT>& c, 
                                node_matrix<NT>& DIST)
{ edge e;
  node v,w;

  node s = G.new_node();
  list<edge> aux_edges;
  forall_nodes(v,G) 
  { if (v == s) continue;
    aux_edges.push(G.new_edge(s,v));
   }

  edge_array<NT>    c1(G);
  forall_edges(e,G) c1[e] = (G.source(e) == s? 0 : c[e]);
  node_array<NT>    dist1(G);
  node_array<edge>  pred(G);

  if (!BELLMAN_FORD_T(G,s,c1,dist1,pred)) 
	{ G.del_node(s);
	  return false; }

  forall(e,aux_edges) G.del_edge(e);
  G.del_node(s);

  forall_edges(e,G) 
    c1[e] = dist1[source(e)] + c[e] - dist1[target(e)];

  // (G,c1) is a non-negative network; for every node v 
  // compute row DIST[v] of the distance matrix DIST 
  // by a call of DIJKSTRA_T(G,v,c1,DIST[v])

  forall_nodes(v,G) DIJKSTRA_T(G,v,c1,DIST[v],pred);

  // correct the entries of DIST

  forall_nodes(v,G)
  { NT dv = dist1[v];
    forall_nodes(w,G) DIST(v,w) += (dist1[w] - dv);
  }

 return true;
}




template <class NT>
__temp_func_inline
bool SHORTEST_PATH_T(const graph& G, node s, 
                     const edge_array<NT>& c, 
                     node_array<NT>& dist, 
                     node_array<edge>& pred ) 
{ 
  if ( Is_Acyclic(G) )  
  { ACYCLIC_SHORTEST_PATH_T(G,s,c,dist,pred); 
    return true; 
  }

  bool non_negative = true; 

  edge e; 

  forall_edges(e,G) 
    if (c[e] < 0) non_negative = false; 

  if (non_negative) 
  { DIJKSTRA_T(G,s,c,dist,pred); 
    return true; 
  }
 
  return BELLMAN_FORD_T(G,s,c,dist,pred); 
}


#if LEDA_ROOT_INCL_ID == 500367
#undef LEDA_ROOT_INCL_ID
#include <LEDA/internal/POSTAMBLE.h>

LEDA_END_NAMESPACE

#endif

