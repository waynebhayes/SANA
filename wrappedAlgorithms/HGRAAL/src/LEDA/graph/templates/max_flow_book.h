/*******************************************************************************
+
+  LEDA 5.0.1  
+
+
+  max_flow_book.t
+
+
+  Copyright (c) 1995-2005
+  by Algorithmic Solutions Software GmbH
+  All rights reserved.
+ 
*******************************************************************************/

// $Revision: 1.2 $  $Date: 2005/04/14 10:45:16 $


#if !defined(LEDA_ROOT_INCL_ID)
#define LEDA_ROOT_INCL_ID 500341
#include <LEDA/internal/PREAMBLE.h>
#endif

#include <LEDA/graph/max_flow.h>
#include <LEDA/core/queue.h>
#include <LEDA/core/string.h>
#include <LEDA/system/assert.h>


#include <LEDA/core/b_queue.h>
#include <LEDA/core/array.h>
#include <LEDA/core/list.h>

LEDA_BEGIN_NAMESPACE

class fifo_set{

  list<node> L;

public:

  fifo_set(){}

  node del() { if (!L.empty()) return L.pop(); else return nil; }
 
  void insert(node v, int)  { L.append(v); }
  void insert0(node v, int) { L.append(v); }

  bool empty() { return L.empty(); }

  void clear() { L.clear(); }

  ~fifo_set(){}
};

class mfifo_set{

  list<node> L;

public:

  mfifo_set(){}

  node del() { if ( !L.empty() ) return L.pop(); else return nil; }
 
  void insert(node v, int) { L.push(v); }
  void insert0(node v, int){ L.append(v); }

  bool empty() { return L.empty(); }

  void clear() { L.clear(); }

  ~mfifo_set(){}
};


class hl_set{

  int max, max_lev;
  array<list<node> > A;

public:

  hl_set(int max_level):A(max_level+1)
  { max = -1; max_lev = max_level;}

  node del()
  { while (max >= 0 && A[max].empty()) max--;
    if (max >= 0) return A[max].pop(); else return nil; 
  }
 
  void insert(node v, int d)
  { A[d].push(v);
    if (d > max) max = d;
  }

  void insert0(node v, int d) { A[d].append(v); }

  bool empty()
  { while (max >= 0 && A[max].empty()) max--;
    return ( max < 0 ); 
  }

  ~hl_set(){}

  void clear()
  { for (int i = 0; i <= max_lev; i++) A[i].clear();  
    max = -1;
  }
};



template <class NT>
__temp_func_inline
bool CHECK_MAX_FLOW_T(const graph& G, node s, node t,
                      const edge_array<NT>& cap, const edge_array<NT>& f)
{ node v; edge e;
  string loc = "CHECK_MAX_FLOW_T: ";
  bool res = true;

  forall_edges(e,G) 
    res = res && leda_assert(f[e] >= 0 && f[e] <= cap[e], 
              loc + string("illegal flow value for edge %d",index(e)),1);
  
  node_array<NT> excess(G,0);
  forall_edges(e,G) 
  { node v = G.source(e); node w = G.target(e);
    excess[v] -= f[e]; excess[w] += f[e];
  }
  forall_nodes(v,G)
    res = res && leda_assert(v == s || v == t || excess[v] == 0,
                 loc + string("node %d has non-zero excess",index(v)),1);
  
  node_array<bool> reached(G,false);
  queue<node> Q;
  
  Q.append(s); reached[s] = true;
  while ( !Q.empty() )
  { node v = Q.pop(); 
    forall_out_edges(e,v) 
    { node w = G.target(e);
      if ( f[e] < cap[e] && !reached[w] ) 
      { reached[w] = true; Q.append(w); }
    }
    forall_in_edges(e,v) 
    { node w = G.source(e);
      if ( f[e] > 0 && !reached[w] ) 
      { reached[w] = true; Q.append(w); }
    }
  }
  res = res && leda_assert(!reached[t],"t is reachable in G_f",1);
  return res;
}
  


