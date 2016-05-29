

#if !defined(LEDA_ROOT_INCL_ID)
#define LEDA_ROOT_INCL_ID 500353
#include <LEDA/internal/PREAMBLE.h>
#endif

#include <LEDA/graph/graph_alg.h>
#include <LEDA/core/b_queue.h>
#include <LEDA/graph/node_pq22.h>
#include <LEDA/internal/std/math.h>
#include <LEDA/system/assert.h>

#include <LEDA/graph/templates/feasible_flow.h>


#define rcost(e,v,w) (rcost_t(cost[e]) - pot[v] + pot[w])



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


template<class graph_t, class cost_array, class pot_array, class rcost_t>
class edge_filter {

typedef typename graph_t::node node;
typedef typename graph_t::edge edge;

const graph_t&    G;
const cost_array& cost;
const pot_array&  pot;
double af_bound;


public:

edge_filter(const graph_t& g, const cost_array& c, const pot_array& p, 
                              int eps, double af) : G(g), cost(c), pot(p)
{
  af_bound = af*eps*g.number_of_nodes();
}


bool operator()(edge e, node u)
{ node v = G.opposite(e,u);
  rcost_t rc = rcost(e,u,v);
  return rc > af_bound || rc < -af_bound;
 }
};



template <class graph, class succ_array>
class node_queue {

typedef typename graph::node node;
typedef typename graph::edge edge;

const node _sentinel;

node  _first;
node  _last;
int   sz;

succ_array& NEXT;

public:

node_queue(succ_array& succ) : _sentinel(node(0xffffffff)), NEXT(succ)
{ _first = _last = _sentinel; sz = 0; }


node_queue(const graph& G, succ_array& succ) : _sentinel(node(0xffffffff)),
                                               NEXT(succ)
{ node v;
  forall_nodes(v,G) NEXT[v] = NULL; 
  _first = _last = _sentinel;
  sz = 0;
 }

bool empty() const { return _first == _sentinel; }

int size() const { return sz; }

bool member(node v) const { return NEXT[v] != NULL; }

void append(node v)
{ 
  if (empty())
    _first = v;
  else
    NEXT[_last] = v;

  NEXT[v] = _sentinel;
  _last = v;
  sz++;
 }

void fast_append(node v) // precondition: not empty
{ 
  NEXT[_last] = v;
  NEXT[v] = _sentinel;
  _last = v;
  sz++;
 }


void fast_push(node v) // precondition: not empty
{ 
  NEXT[v] = _first;
  _first = v;
  sz++;
 }

void push(node v) 
{ NEXT[v] = _first;
  _first = v;
  if (_last == _sentinel) _last = v;
  sz++;
 }


node succ(node v) { return NEXT[v]; }

node top()       { return _first; }
node first()     { return _first; }
node last()      { return _last; }
node sentinel()  { return _sentinel; }


void del_top() 
{ node v = _first;
  _first = NEXT[v];
  NEXT[v] = NULL;
  sz--;
}

void del_top(node v) 
{ _first = NEXT[v];
  NEXT[v] = NULL;
  sz--;
}

node pop() 
{ node v = top();
  del_top(v);
  return v;
}


};



