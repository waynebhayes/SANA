/*******************************************************************************
+
+  LEDA 5.0.1  
+
+
+  max_flow.t
+
+
+  Copyright (c) 1995-2005
+  by Algorithmic Solutions Software GmbH
+  All rights reserved.
+ 
*******************************************************************************/

// $Revision: 1.6 $  $Date: 2005/04/14 10:45:16 $

#ifndef _LEDA_MAX_FLOW_T
#define _LEDA_MAX_FLOW_T

#if !defined(LEDA_ROOT_INCL_ID)
#define LEDA_ROOT_INCL_ID 500343
#include <LEDA/internal/PREAMBLE.h>
#endif

//-----------------------------------------------------------------------------
//
// MAX_FLOW
//
// preflow-push + highest level + gap heuristic 
//
//-----------------------------------------------------------------------------


/*
#include <LEDA/graph/graph.h>
*/

#include <LEDA/system/assert.h>

LEDA_BEGIN_NAMESPACE

/*
#define out_cap(e)      cap[e]
#define in_cap(e)       flow[e]
#define total_cap(e)    (cap[e]+flow[e])
#define change_flow(e,x) { flow[e] += x; cap[e] -= x; }
*/

#define out_cap(e)      (cap[e]-flow[e])
#define in_cap(e)       flow[e]
#define total_cap(e)    cap[e]
#define change_flow(e,x)  { flow[e] += x; }



template <class graph, class succ_array>
class node_level_queue {

typedef typename graph::node node;
typedef typename graph::edge edge;

node*  T;
node*  max;
int    sz;

succ_array& NEXT;

public:

node_level_queue(const graph& G, succ_array& succ) : NEXT(succ)
{ 
  sz = G.number_of_nodes();
  T = new node[sz];
  for(int i=0; i<sz; i++) T[i] = 0;
  max = T;
 }

~node_level_queue() { delete[] T; }

void set_max(int i) { max = T + i; }

void insert(node v, int i)
{ // insert on level i
  node* h = T + i;
  if (h > max) max = h;
  NEXT[v] = *h;
  *h = v;
 }


void insert_non_max(node v, int i)
{ // do not update max pointer
  node* h = T + i;
  NEXT[v] = *h;
  *h = v;
 }


void insert_max(node v, int i)
{ // new max at level i
  max = T + i;
  *max = v;
  NEXT[v] = 0;
 }


node find_max()
{ while (*max == 0) max--;
  return *max;
}

node del_max()
{ node* p = max;
  while (*p == 0) p--;
  node v = *p;
  *p = NEXT[v];
  max = p;
  return v; 
}


void del_max(node v) { *max = NEXT[v]; }


void clear() { 
  for(int i=0; i<sz; i++) T[i] = 0;
  max = T;
}


};



template<class NT, class graph,
#if !defined(_MSC_VER)
         class succ_array   = node_array<typename graph::node,graph>,
#else
         class succ_array   = node_array<graph::node,graph>,
#endif
         class excess_array = node_array<NT, graph>,
         class dist_array   = node_array<int,graph>,
         class curr_array   = node_array<int,graph> >
class max_flow  
{
  const int node_slots;
  const int edge_slots;

  typedef typename graph::node node;
  typedef typename graph::edge edge;

  typedef node_level_queue<graph,succ_array>  level_queue;


  float h;

  bool run_phase0;

  NT fval;
  NT cut_bound;


  // statistics (phase 1, 2, and sum)

  string ph_name[3];

  int num_pushes[3];
  int num_relabels[3];
  int num_updates[3];
  int num_gaps[3]; 
  int num_gap_nodes[3]; 
  int num_inspections[3];

  float cputime[3];
  float update_time;
  float gap_time;

  node source(const graph& G, edge e, node t)
  { if (graph::category() == opposite_graph_category)
      return G.opposite(e,t);
    else
      return G.source(e);
  }
  