template<class NT, class SET>
__temp_func_inline
NT MAX_FLOW_BASIC_T(const graph& G, node s, node t, 
                    const edge_array<NT>& cap, edge_array<NT>& flow,
                    SET& U,
                    int& num_pushes, int& num_edge_inspections, 
                    int& num_relabels)
{ if (s == t) error_handler(1,"MAXFLOW: source == sink");

  
  
  flow.init(G,0);
  //if (G.outdeg(s) == 0) return 0;
  if (G.first_adj_edge(s) == nil) return 0;

  int n = G.number_of_nodes(); int max_level = 2*n - 1;
  int m = G.number_of_edges(); 

  node_array<NT>  excess(G,0);

  // saturate all edges leaving s

  edge e;
  forall_out_edges(e,s) 
  { NT c = cap[e];
    if (c == 0) continue;
    node v = target(e);
    flow[e] = c;
    excess[s] -= c;
    excess[v] += c;
   } 


  
  node_array<int> dist(G,0); dist[s] = n;
  node v;
  forall_nodes(v,G) 
   if ( excess[v] > 0 ) U.insert(v,dist[v]);


  
  num_relabels = num_pushes = num_edge_inspections = 0;



  
  for(;;) 
  {
    node v = U.del();
   
    if (v == nil) break;
    if (v == t) continue;

    NT  ev = excess[v]; // excess of v 
    int dv = dist[v];   // level of v
    edge e;

    
    for (e = G.first_adj_edge(v); e; e = G.adj_succ(e))
    { num_edge_inspections++;
      NT& fe = flow[e];
      NT  rc = cap[e] - fe;
      if (rc == 0) continue;
      node w = target(e);
      int dw = dist[w];
      if ( dw < dv ) // equivalent to ( dw == dv - 1 )
      { num_pushes++;
        NT& ew = excess[w];
        if (ew == 0) U.insert0(w,dw); 
        if (ev <= rc) 
        { ew += ev; fe += ev;
          ev = 0;   // stop: excess[v] exhausted
          break;
        }
        else
        { ew += rc; fe += rc;
          ev -= rc;
        }
      }
    }

    if ( ev > 0 )
    { 
      for (e = G.first_in_edge(v); e; e = G.in_succ(e))
      { num_edge_inspections++;
        NT& fe = flow[e];
        if (fe == 0) continue;
        node w = source(e);
        int dw = dist[w];
        if ( dw < dv ) // equivalent to ( dw == dv - 1 )
        { num_pushes++;
          NT& ew = excess[w];
          if (ew == 0) U.insert0(w,dw); 
          if (ev <= fe) 
          { fe -= ev; ew += ev;
            ev = 0;   // stop: excess[v] exhausted
            break;
          }
          else
          { ew += fe; ev -= fe;
            fe = 0;
          }
        }
      }
 }

    excess[v] = ev;

    if (ev > 0) 
    { dist[v]++; 
      num_relabels++;
      U.insert(v,dist[v]);
    }
  } 
    

#ifdef TEST
  assert(CHECK_MAX_FLOW_T(G,s,t,cap,flow));
#endif

  return excess[t];
}



template<class NT, class SET>
__temp_func_inline
NT MAX_FLOW_LH_T(const graph& G, node s, node t, 
                  const edge_array<NT>& cap, 
                  edge_array<NT>& flow,
                  SET& U,
                  int& num_pushes, 
                  int& num_edge_inspections,
                  int& num_relabels)
{ if (s == t) error_handler(1,"MAXFLOW: source == sink");

  
  
  flow.init(G,0);
  //if (G.outdeg(s) == 0) return 0;
  if (G.first_adj_edge(s) == nil) return 0;

  int n = G.number_of_nodes(); int max_level = 2*n - 1;
  int m = G.number_of_edges(); 

  node_array<NT>  excess(G,0);

  // saturate all edges leaving s

  edge e;
  forall_out_edges(e,s) 
  { NT c = cap[e];
    if (c == 0) continue;
    node v = target(e);
    flow[e] = c;
    excess[s] -= c;
    excess[v] += c;
   } 


  
  node_array<int> dist(G,0); dist[s] = n;
  node v;
  forall_nodes(v,G) 
   if ( excess[v] > 0 ) U.insert(v,dist[v]);


  
  num_relabels = num_pushes = num_edge_inspections = 0;



  
  for(;;) 
  {
    node v = U.del();
   
    if (v == nil) break;
    if (v == t) continue;

    NT  ev = excess[v]; // excess of v 
    int dv = dist[v];   // level of v
    edge e;

    if ( dist[v] < n )
    { 
      for (e = G.first_adj_edge(v); e; e = G.adj_succ(e))
      { num_edge_inspections++;
        NT& fe = flow[e];
        NT  rc = cap[e] - fe;
        if (rc == 0) continue;
        node w = target(e);
        int dw = dist[w];
        if ( dw < dv ) // equivalent to ( dw == dv - 1 )
        { num_pushes++;
          NT& ew = excess[w];
          if (ew == 0) U.insert0(w,dw); 
          if (ev <= rc) 
          { ew += ev; fe += ev;
            ev = 0;   // stop: excess[v] exhausted
            break;
          }
          else
          { ew += rc; fe += rc;
            ev -= rc;
          }
        }
      }
 }
    if ( ev > 0 )
    { 
      for (e = G.first_in_edge(v); e; e = G.in_succ(e))
      { num_edge_inspections++;
        NT& fe = flow[e];
        if (fe == 0) continue;
        node w = source(e);
        int dw = dist[w];
        if ( dw < dv ) // equivalent to ( dw == dv - 1 )
        { num_pushes++;
          NT& ew = excess[w];
          if (ew == 0) U.insert0(w,dw); 
          if (ev <= fe) 
          { fe -= ev; ew += ev;
            ev = 0;   // stop: excess[v] exhausted
            break;
          }
          else
          { ew += fe; ev -= fe;
            fe = 0;
          }
        }
      }
 }

    excess[v] = ev;

    if (ev > 0) 
    { dist[v]++; 
      num_relabels++;
      U.insert(v,dist[v]);
    }
  }

#ifdef TEST
  assert(CHECK_MAX_FLOW_T(G,s,t,cap,flow));
#endif

  return excess[t];
}



