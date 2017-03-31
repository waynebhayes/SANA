#ifndef _MAGNA_GENETIC_H
#define _MAGNA_GENETIC_H

#include "graph.h"

struct population {
	struct alignment** alignment_set;
	int pop_size;
	int pop_elite;
	int pop_temp;
    int n_threads;
	struct tensor_aux_space** tauxs;
    struct compute_aux_space **cauxs;    
};

struct population* population_malloc(int pop_size, int pop_elite, struct graph* g1, struct graph* g2, int n_threads);
struct population* population_read(struct carrier* rel, int pop_size, int pop_elite, struct graph* g1, struct graph* g2, char* file_name, int n_threads);
struct population* population_random(struct carrier* rel, int pop_size, int pop_elite, struct graph* g1, struct graph* g2, int n_threads);
void population_sort(struct population* pop, struct carrier* rel);
void population_step_roulette(struct population* pop, struct carrier* rel);
void run_simulation(char* graph_file1, char* graph_file2, char* init_pop_file, char* output_file_name, int rel_rel, int pop_size, int pop_elite, int n_gen, int freq, float alpha, char *cmpdata_file, int n_threads);
void population_save_best_stats(int geni, struct population* pop, char* stats_file);
void population_save_best(struct population* pop, char* alignment_file);
void population_delete(struct population* pop);

void swap_align(struct alignment** a1, struct alignment** a2);
void insertion_sort(struct alignment** alignment_set, struct carrier* rel, int first, int last);
int partition(struct alignment** alignment_set, struct carrier* rel, int first, int last);
void sub_quick_locally_insertion(struct alignment** alignment_set, struct carrier* rel, int first, int last);
void quick_locally_insertion(struct alignment** alignment_set, struct carrier* rel, int n);
int roulette(struct alignment** alignment_set, int n, float total_weight);

/* Parallel interface */
struct parallel_compute_task {
	struct population* pop;
	struct carrier* rel;
	int index;
};

struct parallel_tensor_task {
	struct population* pop;
	float pop_weight;
	int index;
	int cur_gen;
};

void* slice_alignment_compute(void *p);
void* slice_alignment_tensor(void *p);

#endif
