/*******************************************************************************
+
+  LEDA 5.0.1  
+
+
+  mwb_matching.t
+
+
+  Copyright (c) 1995-2005
+  by Algorithmic Solutions Software GmbH
+  All rights reserved.
+ 
*******************************************************************************/

// $Revision: 1.2 $  $Date: 2005/04/14 10:45:18 $


#if !defined(LEDA_ROOT_INCL_ID)
#define LEDA_ROOT_INCL_ID 500363
#include <LEDA/internal/PREAMBLE.h>
#endif

#include <LEDA/graph/mwb_matching.h>
#include <LEDA/graph/node_pq.h>
#include <LEDA/core/stack.h>
#include <LEDA/system/assert.h>

LEDA_BEGIN_NAMESPACE


// mod. 12/2000 by GS
bool PRUNE = true;


////////////// MAX WEIGHT BIPARTITE MATCHING

template <class NT>
__temp_func_inline
list<edge> MAX_WEIGHT_BIPARTITE_MATCHING_T(graph& G,
                               const edge_array<NT>& c)
{ node_array<NT> pot(G);
  list<edge> M = MAX_WEIGHT_BIPARTITE_MATCHING_T(G,c,pot);
#ifdef TEST
  assert(CHECK_MWBM_T(G,c,M,pot));
#endif
  return M;
}


template <class NT>
__temp_func_inline
list<edge> MAX_WEIGHT_BIPARTITE_MATCHING_T(graph& G,                                    
                      const list<node>& A, const list<node>& B,const edge_array<NT>& c)
{ node_array<NT> pot(G);
  list<edge> M = MAX_WEIGHT_BIPARTITE_MATCHING_T(G,A,B,c,pot);
#ifdef TEST
  assert(CHECK_MWBM_T(G,c,M,pot));
#endif
  return M;
}


template <class NT>
__temp_func_inline
list<edge> MAX_WEIGHT_BIPARTITE_MATCHING_T(graph& G,                      
          const edge_array<NT>& c,node_array<NT>& pot)
{
  list<node> A,B;
  node v; edge e;

  if ( !Is_Bipartite(G,A,B) )
    error_handler(1,"MAX_WEIGHT_BIPARTITE_MATCHING: \
G is not bipartite");

  edge_array<int> edge_number(G); int i = 0;
  forall_nodes(v,G) 
    forall_adj_edges(e,v) edge_number[e] = i++;
  
  list<edge> edges_out_of_B;
  forall(v,B) 
  { list<edge> outedges = G.adj_edges(v);
    edges_out_of_B.conc(outedges);
  }
  forall(e,edges_out_of_B) G.rev_edge(e);

  list<edge> result = MAX_WEIGHT_BIPARTITE_MATCHING_T(G,A,B,c,pot);
  
  forall(e,edges_out_of_B) G.rev_edge(e);

  G.sort_edges(edge_number);
#ifdef TEST
  assert(CHECK_MWBM_T(G,c,result,pot));
#endif

  return result;

}



////////////// MAX ASSIGNMENT

template <class NT>
__temp_func_inline
list<edge> MAX_WEIGHT_ASSIGNMENT_T(graph& G,
                               const edge_array<NT>& c)
{ node_array<NT> pot(G);
  list<edge> M = MAX_WEIGHT_ASSIGNMENT_T(G,c,pot);
  if ( M.length() == 0 ) return M;
#ifdef TEST
  assert(CHECK_MAX_WEIGHT_ASSIGNMENT_T(G,c,M,pot));
#endif
  return M;
}


template <class NT>
__temp_func_inline
list<edge> MAX_WEIGHT_ASSIGNMENT_T(graph& G,                                    
                      const list<node>& A, const list<node>& B,const edge_array<NT>& c)
{ node_array<NT> pot(G);
  list<edge> M = MAX_WEIGHT_ASSIGNMENT_T(G,A,B,c,pot);
  if ( M.length() == 0 ) return M;
#ifdef TEST
  assert(CHECK_MAX_WEIGHT_ASSIGNMENT_T(G,c,M,pot));
#endif
  return M;
}