template<class NT, class SET>
__temp_func_inline
NT MAX_FLOW_LRH_T(const graph& G, node s, node t, 
                  const edge_array<NT>& cap, 
                  edge_array<NT>& flow,
                  SET& U,
                  int& num_pushes, 
                  int& num_edge_inspections,
                  int& num_relabels)
{ if (s == t) error_handler(1,"MAXFLOW: source == sink");

  
  
  flow.init(G,0);
  //if (G.outdeg(s) == 0) return 0;
  if (G.first_adj_edge(s) == nil) return 0;

  int n = G.number_of_nodes(); int max_level = 2*n - 1;
  int m = G.number_of_edges(); 

  node_array<NT>  excess(G,0);

  // saturate all edges leaving s

  edge e;
  forall_out_edges(e,s) 
  { NT c = cap[e];
    if (c == 0) continue;
    node v = target(e);
    flow[e] = c;
    excess[s] -= c;
    excess[v] += c;
   } 


  
  node_array<int> dist(G,0); dist[s] = n;
  node v;
  forall_nodes(v,G) 
   if ( excess[v] > 0 ) U.insert(v,dist[v]);


  
  num_relabels = num_pushes = num_edge_inspections = 0;



  
  for(;;) 
  {
    node v = U.del();
   
    if (v == nil) break;
    if (v == t) continue;

    NT  ev   = excess[v]; // excess of v 
    int dv   = dist[v];   // level of v
    int dmin = MAXINT;     // for local relabeling heuristic
    edge e;

    if (dv < n)
    { 
      for (e = G.first_adj_edge(v); e; e = G.adj_succ(e))
      { num_edge_inspections++;
        NT& fe = flow[e];
        NT  rc = cap[e] - fe;
        if (rc == 0) continue;
        node w = target(e);
        int dw = dist[w];
        if ( dw < dv ) // equivalent to ( dw == dv - 1 )
        { num_pushes++;
          NT& ew = excess[w];
          if (ew == 0) U.insert0(w,dw); 
          if (ev <= rc) 
          { ew += ev; fe += ev;
            ev = 0;   // stop: excess[v] exhausted
            break;
          }
          else
          { ew += rc; fe += rc;
            ev -= rc;
          }
        }
        else { if ( dw < dmin ) dmin = dw; }
      }
 }
    if ( ev > 0 )
    { 
      for (e = G.first_in_edge(v); e; e = G.in_succ(e))
      { num_edge_inspections++;
        NT& fe = flow[e];
        if (fe == 0) continue;
        node w = source(e);
        int dw = dist[w];
        if ( dw < dv ) // equivalent to ( dw == dv - 1 )
        { num_pushes++;
          NT& ew = excess[w];
          if (ew == 0) U.insert0(w,dw); 
          if (ev <= fe) 
          { fe -= ev; ew += ev;
            ev = 0;   // stop: excess[v] exhausted
            break;
          }
          else
          { ew += fe; ev -= fe;
            fe = 0;
          }
        }
        else { if ( dw < dmin ) dmin = dw; }
      }
 }

    excess[v] = ev;

    if (ev > 0) 
    { dist[v] = 1 + dmin; 
      num_relabels++;
      U.insert(v,dist[v]);
    }
  } 


#ifdef TEST
  assert(CHECK_MAX_FLOW_T(G,s,t,cap,flow));
#endif

  return excess[t];
}




