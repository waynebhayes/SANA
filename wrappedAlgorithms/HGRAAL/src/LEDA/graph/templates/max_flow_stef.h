/*******************************************************************************
+
+  LEDA 5.0.1  
+
+
+  max_flow_stef.t
+
+
+  Copyright (c) 1995-2005
+  by Algorithmic Solutions Software GmbH
+  All rights reserved.
+ 
*******************************************************************************/

// $Revision: 1.2 $  $Date: 2005/04/14 10:45:16 $

#if !defined(LEDA_ROOT_INCL_ID)
#define LEDA_ROOT_INCL_ID 500342
#include <LEDA/internal/PREAMBLE.h>
#endif


//-----------------------------------------------------------------------------
//
// MAX_FLOW
//
// preflow-push + highest level + gap heuristic 
//
// Stefan's Version
//
//-----------------------------------------------------------------------------

#include <LEDA/graph/graph_alg.h>
#include <LEDA/graph/node_list.h>
#include <LEDA/core/b_queue.h>
#include <LEDA/system/assert.h>

LEDA_BEGIN_NAMESPACE

//------------------------------------------------------------------------------
// level queue data structure
// used to implement (max) priority queue of nodes with positive excess 
// ordered by levels (dist values) 
//
// level_queue1 Q(max_level);
//
// void Q.insert(node v, int d)    insert v with level d (update max)
// void Q.insert0(node v, int d)   d is not greater than current max
// node Q.del()                    remove and return a node from max level
//                                 (nil if Q is empty)
// void Q.clear()                  
//------------------------------------------------------------------------------


// dummy graph for (mis)using node-info fields

static GRAPH<node,edge> NEXT;