  node target(const graph& G, edge e, node s)
  { if (graph::category() == opposite_graph_category)
      return G.opposite(e,s);
    else
      return G.target(e);
  }

template<class T>
T count(T* counter, int i = -1) 
{ if (i < 0) 
    return counter[0] + counter[1] + counter[2];
  else 
    return counter[i];
}

public:

max_flow() : node_slots(3), edge_slots(0), h(5.0), run_phase0(true), 
                                                   fval(0),
                                                   cut_bound(0)
{ reset_counters(); }

int pushes(int i=-1)      { return count(num_pushes,i); }
int relabels(int i=-1)    { return count(num_relabels,i); }
int updates(int i=-1)     { return count(num_updates,i); }
int gaps(int i=-1)        { return count(num_gaps,i); }
int gap_nodes(int i=-1)   { return count(num_gap_nodes,i); }
int inspections(int i=-1) { return count(num_inspections,i); }

float cpu_time(int i=-1) { return count(cputime,i); }

string name(int i=-1)
{ if (i==-1) return "total";
  else return ph_name[i];
}

void set_heuristic(float hh) { h = hh; }

void set_run_phase0(bool b) { run_phase0 = b; }


NT flow() { return fval; }


void reset_counters()
{ for(int i=0; i<3; i++) 
  { num_pushes[i] = 0;
    num_relabels[i] = 0;
    num_updates[i] = 0;
    num_gaps[i] = 0;
    num_gap_nodes[i] = 0;
    num_inspections[i] = 0;
    cputime[i] = 0;
    update_time = 0;
    gap_time = 0;
   }

  ph_name[0] = "phase 0";
  ph_name[1] = "phase 1";
  ph_name[2] = "phase 2";
}


template<class cap_array, class flow_array>
bool check_dist_labels(const graph& G, const cap_array& cap,
                                       const flow_array& flow,
                                       dist_array& dist)
{ 
  int err_count = 0;
  node v;
  forall_nodes(v,G)
  { edge e;
    forall_out_edges(e,v)
    { if (out_cap(e) == 0) continue;
      node w = target(G,e,v);
      if (dist[v] > dist[w]+1) err_count++;
     }
    forall_in_edges(e,v)
    { if (in_cap(e) == 0) continue;
      node w = source(G,e,v);
      if (dist[v] > dist[w]+1) 
      if (dist[v] > dist[w]+1) err_count++;
     }
   }

  return err_count == 0;
}



template<class cap_array, class flow_array>
void compute_dist0(const graph& G, node* queue, node s, node t,
                   const cap_array& cap,
                   const flow_array& flow,
                   excess_array& excess,
                   dist_array& dist,
                   level_queue& U)
{ 
  // compute exact distance values by a "backward" bfs in the
  // residual network starting at t

  float T = used_time();

  U.clear();

  num_updates[0]++;

  int n = G.number_of_nodes();


  dist.init(G,n);

  dist[t] = n+1;
  queue[0] = t;

  node* queue_cur  = queue;
  node* queue_last = queue;
  node* level_last = queue;

  int       d = 0;
  int min_cut = MAXINT;
  double  cut = 0;

  double max_edge_insp = 0.1*n*n;
  int edge_insp = 0;

  while (queue_cur <= queue_last)
  { 
    node v = *queue_cur;

    assert(dist[v] == n+1);

    dist[v] = d;
    if (excess[v] > 0 || v == t) U.insert(v,d);

    cut += excess[v];

    if ((queue_last-queue+1) < n-1  && edge_insp < max_edge_insp)
    { edge e;
      forall_in_edges(e,v)
      { edge_insp++;
        node u = source(G,e,v); 
        int du = dist[u];
        if (du > d) cut += out_cap(e);
        if (du == n && u != s)
        { dist[u] = n+1;
          *++queue_last = u;
         }
       }
  
      forall_out_edges(e,v)
      { node u = target(G,e,v); 
        if (u != v && dist[u] <= d) cut -= out_cap(e);
       }
  
      if (cut < min_cut) min_cut = (int)cut;
    }

    if (queue_cur == level_last)  // finish level d
    { d++; 
      level_last = queue_last;
     }

    queue_cur++;
  }

 assert(dist[s] == n);

 if ((queue_last-queue+1) < n-1) // not all nodes (!= s) reached
 { node v;
   forall_nodes(v,G)
   { if (dist[v] < n || v == s) continue; 
     dist[v] = d;
     if (excess[v] > 0) U.insert(v,d);
     *queue_cur++ = v;
    }
  }

 *queue_cur = 0;

 cut_bound = min_cut;

 float tt = used_time(T);
 update_time += tt;
}



template<class cap_array, class flow_array>
int compute_dist1(const graph& G, node* V, node s, node t,
                   const cap_array& cap,
                   const flow_array& flow,
                   excess_array& excess,
                   dist_array& dist,
                   level_queue& U,
                   int* count, bool filter=true)
{ 
  // compute exact distance values by a "backward" bfs in the
  // residual network starting at t

  float T = used_time();

  num_updates[1]++;

  int n = G.number_of_nodes();

  U.clear();

  int V_sz = 0;

  for(node* p = V; *p; p++) 
  { node v = *p;
    if (!filter || dist[v] < n) V_sz++;
    dist[v] = n;
   }

  node* queue = V;

  dist[t] = 0;
  queue[0] = t;
  count[0] = 1;

  node* queue_cur  = queue;
  node* queue_last = queue;
  node* queue_max  = queue + V_sz - 1;
  node* level_last = queue;

  int d = 0;

  while (queue_cur <= queue_last) 
  { 

    node v = *queue_cur;

    if (excess[v] > 0 || v == t) U.insert(v,d);

    if (queue_last < queue_max)
    { edge e;
      forall_out_edges(e,v)
      { if (in_cap(e) == 0) continue;
        node u = target(G,e,v); 
        if (dist[u] == n)
        { dist[u] = d+1;
          *++queue_last = u;
         }
       }
   
      forall_in_edges(e,v)
      { if (out_cap(e) == 0) continue;
        node u = source(G,e,v); 
        if (dist[u] == n)
        { dist[u] = d+1;
          *++queue_last = u;
         }
       }
     }
     //else cout << "stop global update" << endl;

    if (queue_cur == level_last)       // finish level d
    { count[++d] = queue_last - level_last;
      level_last = queue_last;
     }

    queue_cur++;
  }

 while (++d < n && count[d] > 0) count[d] = 0;

 *queue_cur = 0;

 assert(dist[s] == n);

 float tt  = used_time(T);
 update_time += tt;

 return V_sz;
}


template<class flow_array>
void compute_dist2(const graph& G, node* V, node s, node t,
                   const flow_array& flow,
                   excess_array& excess,
                   dist_array& dist,
                   level_queue& U)
{ 
  // compute exact distance values by a forward bfs in the
  // residual network starting at s

  float T = used_time();

  num_updates[2]++;

  int n = G.number_of_nodes();

  U.clear();
  U.insert(s,0);

  node v;
  forall_nodes(v,G) dist[v] = n;

  node* queue = V;

  dist[s] = 0;
  queue[0] = s;

  int d = 0;

  node* queue_cur  = queue;
  node* queue_last = queue;
  node* level_last = queue;

  while (queue_cur <= queue_last) 
  { node v = *queue_cur;
    if (v != t && excess[v] > 0) U.insert(v,d);
    edge e;
    forall_out_edges(e,v)
    { if (in_cap(e) == 0) continue;
      node u = target(G,e,v); 
      if (dist[u] == n)
      { dist[u] = d+1;
        *++queue_last = u;
       }
     }

    if (queue_cur == level_last)  // finish level d
    { d++;
      level_last = queue_last;
     }

    queue_cur++;
  }

 *queue_cur = 0;

 update_time += used_time(T);
}


void handle_gap(const graph& G, node* V, node v, dist_array& dist, int* count)
{
  float T = used_time();

  int n = G.number_of_nodes();
  int gap_nodes = 1;

  int dv = dist[v];

  for(int i = dv+1; i < n && count[i] > 0; i++) 
  { gap_nodes += count[i];
    count[i] = 0;
   }

  dist[v] = n;

  if (gap_nodes > 1) 
  { 
    num_gaps[1]++;
    num_gap_nodes[1] += gap_nodes;

    node* q = V;
    node* p = V;
    while (*p)
    { node v = *p++;
      if (dist[v] >= dv)
        dist[v] = n;
      else 
        *q++ = v;
    }
    *q = 0;
  }

 gap_time += used_time(T);
}
   

void handle_gap_old(const graph& G, node v, dist_array& dist, succ_array& succ, int* count)
{

  float T = used_time();

  int n  = G.number_of_nodes();

  int above_gap = 1;

  int dv = dist[v];

  for (int i = dv+1; i < n && count[i] > 0; i++) 
  { above_gap += count[i];
    count[i] = 0;
  }

  dist[v] = n;

if (above_gap > 1)
{ 
  int gap_nodes = 0;

  node queue_last = v;
  succ[v] = v;

  do { gap_nodes++;

       v = succ[v];

       edge e;
       forall_out_edges(e,v)
       { node u = target(G,e,v);
         int du = dist[u];
         if (du < n && du > dv)
         { succ[queue_last] = u;
           queue_last = u; 
           dist[u] = n;
          }
       }

       forall_in_edges(e,v)
       { node u = source(G,e,v);
         int du = dist[u];
         if (du < n && du > dv)
         { succ[queue_last] = u;
           queue_last = u;
           dist[u] = n;
          }
        }

   } while (v != queue_last);

 num_gaps[1]++;
 num_gap_nodes[1] += gap_nodes;

}

 gap_time += used_time(T);
}



template<class cap_array, class flow_array>
void phase0(const graph& G, node* V, node s, node t, cap_array& cap,
                                                     flow_array& flow,
                                                     excess_array& excess,
                                                     dist_array& dist,
                                                     level_queue& U)
{ 
  int n = G.number_of_nodes();

  int relabel_count = 0;
  int push_count = 0;
  int insp_count = 0;

  excess[t] += 1;  // then t will never be inserted into U

  node v;
  while ((v = U.del_max()) != t) 
  {
    for(;;)
    {
      NT  ev = excess[v]; 
      int dv = dist[v]; 
  
      int  dmin = n;    
      edge emin = 0;
      NT   rmin = 0;
  
      edge e;
      forall_out_edges(e,v)
      { insp_count++;
        NT x = out_cap(e);
        if (x == 0) continue;
        node w = target(G,e,v);
        int dw = dist[w];
        if (dw == dv-1)
        { push_count++;
          NT ew = excess[w];
          if (ew == 0) U.insert_non_max(w,dw);
          if (ev <= x) 
          { change_flow(e,ev); 
            excess[w] += ev;
            excess[v] = 0; 
            goto NEXT_NODE0;
           }
          change_flow(e,x);
          excess[w] += x;
          ev -= x;
         }
        else
         if (v != w && dw < dmin) { dmin = dw; emin = e; rmin = x; }
      }
  
      excess[v] = ev;
  
      // remaining excess: relabel vertex v 
  
      relabel_count++;
      dv = dmin + 1;
      dist[v] = dv;
      if (dv >= n) goto NEXT_NODE0;
  
      if (ev <= rmin)
      { push_count++;
        node w = G.opposite(emin,v);
        if (excess[w] == 0) U.insert_non_max(w,dmin);
        excess[w]  += ev;
        change_flow(emin,ev)
        excess[v]  = 0;
        goto NEXT_NODE0;
       }
    }

   NEXT_NODE0:;
  }

  excess[t] -= 1;

  num_pushes[0] = push_count;
  num_relabels[0] = relabel_count;
  num_inspections[0] = insp_count;
}




template<class cap_array, class flow_array>
void phase1(const graph& G, node* V, node s, node t, cap_array& cap,
                                                     flow_array& flow,
                                                     excess_array& excess,
                                                     dist_array& dist,
                                                     int* count,
                                                     level_queue& U)
{ 
  int n = G.number_of_nodes();
  int m = G.number_of_edges();

  curr_array  current_index;
  current_index.use_node_data(G);

  edge f_edge = G.first_edge()-1;

  node v;
  forall_nodes(v,G) 
     current_index[v] = v->first_out_edge()-f_edge;


  int V_sz = n;

  int heuristic  = (h > 0) ? int(h*m) : int(-h*n);
  int limit_heur = heuristic;

  int relabel_count = 0;
  int push_count = 0;
  int insp_count = 0;

  excess[t] += 1;  // then t will never be inserted into U

  while ((v = U.del_max()) != t) 
  {
    int cur_i = current_index[v];

    for(;;)
    {
      NT  ev = excess[v]; 
      int dv = dist[v]; 
  
      edge e_out = v->stop_out_edge();
      edge e_in = v->first_in_edge();
  
      if (cur_i > 0) 
        e_out = f_edge + cur_i;
      else
        e_in = f_edge - cur_i;
  
      int  dmin = n;    
  
      edge e;
      for(e = e_out; e != v->stop_out_edge(); e = e->next_out_edge())
      { insp_count++;
        NT x = out_cap(e);
        if (x == 0) continue;
        node w = target(G,e,v);
        int dw = dist[w];
        if (dw == dv-1)
        { push_count++;
          NT ew = excess[w];
          if (ew == 0) U.insert_non_max(w,dw);
          if (ev <= x) 
          { change_flow(e,ev); 
            excess[w] += ev;
            excess[v] = 0; 
            cur_i = (e-f_edge);
            goto NEXT_NODE;
           }
          change_flow(e,x);
          excess[w] += x;
          ev -= x;
         }
        else if (dist[w] < dmin) { dmin = dist[w]; cur_i = (e-f_edge); }
       }

      for(e = e_in; e; e = e->next_in_edge())
      { insp_count++;
        NT x = in_cap(e);
        if (x == 0) continue;
        node w = source(G,e,v);
        int dw = dist[w];
        if (dw == dv-1)
        { push_count++;
          NT ew = excess[w];
          if (ew == 0) U.insert_non_max(w,dw);
          if (ev <= x) 
          { change_flow(e,-ev); 
            excess[w] += ev;
            excess[v] = 0; 
            cur_i = -(e-f_edge);
            goto NEXT_NODE;
           }
          change_flow(e,-x);
          excess[w] += x;
          ev -= x;
         }
       else if (dist[w] < dmin) { dmin = dist[w]; cur_i = -(e-f_edge); }
      }


      excess[v] = ev;
  
      // remaining excess at v
      // relabel vertex v (i.e. update dist[v]) because all
      // admissible edges in the residual graph have been saturated 
  
      if (--count[dv] == 0) // gap
      { //handle_gap_old(G,v,dist,succ,count);
        handle_gap(G,V,v,dist,count);
        goto NEXT_NODE;
       }


      if ((h > 0 && insp_count  >= limit_heur) || 
          (h < 0 && relabel_count >= limit_heur))
      { V_sz = compute_dist1(G,V,s,t,cap,flow,excess,dist,U,count);
        for(node* p = V;*p;p++) 
        { node u = *p;
          current_index[u] = (u->first_out_edge() - f_edge);
         }
        cur_i = current_index[v];
        limit_heur += heuristic;
        goto NEXT_NODE;
       }

      bool min_in_front = true;
      for(e = v->first_out_edge(); e != e_out; e = e->next_out_edge())
      { NT x = out_cap(e);
        if (x==0) continue;
        node w = target(G,e,v);
        int dw = dist[w];
        if (dw < dmin || (min_in_front && dw == dmin)) 
           { dmin = dw; cur_i = (e-f_edge); min_in_front = false; }
       }

      min_in_front = (cur_i < 0);
      for(e = v->first_in_edge(); e != e_in; e = e->next_in_edge())
      { NT x = in_cap(e);
        if (x==0) continue;
        node w = source(G,e,v);
        int dw = dist[w];
        if (dw < dmin || (min_in_front && dw == dmin)) 
          { dmin = dw; cur_i = -(e-f_edge); min_in_front = false; }
       }

      relabel_count++;
      dv = dmin + 1;
      dist[v] = dv;
      if (dv >= n) goto NEXT_NODE;
      assert(dv < V_sz);
      count[dv]++;
      U.set_max(dv);

    }

   NEXT_NODE:;
   current_index[v] = cur_i;
   if (cut_bound == excess[t]-1) break;
  }

  excess[t] -= 1;

  num_pushes[1] = push_count;
  num_relabels[1] = relabel_count;
  num_inspections[1] = insp_count;

  fval = excess[t];
}



template<class cap_array, class flow_array>
void phase2(const graph& G, node* V, node s, node t, cap_array& cap,
                                                     flow_array& flow,
                                                     excess_array& excess,
                                                     dist_array& dist,
                                                     level_queue& U)
{ 
  int n = G.number_of_nodes();
  int m = G.number_of_edges();
  
  int heuristic  = (h > 0) ? int(h*m) : int(-h*n);
  int limit_heur = heuristic;

  int relabel_count = 0;
  int push_count = 0;
  int insp_count = 0;

  node v;
  while ((v = U.del_max()) != s) 
  {
    NT  ev = excess[v]; 
    int dv = dist[v]; 
    int dmin = n;

    edge e;
    forall_in_edges(e,v)
    { insp_count++;
      NT x = in_cap(e);
      if (x == 0) continue;
      node w = source(G,e,v);
      int dw = dist[w];
      if (dw == dv-1)
       { push_count++;
         NT ew = excess[w];
         if (ew == 0) U.insert_non_max(w,dw);
         if (ev <= x) 
         { change_flow(e,-ev); 
           excess[w] += ev;
           ev = 0; 
           break;
          }
         else
         { change_flow(e,-x);
           excess[w] += x;
           ev -= x;
          }
        }
      else
       if (dw < dmin) dmin = dw;
    } 
    
    excess[v] = ev;
    if (ev == 0) continue;

    if ((h > 0 && insp_count    >= limit_heur) || 
        (h < 0 && relabel_count >= limit_heur))
      { compute_dist2(G,V,s,t,flow,excess,dist,U);
        limit_heur += heuristic;
       }
    else
      { relabel_count++;
        dv = dmin+1;
        dist[v] = dv;
        U.insert_max(v,dv);
       }
  }

  num_pushes[2] = push_count;
  num_relabels[2] = relabel_count;
  num_inspections[2] = insp_count;

}






template<class cap_array, class flow_array>
NT run(const graph& G, node s, node t, const cap_array& cap, flow_array& flow)
{ 
  float T = used_time();
  int n = G.number_of_nodes();

  if (s == t) error_handler(1,"MAXFLOW: source == sink");

  reset_counters();
  flow.init(G,0);

  excess_array excess;
  excess.use_node_data(G,0);

  succ_array succ;
  succ.use_node_data(G);

  dist_array dist;
  dist.use_node_data(G);

  level_queue U(G,succ);

  int*  count = new int[n];
  node* V = new node[n+1];

/*
  cap_array& cap = (cap_array&)cap0;
*/

  edge e;
  forall_out_edges(e,s) 
  { node u = target(G,e,s);
    NT c = out_cap(e);
    change_flow(e,c);
    excess[s] -= c;
    excess[u] += c;
  }


  compute_dist0(G,V,s,t,cap,flow,excess,dist,U);

  if (run_phase0)
  { phase0(G,V,s,t,cap,flow,excess,dist,U);
    cputime[0] = used_time(T);
   }
  
  compute_dist1(G,V,s,t,cap,flow,excess,dist,U,count,false);
  phase1(G,V,s,t,cap,flow,excess,dist,count,U);
  cputime[1] = used_time(T);

  compute_dist2(G,V,s,t,flow,excess,dist,U);
  phase2(G,V,s,t,cap,flow,excess,dist,U);
  cputime[2] = used_time(T);

/*
  edge e;
  forall_edges(e,G) cap[e] = total_cap(e);
*/

  delete[] count;
  delete[] V;

  return fval;
}


template<class cap_array, class flow_array>
NT operator()(const graph& G, node s, node t, 
              const cap_array& cap, flow_array& flow) 
{ 
  return run(G,s,t,cap,flow); 
 }



template<class cap_array, class flow_array>
bool compute_min_st_cut(const graph& G, node s, node t, const cap_array& cap, 
                        const flow_array& flow, list<node>& cut)
{
  node_array<bool,graph> reached(G,false);
  reached[s] = true;

  list<node> Q;
  Q.append(s); cut.append(s);

  while ( !Q.empty() )
  { node v = Q.pop(); 
    edge e;
    forall_out_edges(e,v) 
    { node w = target(G,e,v);
      if ( flow[e] < cap[e] && !reached[w] ) 
      { reached[w] = true; 
        Q.append(w); cut.append(w);
       }
    }
    forall_in_edges(e,v) 
    { node w = source(G,e,v);
      if ( flow[e] > 0 && !reached[w] ) 
      { reached[w] = true; 
        Q.append(w); cut.append(w);
       }
    }
  }

  if (reached[t]) { cut.clear(); return false; } // error
  return true;
}


void statistics(ostream& out)
{ 
 out << endl;
 out << "              pushes   relabels   inspects updates gaps g-nodes  time";
 out << endl;
 out << "---------------------------------------------------------------------";
 out << endl;

  for(int i=-1; i<=2; i++)
  { out << string("%9s %10d %10d %10d %7d %4d %7d  %.2f  ",
                   ~name(i),pushes(i), relabels(i), inspections(i), updates(i), 
                   gaps(i), gap_nodes(i), cpu_time(i));
    out << endl;
    if (i == -1)
    { out << "--------------------------------------------------------------------";
      out << endl;
     }
   }
  out << endl;
  out << string("update time: %.2f   ",update_time);
  out << string("gap time: %.2f   ",gap_time);
  out << string("cut upper bound: %d", cut_bound);
  out << endl;
}


void statistics1(ostream& out)
{ 
  out << endl;
  
  out << string("Maxflow statistics:\n\n");
  out << string("  flow          %8.2f\n", double(fval));
  out << string("  time          %8.2f\n", cpu_time(-1));
  out << string("  update_time   %8.2f\n", update_time);
  out << string("  gap_time      %8.2f\n", gap_time);
  out << string("  pushes        %8.2f\n", double(pushes(-1)));
  out << string("  relabels      %8.2f\n", double(relabels(-1)));
  out << string("  updates       %8.2f\n", double(updates(-1)));
  out << string("  inspections   %8.2f\n", double(inspections(-1)));
  out << string("  gaps          %8.2f\n", double(gaps(-1)));
  out << string("  gap_nodes     %8.2f\n", double(gap_nodes(-1)));
  
  out << endl;

  out << string("phase 0:\n");
  out << string("  p0_time        %8.2f\n", cputime[0]);
  out << string("  p0_pushes      %8.2f\n", double(pushes(0)));
  out << string("  p0_relabels    %8.2f\n", double(relabels(0)));
  out << string("  p0_updates     %8.2f\n", double(updates(0)));
  out << string("  p0_inspections %8.2f\n", double(inspections(0)));
  out << string("  p0_gaps        %8.2f\n", double(gaps(0)));
  out << string("  p0_gap_nodes   %8.2f\n", double(gap_nodes(0)));
  
  out << endl;
  
  out << string("phase 1:\n");
  out << string("  p1_time        %8.2f\n", cputime[1]);
  out << string("  p1_pushes      %8.2f\n", double(pushes(1)));
  out << string("  p1_relabels    %8.2f\n", double(relabels(1)));
  out << string("  p1_updates     %8.2f\n", double(updates(1)));
  out << string("  p1_inspections %8.2f\n", double(inspections(1)));
  out << string("  p1_gaps        %8.2f\n", double(gaps(1)));
  out << string("  p1_gap_nodes   %8.2f\n", double(gap_nodes(1)));
  
  out << endl;
  
  out << string("phase 2:\n");
  out << string("  p2_time        %8.2f\n", cputime[2]);
  out << string("  p2_pushes      %8.2f\n", double(pushes(2)));
  out << string("  p2_relabels    %8.2f\n", double(relabels(2)));
  out << string("  p2_updates     %8.2f\n", double(updates(2)));
  out << string("  p2_inspections %8.2f\n", double(inspections(2)));
  out << string("  p2_gaps        %8.2f\n", double(gaps(2)));
  out << string("  p2_gap_nodes   %8.2f\n", double(gap_nodes(2)));

  out << endl;
}




template<class cap_array, class flow_array>
bool check(const graph& G, node s, node t, 
           const cap_array& cap, const flow_array& flow,
           string& msg)
{
  edge e;
  forall_edges(e,G) 
  { if (flow[e] < 0 || flow[e] > cap[e]) 
    { msg = string("illegal flow value for edge %d",G.index(e));
      return false;
     }
   }
  
  excess_array excess(G);

  node v;
  forall_nodes(v,G) excess[v] = 0;

  forall_nodes(v,G) 
  { edge e;
    forall_out_edges(e,v) 
    { node w = target(G,e,v);
      excess[v] -= flow[e]; 
      excess[w] += flow[e];
     }
  }

  forall_nodes(v,G) 
  { if (v == s  || v == t || excess[v] == 0) continue;
    msg = "node with non-zero excess";
    return false;
  }

/*
  if (fval != excess[t])
  { msg = "fval != excess[t]";
    return false;
   }
*/
  
  node_array<bool,graph> reached(G,false);
  reached[s] = true;

  list<node> Q;
  Q.append(s); 

  while ( !Q.empty() )
  { node v = Q.pop(); 
    forall_out_edges(e,v) 
    { node w = target(G,e,v);
      if ( flow[e] < cap[e] && !reached[w] ) 
      { reached[w] = true; 
        Q.append(w); 
       }
    }
    forall_in_edges(e,v) 
    { node w = source(G,e,v);
      if ( flow[e] > 0 && !reached[w] ) 
      { reached[w] = true; 
        Q.append(w); 
       }
    }
  }

  if (reached[t])
  { msg = "t is reachable in G_f";
    return false;
   }

  forall_nodes(v,G)
  { if (v != s && v != t && excess[v] != 0)
    { msg = string("node %d has non-zero excess",G.index(v));
      return false;
     }
   }

  return true;
}


template <class cap_array, class flow_array>
void print(const graph& G, node s, node t, 
           const cap_array& cap, const flow_array& flow)
{
  cout << "s = " << G.index(s) << endl;
  cout << "t = " << G.index(t) << endl;
  cout << endl;

  node v;
  forall_nodes(v,G)
  { cout << string("%2d :",G.index(v));
    edge e;
    forall_out_edges(e,v)
      cout << string(" --%d/%d-->%d",flow[e],cap[e],G.index(target(G,e,v)));
    cout << endl;
   }
  cout << endl;
}
  
 
};