template <class NT>
__temp_func_inline
list<edge> MAX_WEIGHT_ASSIGNMENT_T(graph& G,                      
          const edge_array<NT>& c,node_array<NT>& pot)
{
  list<node> A,B;
  node v; edge e;

  if ( !Is_Bipartite(G,A,B) )
    error_handler(1,"MAX_WEIGHT_ASSIGNMENT: \
G is not bipartite");

  edge_array<int> edge_number(G); int i = 0;
  forall_nodes(v,G) 
    forall_adj_edges(e,v) edge_number[e] = i++;
  
  list<edge> edges_out_of_B;
  forall(v,B) 
  { list<edge> outedges = G.adj_edges(v);
    edges_out_of_B.conc(outedges);
  }
  forall(e,edges_out_of_B) G.rev_edge(e);

  list<edge> result = MAX_WEIGHT_ASSIGNMENT_T(G,A,B,c,pot); 

  forall(e,edges_out_of_B) G.rev_edge(e);

  G.sort_edges(edge_number);

  if ( result.length() == 0 ) return result;
#ifdef TEST
  assert(CHECK_MAX_WEIGHT_ASSIGNMENT_T(G,c,result,pot));
#endif
  return result;

}

////////////// MIN ASSIGNMENT

template <class NT>
__temp_func_inline
list<edge> MIN_WEIGHT_ASSIGNMENT_T(graph& G,
                               const edge_array<NT>& c)
{ node_array<NT> pot(G);
  list<edge> M = MIN_WEIGHT_ASSIGNMENT_T(G,c,pot);
  if (M.length() == 0) return M;
#ifdef TEST
  assert(CHECK_MIN_WEIGHT_ASSIGNMENT_T(G,c,result,pot));
#endif
  return M;
}


template <class NT>
__temp_func_inline
list<edge> MIN_WEIGHT_ASSIGNMENT_T(graph& G,                                    
                      const list<node>& A, const list<node>& B,const edge_array<NT>& c)
{ node_array<NT> pot(G);
  list<edge> M = MIN_WEIGHT_ASSIGNMENT_T(G,A,B,c,pot);
  if (M.length() == 0) return M;
#ifdef TEST
  assert(CHECK_MIN_WEIGHT_ASSIGNMENT_T(G,c,result,pot));
#endif
  return M;
}


template <class NT>
__temp_func_inline
list<edge> MIN_WEIGHT_ASSIGNMENT_T(graph& G,                      
          const edge_array<NT>& c,node_array<NT>& pot)
{
  list<node> A,B;
  node v; edge e;

  if ( !Is_Bipartite(G,A,B) )
    error_handler(1,"MIN_WEIGHT_ASSIGNMENT: \
G is not bipartite");

  edge_array<int> edge_number(G); int i = 0;
  forall_nodes(v,G) 
    forall_adj_edges(e,v) edge_number[e] = i++;
  
  list<edge> edges_out_of_B;
  forall(v,B) 
  { list<edge> outedges = G.adj_edges(v);
    edges_out_of_B.conc(outedges);
  }
  forall(e,edges_out_of_B) G.rev_edge(e);

  list<edge> result = MIN_WEIGHT_ASSIGNMENT_T(G,A,B,c,pot); 

  forall(e,edges_out_of_B) G.rev_edge(e);

  G.sort_edges(edge_number);

  if (result.length() == 0) return result;
#ifdef TEST
  assert(CHECK_MIN_WEIGHT_ASSIGNMENT_T(G,c,result,pot));
#endif
  return result;

}

LEDA_END_NAMESPACE
#include <LEDA/system/misc.h>
LEDA_BEGIN_NAMESPACE

template <class NT>
__temp_func_inline
bool CHECK_MWBM_T(const graph& G, const edge_array<NT>& c,
                  const list<edge>& M, const node_array<NT>& pot)
{ node v; edge e;

  string s = "\nCHECK_MWBM_T: ";
  bool res = true;

  // M is a matching
  node_array<int> deg_in_M(G,0);
  forall(e,M) 
  { deg_in_M[G.source(e)]++;
    deg_in_M[G.target(e)]++;
  }
  forall_nodes(v,G) 
    res = res && leda_assert(deg_in_M[v] <= 1,s + "M is not a matching");

  // node potentials are non-negative
  forall_nodes(v,G) 
    res = res && leda_assert(pot[v] >= 0,s + "negative node potential");;

  // edges have non-negative reduced cost
  forall_edges(e,G)
  { node v = G.source(e); node w = G.target(e); 
    res = res && leda_assert(c[e] <= pot[v] + pot[w],s + "negative reduced cost");
  }

  // edges in M have reduced cost equal to zero
  forall(e,M)
  { node v = G.source(e); node w = G.target(e); 
    res = res && leda_assert(c[e] == pot[v] + pot[w],s + "non-tight matching edge");
  }

  // free nodes have potential equal to zero
  forall_nodes(v,G) 
    res = res && leda_assert((deg_in_M[v] != 0 || pot[v] == 0),
                        s + "free node with non-zero potential");

  return res;
}