template <class graph, class succ_array, class pred_array>
class node_level_queue {

typedef typename graph::node node;
typedef typename graph::edge edge;

succ_array& NEXT;
pred_array& PRED;

int    count;
int    size;

node*  T;
node*  min;


public:



node_level_queue(succ_array& succ, pred_array& pred, int sz) 
            : NEXT(succ),PRED(pred),count(0),size(sz)
{ 
  T = new node[sz];
  for(int i=0; i<sz; i++) T[i] = 0;
  min = T;
 }

~node_level_queue() { delete[] T; }

void insert(node v, int i)
{ assert(i<size);
  node u = T[i];
  NEXT[v] = u;
  PRED[v] = 0;
  if (u) PRED[u] = v;
  T[i] = v;
  count++;
 }


void clear() 
{ node* stop = T+size;
  for(node* p = min; p<stop; p++) *p = 0;
  min = T; 
  count = 0;
}

node find_min()
{ while (*min == 0) min++;
  return *min;
}

node del_min()
{ while (*min == 0) min++;
  node u = *min;
  node w = NEXT[u];
  *min = w;
  if (w) PRED[w] = 0;
  count--;
  return u;
}

void remove(node v, int i)
{ node u = PRED[v];
  node w = NEXT[v];

  if (u) NEXT[u] = w;
  else T[i] = w;

  if (w) PRED[w] = u;
  count--;
}

  

void decrease_p(node v, int old_d, int new_d)
{ remove(v,old_d);
  insert(v,new_d);
}

bool empty() const { return count == 0; }

};


/*

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

*/



template <class NT, class graph_t      = graph,
                    class pot_array    = node_array<NT,graph_t>,
                    class excess_array = node_array<NT,graph_t>,
                    class dist_array   = node_array<double,graph_t>,
                    class succ_array   = node_array<node,graph_t>,
                    class succ_array2  = node_array<node,graph_t>,
                    class pred_array   = node_array<node,graph_t> >


class mcf_cost_scaling 
{
  typedef typename graph_t::node node;
  typedef typename graph_t::edge edge;
  typedef typename pot_array::value_type rcost_t;

  int   alpha;
  int   beta;
  float f_update;
  float af_factor;
  
  int num_nodes;
  int num_edges;
  int num_discharges;
  int num_pushes;
  int num_relabels;
  int num_updates;

  rcost_t err_eps;
  bool write_log;

  float T;


/*
node source(const graph_t& G, edge e, node t)
{ return Opposite<typename graph_t::category_tag>::source(G,e,t); }

node target(const graph_t& G, edge e, node s) 
{ return Opposite<typename graph_t::category_tag>::target(G,e,s); }
*/

node source(const graph_t& G, edge e, node)
{ return G.source(e); }

node target(const graph_t& G, edge e, node) 
{ return G.target(e); }


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
      if (flow[e] < cap[e] &&  rcost(e,v,w) < -eps 
       || flow[e] > 0      && -rcost(e,v,w) < -eps) count++; 
     }
   }

   if (count && msg != "")
         error_handler(1,"check_eps_opt: " + msg);

   return count == 0;
}

template<class flow_array,class cap_array,class cost_array>
rcost_t compute_eps(const graph_t& G, const cost_array& cost,
                                            const pot_array& pot,
                                            const cap_array&   cap,
                                            const flow_array& flow, int& af_count)
{
  node v;
  rcost_t eps = 0;
  forall_nodes(v,G)
  { edge e;
    //forall_hidden_edges(e,v)
    forall_out_edges(e,v)
    { if (!G.is_hidden(e)) continue;
      node w = target(G,e,v);
      rcost_t rc = rcost(e,v,w);
      if (flow[e] < cap[e] &&  rc < -eps) eps = -rc; 
      if (flow[e] > 0      && -rc < -eps) eps =  rc; 
     }
   }

  return eps;
}




