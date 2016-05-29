/*******************************************************************************
+
+  LEDA 5.0.1  
+
+
+  max_flow0.t
+
+
+  Copyright (c) 1995-2005
+  by Algorithmic Solutions Software GmbH
+  All rights reserved.
+ 
*******************************************************************************/

// $Revision: 1.2 $  $Date: 2005/04/14 10:45:15 $

#if !defined(LEDA_ROOT_INCL_ID)
#define LEDA_ROOT_INCL_ID 500340
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


template <class graph, class succ_array>
class node_level_queue {

typedef typename graph::node node;
typedef typename graph::edge edge;

node*  T;
node*  head;
node*  max;

succ_array& NEXT;

public:

node_level_queue(const graph& G, succ_array& succ) : NEXT(succ)
{ int sz = G.number_of_nodes();
  T = new node[sz+1];
  for(node* p = T+sz; p >= T;  p--) *p = nil; 
  head = T+1;
  max = T;
 }

~node_level_queue() { delete[] T; }

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

node del_max() 
{ while (*max == nil) max--;
  node v = *max;
  *max = NEXT[v];
  return v; 
}


void clear() { while (max >= T) *max-- =  nil; }

};



template<class NT, class graph,
#if !defined(_MSC_VER)
         class succ_array   = node_array<typename graph::node,graph>,
#else
         class succ_array   = node_array<graph::node,graph>,
#endif
         class excess_array = node_array<NT, graph>,
         class dist_array   = node_array<int,graph> >
class max_flow  
{
  const int node_slots;
  const int edge_slots;

  typedef typename graph::node node;
  typedef typename graph::edge edge;


  float h;

  NT fval;

  // statistics (phase 1, 2, and sum)