/*
template <class NodeArray, class EdgeArray>
__temp_func_inline
bool CHECK_ASSIGNMENT_T(const graph& G, const EdgeArray& c,
        const list<edge>& M, const NodeArray& pot, bool max_cost)
*/

template <class NT>
__temp_func_inline
bool CHECK_ASSIGNMENT_T(const graph& G, const edge_array<NT>& c,
        const list<edge>& M, const node_array<NT>& pot, int max_cost)
{ node v; edge e;
  string s = "CHECK_ASSIGNMENT_T";
  if ( max_cost ) s += "(max): "; else s += "(min): ";
  bool res = true;

  node_array<int> deg_in_M(G,0);
  forall(e,M) 
  { deg_in_M[G.source(e)]++;
    deg_in_M[G.target(e)]++;
  }
  forall_nodes(v,G) res = res && leda_assert(deg_in_M[v] <= 1,s + "M is not a matching");
  forall_edges(e,G)
  { node v = G.source(e); node w = G.target(e); 
    if (max_cost)
      { res = res && leda_assert(c[e] <= pot[v] + pot[w],s + "illegal reduced cost"); }
    else
      { res = res && leda_assert(c[e] >= pot[v] + pot[w],s + "illegal reduced cost"); }
     
  }
  forall(e,M)
  { node v = G.source(e); node w = G.target(e); 
    res = res && leda_assert(c[e] == pot[v] + pot[w],s + "non_tight edge in M");
  }
  return res;
}



template <class NT>
__temp_func_inline
bool CHECK_MIN_WEIGHT_ASSIGNMENT_T(const graph& G, const edge_array<NT>& c,
        const list<edge>& M, const node_array<NT>& pot)
{ return CHECK_ASSIGNMENT_T(G, c, M, pot, 0); }

template <class NT>
__temp_func_inline
bool CHECK_MAX_WEIGHT_ASSIGNMENT_T(const graph& G, const edge_array<NT>& c,
        const list<edge>& M, const node_array<NT>& pot)
{ return CHECK_ASSIGNMENT_T(G, c, M, pot, 1); }






inline void augment_path_to(graph& G, node v, const node_array<edge>& pred)
{ edge e = pred[v];
  while (e)
  { G.rev_edge(e);
    e = pred[G.target(e)]; // not source (!!!)
  }
}


template <class NT>
inline void augment(graph& G, node a, const edge_array<NT>& c,
                node_array<NT>& pot, node_array<bool>& free, 
                node_array<edge>& pred, node_array<NT>& dist, 
                node_pq<NT>& PQ)
{ 
  dist[a] = 0;

  node best_node_in_A = a; 
  NT   minA           = pot[a];
  NT   Delta;

  // mod. 12/2000 by GS
  NT   upper_bound    = minA;

  stack<node> RA;  RA.push(a);
  stack<node> RB;

  node a1 = a; edge e;
  
  forall_adj_edges(e,a1)
  { node b = G.target(e);
    NT db = dist[a1] + (pot[a1] + pot[b] - c[e]); 

    // mod. 12/2000 by GS
    if (PRUNE && db >= upper_bound) continue;
    if (free[b]) upper_bound = db;

    if ( pred[b] == nil ) 
    { dist[b] = db; pred[b] = e; RB.push(b);  
      PQ.insert(b,db); 
    }
    else
    if ( db < dist[b] )
    { dist[b] = db; pred[b] = e;
      PQ.decrease_p(b,db); 
    }
  }



  while ( true )
  {     
    node b;
    NT db = 0;
    if (PQ.empty()) b = nil;
    else { b = PQ.del_min(); db = dist[b]; }
       

    
    if ( b == nil || db >= minA ) 
    { Delta = minA;
      
      augment_path_to(G,best_node_in_A,pred);  

      free[a] = false; free[best_node_in_A] = true; // order is important
      break;
 
    }
    else
    { if ( free[b] )
      { Delta = db;
        
        augment_path_to(G,b,pred);
        free[a] = free[b] = false;
        break;
 
      }
      else
      { 
        e = G.first_adj_edge(b); 
        node a1 = G.target(e);
        pred[a1] = e; RA.push(a1);
        dist[a1] = db; 
              
        if (db + pot[a1] < minA)
        { best_node_in_A = a1;
          minA = db + pot[a1];

          // mod. 12/2000 by GS
	  upper_bound = leda_min(upper_bound, minA);
        }

        
        forall_adj_edges(e,a1)
        { node b = G.target(e);
          NT db = dist[a1] + (pot[a1] + pot[b] - c[e]); 

          // mod. 12/2000 by GS
          if (PRUNE && db >= upper_bound) continue;
          if (free[b]) upper_bound = db;

          if ( pred[b] == nil ) 
          { dist[b] = db; pred[b] = e; RB.push(b);  
            PQ.insert(b,db); 
          }
          else
          if ( db < dist[b] )
          { dist[b] = db; pred[b] = e;
            PQ.decrease_p(b,db); 
          }
        }

 }
    }

  }

 
 while (!RA.empty() )
 { node a = RA.pop();
   pred[a] = nil;
   NT pot_change = Delta - dist[a];
   if (pot_change <= 0 ) continue;
   pot[a] = pot[a] - pot_change;
 }

 while (!RB.empty() )
 { node b = RB.pop();
   pred[b] = nil;
   if (PQ.member(b)) PQ.del(b);
   NT pot_change = Delta - dist[b];
   if (pot_change <= 0 ) continue;
   pot[b] = pot[b] + pot_change;
 }

}


