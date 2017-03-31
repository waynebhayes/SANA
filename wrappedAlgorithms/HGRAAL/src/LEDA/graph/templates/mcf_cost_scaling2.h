#if !defined(LEDA_ROOT_INCL_ID)
#define LEDA_ROOT_INCL_ID 500353
#include <LEDA/internal/PREAMBLE.h>
#endif

#include <LEDA/graph/graph_alg.h>
#include <LEDA/core/b_queue.h>
#include <LEDA/core/b_stack.h>
#include <LEDA/internal/std/math.h>
#include <LEDA/system/assert.h>

#define MAXPOT MAXDOUBLE

/*
#define COST(e)      (e->usr_data << 16)
*/

#define COST(e)      cost[e]
#define rcost(e,v,w) (rcost_t(COST(e)) - pot[v] + pot[w])

/*
#define out_cap(e)      cap[e]
#define in_cap(e)       flow[e]
#define total_cap(e)    (cap[e]+flow[e])
#define push_flow(e,x)  { flow[e] += x; cap[e] -= x; }
*/

#define out_cap(e)      (cap[e]-flow[e])
#define in_cap(e)       flow[e]
#define total_cap(e)    cap[e]
#define push_flow(e,x)  { flow[e] += x; }


#define FINAL_PUSH


LEDA_BEGIN_NAMESPACE

/*
class counter  {
public:
  counter(int) {}
  void operator++(int) {}
  operator int() { return 0; }
};
*/

typedef int counter;


template<class graph_t, class cap_array>
class edge_smaller {

typedef typename graph_t::edge edge;

const cap_array&  cap;

public:

edge_smaller(const cap_array& c) : cap(c) {}

bool operator()(edge x, edge y) const { return (cap[x] > cap[y]); }

};



template<class graph_t, class cap_array, class flow_array, 
                        class cost_array, class pot_array, class rcost_t>
class edge_filter {

typedef typename graph_t::node node;
typedef typename graph_t::edge edge;

const graph_t&    G;
const cap_array&  cap;
const flow_array& flow;
const cost_array& cost;
const pot_array&  pot;
double af_bound;


public:

edge_filter(const graph_t& g, const cap_array& ca, const flow_array& f, 
                              const cost_array& c, const pot_array& p, 
                              double af, int eps) : G(g), cap(ca), flow(f), 
                                                          cost(c), pot(p)
{
  af_bound = af*eps;
}


bool operator()(edge e, node u)
{ node v = G.opposite(e,u);
  rcost_t rc = rcost(e,u,v);
  return ((rc > af_bound && in_cap(e) == 0) || 
          (rc < -af_bound && out_cap(e) == 0));
 }
};




template <class graph, class succ_array, class pred_array>
class node_level_queue {

typedef typename graph::node node;
typedef typename graph::edge edge;

succ_array& NEXT;
pred_array& PRED;

node   senti;
node*  T;
int    max; 

public:

node_level_queue(succ_array& succ, pred_array& pred, int n, node s)
            : NEXT(succ),PRED(pred), senti(s)
{ T = new node[n];
  for(int i=0; i<n; i++) T[i] = s;
  max = -1;
  NEXT[s] = s;
  PRED[s] = s;
 }

~node_level_queue() { delete[] T; }

int   max_level() const { return max; }
node  sentinel() const { return senti; }

void reset() { max = -1; }

void push(node v, int i)
{ node u = T[i];
  NEXT[v] = u;
  PRED[u] = v;
  T[i] = v;
  if (i > max) max = i;
 }

node top(int i) const   { return T[i]; }

void pop(node v, int i) { T[i] = NEXT[v]; }

void remove(node v, int i)
{ if (v == T[i])
    pop(v,i);
  else
  { node n = NEXT[v];
    node p = PRED[v];
    NEXT[p] = n;
    PRED[n] = p;
   }
}

};



template <class CAT>
struct Opposite {

  template <class graph, class edge, class node>
  static node source(const graph& G, edge e, node) { return G.source(e); }
  
  template <class graph, class edge, class node>
  static node target(const graph& G, edge e, node) { return G.target(e); }

};


template <>
struct Opposite<opposite_graph> {

  template <class graph, class edge, class node>
  static node source(const graph& G, edge e, node t) 
  { return G.opposite(e,t); }
  
  template <class graph, class edge, class node>
  static node target(const graph& G, edge e, node s) 
  { return G.opposite(e,s); }
};



template <class graph_t>
class zero_cap {
typedef typename graph_t::edge edge;
public:
int operator[](edge) const { return 0; }
};


template <class T>
class zero_queue {
public:
void append(const T&)  {}
void push(const T&)  {}
void clear() {}
};


template <class NT, class graph_t,
                    class pot_array,
                    class excess_array,
                    class dist_array,
                    class succ_array,
                    class pred_array>


class mcf_cost_scaling 
{
  typedef typename graph_t::node node;
  typedef typename graph_t::edge edge;
  typedef typename pot_array::value_type rcost_t;

  int   alpha;
  int   beta;
  float f_update;
  double af_factor;

  int inf;
  
  int num_nodes;
  int num_edges;
  int num_discharges;
  int num_pushes;
  int num_final_pushes;
  int num_relabels;
  int num_updates;
  int num_arcfixes;
  int num_restarts;

  int max_level;
 
  bool write_log;