template<class cap_array, class flow_array, class cost_array>
void price_refine_phase2(const graph_t& G, rcost_t eps,
                                          const cap_array& cap,
                                          const flow_array& flow,
                                          const cost_array& cost,
                                          const pot_array& pot,
                                          dist_array& dist,
                                          succ_array2& succ, int& n)
{

  node_queue<graph_t,succ_array2> Q(succ);

  node v;
  forall_nodes(v,G)
  { 
    if (succ[v] != 0) continue;

    int deg = 0;

    edge e;

    forall_out_edges(e,v)
    { node w = target(G,e,v);
      if (flow[e] < cap[e] && rcost(e,v,w) < 0) deg++;
     }

    forall_in_edges(e,v)
    { node w = source(G,e,v);
      if (flow[e] > 0  && -rcost(e,w,v) < 0) deg++;
     }


    dist[v] = deg; // outdeg[v]

    if (deg == 0) Q.append(v);
   }

  for(node u = Q.first(); u != Q.sentinel(); u = Q.succ(u))
  { n--; 
    edge e;
    forall_out_edges(e,u)
    { if (flow[e] == 0) continue;
      node v = target(G,e,u);
      rcost_t rc = -rcost(e,u,v);
      if (rc >= 0 ) continue; 
      if (--dist[v] == 0) Q.append(v);
     }
    forall_in_edges(e,u)
    { if (flow[e] == cap[e]) continue;
      node v = source(G,e,u);
      rcost_t rc = rcost(e,v,u);
      if (rc >= 0) continue;
      if (--dist[v] == 0) Q.append(v);
     }
   }
}



template<class cap_array, class flow_array, class cost_array>
bool price_refine_phase1(const graph_t& G, rcost_t eps,
                                          const cap_array& cap,
                                          const flow_array& flow,
                                          const cost_array& cost,
                                          const pot_array& pot,
                                          dist_array& dist,
                                          succ_array2& succ)
{

  // sort admissible network topologically

  node_queue<graph_t,succ_array2> Q(G,succ);

  int n = G.number_of_nodes();

  node v;
  forall_nodes(v,G)
  { 
    int deg = 0;

    edge e;
    forall_out_edges(e,v)
    { node w = target(G,e,v);
      if (flow[e] > 0  && -rcost(e,v,w) < 0) deg++;
     }

    forall_in_edges(e,v)
    { node w = source(G,e,v);
      if (flow[e] < cap[e] && rcost(e,w,v) < 0) deg++;
     }

    dist[v] = deg; // indeg[v]

    if (deg == 0) Q.append(v);
   }

  node u;
  for(u = Q.first(); u != Q.sentinel(); u = Q.succ(u))
  { n--; 
    edge e;
    forall_out_edges(e,u)
    { if (flow[e] == cap[e]) continue;
      node v = target(G,e,u);
      rcost_t rc = rcost(e,u,v);
      if (rc >= 0) continue;
      if (--dist[v] == 0) Q.append(v);
     }

    forall_in_edges(e,u)
    { if (flow[e] == 0) continue;
      node v = source(G,e,u);
      rcost_t rc = -rcost(e,v,u);
      if (rc >= 0 ) continue; 
      if (--dist[v] == 0) Q.append(v);
     }
   }

  if (n > 0) 
  { //cout << "cycle detected: n = " << n << endl;
    //price_refine_phase2(G,eps,cap,flow,cost,pot,dist,succ,n); 
    //cout << "after phase2:   n = " << n << endl;
    return false;  // cycle detected
  }


  for(u = Q.first(); u != Q.sentinel(); u = Q.succ(u))
  { int du = dist[u];
    edge e;
    forall_out_edges(e,u)
    { if (flow[e] == cap[e]) continue;
      node v = target(G,e,u);
      rcost_t rc = rcost(e,u,v);
      if (rc >= 0) continue;
      //int d = du + int((rc+0.5)/eps);
      int d = du + int(rc/eps+1);
      if (d < dist[v]) dist[v] = d;
     }

    forall_in_edges(e,u)
    { if (flow[e] == 0) continue;
      node v = source(G,e,u);
      rcost_t rc = -rcost(e,v,u);
      if (rc >= 0 ) continue; 
      //int d = du + int((rc+0.5)/eps);
      int d = du + int(rc/eps+1);
      if (d < dist[v]) dist[v] = d;
     }
   }

  return true;
}


template<class flow_array, class cap_array, class cost_array>
bool price_refinement(const graph_t& G, rcost_t eps, 
                                               const cap_array& cap,
                                               const flow_array& flow,
                                               const cost_array& cost,
                                               pot_array& pot, 
                                               succ_array2& succ, rcost_t old_eps)