template <class NT>
__temp_func_inline
void mwbm_heuristic(graph& G, const list<node>& A, 
                    const edge_array<NT>& c, node_array<NT>& pot,
                    node_array<bool>& free)
{ 
  node a, b;  edge e, e2, eb; 
  node_array<edge> sec_edge(G,nil);

  forall( a, A )                      
  { NT max2 = 0; NT max = 0; eb = e2 = nil;
    // compute edges with largest and second largest slack
    forall_adj_edges( e, a )           
    { NT we = c[e] - pot[target(e)];     
      if ( we >= max2 ) 
      { if( we >= max )
        { max2 = max;  e2 = eb;
          max = we;    eb = e;
        }
        else 
        { max2 = we;   e2 = e;
        }
      }
    }

    if( eb ) 
    { b = target(eb);
      if( free[b] )                    
      { // match eb and change pot[] to make slack of e2 zero
        sec_edge[a] = e2;                
        pot[a] = max2;
        pot[b] = max-max2;
        G.rev_edge(eb);
        free[a] = free[b] = false;
      }
      else                              
      { // try to augment matching along 
        // path of length 3 given by sec_edge[]
        pot[a] = max;                  
        e2 = G.first_adj_edge(b);
        e = sec_edge[target(e2)];
        if( e && G.outdeg(target(e)) == 0 ) 
        { free[a] = free[G.target(e)] = false;
          G.rev_edge(e); G.rev_edge(e2); G.rev_edge(eb);
        }
      }
    }
    else pot[a] = 0;
  }
}
  

static int which_heuristic = 2;

template <class NT>
__temp_func_inline
list<edge> MAX_WEIGHT_BIPARTITE_MATCHING_T(graph& G,
                          const list<node>& A, const list<node>& B,   
                          const edge_array<NT>& c, node_array<NT>& pot)
{ node a,b,v; edge e;

  list<edge> result;
  forall_nodes(v,G) pot[v] = 0;

  if (G.number_of_edges() == 0 ) return result;
  
  // check that all edges are directed from A to B
  forall(b,B) assert(G.outdeg(b) == 0);

  node_array<bool> free(G,true);

  node_array<edge> pred(G,nil);
  node_array<NT>   dist(G,0);  
  node_pq<NT>      PQ(G);

  switch (which_heuristic) 
  { case 0: { // naive heuristic 
             NT C = 0;
             forall_edges(e,G) if (c[e] > C) C = c[e];
             forall(a,A)       pot[a] = C;
             break;
           }

    case 1: { // simple heuristic
             
             forall(a,A) 
             { edge e_max = nil; NT C_max = 0;
               forall_adj_edges(e,a)
                 if (c[e] >  C_max) { e_max = e; C_max = c[e]; }
               pot[a] = C_max;  
               if ( e_max != nil && free[b = G.target(e_max)] )
               { G.rev_edge(e_max);
                 free[a] = free[b] = false;
               }
             }
                 
             break;
           }

    default: { // refined heuristic
             mwbm_heuristic( G, A, c, pot, free);
             break;
           }
  }

  forall(a,A)
    if (free[a]) augment(G,a,c,pot,free,pred,dist,PQ);

  forall(b,B)
    { forall_out_edges(e,b) result.append(e); }

  forall(e,result) G.rev_edge(e);

  return result;
}  



