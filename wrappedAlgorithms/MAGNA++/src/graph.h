#ifndef _MAGNA_GRAPH_H
#define _MAGNA_GRAPH_H

#include <stdio.h>
#include "permutation.h"
#include "utils.h"

#include <algorithm> // std::min
// #include <Eigen/SparseCore>
// typedef Eigen::SparseMatrix<int> SpMat;
// typedef Eigen::Triplet<int> EigTriplet;

struct vertex {
	char* name;
	int number;
};

struct vertex* vertex_malloc(char* name, int number);
struct vertex* vertex_realloc(char* name, int number);
int vertex_cmp(struct vertex* v1, struct vertex* v2);
void vertex_print(struct vertex* v);
void vertex_delete(struct vertex* v);

struct edge {
	int vertex1;
	int vertex2;
};

struct edge* edge_malloc(int vertex1, int vertex2);
struct edge* edge_realloc(int vertex1, int vertex2);
int edge_cmp(struct edge* e1, struct edge* e2);
void edge_print(struct edge* e);
void edge_delete(struct edge* e);

struct graph {
	struct vertex** vertex_set_by_name;
	struct vertex** vertex_set_by_number;
	int n_vertices;
	struct edge** edge_set;
	int n_edges;
};

struct graph* graph_malloc(void);
void graph_vertex_malloc(struct graph* g, int n_vertices);
void graph_edge_malloc(struct graph* g, int n_edges);

struct graph* graph_read(char* file_name);
void graph_edge_list_read(struct graph* g, FILE*input_file, char* file_name);
void graph_read_vertices(struct graph* g, FILE* input_file, char* file_name);
void graph_read_edges(struct graph* g, FILE* input_file, char* file_name);
void graph_sif_read(struct graph *g, FILE *input_file, char *file_name);

void graph_print_vertices(struct graph* g);
void graph_print_edges(struct graph* g);
void graph_sort_vertices(struct graph* g);
void graph_sort_edges(struct graph* g);
int graph_find_vertex_number(struct graph* g, char* name);
int graph_find_vertex_number_unsorted(struct graph* g, char* name);
char* graph_find_vertex_name(struct graph* g, int number);
int graph_find_edge(struct graph* g, struct edge* e);
int graph_is_edge(struct graph* g, int vertex1, int vertex2, struct edge* e_temp);
void graph_delete(struct graph* g);

struct alignment {
	struct graph* domain;
	struct graph* range;
	struct permutation* perm;
    //	struct permutation* invperm;
	int n_edges_preserved;
	int n_edges_induced;
	int n_edges_unique;
	float score, edge_score, node_score;
	int is_computed;
};

struct compute_aux_space {
//    SpMat cg; // composite graphs
//    std::vector<EigTriplet> cg_triplets;
//    int *v_weights;
    int *spA; // values
    int *spIA; // ind first elt of each row
    int *spJA; // col inds
    int *spJR; // col inds
    int *spJC; // row inds
    int *spINZ; // nnz of each row
    int *spWI; // for the accum. could be done w/ spINZ
    int *v_weights;    
};

struct alignment* alignment_calloc(struct graph* g1, struct graph* g2);
void alignment_randomize(struct alignment* a);
int alignment_read(struct alignment* a, char* file_name);
void alignment_tensor(struct alignment* a3, struct alignment* a1, struct alignment* a2, struct tensor_aux_space* taux);
int alignment_is_preserved(struct alignment* a, struct edge* e, struct edge* e_temp);
int alignment_is_mapped(struct alignment* a, struct edge* e, struct edge* e_temp);
int alignment_edge_correctness(struct alignment* a);
int alignment_induced_edge_correctness(struct alignment* a);
int alignment_compare(struct alignment* a1, struct alignment* a2, struct carrier* rel);
int alignment_compare_ec(const void* a1, const void* a2);
int alignment_compare_ics(const void* a1, const void* a2);
int alignment_compare_3s(const void* a1, const void* a2);
void alignment_update_inverse(struct alignment* a);
void alignment_write(struct alignment* a, char* file_name);
void alignment_graph_write(struct alignment* a, char* file_name);
void alignment_common_subgraph_write(struct alignment* a, char* file_name,
                                     compute_aux_space *caux);
void alignment_print(struct alignment* a);
void alignment_delete(struct alignment* a);
float alignment_nodescore_compute(struct alignment *a, struct carrier *rel);
float alignment_node_correctness(struct alignment *a);

// float alignment_partial_substructure_score(SpMat G, int *v_weights);
// void alignment_composite_graph(struct alignment *a, compute_aux_space *caux);
// void alignment_edges_preserved(SpMat& G, int *v_weights,
//                                struct alignment *a,
//                                int *n_edges_preserved, int *n_unique_edges);
void alignment_compute(struct alignment* a, struct carrier* rel,
                       compute_aux_space *caux);

struct compute_aux_space* compute_aux_space_malloc(struct graph *g1, struct graph *g2);
void compute_aux_space_delete(struct compute_aux_space* aux);

#endif