template<class NT, class SET>
inline void compute_dist_t(const graph& G, node t, const edge_array<NT>& flow, 
                           const edge_array<NT>& cap, 
                           const node_array<NT>& excess, node_array<int>& dist, 
                           SET& U, b_queue<node>& Q, array<int>& count)
{ 
  int n = G.number_of_nodes();
  Q.append(t);

  dist[t] = 0; 

  count.init(0);
  count[0] = 1;

  while ( !Q.empty() )
  { node v = Q.pop();
    int  d = dist[v] + 1;
    edge e;

    for(e = G.first_adj_edge(v); e; e = G.adj_succ(e))
    { if ( flow[e] == 0 ) continue;
      node u = target(e); 
      int& du = dist[u];
      if ( du >= n ) 
      { du = d;
        Q.append(u); count[d]++; 
        if ( excess[u] > 0 ) U.insert(u,d); 
       }
     }

    for(e = G.first_in_edge(v); e; e = G.in_succ(e))
    { if ( cap[e] == flow[e] ) continue;
      node u = source(e); 
      int& du = dist[u];
      if ( du >= n )  
      { du = d;  
        Q.append(u); count[d]++;
       if (excess[u] > 0) U.insert(u,d); 
      }
    }
  }
}


template<class NT, class SET>
inline void compute_dist_s(const graph& G, node s, const edge_array<NT>& flow,
                           const node_array<NT>& excess, node_array<int>& dist, 
                           SET& U, b_queue<node>& Q)
{ 
  int n = G.number_of_nodes();
  int max_level = 2*n - 1;

  Q.append(s);
  dist[s] = n;

  while ( !Q.empty() )
  { node v = Q.pop();
    int  d = dist[v] + 1;
    edge e;
    for(e = G.first_adj_edge(v); e; e = G.adj_succ(e))
    { if ( flow[e] == 0 ) continue;
      node u = target(e); 
      int& du = dist[u];
      if ( du == max_level )
      { du  = d;  
        if (excess[u] > 0) U.insert(u,d);
        Q.append(u);
       }
     }
   } 
}



