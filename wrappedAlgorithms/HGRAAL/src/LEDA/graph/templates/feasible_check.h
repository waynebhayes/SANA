
LEDA_BEGIN_NAMESPACE

template <class graph_t, class NT>
class zero_array {

typedef typename graph_t::node node;
typedef typename graph_t::edge edge;

public:

NT operator[](node x) const { return 0; }
NT operator[](edge x) const { return 0; }

};



template<class graph_t, class lcap_array, class ucap_array, class supply_array,
                                                            class flow_array>
bool ff_check(const graph_t& G, const lcap_array& lcap, 
                                const ucap_array& ucap,
                                const supply_array& supply, 
                                const flow_array& flow,
                                string& msg)
{
  // feasible flow check

  typedef typename graph_t::node node;
  typedef typename graph_t::edge edge;
  typedef typename flow_array::value_type NT;
  
  int count1 = 0;
  edge e;
  forall_edges(e,G)
    if (flow[e] < lcap[e] || flow[e] > ucap[e]) count1++;

  if (count1 > 0)
  { if (count1 == 1) msg = " 1 illegal flow value";
    if (count1 >= 2) msg = string("%d illegal flow values",count1);
   }

  int count2 = 0;

  node v;
  forall_nodes(v,G)
  { NT ev = supply[v];
    edge e;
    forall_out_edges(e,v) ev -= flow[e];
    forall_in_edges(e,v) ev += flow[e];
    if (ev != 0) count2++;
   }

  if (count2 > 0)
  { if (count2 == 1) msg += " + 1 non-zero excess value";
    if (count2 >= 2) msg += string(" + %d non-zero excess values",count2);
   }

  return count1 == 0 && count2 == 0;
}

template<class graph_t, class cap_array, class supply_array, class flow_array>
bool ff_check(const graph_t& G, const cap_array& cap,
                                const supply_array& supply, 
                                const flow_array& flow,
                                string& msg)
{
  typedef typename cap_array::value_type NT;

  zero_array<graph_t,NT> lcap;

  return ff_check(G,lcap,cap,supply,flow,msg);
}

LEDA_END_NAMESPACE