{
  //float tt = used_time();
  //cout << string("price_refine(%8.0f) ",eps) << endl;

  int n = G.number_of_nodes();
  int m = G.number_of_edges();

  dist_array dist(G);

  node_pq22<int,graph_t> PQ(n+m);

  bool result = false;

  while (price_refine_phase1(G,eps,cap,flow,cost,pot,dist,succ)) 
  { 
    // admissible network is acyclic

    int count = 0;

    node v;
    forall_nodes(v,G) 
    { int dv = dist[v];
      assert(dv <= 0);
      if (dv < 0) 
      { count++;
        PQ.insert(v,dv);
       }
     }

    if (count == 0) // eps-optimal
    { result = true;
      break;
     }

    while (!PQ.empty())
    { node v = PQ.del_min();
      int dv = dist[v];
      if (dv > 0) continue;
      dist[v] = 1-dv;              // mark v as scanned (dist > 0)
      edge e;
      forall_out_edges(e,v)
      { if (cap[e] == flow[e]) continue;
        node w = target(G,e,v); 
        if (dist[w] > 0) continue; // already scanned
        rcost_t rc = rcost(e,v,w);
        int d = dv;
        if (rc >= 0) d += int(rc/eps+1);
        if (dist[w] > d) 
        { dist[w] = d;
          PQ.insert(w,d);
         }
      }
      
      forall_in_edges(e,v)
      { if (flow[e] == 0) continue;
        node w = source(G,e,v); 
        if (dist[w] > 0) continue; // already scanned
        rcost_t rc = -rcost(e,w,v);
        int d = dv;
        if (rc >= 0) d += int(rc/eps+1);
        if (dist[w] > d) 
        { dist[w] = d;
          PQ.insert(w,d);
         }
      }
   }

  // increase potential
  forall_nodes(v,G) {
   int d = dist[v];
   if (d > 0) d = 1-d; // unmark v
   pot[v] -= (d*eps);
  }
 }

/*
 if (result) cout << "success";
 else cout << "failed ";
*/

/*
 node v;
 forall_nodes(v,G) succ[v] = 0;
*/

 //cout << string(" time: %.2f sec",used_time(tt)) << endl;

 return result;
}


    
template<class flow_array, class cap_array, class cost_array>
int global_price_update(const graph_t& G, const cap_array& cap,
                                           const flow_array& flow,
                                           const excess_array& excess,
                                           const cost_array& cost,
                                           pot_array& pot,
                                           rcost_t eps)
{

 //float t1 = used_time();

 num_updates++;

 int n = G.number_of_nodes();
 //int m = G.number_of_edges();

 int inf = alpha*n;

 //node_pq22<int,graph_t> PQ(n+m);

 node_pq22<int,graph_t> PQ(2*n);

 dist_array dist(G);

 int e_count = 0;

 node v;
 forall_nodes(v,G) 
 { NT ev = excess[v];
   if (ev < 0)
     { dist[v] = 0;
       PQ.insert(v,0);
      }
   else 
     { dist[v] = inf;
       if (ev > 0) e_count++;
      }
  }


  int du;
  while (!PQ.empty()) 
  { 
    node u = PQ.del_min(du,dist);
    if (du != dist[u]) continue;

    if (excess[u] > 0 && --e_count == 0) break;

    rcost_t pu = pot[u];

    edge e;
    forall_out_edges(e,u)
    { if (flow[e] == 0) continue;
      node v = target(G,e,u);
      int dv = dist[v];
      if (dv <= du) continue;
      //rcost_t rc = -rcost(e,u,v);
      rcost_t rc = -(cost[e]-pu+pot[v]);
      int c = du;
      if (rc >=0) c += int(rc/eps)+1;
      if (c < dv)  
      {  PQ.insert(v,c,dist);
         dist[v] = c;
       }
     }

    forall_in_edges(e,u)
    { if (flow[e] == cap[e]) continue;
      node v = source(G,e,u);
      int dv = dist[v];
      if (dv <= du) continue;
      //rcost_t rc = rcost(e,v,u);
      rcost_t rc = cost[e]-pot[v]+pu;
      int c = du;
      if (rc >=0) c += int(rc/eps)+1;
      if (c < dv)  
      {  PQ.insert(v,c,dist);
         dist[v] = c;
       }
     }
   }


  forall_nodes(v,G) 
  { int dv = dist[v];
    if (dv > du) dv = du;
    pot[v] += dv*eps;
    //succ[v] = 0;
   }

  //cout << string("global price update:  %.2f sec",used_time(t1)) << endl;

  return du;
}

    
template<class flow_array, class cap_array, class cost_array>
int global_price_update2(const graph_t& G, const cap_array& cap,
                                          const flow_array& flow,
                                          const excess_array& excess,
                                          const cost_array& cost,
                                          pot_array& pot,
 node_level_queue<graph_t,succ_array2,pred_array>& PQ,
                                          rcost_t eps)
{

 //float t1 = used_time();

 num_updates++;

 int n = G.number_of_nodes();

 int inf = alpha*n;

 dist_array dist(G);

 int e_count = 0;

 node v;
 forall_nodes(v,G) 
 { NT ev = excess[v];
   if (ev < 0)
     { dist[v] = 0;
       PQ.insert(v,0);
      }
   else 
     { dist[v] = inf;
       if (ev > 0) e_count++;
      }
  }


  int du = 0;
  while (!PQ.empty()) 
  { 
    node u = PQ.del_min();
    du = dist[u];

    if (excess[u] > 0 && --e_count == 0) break;

    rcost_t pu = pot[u];

    edge e;
    forall_out_edges(e,u)
    { if (flow[e] == 0) continue;
      node v = target(G,e,u);
      int dv = dist[v];
      if (dv <= du) continue;
      //rcost_t rc = -rcost(e,u,v);
      rcost_t rc = -(cost[e]-pu+pot[v]);
      int c = du;
      if (rc >=0) c += int(rc/eps)+1;
      if (c < dv)  
      {  if (dv == inf) PQ.insert(v,c);
         else PQ.decrease_p(v,dv,c);
         dist[v] = c;
       }
     }

    forall_in_edges(e,u)
    { if (flow[e] == cap[e]) continue;
      node v = source(G,e,u);
      int dv = dist[v];
      if (dv <= du) continue;
      //rcost_t rc = rcost(e,v,u);
      rcost_t rc = cost[e]-pot[v]+pu;
      int c = du;
      if (rc >=0) c += int(rc/eps)+1;
      if (c < dv)  
      {  if (dv == inf) PQ.insert(v,c);
         else PQ.decrease_p(v,dv,c);
         dist[v] = c;
       }
     }
   }


  forall_nodes(v,G) 
  { int dv = dist[v];
    if (dv > du) dv = du;
    pot[v] += dv*eps;
   }

  PQ.clear();

  //cout << string("global price update:  %.2f sec",used_time(t1)) << endl;

  return du;
}


      


