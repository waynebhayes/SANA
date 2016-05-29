
extern __exportF list<edge> MAX_WEIGHT_BIPARTITE_MATCHING(graph& G,
                                const edge_array<int>& c,
                                node_array<int>& pot);

extern __exportF list<edge> MAX_WEIGHT_BIPARTITE_MATCHING(graph& G,
                                const edge_array<double>& c,
                                node_array<double>& pot);

extern __exportF list<edge> MAX_WEIGHT_BIPARTITE_MATCHING(graph& G,
                               const edge_array<int>& c);

extern __exportF list<edge> MAX_WEIGHT_BIPARTITE_MATCHING(graph& G,
                               const edge_array<double>& c);

extern __exportF list<edge> MAX_WEIGHT_BIPARTITE_MATCHING(graph& G, 
                               const list<node>& A, 
                               const list<node>& B,    
                         const edge_array<int>& c,  
                               node_array<int>& pot);

extern __exportF list<edge> MAX_WEIGHT_BIPARTITE_MATCHING(graph& G, 
                               const list<node>& A, 
                               const list<node>& B,    
                         const edge_array<double>& c,  
                               node_array<double>& pot);

extern __exportF list<edge> MAX_WEIGHT_BIPARTITE_MATCHING(graph& G, 
                const list<node>& A, const list<node>& B, const edge_array<int>& c);

extern __exportF list<edge> MAX_WEIGHT_BIPARTITE_MATCHING(graph& G, 
                const list<node>& A, const list<node>& B, const edge_array<double>& c);

extern __exportF bool CHECK_MWBM(const graph& G, const edge_array<int>& c,
        const list<edge>& M, const node_array<int>& pot);

extern __exportF bool CHECK_MWBM(const graph& G, const edge_array<double>& c,
        const list<edge>& M, const node_array<double>& pot);

extern __exportF list<edge> MAX_WEIGHT_ASSIGNMENT(graph& G,
                               const edge_array<int>& c,
                               node_array<int>& pot);

extern __exportF list<edge> MAX_WEIGHT_ASSIGNMENT(graph& G,
                               const edge_array<double>& c,
                               node_array<double>& pot);

extern __exportF list<edge> MAX_WEIGHT_ASSIGNMENT(graph& G, 
                 const list<node>& A, const list<node>& B,    const edge_array<int>& c, 
                                    node_array<int>& pot);

extern __exportF list<edge> MAX_WEIGHT_ASSIGNMENT(graph& G, 
                 const list<node>& A, const list<node>& B,    const edge_array<double>& c, 
                                    node_array<double>& pot);

extern __exportF bool CHECK_MAX_WEIGHT_ASSIGNMENT(const graph& G, const edge_array<int>& c,
        const list<edge>& M, const node_array<int>& pot);

extern __exportF bool CHECK_MAX_WEIGHT_ASSIGNMENT(const graph& G, const edge_array<double>& c,
        const list<edge>& M, const node_array<double>& pot);

extern __exportF list<edge> MIN_WEIGHT_ASSIGNMENT(graph& G,
                               const edge_array<int>& c,
                               node_array<int>& pot);

extern __exportF list<edge> MIN_WEIGHT_ASSIGNMENT(graph& G,
                               const edge_array<double>& c,
                               node_array<double>& pot);

extern __exportF list<edge> MIN_WEIGHT_ASSIGNMENT(graph& G,                                    
                      const list<node>& A, const list<node>& B,    
                      const edge_array<int>& c,
                      node_array<int>& pot);

extern __exportF list<edge> MIN_WEIGHT_ASSIGNMENT(graph& G,                                    
                      const list<node>& A, const list<node>& B,    
                      const edge_array<double>& c,
                      node_array<double>& pot);

extern __exportF bool CHECK_MIN_WEIGHT_ASSIGNMENT(const graph& G, const edge_array<int>& c,
        const list<edge>& M, const node_array<int>& pot);

extern __exportF bool CHECK_MIN_WEIGHT_ASSIGNMENT(const graph& G, const edge_array<double>& c,
        const list<edge>& M, const node_array<double>& pot);

extern __exportF list<edge> MAX_WEIGHT_ASSIGNMENT(graph& G,
                               const edge_array<int>& c);

extern __exportF list<edge> MAX_WEIGHT_ASSIGNMENT(graph& G,
                               const edge_array<double>& c);

extern __exportF list<edge> MAX_WEIGHT_ASSIGNMENT(graph& G,                                   
                      const list<node>& A, const list<node>& B, const edge_array<int>& c);

extern __exportF list<edge> MAX_WEIGHT_ASSIGNMENT(graph& G,                                   
                      const list<node>& A, const list<node>& B, const edge_array<double>& c);

extern __exportF list<edge> MIN_WEIGHT_ASSIGNMENT(graph& G,
                               const edge_array<int>& c);

extern __exportF list<edge> MIN_WEIGHT_ASSIGNMENT(graph& G,
                               const edge_array<double>& c);

extern __exportF list<edge> MIN_WEIGHT_ASSIGNMENT(graph& G,                                  
                      const list<node>& A, const list<node>& B,  const edge_array<int>& c);

extern __exportF list<edge> MIN_WEIGHT_ASSIGNMENT(graph& G,                                  
                      const list<node>& A, const list<node>& B,  const edge_array<double>& c);

extern __exportF list<edge> MWMCB_MATCHING(graph& G, 
                                         const list<node>& A, 
                                         const list<node>& B,  
                                  const edge_array<int>& c, 
                                        node_array<int>& pot);

extern __exportF list<edge> MWMCB_MATCHING(graph& G, 
                                         const list<node>& A, 
                                         const list<node>& B,  
                                  const edge_array<double>& c, 
                                        node_array<double>& pot);

extern __exportF list<edge> MWMCB_MATCHING(graph& G, 
                          const list<node>& A, 
                          const list<node>& B,
                          const edge_array<int>& c);

extern __exportF list<edge> MWMCB_MATCHING(graph& G, 
                          const list<node>& A, 
                          const list<node>& B,
                          const edge_array<double>& c);