  int num_pushes[3];
  int num_relabels[3];
  int num_updates[3];
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

public:

max_flow() : node_slots(3), edge_slots(0), h(5.0),fval(0)
{ reset_counters(); }

int pushes(int i=0)      { return num_pushes[i]; }
int relabels(int i=0)    { return num_relabels[i]; }
int updates(int i=0)     { return num_updates[i]; }
int gap_nodes(int i=0)   { return num_gap_nodes[i]; }
int inspections(int i=0) { return num_inspections[i]; }

float cpu_time(int i=0) { return cputime[i]; }

void set_heuristic(float hh) { h = hh; }


NT flow() { return fval; }


void reset_counters()
{ for(int i=0; i<3; i++) 
  { num_pushes[i] = 0;
    num_relabels[i] = 0;
    num_updates[i] = 0;
    num_gap_nodes[i] = 0;
    num_inspections[i] = 0;
    cputime[i] = 0;
    update_time = 0;
    gap_time = 0;
   }
}




void compute_dist0(const graph& G, node s, node t,
                   excess_array& excess,
                   succ_array& succ,
                   dist_array& dist,
                   node_level_queue<graph,succ_array>& U, int* count)
{ 
  // compute exact distance values by a "backward" bfs in the
  // residual network starting at t

  float T = used_time();

  num_updates[1]++;

  int n = G.number_of_nodes();

  for(int i=0; i<n; i++) count[i] = 0;

  if (h < 0)
  { U.insert(t,0);
    node v;
    forall_nodes(v,G)
    { dist[v] = 0;
      if (v != t && excess[v] > 0) U.insert(v,0);
     }
    count[0] = n-1;
    dist[s] = n;
    return;
   }


  dist.init(G,n);

  excess[t] += 1;

  dist[t] = 0;
  dist[s] = 0;

  node v = t;
  node level_last = t;
  node queue_last = t;


  int d = 0;
  int level_sz = 0;

  for(;;)
  { 
    level_sz++;

    edge e;
    forall_in_edges(e,v)
    { node u = source(G,e,v); 
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
      count[d++] = level_sz;
      level_sz = 0;
      if (queue_last == level_last) break; // no level d+1: stop
      level_last = queue_last;
     }
    v = next;
  }

 dist[s] = n;
 excess[t] -= 1;

 update_time += used_time(T);
}




template<class cap_array, class flow_array, class n_level_queue>
void compute_dist1(const graph& G, node s, node t,
                   const cap_array& cap,
                   const flow_array& flow,
                   excess_array& excess,
                   succ_array& succ,
                   dist_array& dist,
                   n_level_queue& U,
                   int* count)
{ 
  // compute exact distance values by a "backward" bfs in the
  // residual network starting at t

  float T = used_time();

  num_updates[1]++;

  int n = G.number_of_nodes();
  dist.init(G,n);

  dist[t] = 0;
  excess[t] += 1; // we want t to be in U (the only node on level 0)

  node v = t;
  node level_last = t;
  node queue_last = t;


  int d = 0;
  int level_sz = 0;

  for(;;)
  { 
    level_sz++;

    edge e;
    forall_out_edges(e,v)
    { if (flow[e] == 0) continue;
      node u = target(G,e,v); 
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
      count[d++] = level_sz;
      level_sz = 0;
      if (queue_last == level_last) break; // no level d+1: stop
      level_last = queue_last;
     }
    v = next;
  }

 while (d < n) count[d++] = 0;

 assert(dist[s] == n);

 excess[t] -= 1;

 update_time += used_time(T);
}



template<class flow_array, class n_level_queue>
void compute_dist2(const graph& G, node s, node t,
                   const flow_array& flow,
                   const excess_array& excess,
                   succ_array& succ,
                   dist_array& dist,
                   n_level_queue& U)
{ 
  float T = used_time();

  num_updates[2]++;

  int n = G.number_of_nodes();

  dist.init(G,n);

  node v = s;
  node level_last = s;
  node queue_last = s;
  dist[s] = 0;

  U.insert(t,0);

  int d = 0;

  for(;;)
  { 
    edge e;
    forall_out_edges(e,v)
    { if (flow[e] == 0) continue;
      node u = target(G,e,v); 
      int& du = dist[u];
      if (du == n)
      { du = d+1;
        succ[queue_last] = u;
        queue_last = u;
       }
     }

    node next = succ[v];

    if (excess[v] > 0 && v != t) U.insert(v,d);

    if (v == level_last)             
    { if (queue_last == level_last) break;
      level_last = queue_last;
      d++;
     }
    v = next;
  }

 update_time +=  used_time(T);
}




void handle_gap(const graph& G, node v, dist_array& dist, succ_array& succ, int* count)
{

  float T = used_time();

  int gap_nodes = 0;

  int n  = G.number_of_nodes();
  int dv = dist[v];
  for (int i = dv; i < n && count[i] > 0; i++) count[i] = 0;

  dist[v] = n;

  node queue_last = v;
  succ[v] = v;

  do { gap_nodes++;

       v = succ[v];

       edge e;
       forall_out_edges(e,v)
       { node u = target(G,e,v);
         int& du = dist[u];
         if (du < n && du > dv)
         { succ[queue_last] = u;
           queue_last = u; 
           du = n;
          }
       }

       forall_in_edges(e,v)
       { node u = source(G,e,v);
         int& du = dist[u];
         if (du < n && du > dv)
         { succ[queue_last] = u;
           queue_last = u;
           du = n;
          }
        }

   } while (v != queue_last);

 num_gap_nodes[1] += gap_nodes;

 gap_time += used_time(T);
}



template<class cap_array, class flow_array>
void phase1(const graph& G, node s, node t,
            const cap_array& cap,
            flow_array& flow,
            excess_array& excess,
            succ_array& succ)
{ 
  int n = G.number_of_nodes();
  int m = G.number_of_edges();

  int heuristic  = (h > 0) ? int(h*m) : int(-h*n);
  int limit_heur = heuristic;

  int relabel_count = 0;
  int push_count = 0;
  int insp_count = 0;

  edge e;
  forall_out_edges(e,s) 
  { node u = target(G,e,s);
    NT c = cap[e];
    flow[e] = c;
    excess[s] -= c;
    excess[u] += c;
  }

  dist_array dist;
  dist.use_node_data(G);

  node_level_queue<graph,succ_array>  U(G,succ);
  int* count = new int[n];

  compute_dist0(G,s,t,excess,succ,dist,U,count);

  excess[t] += 1;  // prevents t from beeing inserted (again)

  node v;
  while ((v = U.del_max()) != t)
  {
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
      int dw = dist[w];
      if (dw == dv-1)
      { push_count++;
        NT ew = excess[w];
        if (ew == 0) U.insert_non_max(w,dw);
        if (ev <= rc) 
        { flow[e] = fe + ev; 
          excess[w] = ew + ev;
          excess[v] = 0; 
          goto NEXT_NODE;
         }
        flow[e] = fe + rc;
        excess[w] = ew + rc;
        ev -= rc;
        continue;
       }
      if (v != w && dw < dmin) { dmin = dw; emin = e; rmin = rc; }
    }

    forall_in_edges(e,v)
    { insp_count++;
      NT fe = flow[e];
      if (fe == 0) continue;
      node w = source(G,e,v);
      int dw = dist[w];
      if (dw == dv-1)
      { push_count++;
        NT ew = excess[w];
        if (ew == 0) U.insert_non_max(w,dw);
        if (ev <= fe) 
        { flow[e] = fe - ev; 
          excess[w] = ew + ev;
          excess[v] = 0; 
          goto NEXT_NODE;
         }
        flow[e] = 0;
        excess[w] = ew + fe;
        ev -= fe;
        continue;
       }
      if (v != w && dw < dmin) { dmin = dw; emin = e; rmin = -fe; }
    } 

    excess[v] = ev;

    // remaining excess at v
    // relabel vertex v (i.e. update dist[v]) because all
    // admissible edges in the residual graph have been saturated 

    if (--count[dv] == 0) // gap
    { handle_gap(G,v,dist,succ,count);
      goto NEXT_NODE;
     }

    relabel_count++;

    dv = dmin + 1;
    dist[v] = dv;

    if (dv >= n) goto NEXT_NODE;

    count[dv]++;

    if (ev <= rmin || ev <= -rmin)
    { node w = G.opposite(emin,v);
      if (excess[w] == 0) U.insert(w,dmin);
      excess[w]  += ev;
      flow[emin] += (rmin > 0) ? ev : -ev;
      excess[v]  = 0;
      goto NEXT_NODE;
     }


    if ((h > 0 && insp_count    >= limit_heur) || 
        (h < 0 && relabel_count >= limit_heur))
    { 
      U.clear();
      compute_dist1(G,s,t,cap,flow,excess,succ,dist,U,count);
      limit_heur += heuristic;
      goto NEXT_NODE;
     }

   U.insert_max(v,dv);

NEXT_NODE:;
  }