template <class cap_array, class cost_array, class flow_array>
bool cost_scaling(const graph_t& G, const cap_array& cap,
                                    const cost_array& cost,
                                    excess_array& excess,
                                    flow_array& flow)
{
  // returns true iff there exists a feasible flow

  int n = G.number_of_nodes();

  pot_array  pot;   // potential
  pot.use_node_data(G,0);

  succ_array succ;
  succ.use_node_data(G,0);

  succ_array2 succ2;
  succ2.use_node_data(G,0);

  pred_array pred;
  pred.use_node_data(G,0);

  node_level_queue<graph_t,succ_array2,pred_array> PQ(succ2,pred,alpha*n);

  node v;
  forall_nodes(v,G)
  { pot[v] = 0;
    succ[v] = 0;
   }

  NT C = 0;   // maximal cost of any edge in G

  cost_array& ca = (cost_array&)cost;

  edge e;
  forall_edges(e,G) 
  { flow[e] = 0;
    NT nc = n*cost[e];
    if (nc/n != cost[e]) error_handler(1,"cost overflow");
    ca[e] = nc;
    if (nc > C) C = nc;
    else if (-nc > C) C = -nc;
   }


  int eps = 1;
  while (eps < C) eps *= alpha;


  int dmax = 0;

  int refinement_bound = eps/alpha;

  feasible_flow<NT,graph_t,succ_array,dist_array> ff;  

  if (!ff.run(G,excess,cap,flow)) return false;

  forall_edges(e,G) flow[e] = 0;


/*
  if (beta > 1)
  { while (eps > 1 && price_refinement(G,eps/beta,cap,flow,cost,pot,succ2,eps)) 
    {  eps /= beta;
       //cout << eps << endl;
     }
  }
*/


  node_queue<graph_t,succ_array> active(G,succ);

  counter push_count = 0;
  counter discharge_count = 0;

  err_eps = 0;

  int relabel_count = 0;

  float af = af_factor;

  int af_count = 0;

  float rs_time = 0;
  int rs_count = 0;

restart:

  while (eps > 0)
  {
   float t1 = used_time();

   node v;

   // compute 0-optimal pseudoflow 

   forall_nodes(v,G)
   { edge e;
     forall_out_edges(e,v)
     { NT f = flow[e];
       node w = target(G,e,v);
       rcost_t rc = rcost(e,v,w);

       NT x = 0;
       if (rc < 0) x = cap[e] - f;
       if (rc > 0) x = -f;

       if (x != 0)
       { excess[v] -= x;
         excess[w] += x;
         flow[e] = f+x;
        }
      }
    }

  double total_excess = 0;

  forall_nodes(v,G) 
  { NT ev = excess[v];
    if (ev > 0) 
    { active.append(v);
      total_excess += ev;
     }
   }

    
   while (!active.empty())
   { 

     //if (relabel_count > 0.4*n + 30*active.size())
     if (relabel_count > int(f_update*n))
     { 
       //int d = global_price_update(G, cap, flow, excess, cost, pot, eps);
       int d = global_price_update2(G, cap, flow, excess, cost, pot, PQ, eps);
       if (d > dmax) dmax = d;
   
       num_relabels += relabel_count;
       relabel_count = 0;
      }

     discharge_count++;

     node v = active.pop();

     NT ev = excess[v];

     while (ev > 0)
     {
       rcost_t pv = pot[v];

       rcost_t min_rc = MAXINT;
       edge min_e = 0;
       NT   min_x = 0;

       edge e;
       forall_out_edges(e,v)
       { NT f = flow[e];
         NT x = cap[e] - f;
         if (x == 0) continue; 
         node w = target(G,e,v);
         //rcost_t rc = rcost(e,v,w);
         rcost_t rc = cost[e]-pv+pot[w];
         if (rc < 0)   // admissible
         { NT ew = excess[w];
           push_count++;
           if (ev < x) x = ev; 
           flow[e] = f + x;
           ew += x;
           ev -= x;
           excess[w] = ew;
           if (ew > 0 && !active.member(w)) active.append(w);
           if (ev == 0) goto FINISH_DISCHARGE;
          }
         else
         if (rc < min_rc) { min_rc = rc; min_e = e; min_x = x; }
       }
  
  
       forall_in_edges(e,v)
       { NT f = flow[e];
         if (f == 0) continue; 
         node w = source(G,e,v);
         //rcost_t rc = -rcost(e,w,v);
         rcost_t rc = -(cost[e]-pot[w]+pv);
         if (rc < 0) // admissible
         { NT ew = excess[w];
           push_count++;
           if (ev < f) f = ev;
           flow[e] -= f;
           ew += f;
           ev -= f;
           excess[w] = ew;
           if (ew > 0 && !active.member(w)) active.append(w);
           if (ev == 0) goto FINISH_DISCHARGE;
          }
         else
         if (rc < min_rc) { min_rc = rc; min_e = e; min_x = -f; }
       }


       relabel_count++;
       pot[v] += (eps+min_rc);

  
/*
       if (ev <= min_x || ev <= -min_x)
       { push_count++;
         if (min_x > 0)
         { node w = target(G,min_e,v);
           NT ew = excess[w];
           ew += ev;
           excess[w] = ew;
           flow[min_e] += ev;
           ev = 0;
           if (ew > 0 && !active.member(w)) active.append(w);
          }
         else
         { node w = source(G,min_e,v);
           NT ew = excess[w];
           ew += ev;
           excess[w] = ew;
           flow[min_e] -= ev;
           ev = 0;
           if (ew > 0 && !active.member(w)) active.append(w);
          }
        }
*/

FINISH_DISCHARGE:;

     } // while ev > 0


     excess[v] = 0;

    } // while active not empty

   if (af > 0)  // arc fixing
   {
     edge_filter<graph_t,cost_array,pot_array,rcost_t> F(G,cost,pot,eps,af);
     //af_count += G.hide_edges(F); // G.hide_edges(F,cap);
     edge e;
     forall_edges(e,G)
     { if (!F(e,G.source(e))) continue;
       ((graph&)G).hide_edge(e);
       af_count++;
      }
    }

   if (write_log) 
   { int err_count = 0;

     forall_nodes(v,G)
     { edge e;
       //forall_hidden_edges(e,v)
       forall_out_edges(e,v)
       { if (!G.is_hidden(e)) continue;
         node w = target(G,e,v);
         rcost_t rc = rcost(e,v,w);
         if (flow[e] < cap[e] &&  rc < -eps) err_count++;
         else if (flow[e] > 0   && -rc < -eps) err_count++;
        }
      }

     cout << string("eps = %9d cost = %12.0f pot = %14.0f fixed = %5d (%d)", 
                        eps, total_cost(G,cost,flow,n), total_pot(G,pot),
                        af_count,err_count);
     cout << endl;
    }

   if (beta > 1 && eps <= refinement_bound)
   { bool pref = true;
     while (eps/beta > 0 && pref)
     { pref = price_refinement(G,eps/beta,cap,flow,cost,pot,succ2,eps); 
       if (pref) eps /= beta;
       if (write_log) 
       { cout << string("eps = %9d cost = %12.0f pot = %14.0f", 
                         eps, total_cost(G,cost,flow,n), total_pot(G,pot));
         cout << endl;
        }
      }
    }

   //check_eps_optimality(G,cost,pot,cap,flow,eps,"after eps-phase");

   eps /= alpha;

   if (write_log)
   { cout << string("%.2f sec", used_time(t1)) << endl;
     cout << endl;
    }


  } // while eps > 0                                    


  num_relabels += relabel_count;
  num_discharges = discharge_count;
  num_pushes = push_count;


  if (af_count > 0) 
  { err_eps = compute_eps(G,cost,pot,cap,flow,af_count);
    ((graph&)G).restore_all_edges();
    if (err_eps >= alpha)
    { eps = alpha; 
      while (eps < err_eps/eps) eps *= alpha;
      af_count = 0;
      af *= 10; 
      if (write_log) cout << "restart:" << endl;
      if (rs_count++ == 0) rs_time = used_time();
      goto restart; 
     }
   }

  if (write_log && rs_count > 0)
  { cout << string("restarts: %d  time: %.2f sec",rs_count,used_time(rs_time));
    cout << endl;
   }

  ((graph&)G).restore_all_edges();

  if (write_log) cout << "dmax = " << dmax << endl;

  forall_edges(e,G) ca[e] /= n;

  return true;

}