template<class NT, class SET>
__temp_func_inline
NT MAX_FLOW_GRH_T(const graph& G, node s, node t, 
                  const edge_array<NT>& cap, edge_array<NT>& flow,
                  SET& U, int& num_pushes, int& num_edge_inspections,
                  int& num_relabels, int& num_global_relabels, float h)
{ if (s == t) error_handler(1,"MAXFLOW: source == sink");

  
  
  flow.init(G,0);
  //if (G.outdeg(s) == 0) return 0;
  if (G.first_adj_edge(s) == nil) return 0;

  int n = G.number_of_nodes(); int max_level = 2*n - 1;
  int m = G.number_of_edges(); 

  node_array<NT>  excess(G,0);

  // saturate all edges leaving s

  edge e;
  forall_out_edges(e,s) 
  { NT c = cap[e];
    if (c == 0) continue;
    node v = target(e);
    flow[e] = c;
    excess[s] -= c;
    excess[v] += c;
   } 


  
  b_queue<node> Q(n);
  int phase_number = 1;
  array<int> count(n);


  list<node> S;


  int heuristic  = (int) (h*m);
  int limit_heur = heuristic;

  
  node_array<int> dist(G);
  dist.init(G,n);

  compute_dist_t(G,t,flow,cap,excess,dist,U,Q,count);

  
  num_relabels = num_pushes = num_edge_inspections = 0;
  num_global_relabels  = 0;



  
  for(;;) 
  {
    
    node v = U.del();

    if (v == nil)
    { 
      if ( phase_number == 2 ) break; // done

      dist.init(G,n);
      compute_dist_t(G,t,flow,cap,excess,dist,U,Q,count);

      node u;
      forall_nodes(u,G) 
      { if (dist[u] == n) 
        { S.append(u);
          dist[u] = max_level;
        }
      }
      phase_number = 2;
      compute_dist_s(G,s,flow,excess,dist,U,Q);
      continue;
    }
      
    if (v == t) continue;


    NT  ev   = excess[v]; // excess of v 
    int dv   = dist[v];   // level of v
    int dmin = MAXINT;
    edge e;

    if ( dist[v] < n )
    { 
      for (e = G.first_adj_edge(v); e; e = G.adj_succ(e))
      { num_edge_inspections++;
        NT& fe = flow[e];
        NT  rc = cap[e] - fe;
        if (rc == 0) continue;
        node w = target(e);
        int dw = dist[w];
        if ( dw < dv ) // equivalent to ( dw == dv - 1 )
        { num_pushes++;
          NT& ew = excess[w];
          if (ew == 0) U.insert0(w,dw); 
          if (ev <= rc) 
          { ew += ev; fe += ev;
            ev = 0;   // stop: excess[v] exhausted
            break;
          }
          else
          { ew += rc; fe += rc;
            ev -= rc;
          }
        }
        else { if ( dw < dmin ) dmin = dw; }
      }
 }
    if ( ev > 0 )
    { 
      for (e = G.first_in_edge(v); e; e = G.in_succ(e))
      { num_edge_inspections++;
        NT& fe = flow[e];
        if (fe == 0) continue;
        node w = source(e);
        int dw = dist[w];
        if ( dw < dv ) // equivalent to ( dw == dv - 1 )
        { num_pushes++;
          NT& ew = excess[w];
          if (ew == 0) U.insert0(w,dw); 
          if (ev <= fe) 
          { fe -= ev; ew += ev;
            ev = 0;   // stop: excess[v] exhausted
            break;
          }
          else
          { ew += fe; ev -= fe;
            fe = 0;
          }
        }
        else { if ( dw < dmin ) dmin = dw; }
      }
 }

    excess[v] = ev;

    if (ev > 0) 
    { 
      if (num_edge_inspections <= limit_heur)
        { 
          dmin++; num_relabels++;
          if ( phase_number == 1 && dmin >= n) dist[v] = n;
          else { dist[v] = dmin;
                 U.insert(v,dmin);
               }
 }
      else
        { limit_heur += heuristic;
          num_global_relabels++;
          
          U.clear();

          if (phase_number == 1)
          { dist.init(G,n);
            compute_dist_t(G,t,flow,cap,excess,dist,U,Q,count);
            if ( U.empty() ) 
            { node u;
              forall_nodes(u,G) 
              { if (dist[u] == n) 
                { S.append(u);
                  dist[u] = max_level;
                }
              }
              phase_number = 2;
              compute_dist_s(G,s,flow,excess,dist,U,Q);
            }
          }
          else
          { node u; 
            forall(u,S) dist[u] = max_level;
            compute_dist_s(G,s,flow,excess,dist,U,Q);
          }
 
        }
 }
  }


#ifdef TEST
  assert(CHECK_MAX_FLOW_T(G,s,t,cap,flow));
#endif

  return excess[t];
}