  float T;


node source(const graph_t& G, edge e, node t)
{ return Opposite<typename graph_t::category_tag>::source(G,e,t); }

node target(const graph_t& G, edge e, node s) 
{ return Opposite<typename graph_t::category_tag>::target(G,e,s); }


void error_handler(int,string msg)
{ cerr << "mcf_cost_scaling: " + msg << endl;
  exit(1);
 }


template<class flow_array,class cap_array,class cost_array>
bool check_eps_optimality(const graph_t& G, const cost_array& cost,
                                            const pot_array& pot,
                                            const cap_array&   cap,
                                            const flow_array& flow,
                                            rcost_t eps, 
                                            string msg="")
{
  int count = 0;
  node v;
  forall_nodes(v,G)
  { edge e;
    forall_out_edges(e,v)
    { node w = target(G,e,v);
      rcost_t rc = rcost(e,v,w);
      if ((rc < -eps && out_cap(e) > 0) || (rc > eps && in_cap(e) > 0)) count++; 
     }
   }

  if (count >  0 && msg != "")
     error_handler(1,"check_eps_opt: " + msg);
     
  return count == 0;
}




template<class cap_array, class cost_array, class flow_array>
bool bellman_ford(const graph_t& G, const cap_array&  cap,
                                    const cost_array& cost,
                                    const pot_array&  pot,
                                    const flow_array& flow)
{
  float t = used_time();

  int n = G.number_of_nodes();

  b_queue<node> Q(n);
  node Q_last = 0;

  succ_array succ(G); // if v in Q then succ[v] = v else succ[v] = 0
  dist_array dist(G);

  node v;
  forall_nodes(v,G) 
  { //dist[v] = (int)pot[v];
    dist[v] = 0;
    Q.append(v);
    succ[v] = v;
    Q_last = v;
   }

  node phase_last = Q_last;
  int  phase_count = 0;

  while (phase_count < n && !Q.empty())
  { 
    node u = Q.pop();
    succ[u] = 0;

    NT du = dist[u];

    edge e;
    forall_out_edges(e,u) 
    { if (out_cap(e)== 0) continue;
      node v = target(G,e,u);    
      NT d = du + cost[e];
      if (d < dist[v])
      { dist[v] = d;
        if (succ[v] == 0) { Q.append(v); succ[v] = v; Q_last = v; }
       }
     } 

    forall_in_edges(e,u) 
    { if (in_cap(e) == 0) continue;
      node v = source(G,e,u);    
      NT d = du - cost[e];
      if (d < dist[v])
      { dist[v] = d;
        if (succ[v] == 0) { Q.append(v); succ[v] = v; Q_last = v; }
       }
     } 

    if (u == phase_last && !Q.empty()) 
    { phase_count++;
      phase_last = Q_last;
     }
  }


 cout << string("|Q| = %d   phases = %d  time: %.2f sec.",Q.size(),phase_count,used_time(t)) << endl;

  return Q.empty(); // no negative cycle
}




template<class cap_array, class flow_array, class cost_array>
void topsort(const graph_t& G, const excess_array& excess,
                               const cap_array& cap,
                               const flow_array& flow,
                               const cost_array& cost,
                               const pot_array& pot,
                               b_queue<node>& active, rcost_t eps)
{

  // sort esp-admissible network topologically

  int n = G.number_of_nodes();

  node* Q = new node[n];
  node* Q_stop = Q;

  dist_array indeg(G);

  active.clear();

  node v;
  forall_nodes(v,G)
  { 
    int deg = 0;

    edge e;
    forall_out_edges(e,v)
    { node w = target(G,e,v);
      if (in_cap(e) > 0  &&  rcost(e,v,w) > eps) deg++;
     }

    forall_in_edges(e,v)
    { node w = source(G,e,v);
      if (out_cap(e) > 0 && rcost(e,w,v) < -eps) deg++;
     }

    indeg[v] = deg;

    if (deg == 0) *Q_stop++ = v;
   }

  for(node* p = Q; p != Q_stop; p++)
  { node u = *p;
    if (excess[u] > 0) active.append(u);
    edge e;
    forall_out_edges(e,u)
    { if (out_cap(e) == 0) continue;
      node v = target(G,e,u);
      rcost_t rc = rcost(e,u,v);
      //if (rc >= 0) continue;
      if (rc >= eps) continue;
      if (--indeg[v] == 0) *Q_stop++ = v;
     }

    forall_in_edges(e,u)
    { if (in_cap(e) == 0) continue;
      node v = source(G,e,u);
      rcost_t rc = -rcost(e,v,u);
      //if (rc >= 0 ) continue; 
      if (rc >= eps) continue;
      if (--indeg[v] == 0) *Q_stop++ = v;
     }
   }

  assert(Q_stop == Q+n);

  delete[] Q;
}



template<class cap_array, class flow_array, class cost_array>
int topsort_dfs(const graph_t& G, cap_array& cap,
                                  flow_array& flow,
                                  const cost_array& cost,
                                  const pot_array& pot,
                                  node* Q)
{
  int n = G.number_of_nodes();

  int cycle_count = 0;

  node* q = Q+n;

  succ_array current_edge(G);
  dist_array current_in(G);
  pred_array pred(G); 

  node sentinel = G.stop_node();
  node top = sentinel;

  node v;
  forall_nodes(v,G) 
  { current_edge[v] = 0;
    current_in[v] = 0;
    pred[v] = 0;
   }

  forall_nodes(v,G)
  { 
    if (pred[v] != 0) continue;

    pred[v] = top; // push v
    top = v;

    while (top != sentinel)
    { 
      node v = top;
      edge e = (edge)current_edge[v];

      if (e == 0)
        if (G.outdeg(v) > 0)
          e = G.first_out_edge(v);
        else
          { e = G.first_in_edge(v);
            current_in[v] = true;
          }
      else
        if (current_in[v])
          e = G.next_in_edge(e);
        else
          { e = G.next_out_edge(e,v);
            if (e == 0) 
            { e = G.first_in_edge(v);
              current_in[v] = true;
             }
           }

      current_edge[v] = (node)e;

      if (e == 0)
      { *--q = v;
        top = pred[v]; // pop v
        pred[v] = v;   // mark v completed
        continue;
       }

      node w;

      if (current_in[v])
        { if (in_cap(e) == 0) continue;
          w = source(G,e,v);
          rcost_t rc = -rcost(e,w,v);
          if (rc >= 0 ) continue;
         }
      else
        { if (out_cap(e) == 0) continue;
          w = target(G,e,v);
          rcost_t rc = rcost(e,v,w);
          if (rc >= 0 ) continue;
         }

      if (pred[w] == 0) // unvisited: tree edge
      { pred[w] = top;
        top = w;
        continue;
       }

      if (pred[w] != w) // w not completed: backward edge
      { 
        cycle_count++;

        // compute minimum residual capacity

        NT   min_rc = MAXINT;
        node min_v  = 0;
        int  count = 0;

        node u_stop = pred[w];

        for(node u = v; u != u_stop; u = pred[u])
        { edge e = (edge)current_edge[u];
          NT rc;
          if (current_in[u])
              rc = in_cap(e);
          else
              rc = out_cap(e);
          if (rc <= min_rc) { min_rc = rc; min_v = u; }
          count++;
         }


        // augment flow
        for(node u = v; u != u_stop; u = pred[u])
        { edge e = (edge)current_edge[u];
          if (current_in[u]) 
             push_flow(e,-min_rc)
          else
             push_flow(e,min_rc)
        }

        // continue dfs at min_v
        while (top != min_v)  // mark all nodes from min_v to top as unvisited 
        { node u = pred[top];
          //current_edge[top] = 0;
          pred[top] = 0;
          top = u;
         }

      } // back-edge
    } // while stack not empty
  } // main loop

 assert(q == Q);

 if (write_log && cycle_count > 0) 
      cout << "cycle_count = " << cycle_count << endl;

 return cycle_count == 0;
}




template<class cap_array, class flow_array, class cost_array>
int topsort_dfs0(const graph_t& G, cap_array& cap,
                                   flow_array& flow,
                                   const cost_array& cost,
                                   const pot_array& pot,
                                   node* Q)
{
  int n = G.number_of_nodes();

  int cycle_count = 0;

  node* q = Q+n;


  dist_array current_edge(G);
  pred_array pred(G); 

  node sentinel = G.stop_node();
  node top = sentinel;

  node v;
  forall_nodes(v,G) 
  { current_edge[v] = 0;
    pred[v] = 0;
   }

  forall_nodes(v,G)
  { 
    if (pred[v] != 0) continue;

    pred[v] = top; // push v
    top = v;

    while (top != sentinel)
    { 
      node v = top;
      int i = current_edge[v];
      edge e = 0;

      if (i == 0)
        { e = G.first_out_edge(v);
          if (e) 
            i = G.index1(e);
          else
            { e = G.first_in_edge(v);
              i = -G.index1(e);
             }
         }
      else
         if (i < 0)
            { e = G.next_in_edge(G.get_edge1(-i));
              i = -G.index1(e);
             }
         else
            { e = G.next_out_edge(G.get_edge1(i),v);
              if (e) 
                 i = G.index1(e); 
              else
                 { e = G.first_in_edge(v);
                   i = -G.index1(e);
                  }
             }


      if (i == 0)
      { *--q = v;
        top = pred[v]; // pop v
        pred[v] = v;   // mark v completed
        continue;
       }

      current_edge[v] = i;

      node w;

      if (i < 0)
        { if (in_cap(e) == 0) continue;
          w = source(G,e,v);
          rcost_t rc = -rcost(e,w,v);
          if (rc >= 0 ) continue;
         }
      else
        { if (out_cap(e) == 0) continue;
          w = target(G,e,v);
          rcost_t rc = rcost(e,v,w);
          if (rc >= 0 ) continue;
         }

      if (pred[w] == 0) // unvisited: tree edge
      { pred[w] = top;
        top = w;
        continue;
       }

      if (pred[w] != w) // w not completed: backward edge
      { 
        cycle_count++;

        // compute minimum residual capacity

        NT   min_rc = MAXINT;
        node min_v  = 0;
        int  count = 0;

        node u_stop = pred[w];

        for(node u = v; u != u_stop; u = pred[u])
        { int i = current_edge[u];
          NT rc;
          if (i < 0)
            { edge e = G.get_edge1(-i);
              rc = in_cap(e);
             }
          else
            { edge e = G.get_edge1(i);
              rc = out_cap(e);
             }
          if (rc <= min_rc) { min_rc = rc; min_v = u; }
          count++;
         }


        // augment flow
        for(node u = v; u != u_stop; u = pred[u])
        { int i = current_edge[u];
          if (i < 0)
              push_flow(G.get_edge1(-i),-min_rc)
          else
              push_flow(G.get_edge1(i),min_rc)
        }

        // continue dfs at min_v
        while (top != min_v)  // mark all nodes from min_v to top as unvisited 
        { node u = pred[top];
          pred[top] = 0;
          top = u;
         }

      } // back-edge
    } // while stack not empty
  } // main loop

 assert(q == Q);

 if (write_log && cycle_count > 0) 
      cout << "cycle_count = " << cycle_count << endl;

 return cycle_count == 0;
}



template<class cap_array, class flow_array, class cost_array>
bool price_refine_phase1(const graph_t& G, rcost_t eps,
                                           cap_array& cap,
                                           flow_array& flow,
                                           const cost_array& cost,
                                           const pot_array& pot,
                                           dist_array& dist) 
{

  // sort admissible network topologically

  int n = G.number_of_nodes();

  node* Q = new node[n];
  node* Q_stop = Q+n;

  if (!topsort_dfs(G,cap,flow,cost,pot,Q))
  { delete[] Q;
    return false; 
   }

  node v;
  forall_nodes(v,G) dist[v] = 0;

  for(node* p = Q; p != Q_stop; p++)
  { node u = *p;
    int du = dist[u];
    edge e;
    forall_out_edges(e,u)
    { if (out_cap(e) == 0) continue;
      node v = target(G,e,u);
      rcost_t rc = rcost(e,u,v);
      if (rc >= 0) continue;
      int d = du + int((rc+0.5)/eps);
      //int d = du + int(rc/eps+1);
      if (d < dist[v]) dist[v] = d;
     }

    forall_in_edges(e,u)
    { if (in_cap(e) == 0) continue;
      node v = source(G,e,u);
      rcost_t rc = -rcost(e,v,u);
      if (rc >= 0 ) continue; 
      int d = du + int((rc+0.5)/eps);
      //int d = du + int(rc/eps+1);
      if (d < dist[v]) dist[v] = d;
     }
   }

  delete[] Q;
  return true;
}



template<class flow_array, class cap_array, class cost_array, class node_pq>
bool price_refinement(const graph_t& G, rcost_t eps, cap_array& cap,
                                                     flow_array& flow,
                                                     const cost_array& cost,
                                                     pot_array& pot, 
                                                     node_pq& PQ)

{
  //float tt = used_time();
  //cout << string("price_refine(%8.0f) ",eps) << endl;

  dist_array dist(G);

  bool result = false;

  while (price_refine_phase1(G,eps,cap,flow,cost,pot,dist))
  { 
    // admissible network is acyclic

    int count = 0;
    int d_min = 0;

    node v;
    forall_nodes(v,G) 
         if (dist[v] < d_min) d_min = dist[v];

    d_min = -d_min;

    PQ.reset();
    node sentinel = PQ.sentinel();

    forall_nodes(v,G) 
    { int dv = dist[v];
      assert(dv <= 0);
      if (dv < 0) 
      { count++;
        PQ.push(v,d_min+dv);
       }
     }

    if (count == 0) // eps-optimal
    { result = true;
      break;
     }

    int level = 0;
    while (level <= PQ.max_level())
    { 
      node v = PQ.top(level); 
      if (v == sentinel) { level++; continue; }

      PQ.pop(v,level);

      int dv = dist[v];
      assert(dv <= 0);

      dist[v] = 1-dv;              // mark v as scanned (dist > 0)

      edge e;
      forall_out_edges(e,v)
      { if (out_cap(e) == 0) continue;
        node w = target(G,e,v); 
        int dw = dist[w];
        if (dw > 0) continue; // already scanned
        rcost_t rc = rcost(e,v,w);
        int d = dv;
        if (rc >= 0) d += int(rc/eps+1);
        if (dw > d) 
        { if (dw != 0) PQ.remove(w,d_min+dw); 
          PQ.push(w,d_min+d);
          dist[w] = d;
         }
      }
      
      forall_in_edges(e,v)
      { if (in_cap(e) == 0) continue;
        node w = source(G,e,v); 
        int dw = dist[w];
        if (dw > 0) continue; // already scanned
        rcost_t rc = -rcost(e,w,v);
        int d = dv;
        if (rc >= 0) d += int(rc/eps+1);
        if (dw > d) 
        { if (dw != 0) PQ.remove(w,d_min+dw); 
          PQ.push(w,d_min+d);
          dist[w] = d;
         }
      }
   }

  if (level > max_level) max_level = level;

  // increase potential
  forall_nodes(v,G)
  { int d = dist[v];
    if (d > 0) d = 1-d; // unmark v
    pot[v] -= (d*eps);
   }

 } // while price_refine_phase1


 //cout << string(" time: %.2f sec",used_time(tt)) << endl;

 return result;
}


template<class flow_array, class cap_array, class cost_array, 
                                            class node_pq,
                                            class ex_queue>
int global_price_update(const graph_t& G, const cap_array& cap,
                                          const flow_array& flow,
                                          const excess_array& excess,
                                          const cost_array& cost,
                                          pot_array& pot,
                                          //current_e_array& current_e,
                                          //current_x_array& current_x,
                                          node_pq& PQ,
                                          ex_queue& active,
                                          rcost_t eps)
{

  //float t1 = used_time();

  num_updates++;

  dist_array dist(G);

  active.clear();

  PQ.reset();
  node sentinel = PQ.sentinel();

  rcost_t pot_min = MAXDOUBLE;
  rcost_t pot_max = 0;

  int count = 0;

  node v;
  forall_nodes(v,G) 
  { //current_x[v] = 0;
    int ev = excess[v];
    if (ev < 0) 
     { dist[v] = 0;
       PQ.push(v,0);
      }
    else
     { dist[v] = inf;
       if (ev > 0) count++; 
      }
    rcost_t pv = pot[v];
    if (pv < pot_min) pot_min = pv;
    if (pv > pot_max) pot_max = pv;
   }

  //assert(pot_max < MAXINT);


  int du = 0;
  int level = 0;
  while (level <= PQ.max_level()) 
  { 
    node u = PQ.top(level);
    if (u == sentinel) { level++; continue; }

    PQ.pop(u,level);

    du = dist[u];

    rcost_t pu = pot[u];

    if (excess[u] > 0) 
    { count--; 
      active.push(u);
     }

    edge e;
    forall_out_edges(e,u)
    { NT x = in_cap(e);
      if (x == 0) continue;
      node v = target(G,e,u);
      int dv = dist[v];
      if (du >= dv) continue;
      rcost_t rc = -COST(e)-pot[v]+pu; // -rcost(e,u,v);
      int c = du;
      if (rc >= 0)  
      { c += int(rc/eps + 1);
        if (c >= dv) continue;
        //if (c == dv && x < ABS(current_x[v])) continue;
       }
      if (dv < inf) PQ.remove(v,dv);
      PQ.push(v,c);
      dist[v] = c;
      //current_e[v] = e;
      //current_x[v] = -x;
     }

    forall_in_edges(e,u)
    { NT x = out_cap(e);
      if (x == 0) continue;
      node v = source(G,e,u);
      int dv = dist[v];
      if (du >= dv) continue;
      rcost_t rc = COST(e)-pot[v]+pu; //rcost(e,v,u);
      int c = du;
      if (rc >= 0)  
      { c += int(rc/eps + 1);
        if (c >= dv) continue;
        //if (c == dv && x < ABS(current_x[v])) continue;
       }
      if (dv < inf) PQ.remove(v,dv);
      PQ.push(v,c);
      dist[v] = c;
      //current_e[v] = e;
      //current_x[v] = x;
     }
   }

  if (level > max_level) max_level = level;

  forall_nodes(v,G)
  { int d = dist[v];
    if (d > du) d = du;
    pot[v] += (d*eps-pot_min);
   }

  //cout << string("global price update:  %.2f sec",used_time(t1)) << endl;

  return count == 0;
}



template<class flow_array, class cap_array, class cost_array, class node_pq >
int global_price_update(const graph_t& G, const cap_array& cap,
                                          const flow_array& flow,
                                          const excess_array& excess,
                                          const cost_array& cost,
                                          pot_array& pot,
                                          //current_e_array& current_e,
                                          //current_x_array& current_x,
                                          node_pq& PQ,
                                          rcost_t eps)
{
  zero_queue<node> dummy;
  return global_price_update(G,cap,flow,excess,cost,pot,
                                 /*current_e,current_x,*/ PQ,dummy,eps);
 }



template <class cap_array, class cost_array, class flow_array>
void establish_eps_optimality(const graph_t& G, cap_array& cap,
                                                const cost_array& cost,
                                                pot_array& pot,
                                                flow_array& flow,
                                                excess_array& excess,
                                                rcost_t eps)
{
    rcost_t pot_min = MAXPOT;
    rcost_t pot_max = 0;

    node v;
    forall_nodes(v,G) 
    { if (pot[v] < pot_min) pot_min = pot[v];
      if (pot[v] > pot_min) pot_max = pot[v];
     }

    forall_nodes(v,G) pot[v] -= pot_min;

    //assert(pot_max < MAXINT);

    int count1 = 0;
    int count2 = 0;

    forall_nodes(v,G)
    { double ev = excess[v];

      edge e;
      forall_out_edges(e,v)
      { node w = target(G,e,v);
        double ew = excess[w];
        rcost_t rc = rcost(e,v,w);

        if (rc < -eps) 
        { NT x = out_cap(e);
          if (v != w)
          { ew += x;
            ev -= x;
           }
          push_flow(e,x)
          count1++;
         }

        if (rc > eps) 
        { NT x = in_cap(e);
          if (v != w)
          { ew -= x;
            ev += x;
           }
          push_flow(e,-x)
          count2++;
         }

        if (ew > MAXINT || ew < -MAXINT)
            error_handler(1, "excess overflow1");
        excess[w] = int(ew);

       }

     if (ev > MAXINT || ev < -MAXINT)
         error_handler(1, "excess overflow2");
      excess[v] = int(ev);
     }

if (write_log)
cout << string("count1 = %d  count2 = %d",count1,count2) << endl;

}

      

/*
template <class cap_array, class flow_array>
int tree_push(const graph_t& G, cap_array& cap,
                                flow_array& flow,
                                excess_array& excess,
                                current_e_array& current_e,
                                current_x_array& current_x,
                                b_queue<node>& active)

{ 
  
  int count = 0;

  while (!active.empty())
  { node v = active.pop();
    NT x = current_x[v];
    if (x == 0) continue;
  
    NT ev = excess[v];
    edge e = current_e[v];
  
    if (x > 0)
    { count++;
      if (ev < x) x = ev; 
      push_flow(e,x);
      node w = target(G,e,v);
      NT ew = excess[w];
      if (ew <= 0 && x > -ew) active.append(w);
      excess[w] = ew+x;
      excess[v] -= x;
      current_x[v] -= x;
     }
  
    if (x < 0)
    { count++;
      x = -x;
      if (ev < x) x = ev; 
      push_flow(e,-x);
      node w = source(G,e,v);
      NT ew = excess[w];
      if (ew <= 0 && x > -ew) active.append(w);
      excess[w] = ew+x;
      excess[v] -= x;
      current_x[v] += x;
     }
  }
  
  node v;
  forall_nodes(v,G)
    if (excess[v] > 0) active.append(v);
  
  return count;
}
*/



template <class cap_array, class cost_array, class flow_array>
bool cost_scaling(const graph_t& G, cap_array& cap,
                                    const cost_array& cost,
                                    excess_array& excess,
                                    flow_array& flow)
{
  // returns true iff there exists a feasible flow
 
  int n  = G.number_of_nodes();

  bool feasible = true;

  max_level = 0;

  //pot_array  pot(G,0);

  pot_array  pot;   // potential
  pot.use_node_data(G,0);

  succ_array succ;
  succ.use_node_data(G,0);

  pred_array pred;
  pred.use_node_data(G,0);


  node_level_queue<graph_t,succ_array,pred_array> 
                                     PQ(succ,pred,inf,G.stop_node());

  NT total_excess = 0;

  node v;
  forall_nodes(v,G) 
  { pot[v] = 0;
    total_excess += excess[v];
   }

  if (total_excess != 0) return false; // infeasible

  NT C = 0;   // maximal cost of any edge in G

  cost_array& ca = (cost_array&)cost;

  edge e;
  forall_edges(e,G) 
  { flow[e] = 0;
    NT nc = (n+1) * cost[e];
    if (nc/(n+1) != cost[e]) 
        error_handler(1,"cost overflow");
    ca[e] = nc;
/*
    NT nc = (cost[e] << 16);
    if ((nc>>16) != cost[e]) 
        error_handler(1,"cost overflow");
    e->usr_data = cost[e];
*/
    if (nc > C) C = nc;
    else if (-nc > C) C = -nc;
   }

  assert(C > 0);

  if (write_log) cout << "C = " << C << endl;

  int eps = C;
  int eps_opt = 1;
//int eps_opt = (1 << 16)/n;

  eps /= alpha;
  if (eps < eps_opt) eps = eps_opt;

  if (write_log) cout << "eps = " << C << endl;


  b_queue<node> active(n);


  counter push_count = 0;
  counter final_push_count = 0;
  counter discharge_count = 0;

  int relabel_count = 0;

  int af_count = 0;
  int rs_count = 0;
  int refine_count = 0;

  int save_eps = eps;
  node_array<rcost_t,graph_t> save_pot(G,0);
  edge_array<int,graph_t> save_flow(G,0);

  while (eps >= eps_opt)
  {
    float t1 = used_time();

    refine_count++;

    if (write_log) 
    { cout << endl;
      cout << string("refine_count = %d",refine_count) << endl;
     }


    if (refine_count > 1 && af_factor > 0)  // arc fixing
    { edge_filter<graph_t,cap_array,flow_array,cost_array,pot_array,rcost_t> 
         F(G,cap,flow,cost,pot,af_factor*alpha,eps);
      af_count += G.hide_edges(F);
/*
      edge_smaller<graph_t,ucap_array> smaller(cap);
      af_count += G.hide_edges(F,smaller);
*/
     }

    if (af_count > 0)
    { // save configuration
      save_eps = eps;
      forall_nodes(v,G) 
      { rcost_t pv = pot[v];
        save_pot[v] = pv;
        forall_out_edges(e,v) save_flow[e] = flow[e];
       }
     }
 
   if (refine_count == 1)
      establish_eps_optimality(G,cap,cost,pot,flow,excess,0);
   else
      establish_eps_optimality(G,cap,cost,pot,flow,excess,eps);

   if (refine_count == 1)
   {  if (!global_price_update(G,cap,flow,excess,cost,pot,PQ,eps))
      { feasible = false;
         if (write_log) cout << "price update: infeasible problem." << endl;
         goto TERMINATE;
       }
    }


    if (write_log) 
    { cout << string("|active| = %d",active.size()) << endl;
      cout << string("eps = %9d cost = %12.0f pot = %14.0f fixed = %5d",
                      eps, total_cost(G,cost,flow,n), total_pot(G,pot),
                      af_count) << endl;

      int zero_count = 0;
      int F = 0;
      node v;
      forall_nodes(v,G)
      { edge e;
        forall_out_edges(e,v)
        { node w = target(G,e,v);
          if (rcost(e,v,w) == 0) zero_count++;
         }
       }
      cout << string("zero_count = %d  F = %d",zero_count,F) << endl;
     }


   forall_nodes(v,G)
     if (excess[v] > 0) active.append(v);

     
    while (!active.empty())
    { 
      //if (relabel_count > int(f_update*n))
      //if (relabel_count > int(0.4*n) + 30*active.size())
      //if (relabel_count > int(f_update*n) + active.size())
      //if (relabel_count > int(n/2 + 10*active.size()))

      if (relabel_count > int(n/2 + 5*active.size()))
      { bool price_up = global_price_update(G,cap,flow,excess,cost,pot,PQ,eps);
        assert(price_up);
        num_relabels += relabel_count;
        relabel_count = 0;
        continue;
       }
 
      discharge_count++;
 
      node v = active.pop();

      NT ev = excess[v];
      rcost_t pv = pot[v];
 
      while (ev > 0)
      {
        rcost_t min_rc = MAXPOT;
        edge min_e = 0;
        NT   min_x = 0;
 
        edge e;
        forall_out_edges(e,v)
        { NT x = out_cap(e);
          if (x == 0) continue; 
          node w = target(G,e,v);
          rcost_t rc = COST(e)-pv+pot[w]; //rcost(e,v,w);
          if (rc < 0)   // admissible
          { NT ew = excess[w];
            push_count++;
            if (ev < x) x = ev; 
            push_flow(e,x)
            if (ew <= 0 && x > -ew) active.append(w);
            excess[w] = ew+x;
            ev -= x;
            if (ev == 0) goto FINISH_DISCHARGE;
           }
          else
          if (rc < min_rc) { min_rc = rc; min_e = e; min_x = x; }
        }
   
   
        forall_in_edges(e,v)
        { NT x = flow[e];
          if (x == 0) continue; 
          node w = source(G,e,v);
          rcost_t rc = -(COST(e)-pot[w]+pv); //-rcost(e,w,v);
          if (rc < 0) // admissible
          { NT ew = excess[w];
            push_count++;
            if (ev < x) x = ev;
            push_flow(e,-x)
            if (ew <= 0 && x > -ew) active.append(w);
            excess[w] = ew+x;
            ev -= x;
            if (ev == 0) goto FINISH_DISCHARGE;
           }
          else
          if (rc < min_rc) { min_rc = rc; min_e = e; min_x = -x; }
        }
 
        if (min_e == 0)
        { feasible = false;
          if (write_log) 
                cout << "cannot relabel node: infeasible problem." << endl;
          goto TERMINATE;
         }

        relabel_count++;
        pv += (eps+min_rc);

#if defined(FINAL_PUSH)
        if (ev <= min_x || ev <= -min_x)
        { final_push_count++;
          node w;
          if (min_x > 0)
          { w = target(G,min_e,v);
            push_flow(min_e,ev)
           }
          else
          { w = source(G,min_e,v);
            push_flow(min_e,-ev)
           }
          NT ew = excess[w];
          if (ew <= 0 && ev > -ew) active.append(w);
          excess[w] = ew+ev;
          ev = 0;
         }
#endif
      } // while ev > 0

FINISH_DISCHARGE:;
 
      excess[v] = 0;
      pot[v] = pv;
 
     } // while active not empty

    if (write_log) 
    { cout << string("eps = %9d cost = %12.0f pot = %14.0f fixed = %5d", 
                      eps, total_cost(G,cost,flow,n), total_pot(G,pot),
                      af_count) << endl;
     }

    if (beta > 1)
    { bool p_ref = true;
      while (p_ref && eps > eps_opt)
      { int new_eps =  eps/beta;
        if (new_eps < eps_opt) new_eps = eps_opt;

        if (write_log) 
          cout << string("price refine(eps = %d)",new_eps) << endl;

        p_ref = price_refinement(G,new_eps,cap,flow,cost,pot,PQ);

        if (p_ref) eps = new_eps;

        if (write_log) 
        { //cout << (p_ref ? "yes" : "no") << endl; 
          cout << string("eps = %9d cost = %12.0f pot = %14.0f fixed = %5d",
                          eps, total_cost(G,cost,flow,n), total_pot(G,pot),
                          af_count) << endl;
         }
       }
     }
 
    if (write_log) 
    { cout << string("%.2f sec", used_time(t1)) << endl;
      cout << endl;
     }


 // test eps-optimality for all fixed arcs

    int err_count = 0;
    forall_nodes(v,G)
    { edge e;
      forall_hidden_edges(e,v)
      { node w = target(G,e,v);
        rcost_t rc = rcost(e,v,w);
        if ((rc < -eps && out_cap(e) > 0) || (rc > eps && in_cap(e) > 0)) 
           err_count++;
       }
      if (err_count > 0) break;
     }

    if (err_count > 0)
    { 
      assert(af_count > 0);
      if (write_log) cout << "RESTART" << endl;

      rs_count++;
      af_factor *= 2;

      forall_nodes(v,G) 
      { rcost_t pv = save_pot[v];
        pot[v] = pv;
        forall_out_edges(e,v) 
        { NT df = save_flow[e] - flow[e];
          push_flow(e,df)
         }
       }
      eps = save_eps;
      G.restore_all_edges();
      af_count = 0;
      continue;
     }

    if (eps == eps_opt) break;

/*
if (refine_count <= 2)
    eps /= (alpha/2);
else
*/
    eps /= alpha;

    if (eps < eps_opt) eps = eps_opt;
 
  } // while eps >= eps_opt                                    


  assert(eps == eps_opt);

  G.restore_all_edges();
  check_eps_optimality(G,cost,pot,cap,flow,eps_opt,"final eps-opt-test");

  num_relabels += relabel_count;
  num_discharges = discharge_count;
  num_pushes = push_count;
  num_final_pushes = final_push_count;
  num_arcfixes = af_count;
  num_restarts = rs_count;


  if (write_log)
  { cout << string("restarts =  %d   max level = %.2f * n", 
                    rs_count, float(max_level)/n) << endl;
    cout << endl;
   }

TERMINATE:

  forall_edges(e,G) ca[e] /= (n+1);
  return feasible;
}



public:

mcf_cost_scaling() : alpha(0), beta(0), f_update(1.0), af_factor(0.0),
                     num_nodes(0), num_edges(0), num_discharges(0), 
                     num_pushes(0), num_final_pushes(0), num_relabels(0), 
                     num_updates(0), num_arcfixes(0), num_restarts(0), 
                     write_log(false), T(0) {}


template<class cap_array, class cost_array, class flow_array>
bool run0(const graph_t& G, const cap_array& cap, const cost_array& cost,
                                                 excess_array& excess,
                                                 flow_array&   flow,
                                                 float f = 0.5,
                                                 int a = 16,
                                                 int b = 16)
{
  T = used_time();

  num_discharges = 0;
  num_relabels = 0;
  num_updates  = 0;

  num_nodes = G.number_of_nodes();
  num_edges = G.number_of_edges();

  alpha = a;
  beta  = b;
  af_factor = f * num_nodes;

  inf = alpha*num_nodes;


  // sort edges by capacities (decreasing)

  float tt = used_time();
  edge_smaller<graph_t,cap_array> smaller(cap);
  G.sort_edges(smaller);
  if (write_log) cout << string("sorting: %.2f sec",used_time(tt)) << endl;

  bool feasible = cost_scaling(G, cap, cost, excess, flow);

  T = used_time(T);

  return feasible;
}



template<class lcap_array, class ucap_array, class cost_array, 
                           class supply_array, class flow_array>
bool run(const graph_t& G, const lcap_array& lcap,
                           const ucap_array& ucap,
                           const cost_array& cost,
                           const supply_array& supply,
                           flow_array&       flow,
                           float f = 0.5,
                           int a = 16,
                           int b = 16)
{
  T = used_time();

  num_discharges = 0;
  num_relabels = 0;
  num_updates  = 0;

  num_nodes = G.number_of_nodes();
  num_edges = G.number_of_edges();

  alpha = a;
  beta  = b;

  //af_factor = f*std::sqrt(double(num_edges));
  //af_factor = 3*f*std::sqrt(double(num_nodes));
  //af_factor = 7.5*f*num_nodes/std::sqrt(double(num_edges));
  //double d = double(num_nodes)/num_edges;
  //af_factor = 0.2*f*d*num_nodes;

  af_factor = f * num_nodes;

  inf = alpha*num_nodes;

  excess_array excess;         // supply values after elimination of
  excess.use_node_data(G,0);   // lower capacity bounds

  node v;
  forall_nodes(v,G) excess[v] = supply[v];


  ucap_array& cap = (ucap_array&)ucap;  // non-const reference to cap array
                                        // used to modify capacities
                                        // temporarily for elimination of
                                        // lower capacity bounds
  int lc_count = 0;

  forall_nodes(v,G) 
  { edge e;
    forall_out_edges(e,v) 
    { NT lc = lcap[e];
      if (lc != 0)                  // nonzero lower capacity bound
      { node w  = target(G,e,v);
        cap[e] -= lc;
        excess[v] -= lc;
        excess[w] += lc;
        lc_count++;
       }
     }
  }

  // sort edges by capacities (decreasing)

  float tt = used_time();
  edge_smaller<graph_t,ucap_array> smaller(cap);
  G.sort_edges(smaller);
  if (write_log) cout << string("sorting: %.2f sec",used_time(tt)) << endl;


  bool feasible = cost_scaling(G, cap, cost, excess, flow);

  if (lc_count)
  { // adjust flow and restore capacities
    edge e;
    forall_edges(e, G) 
    { NT lc = lcap[e];
      if (lc != 0)
      { flow[e] += lc;
        cap[e] = total_cap(e);
       }
     }
   }

  T = used_time(T);

  return feasible;
}



template<class lcap_array, class ucap_array, class cost_array, 
                                             class supply_array,
                                             class flow_array>
bool check(const graph_t& G, const lcap_array& lcap, const ucap_array&   ucap,
                                                     const cost_array&   cost,
                                                     const supply_array& supply,
                                                     const flow_array&   flow,
                                                     string& msg)
{
  if (write_log) cout << "checking flow " << endl;

  float t = used_time();

  msg = "";

  //check feasibility

  int count1 = 0;
  int count2 = 0;

  edge e;
  forall_edges(e,G)
    if (flow[e] < lcap[e] || flow[e] > ucap[e]) count1++;

  if (count1 > 0)
         msg = string("illegal flow value for %d edge(s)",count1);
  
  node v;
  forall_nodes(v,G)
  { edge e;
    double ev = double(supply[v]);
    forall_out_edges(e,v) ev -= flow[e];
    forall_in_edges(e,v)  ev += flow[e];
    if (ev != 0) count2++;
   }

  if (count2 > 0)
  { if (msg != "") msg += " + ";
     msg = string("non-zero excess at %d node(s)",count2);
   }

  if (count1 || count2) return false;


  // check optimality (negative cycle condition)
  // bellman ford:

  int n = G.number_of_nodes();

  b_queue<node> Q(n);
  node Q_last = 0;

  succ_array succ(G); // if v in Q then succ[v] = v else succ[v] = 0
  dist_array dist(G);

  forall_nodes(v,G) 
  { dist[v] = 0;
    Q.append(v);
    succ[v] = v;
    Q_last = v;
   }

  node phase_last = Q_last;
  int  phase_count = 0;

  while (phase_count < n && !Q.empty())
  { 
    node u = Q.pop();
    succ[u] = 0;

    NT du = dist[u];

    edge e;
    forall_out_edges(e,u) 
    { if (flow[e] == ucap[e]) continue;
      node v = target(G,e,u);    
      NT d = du + cost[e];
      if (d < dist[v])
      { dist[v] = d;
        if (succ[v] == 0) { Q.append(v); succ[v] = v; Q_last = v; }
       }
     } 

    forall_in_edges(e,u) 
    { if (flow[e] == lcap[e]) continue;
      node v = source(G,e,u);    
      NT d = du - cost[e];
      if (d < dist[v])
      { dist[v] = d;
        if (succ[v] == 0) { Q.append(v); succ[v] = v; Q_last = v; }
       }
     } 

    if (u == phase_last && !Q.empty()) 
    { phase_count++;
      phase_last = Q_last;
     }
  }

  if (!Q.empty())
  { if (msg != "") msg += " + ";
    msg += "negative cycle in residual network."; 
   }

  if (write_log) 
      cout << string("phases: %d  time: %.2f sec.",phase_count,used_time(t)) << endl;

  return Q.empty(); // no negative cycle
}


void set_write_log(bool b) { write_log = b; }

float  cpu_time() const { return T;        }
int    restarts() const { return num_restarts; }
double af()       const { return af_factor; }

template<class cost_array, class flow_array>
double total_cost(const graph_t& G, const cost_array& cost, 
                                    const flow_array& flow, int f=1)
{ double c = 0;
  edge e;
  forall_edges(e,G) c += double(cost[e]/f)*flow[e];
  return c;
}

rcost_t total_pot(const graph_t& G, const pot_array& pot)
{ rcost_t P = 0;
  node v;
  forall_nodes(v,G) P += pot[v];
  return P;
}


void statistics(ostream& out) const
{ out << string("%5d nodes  %5d edges  a = %d  b = %d  af = %f",
                 num_nodes,num_edges,alpha,beta,af_factor) << endl;
  out << string("%d discharges %d+%d pushes %d relabels %d updates %d fixes",
   num_discharges, num_pushes,num_final_pushes,num_relabels, num_updates, num_arcfixes) << endl;
  out << endl;
}


};


LEDA_END_NAMESPACE

#if LEDA_ROOT_INCL_ID == 500353
#undef LEDA_ROOT_INCL_ID
#include <LEDA/internal/POSTAMBLE.h>
#endif
