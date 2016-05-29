/*******************************************************************************
+
+  LEDA 5.0.1  
+
+
+  mcf_cap_scaling1.t
+
+
+  Copyright (c) 1995-2005
+  by Algorithmic Solutions Software GmbH
+  All rights reserved.
+ 
*******************************************************************************/

// $Revision: 1.7 $  $Date: 2005/04/14 10:45:17 $

#if !defined(LEDA_ROOT_INCL_ID)
#define LEDA_ROOT_INCL_ID 500345
#include <LEDA/internal/PREAMBLE.h>
#endif

#include <LEDA/graph/graph_alg.h>
#include <LEDA/graph/node_pq22.h>
#include <LEDA/core/b_stack.h>
#include <LEDA/system/assert.h>


LEDA_BEGIN_NAMESPACE

template <class NT, class graph_t      = graph,
                    class pot_array    = node_array<NT,graph_t>,
                    class excess_array = node_array<NT,graph_t>,
                    class dist_array   = node_array<NT,graph_t>,
                    class pred_array   = node_array<edge,graph_t>,
                    class mark_array   = node_array<NT,graph_t>,
                    class node_pq_t    = node_pq22<NT,graph_t> >

class mcf_cap_scaling 
{
  typedef typename graph_t::node node;
  typedef typename graph_t::edge edge;
  typedef node_pq_t node_pq;
  
  float T;