template<class NT, class SET>
__temp_func_inline
NT MAX_FLOW_GAP_T(const graph& G, node s, node t, 
                  const edge_array<NT>& cap, edge_array<NT>& flow,
                  SET& U, int& num_pushes, int& num_edge_inspections,
                  int& num_relabels, int& num_global_relabels,
                  int& num_gaps, float h)
{ if (s == t) error_handler(1,"MAXFLOW: source == sink");

  
  
  flow.init(G,0);
  //if (G.outdeg(s) == 0) return 0;
  if (G.first_adj_edge(s) == nil) return 0;

  int n = G.number_of_nodes(); int max_level = 2*n - 1;
  int m = G.number_of_edges(); 

  node_array<NT>  excess(G,0);

  // saturate all edges leaving s

  edge e;
  forall_out_edges(e,s) 
  { NT c = cap[e];
    if (c == 0) continue;
    node v = target(e);
    flow[e] = c;
    excess[s] -= c;
    excess[v] += c;
   } 


  
  b_queue<node> Q(n);
  int phase_number = 1;
  array<int> count(n);


  list<node> S;


  int heuristic  = (int) (h*m);
  int limit_heur = heuristic;

  
  node_array<int> dist(G);
  dist.init(G,n);

  compute_dist_t(G,t,flow,cap,excess,dist,U,Q,count);

  
  num_relabels = num_pushes = num_edge_inspections = 0;
  num_global_relabels  = 0;


  num_gaps = 0;

  
  for(;;) 
  {
    
    node v = U.del();

    if (v == nil)
    { 
      if ( phase_number == 2 ) break; // done

      dist.init(G,n);
      compute_dist_t(G,t,flow,cap,excess,dist,U,Q,count);

      node u;
      forall_nodes(u,G) 
      { if (dist[u] == n) 
        { S.append(u);
          dist[u] = max_level;
        }
      }
      phase_number = 2;
      compute_dist_s(G,s,flow,excess,dist,U,Q);
      continue;
    }
      
    if (v == t) continue;

    if (dist[v] == n && phase_number == 1) continue;
   
    NT  ev   = excess[v]; // excess of v 
    int dv   = dist[v];   // level of v
    int dmin = MAXINT;
    edge e;

    if ( dist[v] < n ) { 
                         for (e = G.first_adj_edge(v); e; e = G.adj_succ(e))
                         { num_edge_inspections++;
                           NT& fe = flow[e];
                           NT  rc = cap[e] - fe;
                           if (rc == 0) continue;
                           node w = target(e);
                           int dw = dist[w];
                           if ( dw < dv ) // equivalent to ( dw == dv - 1 )
                           { num_pushes++;
                             NT& ew = excess[w];
                             if (ew == 0) U.insert0(w,dw); 
                             if (ev <= rc) 
                             { ew += ev; fe += ev;
                               ev = 0;   // stop: excess[v] exhausted
                               break;
                             }
                             else
                             { ew += rc; fe += rc;
                               ev -= rc;
                             }
                           }
                           else { if ( dw < dmin ) dmin = dw; }
                         }
 }
    
    if ( ev > 0 ) { 
                    for (e = G.first_in_edge(v); e; e = G.in_succ(e))
                    { num_edge_inspections++;
                      NT& fe = flow[e];
                      if (fe == 0) continue;
                      node w = source(e);
                      int dw = dist[w];
                      if ( dw < dv ) // equivalent to ( dw == dv - 1 )
                      { num_pushes++;
                        NT& ew = excess[w];
                        if (ew == 0) U.insert0(w,dw); 
                        if (ev <= fe) 
                        { fe -= ev; ew += ev;
                          ev = 0;   // stop: excess[v] exhausted
                          break;
                        }
                        else
                        { ew += fe; ev -= fe;
                          fe = 0;
                        }
                      }
                      else { if ( dw < dmin ) dmin = dw; }
                    }
 }

    excess[v] = ev;

    if (ev > 0) { 
                  if (num_edge_inspections <= limit_heur)
                    {   
                      num_relabels++;
                      if (phase_number == 1)
                      { if ( --count[dv] == 0 || dmin >= n - 1)
                        { // v cannot reach t anymore 
                          
                          dist[v] = n; 
                          if ( dmin < n )
                          { Q.append(v); 
                            node w,z;
                            while ( !Q.empty() )
                            { edge e; 
                              w = Q.pop(); num_gaps++;
                              forall_out_edges(e,w)
                              { if ( flow[e] < cap[e] && dist[z = G.target(e)] < n)
                                { Q.append(z);  
                                  count[dist[z]]--; dist[z] = n;
                                }
                              }
                              forall_in_edges(e,w)
                              { if ( flow[e] > 0 && dist[z = G.source(e)] < n)
                                { Q.append(z); 
                                  count[dist[z]]--; dist[z] = n;
                                }
                              }
                            }
                          }

                        }
                        else 
                        { dist[v] = ++dmin; count[dmin]++;
                          U.insert(v,dmin); 
                        }
                      }
                      else // phase_number == 2
                      { dist[v] = ++dmin; 
                        U.insert(v,dmin); 
                      }
 }
                  else
                    { limit_heur += heuristic;
                      num_global_relabels++;
                      
                      U.clear();

                      if (phase_number == 1)
                      { dist.init(G,n);
                        compute_dist_t(G,t,flow,cap,excess,dist,U,Q,count);
                        if ( U.empty() ) 
                        { node u;
                          forall_nodes(u,G) 
                          { if (dist[u] == n) 
                            { S.append(u);
                              dist[u] = max_level;
                            }
                          }
                          phase_number = 2;
                          compute_dist_s(G,s,flow,excess,dist,U,Q);
                        }
                      }
                      else
                      { node u; 
                        forall(u,S) dist[u] = max_level;
                        compute_dist_s(G,s,flow,excess,dist,U,Q);
                      }
 
                    }
 }
  }


#ifdef TEST
  assert(CHECK_MAX_FLOW_T(G,s,t,cap,flow));
#endif

  return excess[t];
}



