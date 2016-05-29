

extern __exportF
list<edge> MAX_WEIGHT_MATCHING(const graph& G, 
                               const edge_array<int>& w, 
                               bool check = true, int heur = 2);



extern __exportF
list<edge> MAX_WEIGHT_MATCHING(const graph& G, 
                               const edge_array<int>& w, 
                               node_array<int>& pot, 
                               array<two_tuple<int, int> >& BT, 
                               node_array<int>& b,
                               bool check = true, int heur = 2);


extern __exportF
bool CHECK_MAX_WEIGHT_MATCHING(const graph& G,
                               const edge_array<int>& w,
                               const list<edge>& M,
                               const node_array<int>& pot,
                               const array<two_tuple<int, int> >& BT,
                               const node_array<int>& b);

extern __exportF
list<edge> MAX_WEIGHT_PERFECT_MATCHING(const graph& G, 
                                       const edge_array<int>& w, 
                                       bool check = true, int heur = 2);

extern __exportF
list<edge> MAX_WEIGHT_PERFECT_MATCHING(const graph& G, 
                                       const edge_array<int>& w, 
                                       node_array<int>& pot, 
                                       array<two_tuple<int, int> >& BT, 
                                       node_array<int>& b,
                                       bool check = true, int heur = 2);

extern __exportF
bool CHECK_MAX_WEIGHT_PERFECT_MATCHING(const graph& G,
                                       const edge_array<int>& w,
                                       const list<edge>& M,
                                       const node_array<int>& pot,
                                       const array<two_tuple<int, int> >& BT,
                                       const node_array<int>& b);


extern __exportF
list<edge> MIN_WEIGHT_PERFECT_MATCHING(const graph& G, 
                                       const edge_array<int>& w, 
                                       bool check = true, int heur = 2);

extern __exportF
list<edge> MIN_WEIGHT_PERFECT_MATCHING(const graph& G, 
                                       const edge_array<int>& w, 
                                       node_array<int>& pot, 
                                       array<two_tuple<int, int> >& BT, 
                                       node_array<int>& b,
                                       bool check = true, int heur = 2);

extern __exportF
bool CHECK_MIN_WEIGHT_PERFECT_MATCHING(const graph& G,
                                       const edge_array<int>& w,
                                       const list<edge>& M,
                                       const node_array<int>& pot,
                                       const array<two_tuple<int, int> >& BT,
                                       const node_array<int>& b);

extern __exportF
bool CHECK_WEIGHTS(const graph& G, edge_array<int>& w, bool perfect);



// double weights




extern __exportF
list<edge> MAX_WEIGHT_MATCHING(const graph& G, 
                               const edge_array<double>& w, 
                               bool check = true, int heur = 2);



extern __exportF
list<edge> MAX_WEIGHT_MATCHING(const graph& G, 
                               const edge_array<double>& w, 
                               node_array<double>& pot, 
                               array<two_tuple<double, int> >& BT, 
                               node_array<int>& b,
                               bool check = true, int heur = 2);


extern __exportF
bool CHECK_MAX_WEIGHT_MATCHING(const graph& G,
                               const edge_array<double>& w,
                               const list<edge>& M,
                               const node_array<double>& pot,
                               const array<two_tuple<double, int> >& BT,
                               const node_array<int>& b);

extern __exportF
list<edge> MAX_WEIGHT_PERFECT_MATCHING(const graph& G, 
                                       const edge_array<double>& w, 
                                       bool check = true, int heur = 2);

extern __exportF
list<edge> MAX_WEIGHT_PERFECT_MATCHING(const graph& G, 
                                       const edge_array<double>& w, 
                                       node_array<double>& pot, 
                                       array<two_tuple<double, int> >& BT, 
                                       node_array<int>& b,
                                       bool check = true, int heur = 2);

extern __exportF
bool CHECK_MAX_WEIGHT_PERFECT_MATCHING(const graph& G,
                                       const edge_array<double>& w,
                                       const list<edge>& M,
                                       const node_array<double>& pot,
                                       const array<two_tuple<double, int> >& BT,
                                       const node_array<int>& b);


extern __exportF
list<edge> MIN_WEIGHT_PERFECT_MATCHING(const graph& G, 
                                       const edge_array<double>& w, 
                                       bool check = true, int heur = 2);

extern __exportF
list<edge> MIN_WEIGHT_PERFECT_MATCHING(const graph& G, 
                                       const edge_array<double>& w, 
                                       node_array<double>& pot, 
                                       array<two_tuple<double, int> >& BT, 
                                       node_array<int>& b,
                                       bool check = true, int heur = 2);

extern __exportF
bool CHECK_MIN_WEIGHT_PERFECT_MATCHING(const graph& G,
                                       const edge_array<double>& w,
                                       const list<edge>& M,
                                       const node_array<double>& pot,
                                       const array<two_tuple<double, int> >& BT,
                                       const node_array<int>& b);

extern __exportF
bool CHECK_WEIGHTS(const graph& G, edge_array<double>& w, bool perfect);



// double weights