  double total_c;


node source(const graph_t& G, edge e, node t)
{ if (graph_t::category() == opposite_graph_category)
    return G.opposite(e,t);
  else
    return G.source(e);
}
  
node target(const graph_t& G, edge e, node s)
{ if (graph_t::category() == opposite_graph_category)
    return G.opposite(e,s);
  else
    return G.target(e);
}



inline void flip_sign(const NT& x) { (NT&)x = -x; }


template<class cost_array, class cap_array, class flow_array>

node DIJKSTRA_IN_RESIDUAL(const graph_t& G, node s, int delta, 
                             const cost_array&   cost,
                             const cap_array&    cap,
                             const flow_array&   flow,
                             const excess_array& excess,
                             pot_array&  pi,
                             dist_array& dist,
                             pred_array& pred,
                             mark_array& mark,
                             node_pq&  PQ, int count)

{
  node t = nil;
  NT dt = MAXINT;

  dist[s] = 0;
  PQ.insert(s,0);
  mark[s] = count;

  while (!PQ.empty())
  { 
    node u = PQ.del_min(dist);

    NT  du = dist[u];
    if (du == -count) continue;

    PQ.push(u);

    du += pi[u]; 
    pi[u] = du;

    if (u == t) break;

    dist[u] = -count;

    edge e;
    forall_out_edges(e,u) 
    { if (cap[e]-flow[e] >= delta)  // e in delta-residual graph
      { node v = target(G,e,u);
        if (dist[v] == -count) continue;

        NT c = du - pi[v] + cost[e];
    
        if (c >= dt) continue;
        if (excess[v] <= -delta) { t = v; dt = c; }

        if ((mark[v] & 0x0fffffff) != count || c < dist[v])
        { PQ.insert(v,c); 
          dist[v] = c;
          pred[v] = e;
          mark[v] = count;
         }
       }
     }

    forall_in_edges(e,u) 
    { if (flow[e] >= delta)  // e in delta-residual graph
      { node v = source(G,e,u);
        if (dist[v] == -count) continue;

        NT c = du - pi[v] - cost[e];

        if (c >= dt) continue;
        if (excess[v] <= -delta) { t = v; dt = c; }

        if ((mark[v] & 0x0fffffff) != count || c < dist[v])
        { PQ.insert(v,c); 
          dist[v] = c;
          pred[v] = e;
          mark[v] = (count | 0x10000000);
         }
       }
     }

   }


  if (t)
  { /*
    node v = PQ.pop();
    while (v != 0) 
    { pi[v] -= dt;
      v = PQ.pop();
     }
     */
    for(node* p = PQ.Top(); p != PQ.Stop(); p++) pi[*p] -= dt;
   }

  PQ.clear(); 

  return t;
}




public:

mcf_cap_scaling() { T = 0; } 

template <class lcap_array, class ucap_array, class cost_array, 
          class supply_array, class flow_array>
bool run(const graph_t& G, const lcap_array& lcap,
                           const ucap_array& ucap,
                           const cost_array& cost,
                           const supply_array& supply, 
                           flow_array& flow, int f = 16)
{ 

  //list<edge> neg_edges;		// list of negative cost edges
  int lc_count = 0;             // number of low-cap edges

  ucap_array& cap = (ucap_array&)ucap;

  excess_array  excess(G,0);

  node v;
  forall_nodes(v, G) excess[v] = supply[v];

  forall_nodes(v,G)
  { edge e;
    forall_out_edges(e,v)
    { flow[e] = 0;

      assert(cost[e] >= 0);
      //if (cost[e] < 0) neg_edges.append(e);


      // removing nonzero lower bounds (Ahuja/Magnanti/Orlin, section 2.4,
      // p. 39 [fig. 2.19 is not correct!]
        
      NT lc = lcap[e];
  
      if (lc != 0) 
      {	// nonzero lower capacity
        lc_count++;
        node i = v;
        node j = target(G,e,v);
        excess[i] -= lc;
        excess[j] += lc;
        cap[e] -= lc;
       }
     }
   }


  // eliminate negative cost edges by edge reversals 
  // (Ahuja/Magnanti/Orlin, section 2.4, p. 40)

/*
  edge e;
  forall(e, neg_edges) 
  { node u = G.source(e);
    node v = G.target(e);
    excess[u] -= cap[e];
    excess[v] += cap[e];
    flip_sign(cost[e]);
    ((graph&)G).rev_edge(e);
   }
*/


  bool result = run0(G,cap,cost,excess,flow,f);

/*
  // restore negative edges
  edge x;
  forall(e, neg_edges) 
  { flow[e] = cap[e] - flow[e];
    ((graph&)G).rev_edge(x);
    flip_sign(cost[e]);
   } 
*/

  // adjust flow

  if (lc_count > 0)
  { edge e;
    forall_edges(e,G) flow[e] += lcap[e];
   }

  return result;
}


template <class cap_array, class cost_array, class flow_array>
bool run0(const graph_t& G, const cap_array& cap, const cost_array& cost,
                                                  excess_array& excess,
                                                  flow_array& flow, int f = 16)
{
  float t0 = used_time();

  int n = G.number_of_nodes();
  int m = G.number_of_edges();

  node v;
  edge e;

  double U = 0;		// maximal excess or capacity

  forall_edges(e, G) { 
    if (cap[e] > U) U = cap[e];
    flow[e] = 0;
    assert(cost[e] >= 0);
  }

  pot_array     pi(G,0);
  dist_array    dist(G,MAXINT);
  pred_array    pred(G,0);
  mark_array    mark(G,0);


  node_pq PQ(n+m);

  int count = 0;

  forall_nodes(v, G) 
  { NT ex = excess[v];
    if ( ex > U) U =  ex; 
    if (-ex > U) U = -ex;
   }


  int delta = 1;

  U = (U*m)/(n*n);

  while (U >= delta) delta *= 2;

  delta *= f;

  while (delta > 0) 
  { // delta scaling phase

//float t = used_time();

    // As long as there is a node s with excess >= delta compute a minimum
    // cost augmenting path from s to a sink node t with excess <= -delta in 
    // the delta-residual network and augment the flow by at least delta units 
    // along P (if there are nodes with excess > delta and excess < -delta
    // respectively

  node s;
  
  forall_nodes(s,G)
  { NT es = excess[s];

    if (es < delta) continue;


//float tt = used_time();

    int dcount = 0;
    while (es >= delta)
    { 
      dcount++;
      node t = DIJKSTRA_IN_RESIDUAL(G,s,delta,cost,cap,flow, excess,pi,
                                    dist,pred,mark,PQ,++count);

      if (t == nil) break; // there is no node with excess < -delta

      NT et = -excess[t];

      // compute maximal amount f of flow that can be pushed through P

      // NT f = min(es,et);

      NT f = es;
      if (f > et) f = et; 

      if (f > delta) 
      { for(v = t; v != s; v = G.opposite(e,v))
        { e = pred[v];
          NT rc = (mark[v]&0x10000000) ? flow[e] : cap[e]-flow[e];
          if (rc < f) f = rc;
         }
       }

      // add f units of flow along the augmenting path 

      for(v = t; v != s; v = G.opposite(e,v))
      { e = pred[v];
        if (mark[v]&0x10000000)
           flow[e] -= f;
        else
           flow[e] += f;
       }
  
/*
cout << string(" d = %5d", delta);
cout << string(" s = %5d (%5d)", G.index(s),es);
cout << string(" t = %5d (%5d)", G.index(t),et);
cout << string(" f = %5d", f);
cout << string(" T = %.2f", used_time(tt));
cout << endl;
*/

       es -= f;
       et -= f;

       excess[t] = -et;
    }

    excess[s] = es;

  } // end of delta-phase

//cout << string("t = %.2f sec",used_time(t)) << endl;

  delta /= 2;

  if (delta > 0)
  {
    // Saturate all edges entering the delta residual network which have
    // a negative reduced edge cost. Then only the reverse edge (with positive
    // reduced edge cost) will stay in the residual network. 

    node u;
    forall_nodes(u,G)
    { edge e;
      forall_out_edges(e,u)
      { NT fe = flow[e];
        if (fe >= delta)
        { node v = target(G,e,u);
          NT c = cost[e] + pi[u] - pi[v];
          if (c > 0)
          { excess[u] += fe;
            excess[v] -= fe;
            flow[e] = 0;
            continue;
           }
         }
        NT ce  = cap[e];
        NT cmf = ce-fe;
        if (cmf >= delta)
        { node v = target(G,e,u);
          NT c = cost[e] + pi[u] - pi[v];
          if (c < 0)
          { excess[u] -= cmf;
            excess[v] += cmf;
            flow[e] = ce;
           }
         }
       }
     }
   }
  } // end of scaling


  bool feasible = true;

  forall_nodes(v,G)
    if (excess[v]  != 0) feasible = false;

  total_c = 0;
  forall_edges(e,G) total_c += double(cost[e])*flow[e];

  T = used_time(t0);

  return feasible;
}


string name()       const { return "mincostflow capacity scaling"; }
float  cpu_time()   const { return T; }
double total_cost() const { return total_c; }
double result()     const { return total_c; }

void statistics(ostream& out) { out << endl; }

};


LEDA_END_NAMESPACE

#if LEDA_ROOT_INCL_ID == 500345
#undef LEDA_ROOT_INCL_ID
#include <LEDA/internal/POSTAMBLE.h>
#endif