class level_queue1 {

const graph* gr;

node*  head;
node*  stop;
node*  max;

int    off;


// array for saving and restoring original node infos
node* save;

public:

level_queue1(const graph& G, int sz, int offset=0)
{ gr = &G;
  int n = G.number_of_nodes();
  head = new node[sz];
  save = new node[n];
  max  = head;
  stop = head + sz;
  off  = offset;
  node* p = head;
  while(p < stop) *p++ = nil; 
  node  v;
  p = save;
  forall_nodes(v,G) *p++ = NEXT[v];
 }

~level_queue1() 
 { node  v;
   node* p = save;
   forall_nodes(v,*gr) NEXT[v] = *p++;
   delete[] save;
   delete[] head; 
  }

void set_min(int offset) { off = offset; }

void insert0(node v, int i)
{ // insert without updating max pointer
   node* h = head + i - off;
  NEXT[v] = *h;
  *h = v;
 }


void insert(node v, int i)
{ // insert and update max pointer
  node* h = head + i - off;
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

bool empty(int i) { return head[i-off] == 0; }

bool empty() { return find_max() == nil; }

void set_max(int i) { max = head+i; }

node del() { return del_max(); }

void clear() 
{ *head = nil;
  while (max > head) *max-- =  nil; 
 }

void clear(int i) 
{ // erase everything on and above level i
  node* p = head + i;
  *p = 0;
  while (max > p) *max-- =  nil; 
}


};






template<class NT>
inline
void compute_dist1(const graph& G, node t, const edge_array<NT>& flow, 
                                           const edge_array<NT>& cap, 
                                           node_array<int>& dist, 
                                           node_list& levels, node* level_head)
{ 
  // compute exact distance values by a "backward" bfs in the
  // residual network starting at t

  int n = G.number_of_nodes();

  levels.clear();

  b_queue<node> Q(n);
  Q.append(t);

  dist[t] = 0;

  int last_d = -1;

  while ( ! Q.empty() )
  { node v = Q.pop();
    int  d = dist[v];

    levels.append(v);

    if (d > last_d)
    { level_head[d] = v; 
      last_d = d;
     }

    d = d+1;

    edge e;
    for(e = G.first_adj_edge(v); e; e = G.adj_succ(e) )
    { if (flow[e] == 0) continue;
      node u = target(e); 
      int& du = dist[u];
      if (du >= n)
      { du = d;
        Q.append(u);
       }
     }

    for(e = G.first_in_edge(v); e; e = G.in_succ(e) )
    { if (cap[e] == flow[e]) continue;
      node u = source(e); 
      int& du = dist[u];
      if (du >= n)
      { du = d;
        Q.append(u);
       }
     }
   } 
}



template<class NT>
inline
void compute_dist2(const graph& G, node s, const edge_array<NT>& flow, 
                                           node_array<int>& dist)
{ // forward bfs starting in s
  // (when excess flows back to s) 

  int n = G.number_of_nodes();

  b_queue<node> Q(n);
  Q.append(s);
  dist[s] = n;

  while (! Q.empty() )
  { node v = Q.pop();
    int  d = dist[v] + 1;
    edge e;
    for(e = G.first_adj_edge(v); e; e = G.adj_succ(e) )
    { if (flow[e] == 0) continue;
      node u = target(e); 
      if (dist[u] >= 2*n)
      { dist[u] = d;
        Q.append(u);
       }
     }
   } 
}




template<class NT>
inline
void init_level_queue(const graph& G, const node_array<NT>& excess, 
                                      const node_array<int>& dist,
                                      level_queue1& U,
                                      int min_level, 
                                      int max_level)
{ U.clear();
  U.set_min(min_level);
  node v;
  forall_nodes(v,G)
  { int dv = dist[v]; 
    if (dv <= max_level && excess[v] > 0) U.insert(v,dv);
   }
}




template<class NT>
__temp_func_inline
NT MAX_FLOW_S0_T(const graph& G, node s, node t, const edge_array<NT>& cap, 
                                                 edge_array<NT>& flow,
                                                 int& num_pushes,
                                                 int& edge_inspections,
                                                 int& num_relabels,
                                                 int& num_updates,
                                                 int& num_gap_nodes, float h)
{ float T = used_time();

  int n = G.number_of_nodes();
  int m = G.number_of_edges(); 
  
  flow.init(G,0);
  
  // use free data slots for node_arrays if available
  
  node_array<int> dist;
  dist.use_node_data(G,n);
  
  node_array<NT> excess;
  excess.use_node_data(G,0);
  
  
  // parameter for heuristic suggested by Goldberg to speed up algorithm 
  // compute exact distance labels after every "heuristic" relabel steps
  
  int heuristic = (int)(h*n);
  int limit_heur = heuristic;
  
  num_pushes   = 0;
  num_relabels = 0;
  num_updates  = 0;
  num_gap_nodes= 0;

  edge_inspections = 0;
  
  
  if (s == t) error_handler(1,"MAXFLOW: source == sink");

  // saturate all edges leaving from s and init level queue
  
  level_queue1  U(G,n);
  
  edge e;
  forall_out_edges(e,s) 
  { NT c = cap[e];
    flow[e] = c;
    excess[s] -= c;
    excess[target(e)] += c;
   }

  node_list levels;
  node*     level_head = new node[n];

  int phase = 1;

  int min_level = 0;
  int max_level = n-1;

  dist.use_node_data(G,max_level+1);
  compute_dist1(G,t,flow,cap,dist,levels,level_head);

  init_level_queue(G,excess,dist,U,min_level,max_level);

  //cout << string("initialization: %.2f",used_time(T)) << endl;

  for(;;)
  {
    node v = U.del();

    if (v == nil) 
    { if (phase == 2) break;
      phase = 2;
      min_level = n;
      max_level = 2*n-1;
      dist.use_node_data(G,max_level+1);
      compute_dist2(G,s,flow,dist);
      init_level_queue(G,excess,dist,U,min_level,max_level);
      continue;
     }
   
    if (v == t) continue;

   
    NT  ev = excess[v]; // temporary excess of v 
    int dv = dist[v];   // level of v

   
    int dmin = MAXINT;  // will contain minimum level of nodes adjacent
                        // in residual network if ev > 0 after push step

    // push excess ev out of v (invariant: ev > 0)
   
    if (dv < n) // do not use outgoing arcs in phase 2 (excess flows back to s)
    { edge e;
      for (e = G.first_adj_edge(v); e; e = G.adj_succ(e))
      { NT& fe = flow[e];
        NT  rc = cap[e] - fe;
        if (rc == 0) continue;
        node w = target(e);
        int dw = dist[w];
        if (dw == dv-1)
         { num_pushes++;
           NT& ew = excess[w];
           if (ew == 0) U.insert(w,dw);
           if (ev <= rc) 
           { ew += ev;
             fe += ev;
             ev = 0;   // excess[v] exhausted (stop)
             break;
            }
           else
           { ew += rc;
             fe += rc;
             ev -= rc;
            }
          }
        else
          if (dw < dmin) dmin = dw;
       }
     }
   
   
     if (ev > 0)  // stop if excess[v] == 0
     { edge e;
       for (e = G.first_in_edge(v); e; e = G.in_succ(e))
       { NT& fe = flow[e];
         if (fe == 0) continue;
         node w = source(e);
         int dw = dist[w];
         if (dw == dv-1)
          { num_pushes++;
            NT& ew = excess[w];
            if (ew == 0) U.insert(w,dw);
            if (ev <= fe) 
            { fe -= ev; 
              ew += ev;
              ev = 0;   // excess[v] exhausted
              break;
             }
            else
            { ew += fe;
              ev -= fe;
              fe = 0;
             }
           }
         else
           if (dw < dmin) dmin = dw;
       } 
     }
   
     
     excess[v] = ev;
   
     if (ev > 0)  
     { 
       // remaining excess at v
       // relabel vertex v (i.e. update dist[v]) because all
       // admissible edges in the residual graph have been saturated 
   
       if (num_relabels <= limit_heur)
         { num_relabels++;
           if (dv < n) //(phase == 1)
           { node vsucc = levels.succ(v);
             levels.del(v);
             if (level_head[dv] == v) 
             { level_head[dv] = vsucc;
               if (dmin < n && (vsucc == nil || dist[vsucc] != dv)) //gap
               { //num_gaps++;
                 num_gap_nodes++;
                 dmin = n-1; //dist[v] = n
                 for(node u = vsucc; u!=nil; u = levels.succ(u))
                 { dist[u] =  n;
                   num_gap_nodes++;
                  }
                 if (vsucc) levels.del(vsucc,levels.last());
                }
              }
            }

           dv = dmin+1;
           dist[v] = dv;

           if (dv < n) 
           { if (dv > dist[levels.last()])
               { levels.append(v);
                 level_head[dv] = v;
                }
             else
               { node u = level_head[dv];
                 levels.insert(v,u);
                }
            }
           if (dv <= max_level) U.insert(v,dv);
          }
       else
         { // heuristic suggested by Goldberg to reduce number of relabels:
           // periodically compute exact dist[] labels by a backward bfs 
           // starting at t
           limit_heur += heuristic;
           if (phase == 1) 
           { //dist.use_node_data(G,max_level+1);
             forall(v,levels) dist[v] = max_level+1;
             compute_dist1(G,t,flow,cap,dist,levels,level_head);
            }
           if (phase == 2)
           { dist.use_node_data(G,max_level+1);
             compute_dist2(G,s,flow,dist);
            }
           init_level_queue(G,excess,dist,U,min_level,max_level);
           num_updates++;
         }
      }
  } // end of main loop

  delete[] level_head;

  return excess[t];  // value of maximum flow from s to t
}



template<class NT>
inline
void compute_dist11(const graph& G, node t, const edge_array<NT>& flow, 
                                            const edge_array<NT>& cap, 
                                            node_array<int>& dist, 
                                            int* count)
{ 
  // compute exact distance values by a "backward" bfs in the
  // residual network starting at t

  int n = G.number_of_nodes();

  for(int i=0; i<n; i++) count[i] = 0;

  b_queue<node> Q(n);
  Q.append(t);

  dist[t] = 0;

  while ( ! Q.empty() )
  { node v = Q.pop();
    int  d = dist[v];

    count[d]++;

    d = d+1;

    edge e;
    for(e = G.first_adj_edge(v); e; e = G.adj_succ(e) )
    { if (flow[e] == 0) continue;
      node u = target(e); 
      int& du = dist[u];
      if (du >= n)
      { du = d;
        Q.append(u);
       }
     }

    for(e = G.first_in_edge(v); e; e = G.in_succ(e) )
    { if (cap[e] == flow[e]) continue;
      node u = source(e); 
      int& du = dist[u];
      if (du >= n)
      { du = d;
        Q.append(u);
       }
     }
   } 
}



template<class NT>
inline
void compute_dist22(const graph& G, node s, const edge_array<NT>& flow, 
                                           node_array<int>& dist)
{ // forward bfs starting in s
  // (when excess flows back to s) 

  int n = G.number_of_nodes();

  b_queue<node> Q(n);
  Q.append(s);
  dist[s] = n;

  while (! Q.empty() )
  { node v = Q.pop();
    int  d = dist[v] + 1;
    edge e;
    for(e = G.first_adj_edge(v); e; e = G.adj_succ(e) )
    { if (flow[e] == 0) continue;
      node u = target(e); 
      if (dist[u] >= 2*n)
      { dist[u] = d;
        Q.append(u);
       }
     }
   } 
}





template<class NT>
__temp_func_inline
NT MAX_FLOW_S_T(const graph& G, node s, node t, const edge_array<NT>& cap, 
                                                edge_array<NT>& flow,
                                                int& num_pushes,
                                                int& edge_inspections,
                                                int& num_relabels,
                                                int& num_updates,
                                                int& num_gap_nodes, float h)
{ 
  int n = G.number_of_nodes();
  
  flow.init(G,0);
  
  // use free data slots for node_arrays if available
  
  node_array<int> dist;
  dist.use_node_data(G,n);
  
  node_array<NT> excess;
  excess.use_node_data(G,0);
  
  
  // parameter for heuristic suggested by Goldberg to speed up algorithm 
  // compute exact distance labels after every "heuristic" relabel steps
  
  int heuristic = (int)(h*n);

  int limit_heur = heuristic;
  
  num_pushes   = 0;
  num_relabels = 0;
  num_updates  = 0;
  num_gap_nodes= 0;

  edge_inspections = 0;
  
  
  if (s == t) error_handler(1,"MAXFLOW: source == sink");

  // saturate all edges leaving from s and init level queue
  
  level_queue1  U(G,n);
  
  edge e;
  forall_out_edges(e,s) 
  { NT c = cap[e];
    flow[e] = c;
    excess[s] -= c;
    excess[target(e)] += c;
   }

  int* count = new int[n];

  int phase = 1;

  int min_level = 0;
  int max_level = n-1;

  dist.use_node_data(G,max_level+1);
  compute_dist11(G,t,flow,cap,dist,count);

  init_level_queue(G,excess,dist,U,min_level,max_level);

  //cout << string("initialization: %.2f",used_time(T)) << endl;

  for(;;)
  {
    node v = U.del();

    if (v == nil) 
    { if (phase == 2) break;
      phase = 2;
      min_level = n;
      max_level = 2*n-1;
      dist.use_node_data(G,max_level+1);
      compute_dist22(G,s,flow,dist);
      init_level_queue(G,excess,dist,U,min_level,max_level);
      continue;
     }
   
    if (v == t) continue;

   
    NT  ev = excess[v]; // temporary excess of v 
    int dv = dist[v];   // level of v

   
    int dmin = MAXINT;  // will contain minimum level of nodes adjacent
                        // in residual network if ev > 0 after push step

    // push excess ev out of v (invariant: ev > 0)
   
    if (dv < n) // do not use outgoing arcs in phase 2 (excess flows back to s)
    { edge e;
      for (e = G.first_adj_edge(v); e; e = G.adj_succ(e))
      { NT& fe = flow[e];
        NT  rc = cap[e] - fe;
        if (rc == 0) continue;
        node w = target(e);
        int dw = dist[w];
        if (dw == dv-1)
         { num_pushes++;
           NT& ew = excess[w];
           if (ew == 0) U.insert(w,dw);
           if (ev <= rc) 
           { ew += ev;
             fe += ev;
             ev = 0;   // excess[v] exhausted (stop)
             break;
            }
           else
           { ew += rc;
             fe += rc;
             ev -= rc;
            }
          }
        else
          if (dw < dmin) dmin = dw;
       }
     }
   
   
     if (ev > 0)  // stop if excess[v] == 0
     { edge e;
       for (e = G.first_in_edge(v); e; e = G.in_succ(e))
       { NT& fe = flow[e];
         if (fe == 0) continue;
         node w = source(e);
         int dw = dist[w];
         if (dw == dv-1)
          { num_pushes++;
            NT& ew = excess[w];
            if (ew == 0) U.insert(w,dw);
            if (ev <= fe) 
            { fe -= ev; 
              ew += ev;
              ev = 0;   // excess[v] exhausted
              break;
             }
            else
            { ew += fe;
              ev -= fe;
              fe = 0;
             }
           }
         else
           if (dw < dmin) dmin = dw;
       } 
     }
   
     
     excess[v] = ev;
   
   
     if (ev > 0)
     { 
       // remaining excess at v
       // relabel vertex v (i.e. update dist[v]) because all
       // admissible edges in the residual graph have been saturated 
   
       if (num_relabels <= limit_heur)
         { num_relabels++;
           if (dv < n && --count[dv] == 0 ) // gap (phase == 1)
           { //num_gaps++;
             b_queue<node> Q(n);
             Q.append(v);
             dist[v] = n;
             while ( !Q.empty() )
             { edge e;
               node w = Q.pop(); 
               num_gap_nodes++;
               forall_out_edges(e,w)
               { node z = G.target(e);
                 int& dz = dist[z];
                 if (dz < n && flow[e] < cap[e])
                 { Q.append(z);
                   count[dz]--; 
                   dz = n;
                 }
               }
               forall_in_edges(e,w)
               { node z = G.source(e);
                 int& dz = dist[z];
                 if (dz < n && flow[e] > 0)
                 { Q.append(z);
                   count[dz]--; 
                   dz = n;
                 }
               }
             }

            if (dmin < n) dmin = n-1; // dist[v] = n
           }

           dv = dmin+1;
           dist[v] = dv;

           if (dv < n) count[dv]++;
           if (dv <= max_level) U.insert(v,dv);
          }
       else
         { // heuristic suggested by Goldberg to reduce number of relabels:
           // periodically compute exact dist[] labels by a backward bfs 
           // starting at t
           limit_heur += heuristic;
           if (phase == 1) 
           { dist.use_node_data(G,max_level+1);
             compute_dist11(G,t,flow,cap,dist,count);
            }
           if (phase == 2)
           { dist.use_node_data(G,max_level+1);
             compute_dist22(G,s,flow,dist);
            }
           init_level_queue(G,excess,dist,U,min_level,max_level);
           num_updates++;
         }
      }
  } // end of main loop

  delete[] count;

  return excess[t];  // value of maximum flow from s to t
}


LEDA_END_NAMESPACE

#if LEDA_ROOT_INCL_ID == 500342
#undef LEDA_ROOT_INCL_ID
#include <LEDA/internal/POSTAMBLE.h>
#endif
