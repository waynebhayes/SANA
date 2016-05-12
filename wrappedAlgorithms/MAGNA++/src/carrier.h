#ifndef _MAGNA_CARRIER_H
#define _MAGNA_CARRIER_H

#include "utils.h"
#include "graph.h"

extern char mg_finalstr[2048];

struct magnaparams {
    char* graph_file_name1;		// G
    char* graph_file_name2;		// H
	char* output_file_dir;
	char* output_file_pref;
	char* output_file_prefix;	// o
	int pop_size;					// p
	int n_gen;						// n
    int n_threads;                  // t    

	char* init_pop_file;			// i
	float pop_elite_ratio;		// e
	int freq;						// f
    float alpha;                      // a
    int use_nodematrix;
    char *nodematrix_file;      // d

    int edge_optimizing_measure_code;
    int pop_elite;
};

int write_simulation_parameters(struct magnaparams *s);

struct carrier {
    int rel;
    int use_alpha; // this is redundant but useful on occasion
    float alpha;
    float *cmpdata;
};

float* matrix_read(char *file_name, int n1, int n2);
float* matrix_read_csv(char *file_name, struct graph *g1, struct graph *g2);
struct carrier* carrier_create(struct graph *g1, struct graph *g2, int rel_rel, float alpha, char *cmpdata_file);
void carrier_delete(struct carrier *rel);

#endif
