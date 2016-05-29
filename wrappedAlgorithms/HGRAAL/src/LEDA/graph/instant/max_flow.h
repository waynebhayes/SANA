
extern __exportF int MAX_FLOW(const graph& G, node s, node t,
                 const edge_array<int>& cap,
                 edge_array<int>& f);

extern __exportF double MAX_FLOW(const graph& G, node s, node t,
                 const edge_array<double>& cap,
                 edge_array<double>& f);

extern __exportF int MAX_FLOW(const graph& G, node s, node t,
                 const edge_array<int>& cap,
                 edge_array<int>& flow, list<node>& st_cut);

extern __exportF double MAX_FLOW(const graph& G, node s, node t,
                 const edge_array<double>& cap,
                 edge_array<double>& flow, list<node>& st_cut);

extern __exportF int MAX_FLOW(const graph& G, node s, node t,
                 const edge_array<int>& cap,
                 edge_array<int>& f,
                 int& num_pushes, 
                 int& num_edge_inspections,
                 int& num_relabels,
                 int& num_global_relabels,
                 int& num_gaps,
                 float h = 5.0);

extern __exportF double MAX_FLOW(const graph& G, node s, node t,
                 const edge_array<double>& cap,
                 edge_array<double>& f,
                 int& num_pushes, 
                 int& num_edge_inspections,
                 int& num_relabels,
                 int& num_global_relabels,
                 int& num_gaps,
                 float h = 5.0);



extern __exportF bool CHECK_MAX_FLOW(const graph& G, node s, node t,
                      const edge_array<int>& cap,
                      const edge_array<int>& f);

extern __exportF bool CHECK_MAX_FLOW(const graph& G, node s, node t,
                      const edge_array<double>& cap,
                      const edge_array<double>& f);


extern __exportF int MAX_FLOW(const graph& G, node s, node t,
                 const edge_array<int>& lcap, const edge_array<int>& ucap,
                 edge_array<int>& f);

extern __exportF double MAX_FLOW(const graph& G, node s, node t,
                 const edge_array<double>& lcap, const edge_array<double>& ucap,
                 edge_array<double>& f);

