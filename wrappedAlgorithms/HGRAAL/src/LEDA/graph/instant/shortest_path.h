LEDA_BEGIN_NAMESPACE

extern __exportF bool SHORTEST_PATH(const graph& G, node s, 
                     const edge_array<int>& c, 
                     node_array<int>& dist, 
                     node_array<edge>& pred );

extern __exportF bool SHORTEST_PATH(const graph& G, node s, 
                     const edge_array<double>& c, 
                     node_array<double>& dist, 
                     node_array<edge>& pred );

extern __exportF node_array<int> CHECK_SP(const graph& G, node s, 
                           const edge_array<int>& c, 
                           const node_array<int>& dist,
                           const node_array<edge>& pred);

extern __exportF node_array<int> CHECK_SP(const graph& G, node s, 
                           const edge_array<double>& c, 
                           const node_array<double>& dist,
                           const node_array<edge>& pred);

extern __exportF void ACYCLIC_SHORTEST_PATH(const graph& G, node s, 
                             const edge_array<int>& c, 
                             node_array<int>& dist,
                             node_array<edge>& pred);

extern __exportF void ACYCLIC_SHORTEST_PATH(const graph& G, node s, 
                             const edge_array<double>& c, 
                             node_array<double>& dist,
                             node_array<edge>& pred);


extern __exportF void DIJKSTRA1(const graph& G, node s, 
                                               const edge_array<int>& cost,
                                               node_array<int>& dist, 
                                               node_array<edge>& pred);

extern __exportF void DIJKSTRA1(const graph& G, node s, 
                                               const edge_array<int>& cost,
                                               node_array<int>& dist);

extern __exportF void DIJKSTRA1(const graph& G, node s, node t,
                                               const edge_array<int>& cost,
                                               node_array<int>& dist, 
                                               node_array<edge>& pred);


extern __exportF void DIJKSTRA(const graph& G, node s, 
                                               const edge_array<int>& cost,
                                               node_array<int>& dist, 
                                               node_array<edge>& pred);

extern __exportF void DIJKSTRA(const graph& G, node s, 
                                               const edge_array<double>& cost,
                                               node_array<double>& dist, 
                                               node_array<edge>& pred);

extern __exportF void DIJKSTRA(const graph& G, node s, 
                                               const edge_array<int>& cost,
                                               node_array<int>& dist);

extern __exportF void DIJKSTRA(const graph& G, node s, 
                                               const edge_array<double>& cost,
                                               node_array<double>& dist);


extern __exportF int DIJKSTRA(const graph& G, node s, node t, 
                                                      const edge_array<int>& c,
                                                      node_array<edge>& pred);

extern __exportF double DIJKSTRA(const graph& G, node s, node t, 
                                                 const edge_array<double>& c,
                                                 node_array<edge>& pred);


extern __exportF int DIJKSTRA(const graph& G, node s, node t, 
                                                      const edge_array<int>& c,
                                                      node_array<int>& dist,
                                                      node_array<edge>& pred);

extern __exportF double DIJKSTRA(const graph& G, node s, node t, 
                                                 const edge_array<double>& c,
                                                 node_array<double>& dist,
                                                 node_array<edge>& pred);



extern __exportF bool BELLMAN_FORD_B(const graph& G, node s, 
                                                     const edge_array<int>& c, 
                                                     node_array<int>& dist, 
                                                     node_array<edge>& pred) ;

extern __exportF bool BELLMAN_FORD_B(const graph& G, node s, 
                                                   const edge_array<double>& c,
                                                   node_array<double>& dist,
                                                   node_array<edge>& pred) ;

extern __exportF bool BELLMAN_FORD(const graph& G, node s, 
                                                   const edge_array<int> & c,
                                                   node_array<int> & dist,
                                                   node_array<edge> & pred);

extern __exportF bool BELLMAN_FORD(const graph& G, node s, 
                                                   const edge_array<double> & c,
                                                   node_array<double> & dist,
                                                   node_array<edge> & pred);



extern __exportF bool ALL_PAIRS_SHORTEST_PATHS(graph& G, 
                              const edge_array<int>& c, 
                              node_matrix<int>& DIST);

extern __exportF bool ALL_PAIRS_SHORTEST_PATHS(graph& G, 
                              const edge_array<double>& c, 
                              node_matrix<double>& DIST);

extern __exportF void  MOORE(const graph& g, node s, const edge_array<int>& cost,
            node_array<int>& dist, node t = nil);

extern __exportF void  MOORE(const graph& g, node s, const edge_array<double>& cost,
            node_array<double>& dist, node t = nil);

LEDA_END_NAMESPACE