// static graph for (mis)using node-info fields
static GRAPH<node,edge> NEXT;

class level_queue {

const graph* gr;

node*  head;
node*  stop;
node*  max;



// array for saving and restoring original node infos
node* save;

public:


level_queue(const graph& G, int sz)
{ gr = &G;
  head = new node[sz];
  save = new node[G.number_of_nodes()];
  max  = head;
  stop = head + sz;
  node* p = head;
  while(p < stop) *p++ = nil; 
  node  v;
  p = save;
  forall_nodes(v,G) *p++ = NEXT[v];
 }

~level_queue() 
 { node  v;
   node* p = save;
   forall_nodes(v,*gr) NEXT[v] = *p++;
   delete[] save;
   delete[] head; 
  }


void insert0(node v, int i)
{ // insert without updating max pointer
   node* h = head + i;
  NEXT[v] = *h;
  *h = v;
 }

void insert(node v, int i)
{ // insert and update max pointer
  node* h = head + i;
  if (h > max) max = h;
  NEXT[v] = *h;
  *h = v;
 }

node del_max()
{ // remove and return maximum (nil if list is empty) 
  while (*max == nil && max > head) max--; 
  node v = *max;
  if (v != nil) *max = NEXT[v];
  return v;
}

node find_max()
{ // return maximum (nil if list is empty) 
  while (*max == nil && max > head) max--; 
  return *max;
}

node del() { return del_max(); }

bool empty() { return find_max() == nil; }

void clear() 
{ *head = nil;
  while (max > head) *max-- =  nil; 
 }

void clear(int i)
{ // erase everything on and above level i
  node* p = head + i;
  while (max >= p) *max-- =  nil;
  if (max < head) max = head;
}

};