template <class NT>
__temp_func_inline
void check_invariants_max_weight_assignment(const graph& G, 
const edge_array<NT>& c,
        const node_array<NT>& pot, const list<node>& A,
        const list<node>& B, const node_array<edge>& pred,node a0)
{ node a,b,v; edge e;
  forall(b,B)
  { assert(G.outdeg(b) <= 1);  assert(pred[b] == nil);
    assert(pot[b] >= 0);
    assert(G.outdeg(b) == 1 || pot[b] == 0);
    e = G.first_adj_edge(b);
    if (e) 
    { a = G.target(e); 
      assert(pot[a] + pot[b] == c[e]);
    }
  }
  a = a0;
  while (a)
  { assert(G.indeg(a) <= 1);  assert(pred[a] == nil);
 
    forall_adj_edges(e,a)
    { b = G.target(e);
      assert(pot[a] + pot[b] >= c[e]);
    }
    a = G.pred_node(a);
  }
}



LEDA_END_NAMESPACE
#include <LEDA/core/stack.h>
LEDA_BEGIN_NAMESPACE

template <class NT>
__temp_func_inline
bool max_weight_assignment_augment(graph& G, 
                    node a, const edge_array<NT>& c,
                    node_array<NT>& pot, node_array<bool>& free, 
                    node_array<edge>& pred, node_array<NT>& dist, 
                    node_pq<NT>& PQ)
{ 
  dist[a] = 0;

  //node best_node_in_A = a; 
  NT   minA           = pot[a];
  NT   Delta;

  // mod. 12/2000 by GS
  NT   upper_bound = 0;
  bool upper_bound_is_defined = false;  // upper_bound = +\infty 

  stack<node> RA;  RA.push(a);
  stack<node> RB;

  node a1 = a; edge e;
  
  forall_adj_edges(e,a1)
  { node b = G.target(e);
    NT db = dist[a1] + (pot[a1] + pot[b] - c[e]); 

    // mod. 12/2000 by GS
    if (PRUNE && upper_bound_is_defined && db >= upper_bound) continue;
    if (free[b]) { upper_bound = db; upper_bound_is_defined = true; }

    if ( pred[b] == nil ) 
    { dist[b] = db; pred[b] = e; RB.push(b);  
      PQ.insert(b,db); 
    }
    else
    if ( db < dist[b] )
    { dist[b] = db; pred[b] = e;
      PQ.decrease_p(b,db); 
    }
  }



  while ( true )
  { node b; NT db;
    if (PQ.empty()) { return false; }
    else { b = PQ.del_min(); db = dist[b]; }

    if ( free[b] )
    { Delta = db;
      
      augment_path_to(G,b,pred);
      free[a] = free[b] = false;
      break;
 
    }
    else
    {
     e = G.first_adj_edge(b); 
     node a1 = G.target(e);
     pred[a1] = e; RA.push(a1);
     dist[a1] = db; 
           
     if (db + pot[a1] < minA)
     { //best_node_in_A = a1;
       minA = db + pot[a1];
     }

     
     forall_adj_edges(e,a1)
     { node b = G.target(e);
       NT db = dist[a1] + (pot[a1] + pot[b] - c[e]); 

       // mod. 12/2000 by GS
       if (PRUNE && upper_bound_is_defined && db >= upper_bound) continue;
       if (free[b]) { upper_bound = db; upper_bound_is_defined = true; }

       if ( pred[b] == nil ) 
       { dist[b] = db; pred[b] = e; RB.push(b);  
         PQ.insert(b,db); 
       }
       else
       if ( db < dist[b] )
       { dist[b] = db; pred[b] = e;
         PQ.decrease_p(b,db); 
       }
     }

 }
  }

 
 while (!RA.empty() )
 { node a = RA.pop();
   pred[a] = nil;
   NT pot_change = Delta - dist[a];
   if (pot_change <= 0 ) continue;
   pot[a] = pot[a] - pot_change;
 }

 while (!RB.empty() )
 { node b = RB.pop();
   pred[b] = nil;
   if (PQ.member(b)) PQ.del(b);
   NT pot_change = Delta - dist[b];
   if (pot_change <= 0 ) continue;
   pot[b] = pot[b] + pot_change;
 }


 return true;
}



