/*******************************************************************************
+
+  LEDA 5.0.1  
+
+
+  feasible_flow.h
+
+
+  Copyright (c) 1995-2005
+  by Algorithmic Solutions Software GmbH
+  All rights reserved.
+ 
*******************************************************************************/

// $Revision: 1.3 $  $Date: 2005/04/14 10:45:14 $

#if !defined(LEDA_ROOT_INCL_ID)
#define LEDA_ROOT_INCL_ID 500338
#include <LEDA/internal/PREAMBLE.h>
#endif

//-----------------------------------------------------------------------------
//
// FEASIBLE FLOW
//
// preflow-push + highest level
//
//-----------------------------------------------------------------------------

#include <LEDA/system/assert.h>


LEDA_BEGIN_NAMESPACE


template<class graph_t>
class all_edges_filter {
public:
typedef typename graph_t::node node;
typedef typename graph_t::edge edge;

bool operator()(edge,node,node) { return true; }
};




#define NULL_NODE  ((node)0xFFFFFFFF)

template <class graph, class succ_array>
class ff_node_level_queue {

typedef typename graph::node node;
typedef typename graph::edge edge;

node*  T;
node*  head;
node*  max;

succ_array& NEXT;

public:



ff_node_level_queue(const graph& G, succ_array& succ) : NEXT(succ)
{ 
  int sz = G.number_of_nodes();
  T = new node[sz+1];
  for(node* p = T+sz; p >= T;  p--) *p = nil; 
  head = T+1;
  max = T;
  T[0] = NULL_NODE; 
 }

~ff_node_level_queue() { delete[] T; }

void set_max(int i) { max = head + i; }

void insert(node v, int i)
{ // insert on level i
  node* h = head + i;
  if (h > max) max = h;
  NEXT[v] = *h;
  *h = v;
 }


void insert_non_max(node v, int i)
{ // do not update max pointer
  node* h = head + i;
  NEXT[v] = *h;
  *h = v;
 }


void insert_max(node v, int i)
{ // new max at level i
  max = head + i;
  *max = v;
  NEXT[v] = 0;
 }

node find_max()
{ while (*max == nil) max--;
  return *max;
}

node del_max()
{ node v = *max;
  *max = NEXT[v];
  return v; 
}

void del_max(node v) { *max = NEXT[v]; }


void clear() { while (max >= head) *max-- =  nil; }


};



template<class NT, class graph, class succ_array, class dist_array>
class feasible_flow  
{
  const int node_slots;
  const int edge_slots;

  typedef typename graph::node node;
  typedef typename graph::edge edge;


  float h;

  // statistics

  int num_pushes;
  int num_relabels;
  int num_updates;
  int num_inspections;

  float cputime;