  excess[t] -= 1;

  num_pushes[1] += push_count;
  num_relabels[1] += relabel_count;
  num_inspections[1] += insp_count;

  delete[] count;

  fval = excess[t];
}



template<class cap_array, class flow_array>
void phase2(const graph& G, node s, node t,
            const cap_array& cap,
            flow_array& flow,
            excess_array& excess,
            succ_array& succ)
{ 
  int n = G.number_of_nodes();
  int m = G.number_of_edges();
  
  int heuristic  = (h > 0) ? int(h*m) : int(-h*n);
  int limit_heur = heuristic;

  int relabel_count = 0;
  int push_count = 0;
  int insp_count = 0;

  node_level_queue<graph,succ_array>  U(G,succ);

  dist_array dist;
  dist.use_node_data(G);

  compute_dist2(G,s,t,flow,excess,succ,dist,U);

  node v;
  while ((v = U.del_max()) != t)
  {
    NT  ev = excess[v]; 
    int dv = dist[v]; 
    int dmin = n;

    edge e;
    forall_in_edges(e,v)
    { insp_count++;
      NT fe = flow[e];
      if (fe == 0) continue;
      node w = source(G,e,v);
      int dw = dist[w];
      if (dw == dv-1)
       { push_count++;
         NT ew = excess[w];
         if (ew == 0) U.insert_non_max(w,dw);
         if (ev <= fe) 
         { flow[e] = fe - ev; 
           excess[w] = ew + ev;
           ev = 0; 
           break;
          }
         else
         { flow[e] = 0;
           excess[w] = ew + fe;
           ev -= fe;
          }
         continue;
        }
      if (dw < dmin) dmin = dw;
    } 
    
    excess[v] = ev;
    if (ev == 0) continue;

    if ((h > 0 && insp_count    >= limit_heur) || 
        (h < 0 && relabel_count >= limit_heur))
      { 
        U.clear();
        compute_dist2(G,s,t,flow,excess,succ,dist,U);
        limit_heur += heuristic;
       }
    else
      { relabel_count++;
        dv = dmin+1;
        dist[v] = dv;
        U.insert_max(v,dv);
       }
  }

  num_pushes[2] += push_count;
  num_relabels[2] += relabel_count;
  num_inspections[2] += insp_count;

}






template<class cap_array, class flow_array>
NT run(const graph& G, node s, node t, 
       const cap_array& cap, 
       flow_array& flow)
{ 
  if (s == t) error_handler(1,"MAXFLOW: source == sink");

  float T = used_time();

  reset_counters();

  excess_array excess;
  excess.use_node_data(G,0);

  succ_array succ;
  succ.use_node_data(G);

  flow.init(G,0);

  phase1(G,s,t,cap,flow,excess,succ);
  cputime[1] = used_time(T);

  phase2(G,s,t,cap,flow,excess,succ);
  cputime[2] = used_time(T);

  num_pushes[0]      = num_pushes[1]      + num_pushes[2];
  num_relabels[0]    = num_relabels[1]    + num_relabels[2];
  num_updates[0]     = num_updates[1]     + num_updates[2];
  num_gap_nodes[0]   = num_gap_nodes[1]   + num_gap_nodes[2];
  num_inspections[0] = num_inspections[1] + num_inspections[2];

  cputime[0] = cputime[1] + cputime[2];

  return fval;
}


template<class cap_array, class flow_array>
NT operator()(const graph& G, node s, node t, 
              const cap_array& cap, flow_array& flow) 
{ 
  return run(G,s,t,cap,flow); 
 }




void statistics(ostream& out)
{ 
  out << endl;

  for(int i=1; i<=2; i++)
  { out << 
    string("%8d pushes %7d relabels %3d updates %9d insp %5d gaps  %.2f s",
            pushes(i), relabels(i), updates(i), inspections(i), gap_nodes(i),
            cputime[i]) << endl;
  }

  cout << string("update: %.2f   gaps: %.2f",update_time,gap_time) << endl;

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


template <class NT>
bool CHECK_MAX_FLOW_T(const graph& G, node s, node t,
                      const edge_array<NT>& cap, const edge_array<NT>& flow)
{ max_flow<NT,graph> mf;
  string msg;
  bool ok = mf.check(G,s,t,cap,flow,msg);
  if (!ok) error_handler(1,string("CHECK_MAX_FLOW_T: ") + msg + ".");
  return ok;
}
  


LEDA_END_NAMESPACE

#if LEDA_ROOT_INCL_ID == 500340
#undef LEDA_ROOT_INCL_ID
#include <LEDA/internal/POSTAMBLE.h>
#endif