template<class NT>
__temp_func_inline
NT MAX_FLOW_T(const graph& G, node s, node t, 
                 const edge_array<NT>& cap, 
                 edge_array<NT>& flow, 
                 int& num_pushes, 
                 int& num_edge_inspections,
                 int& num_relabels,
                 int& num_global_relabels,
                 int& num_gaps,
                 float h)
{  if (s == t) error_handler(1,"MAXFLOW: source == sink");


  int n = G.number_of_nodes(); int max_level = 2*n - 1;
  int m = G.number_of_edges(); 

/*
  node_array<int> dist(G,0);
  node_array<NT>  excess(G,0);
*/

// use free data slots for node_arrays if available

  node_array<NT> excess;
  excess.use_node_data(G,0);

  //flow.init(G,0);
  edge e;
  forall_edges(e,G) flow[e] = 0;

  forall_out_edges(e,s) 
  { NT c = cap[e];
    if (c == 0) continue;
    node v = target(e);
    flow[e] = c;
    excess[s] -= c;
    excess[v] += c;
  } 

  level_queue  U(G,2*n);
  b_queue<node> Q(n);
  int phase_number = 1;
  array<int> count(n);
  list<node> S;
  int heuristic  = (int) (h*m);
  int limit_heur = heuristic;

  //node_array<int> dist(G);
  //dist.init(G,n);
  node_array<int> dist;
  dist.use_node_data(G,n);

  compute_dist_t(G,t,flow,cap,excess,dist,U,Q,count);

  num_relabels         = 0;
  num_pushes           = 0;
  num_edge_inspections = 0;
  num_global_relabels  = 0;
  num_gaps = 0;

  
  for(;;) 
  {
    
    node v = U.del();

    if (v == nil)
    { 
      if ( phase_number == 2 ) break; // done

      //dist.init(G,n);
      dist.use_node_data(G,n);
      compute_dist_t(G,t,flow,cap,excess,dist,U,Q,count);

      node u;
      forall_nodes(u,G) 
      { if (dist[u] == n) 
        { S.append(u);
          dist[u] = max_level;
        }
      }
      phase_number = 2;
      compute_dist_s(G,s,flow,excess,dist,U,Q);
      continue;
    }
      
    if (v == t) continue;

    if (dist[v] == n && phase_number == 1) continue;
   
    NT  ev   = excess[v]; // excess of v 
    int dv   = dist[v];   // level of v
    int dmin = MAXINT;
    edge e;

    if ( dist[v] < n )
    { 
      for (e = G.first_adj_edge(v); e; e = G.adj_succ(e))
      { num_edge_inspections++;
        NT& fe = flow[e];
        NT  rc = cap[e] - fe;
        if (rc == 0) continue;
        node w = target(e);
        int dw = dist[w];
        if ( dw < dv ) // equivalent to ( dw == dv - 1 )
        { num_pushes++;
          NT& ew = excess[w];
          if (ew == 0) U.insert0(w,dw); 
          if (ev <= rc) 
          { ew += ev; fe += ev;
            ev = 0;   // stop: excess[v] exhausted
            break;
          }
          else
          { ew += rc; fe += rc;
            ev -= rc;
          }
        }
        else { if ( dw < dmin ) dmin = dw; }
      }
 }
    if ( ev > 0 )
    { 
      for (e = G.first_in_edge(v); e; e = G.in_succ(e))
      { num_edge_inspections++;
        NT& fe = flow[e];
        if (fe == 0) continue;
        node w = source(e);
        int dw = dist[w];
        if ( dw < dv ) // equivalent to ( dw == dv - 1 )
        { num_pushes++;
          NT& ew = excess[w];
          if (ew == 0) U.insert0(w,dw); 
          if (ev <= fe) 
          { fe -= ev; ew += ev;
            ev = 0;   // stop: excess[v] exhausted
            break;
          }
          else
          { ew += fe; ev -= fe;
            fe = 0;
          }
        }
        else { if ( dw < dmin ) dmin = dw; }
      }
 }

    excess[v] = ev;

    if (ev > 0) 
    { 
      if (num_edge_inspections <= limit_heur)
        {   
          num_relabels++;
          if (phase_number == 1)
          { if ( --count[dv] == 0 || dmin >= n - 1)
            { // v cannot reach t anymore 
              
              dist[v] = n; 
              if ( dmin < n )
              { Q.append(v); 
                node w,z;
                while ( !Q.empty() )
                { edge e; 
                  w = Q.pop(); num_gaps++;
                  forall_out_edges(e,w)
                  { if ( flow[e] < cap[e] && dist[z = G.target(e)] < n)
                    { Q.append(z);  
                      count[dist[z]]--; dist[z] = n;
                    }
                  }
                  forall_in_edges(e,w)
                  { if ( flow[e] > 0 && dist[z = G.source(e)] < n)
                    { Q.append(z); 
                      count[dist[z]]--; dist[z] = n;
                    }
                  }
                }
              }

            }
            else 
            { dist[v] = ++dmin; count[dmin]++;
              U.insert(v,dmin); 
            }
          }
          else // phase_number == 2
          { dist[v] = ++dmin; 
            U.insert(v,dmin); 
          }
 }
      else
        { limit_heur += heuristic;
          num_global_relabels++;
          
          U.clear();

          if (phase_number == 1)
          { //dist.init(G,n);
            dist.use_node_data(G,n);
            compute_dist_t(G,t,flow,cap,excess,dist,U,Q,count);
            if ( U.empty() ) // U is empty
            { node u;
              forall_nodes(u,G) 
              { if (dist[u] == n) 
                { S.append(u);
                  dist[u] = max_level;
                }
              }
              phase_number = 2;
              compute_dist_s(G,s,flow,excess,dist,U,Q);
            }
          }
          else
          { node u; 
            forall(u,S) dist[u] = max_level;
            compute_dist_s(G,s,flow,excess,dist,U,Q);
          }

                   
        }
 }
  }


#ifdef TEST
  assert(CHECK_MAX_FLOW_T(G,s,t,cap,flow));
#endif

  return excess[t];
}


template <class NT>
__temp_func_inline
NT MAX_FLOW_T(const graph& G, node s, node t, 
                 const edge_array<NT>& cap, 
                 edge_array<NT>& flow)
{ int num_pushes, num_edge_inspections, num_relabels, num_global_relabels,
      num_gaps;
  float h = 5.0;
  return MAX_FLOW_T(G,s,t,cap,flow,num_pushes,num_edge_inspections,
        num_relabels,num_global_relabels,num_gaps,h);
}




#if LEDA_ROOT_INCL_ID == 500341
#undef LEDA_ROOT_INCL_ID
#include <LEDA/internal/POSTAMBLE.h>
#endif


LEDA_END_NAMESPACE