  NT fval;

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

public:

feasible_flow() : node_slots(3), edge_slots(0), h(5.0)
{ reset_counters(); }

int pushes()      { return num_pushes; }
int relabels()    { return num_relabels; }
int updates()     { return num_updates; }
int inspections() { return num_inspections; }
float cpu_time()  { return cputime; }

void set_heuristic(float hh) { h = hh; }


void reset_counters()
{ num_pushes = 0;
  num_relabels = 0;
  num_updates = 0;
  num_inspections = 0;
  cputime = 0;
}


template<class cap_array, class flow_array, class excess_array, 
                                            class n_level_queue,
                                            class edge_filter>
void compute_dist(const graph& G, 
                   const list<node>& demand_nodes,
                   const cap_array& cap,
                   const flow_array& flow,
                   excess_array& excess,
                   succ_array& succ,
                   dist_array& dist,
                   n_level_queue& U,
                   edge_filter& filt)
{ 
  // compute exact distance values by a "backward" bfs in the
  // residual network starting at demand nodes

  num_updates++;

  int n = G.number_of_nodes();

  node v;
  forall_nodes(v,G) dist[v] = n;

  node level_last  = 0;
  node queue_last  = 0;
  node queue_first = 0;

  forall(v,demand_nodes)
  { 
    if (excess[v] >= 0)  continue;

    dist[v] = 0;
    if (queue_last == 0)
      { queue_first= v;
        queue_last = v;
        level_last = v;
       }
    else
      { succ[queue_last] = v;
        queue_last = v;
        level_last = v;
       }
   }
  

  if (!queue_first) return;

  int d = 0;

  v = queue_first;

  for(;;)
  { 
    edge e;
    forall_out_edges(e,v)
    { if (flow[e] == 0) continue;
      node u = target(G,e,v); 
      if (!filt(e,v,u)) continue;
      int& du = dist[u];
      if (du == n)
      { du = d+1;
        succ[queue_last] = u;
        queue_last = u;
       }
     }

    forall_in_edges(e,v)
    { if (cap[e] == flow[e]) continue;
      node u = source(G,e,v); 
      if (!filt(e,u,v)) continue;
      int& du = dist[u];
      if (du == n)
      { du = d+1;
        succ[queue_last] = u;
        queue_last = u;
       }
    }

    node next = succ[v];

    if (excess[v] > 0) U.insert(v,d);

    if (v == level_last)             
    { // finish level d
      if (queue_last == level_last) break; // no level d+1: stop
      level_last = queue_last;
     }
    v = next;
  }
}



template<class excess_array, class cap_array, class flow_array,
                                              class edge_filter>
bool feasible(const graph& G, excess_array& excess, const cap_array& cap, 
                                               flow_array& flow,
                                               edge_filter& filt)
{ 

  float T = used_time();

  reset_counters();

  succ_array succ;
  succ.use_node_data(G,0);

  int n = G.number_of_nodes();
  int m = G.number_of_edges();

  int heuristic  = (h > 0) ? int(h*m) : int(-h*n);
  int limit_heur = heuristic;

  int relabel_count = 0;
  int push_count = 0;
  int insp_count = 0;

  list<node> demand_nodes;

  NT total_excess = 0;

  node v;
  forall_nodes(v,G) 
  { NT s = excess[v];
    if (s < 0) demand_nodes.append(v);
    total_excess += s;
   }

  if (total_excess != 0) return false;

  if (demand_nodes.empty()) return true;

  dist_array dist;
  dist.use_node_data(G,0);

  ff_node_level_queue<graph,succ_array>  U(G,succ);

  compute_dist(G,demand_nodes,cap,flow,excess,succ,dist,U,filt);

  while ((v = U.find_max()) != NULL_NODE) 
  {


    U.del_max(v);

    NT  ev = excess[v]; 
    int dv = dist[v]; 

    int  dmin = n;    
    edge emin = 0;
    NT   rmin = 0;

    edge e;
    forall_out_edges(e,v)
    { insp_count++;
      NT fe = flow[e];
      NT rc = cap[e] - fe;
      if (rc == 0) continue;
      node w = target(G,e,v);
      if (!filt(e,v,w)) continue;
      int dw = dist[w];
      if (dw == dv-1)
      { push_count++;
        NT ew = excess[w];
        if (ev <= rc) 
        { flow[e] = fe + ev; 
          excess[w] = ew + ev;
          excess[v] = 0; 
          if (ew <= 0 && excess[w] > 0) U.insert_non_max(w,dw);
          goto NEXT_NODE;
         }
        flow[e] = fe + rc;
        excess[w] = ew + rc;
        ev -= rc;
        if (ew <= 0 && excess[w] > 0) U.insert_non_max(w,dw);
        continue;
       }
      if (v != w && dw < dmin) { dmin = dw; emin = e; rmin = rc; }
    }

    forall_in_edges(e,v)
    { insp_count++;
      NT fe = flow[e];
      if (fe == 0) continue;
      node w = source(G,e,v);
      if (!filt(e,w,v)) continue;
      int dw = dist[w];
      if (dw == dv-1)
      { push_count++;
        NT ew = excess[w];
        if (ev <= fe) 
        { flow[e] = fe - ev; 
          excess[w] = ew + ev;
          excess[v] = 0; 
          if (ew <= 0 && excess[w] > 0) U.insert_non_max(w,dw);
          goto NEXT_NODE;
         }
        flow[e] = 0;
        excess[w] = ew + fe;
        ev -= fe;
        if (ew <= 0 && excess[w] > 0) U.insert_non_max(w,dw);
        continue;
       }
      if (v != w && dw < dmin) { dmin = dw; emin = e; rmin = -fe; }
    } 

    excess[v] = ev;

    // remaining excess at v
    // relabel vertex v (i.e. update dist[v]) because all
    // admissible edges in the residual graph have been saturated 

    relabel_count++;

    dv = dmin + 1;
    dist[v] = dv;

    if (dv >= n) goto NEXT_NODE;

    if (ev <= rmin || ev <= -rmin)
    { node w = G.opposite(emin,v);
      NT ew = excess[w];
      excess[w] = ew + ev;
      flow[emin] += (rmin > 0) ? ev : -ev;
      excess[v]  = 0;
      if (ew <= 0 && excess[w] > 0) U.insert(w,dmin); // possible new max
      goto NEXT_NODE;
     }

    if ((h > 0 && insp_count    >= limit_heur) || 
        (h < 0 && relabel_count >= limit_heur))
    { 
      U.clear();
      compute_dist(G,demand_nodes,cap,flow,excess,succ,dist,U,filt);
      limit_heur += heuristic;
      goto NEXT_NODE;
     }


   U.insert_max(v,dv);

NEXT_NODE:;
  }

  num_pushes = push_count;
  num_relabels = relabel_count;
  num_inspections = insp_count;

  int excess_count = 0;

  forall_nodes(v,G) 
     if (excess[v] != 0) excess_count++;

  cputime = used_time(T);

  return excess_count == 0;

}


template<class excess_array, class cap_array, class flow_array>
bool feasible(const graph& G, excess_array& excess, const cap_array& cap, 
                                                    flow_array& flow)
{ all_edges_filter<graph> filt;
  return feasible(G,excess,cap,flow,filt);
}


template<class cap_array, class flow_array>
void maxflow(const graph& G, node s, node t, const cap_array& cap, 
                                             flow_array& flow)
{
  node_array<int,graph> excess(G);

  node v;
  forall_nodes(v,G) excess[v] = 0;

  int total_excess = 0;
  int total_demand = 0;

  edge e;
  forall_out_edges(e,s) total_excess += cap[e];

  forall_in_edges(e,t) total_demand += cap[e];

  if (total_excess > total_demand) total_excess = total_demand;

  excess[s] = total_excess;
  excess[t] = -total_excess;

  cout << endl;
  cout << "excess = " << total_excess << endl;

  feasible(G,excess,cap,flow);

  int ex = 0;
  forall_nodes(v,G) 
  { if (excess[v] > 0) ex += excess[v];
   }

  fval = total_excess-ex;
}


NT flow() { return fval; }

void statistics(ostream& out)
{ 
  out << string("%8d pushes %7d relabels %3d updates %9d insp  %.2f s",
                  pushes(), relabels(), updates(), inspections(), cpu_time());
  out << endl;
}


};


LEDA_END_NAMESPACE

#if LEDA_ROOT_INCL_ID == 500338
#undef LEDA_ROOT_INCL_ID
#include <LEDA/internal/POSTAMBLE.h>
#endif