public:

mcf_cost_scaling() : alpha(0), beta(0), f_update(1.0), af_factor(0.01),
                     num_nodes(0), num_edges(0), num_discharges(0), 
                     num_pushes(0), num_relabels(0), num_updates(0), 
                     write_log(false), T(0) {}


template<class lcap_array, class ucap_array, class cost_array, 
                           class supply_array, class flow_array>
bool run(const graph_t& G, const lcap_array& lcap,
                           const ucap_array& ucap,
                           const cost_array& cost,
                           const supply_array& supply,
                           flow_array&       flow,
                           float f = 0.01,
                           int a = 16,
                           int b = 16)
{

  T = used_time();

  num_discharges = 0;
  num_relabels = 0;
  num_updates  = 0;

  alpha = a;
  beta  = b;
  af_factor = f;

  num_nodes = G.number_of_nodes();
  num_edges = G.number_of_edges();


  excess_array excess;         // supply values after elimination of
  excess.use_node_data(G,0);   // lower capacity bounds

  ucap_array& cap = (ucap_array&)ucap;  // non-const reference to cap array
                                        // used to modify capacities
                                        // temporarily for elimination of
                                        // lower capacity bounds
  int lc_count = 0;

  node v;
  forall_nodes(v,G) excess[v] = supply[v];

  forall_nodes(v,G) 
  { edge e;
    forall_out_edges(e,v) 
    { NT lc = lcap[e];
      if (lc != 0)                  // nonzero lower capacity bound
      { node w  = target(G,e,v);
        excess[v] -= lc;
        excess[w] += lc;
        cap[e] -= lc;
        lc_count++;
       }
     }
  }
  



  // sort edges by capacities (decreasing)

  float tsort = used_time();

  ((graph&)G).sort_edges(cap);

  if (write_log)
    cout << string("sorting: %.2f sec",used_time(tsort)) << endl;


 
  bool feasible = cost_scaling(G, cap, cost, excess, flow);

  
  if (lc_count)
  { // adjust flow and restore capacities
    edge e;
    forall_edges(e, G) 
    { NT lc = lcap[e];
      if (lc != 0)
      { flow[e] += lc;
        cap[e] += lc;
       }
     }
   }

  T = used_time(T);


  if (feasible)
  { // check flow
    edge e;
    forall_edges(e,G)
    {
      if (flow[e] < 0 || flow[e] > cap[e])
           error_handler(1,"check_flow: illegal flow value");
     }
  
    forall_nodes(v,G)
    { NT ev = supply[v];
      edge e;
      forall_out_edges(e,v) ev -= flow[e];
      forall_in_edges(e,v) ev += flow[e];
      if (ev != 0)
           error_handler(1,"check_flow: non-zero excess");
     }
  }

  return feasible;
}


float cpu_time() { return T; }

int get_error() { return int(err_eps); }

template<class cost_array, class flow_array>
double total_cost(const graph_t& G, const cost_array& cost, 
                                    const flow_array& flow, int f=1)
{ edge e;
  double c = 0;
  forall_edges(e,G) c += double(cost[e]/f)*flow[e];
  return c;
}


rcost_t total_pot(const graph_t& G, const pot_array& pot)
{
  rcost_t P = 0;
  node v;
  forall_nodes(v,G) P += pot[v];
  return P;
}


 
void statistics(ostream& out)
{
  out << string("%5d nodes  %5d edges  a = %d  b = %d  af = %f",
         num_nodes,num_edges,alpha,beta,af_factor) << endl;
  out << string("%8d discharges %10d pushes %8d relabels %3d updates",
                   num_discharges, num_pushes,num_relabels, num_updates) << endl;

  out << endl;
}

void set_write_log(bool b) { write_log = b; }

};


LEDA_END_NAMESPACE

#if LEDA_ROOT_INCL_ID == 500353
#undef LEDA_ROOT_INCL_ID
#include <LEDA/internal/POSTAMBLE.h>
#endif