#ifndef LEDA_NO_TEMPL_FCTS_FOR_STD_GRAPH

template<class NT>
NT MAX_FLOW_T(const graph& G, node s, node t, const edge_array<NT>& cap, 
                                              edge_array<NT>& flow, 
                                              int& num_pushes, 
                                              int& num_edge_inspections,
                                              int& num_relabels,
                                              int& num_global_relabels,
                                              int& num_gaps,
                                              float h)
{ max_flow<NT,graph> mf;
  mf.set_heuristic(h);
  NT f = mf.run(G,s,t,cap,flow);
  num_pushes = mf.pushes(0);
  num_relabels = mf.relabels(0);
  num_global_relabels = mf.updates(0);
  num_edge_inspections = mf.inspections(0);
  num_gaps = mf.gap_nodes(0);
  return f;
}



template<class NT>
NT MAX_FLOW_T(const graph& G, node s, node t, const edge_array<NT>& cap, 
                                              edge_array<NT>& flow)
{ max_flow<NT,graph> mf;
  return mf.run(G,s,t,cap,flow);
}


template<class NT>
NT MAX_FLOW_T(const graph& G, node s, node t, const edge_array<NT>& cap, 
              edge_array<NT>& flow, list<node>& st_cut)
{ max_flow<NT,graph> mf;
  NT val = mf.run(G,s,t,cap,flow);
  mf.compute_min_st_cut(G, s, t, cap, flow, st_cut);
  return val;
}


template <class NT>
bool CHECK_MAX_FLOW_T(const graph& G, node s, node t,
                      const edge_array<NT>& cap, const edge_array<NT>& flow)
{ max_flow<NT,graph> mf;
  string msg;
  bool ok = mf.check(G,s,t,cap,flow,msg);
  if (!ok) error_handler(1,string("CHECK_MAX_FLOW_T: ") + msg + ".");
  return ok;
}

#endif // #ifdef LEDA_NO_TEMPL_FCTS_FOR_STD_GRAPH
  
LEDA_END_NAMESPACE

#if !defined(_LEDA_MAX_FLOW_WITH_LCAPS_T)
#include <LEDA/graph/templates/max_flow_with_lcaps.h>
#endif

#if LEDA_ROOT_INCL_ID == 500343
#undef LEDA_ROOT_INCL_ID
#include <LEDA/internal/POSTAMBLE.h>
#endif

#endif