template <class NT>
__temp_func_inline
list<edge> MAX_WEIGHT_ASSIGNMENT_T(graph& G, 
                          const list<node>& A, 
                          const list<node>& B,  
                          const edge_array<NT>& c,  
                          node_array<NT>& pot)
{ node a,b; edge e;
  
  // check that all edges are directed from A to B
  forall(b,B) assert(G.outdeg(b) == 0);

  node_array<bool> free(G,true);

  node_array<edge> pred(G,nil);
  node_array<NT>   dist(G,0);  
  node_pq<NT>      PQ(G);

  forall(b,B) pot[b] = 0;

  switch (which_heuristic) 
  { case 0: { // no heuristic 
             NT C = 0;
             forall_edges(e,G) if (c[e] > C) C = c[e];
             forall(a,A)       pot[a] = C;
             break;
           }

    case 1: { // simple heuristic
             
             forall(a,A) 
             { edge e_max = nil; NT C_max = 0;
               forall_adj_edges(e,a)
                 if (c[e] >  C_max) { e_max = e; C_max = c[e]; }
               pot[a] = C_max;  
               if ( e_max != nil && free[b = G.target(e_max)] )
               { G.rev_edge(e_max);
                 free[a] = free[b] = false;
               }
             }
                 
             break;
           }

    default: { // refined heuristic
             mwbm_heuristic( G, A, c, pot, free);
             break;
           }
  }

  list<edge> result;

  forall(a,A)
  { if (free[a] && 
      !max_weight_assignment_augment(G,a,c,pot,free,pred,dist,PQ))
    { // graph has no perfect matching
      forall(b,B) 
       forall_out_edges(e,b) G.rev_edge(e);
      list<edge> result;
      return result;  // return empty list
    }
  }

  // TODO

  //forall(b,B) assert(G.outdeg(b) == 1);
  //forall(a,A) assert(G.indeg(a) == 1);

  forall(b,B)
    forall_adj_edges(e,b) result.append(e);

  forall(e,result) G.rev_edge(e);

  //forall(b,B) assert(G.outdeg(b) == 0);

  return result;
}  



template <class NT>
__temp_func_inline
list<edge> MIN_WEIGHT_ASSIGNMENT_T(graph& G,                         
                        const list<node>& A, const list<node>& B, 
                        const edge_array<NT>& c, node_array<NT>& pot)
{ edge_array<NT> w(G);
  edge e;
  forall_edges(e,G) w[e] = - c[e];

  list<edge> M = MAX_WEIGHT_ASSIGNMENT_T(G,A,B,w,pot);
  node v;
  forall_nodes(v,G) pot[v] = -pot[v];
  return M;
}


template <class NT>
__temp_func_inline
list<edge> MWMCB_MATCHING_T(graph& G, 
                          const list<node>& A, const list<node>& B,
                          const edge_array<NT>& c, node_array<NT>& pot)
{ NT C = 0;
  edge e;
  forall_edges(e,G)
  { if (c[e] > C) C = c[e];
    if (-c[e] > C) C = -c[e];
  }
  int k = leda_max(A.size(),B.size());
  
  C = 1 + 2*k*C;
  edge_array<NT> c_L(G);
  forall_edges(e,G) c_L[e] = c[e] + C;
  list<edge> M = MAX_WEIGHT_BIPARTITE_MATCHING_T(G,A,B,c_L,pot);
#ifdef TEST
  leda_assert(CHECK_MWBM_T(G,c_L,M,pot),"check in MWMCB_MATCHING_T failed");
#endif
  return M;
}
                                    

template <class NT>
__temp_func_inline
list<edge> MWMCB_MATCHING_T(graph& G, 
                          const list<node>& A, 
                          const list<node>& B,
                          const edge_array<NT>& c)
{ node_array<NT> pot(G);
  list<edge> M = MWMCB_MATCHING_T(G, 
                           A,B,c,pot);
  return M;
}

#if LEDA_ROOT_INCL_ID == 500363
#undef LEDA_ROOT_INCL_ID
#include <LEDA/internal/POSTAMBLE.h>
#endif



LEDA_END_NAMESPACE


