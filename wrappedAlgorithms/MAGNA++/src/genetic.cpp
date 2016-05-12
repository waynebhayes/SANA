// Written by Vikram Saraph

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <libgen.h>
#include <string.h>
#include <pthread.h>
#include "genetic.h"

#define DEBUG_CROSS 0
#define SORT 1

struct population* population_malloc(int pop_size, int pop_elite, struct graph* g1, struct graph* g2, int n_threads) {
	struct population* pop = (struct population*)malloc(sizeof *pop);
	int pop_temp = pop_size - pop_elite;
	struct alignment** alignment_set = (struct alignment**)malloc((pop_size + pop_temp) * (sizeof *alignment_set));
	struct tensor_aux_space** tauxs = (struct tensor_aux_space **)malloc(n_threads * (sizeof *tauxs));
    struct compute_aux_space **cauxs;
    cauxs = (struct compute_aux_space **)malloc(n_threads*(sizeof *cauxs));	

    if (pop == NULL || alignment_set == NULL || tauxs == NULL) {
        mg_error("Error allocating alignment population. Not enough memory?");
        mg_quit(EXIT_FAILURE);
    }

	
	/* Allocate memory for alignments */
	int i;
	for (i = 0; i < pop_size + pop_temp; i++)
		alignment_set[i] = alignment_calloc(g1, g2);
	
	/* Allocate tensor auxiliary space */
	int degree = alignment_set[0]->perm->degree;
	for (i = 0; i < n_threads; i++) {
		tauxs[i] = tensor_aux_space_malloc(degree);
    }
	for (i = 0; i < n_threads; i++) {    
        cauxs[i] = compute_aux_space_malloc(g1,g2);
    }
    
	pop->alignment_set = alignment_set;
	pop->tauxs = tauxs;
    pop->cauxs = cauxs;    
	pop->pop_size = pop_size;
	pop->pop_elite = pop_elite;
	pop->pop_temp = pop_temp;
    pop->n_threads = n_threads;
	
	return pop;
}

struct population* population_read(struct carrier* rel, int pop_size, int pop_elite, struct graph* g1, struct graph* g2, char* file_name, int n_threads) {
	FILE* input_file = fopen(file_name, "r");
	if (input_file == NULL) {
		mg_error("Error reading population file: %s", file_name);
		mg_quit(EXIT_FAILURE);
	}
	
	printf("Reading alignments from: %s\n", file_name);
	
	struct population* pop = population_malloc(pop_size, pop_elite, g1, g2, n_threads);
	struct alignment** alignment_set = pop->alignment_set;

    char *input_file_dir = strdup(file_name);
    input_file_dir = dirname(input_file_dir);
	
	// Read in alignments
	int i;
	char aln_file_name[512];
    char aln_full_file_name[1024];
	struct alignment* a;
	for (i = 0; i < pop_size; i++) {
		if (fgets(aln_file_name, sizeof aln_file_name, input_file) == NULL)
			break;
		// get rid of newline character
//		int len = strlen(aln_file_name) - 1;
//		if (aln_file_name[len] == '\n')	aln_file_name[len] = '\0';
//        len = strlen(aln_file_name) - 1;
//		if (aln_file_name[len] == '\r')	aln_file_name[len] = '\0';        

        sprintf(aln_full_file_name, "%s/%s", input_file_dir, trimwhitespace(aln_file_name));
		
		a = alignment_set[i];
		alignment_read(a, aln_full_file_name);
	}
	
	// Randomize remaining
	int j;
	for (j = i; j < pop_size; j++)
		alignment_randomize(alignment_set[j]);
	
	pop->alignment_set = alignment_set;
	pop->pop_size = pop_size;
	pop->pop_elite = pop_elite;
		
	population_sort(pop, rel);
	
	fclose(input_file);
		
	return pop;
}

struct population* population_random(struct carrier* rel, int pop_size, int pop_elite, struct graph* g1, struct graph* g2, int n_threads) {
	struct population* pop = population_malloc(pop_size, pop_elite, g1, g2, n_threads);
	struct alignment** alignment_set = pop->alignment_set;
	
	int i;
	for (i = 0; i < pop_size; i++)
		alignment_randomize(alignment_set[i]);
		
	population_sort(pop, rel);
	
	return pop;
}

void population_sort(struct population* pop, struct carrier* rel) {
	int pop_size = pop->pop_size;
    int n_threads = pop->n_threads;
	struct alignment** alignment_set = pop->alignment_set;

    int ret;
	int i;
    if (n_threads == 1) {
        for (i = 0; i < pop_size; i++) {
            struct alignment* a = alignment_set[i];
            if (a->is_computed == 0)
                alignment_compute(a, rel, pop->cauxs[0]);
        }
	} else if (n_threads >= 1) {
        pthread_t threads[n_threads];
        //		struct parallel_compute_task tasks[n_threads];
		struct parallel_compute_task *tasks;
        tasks = (struct parallel_compute_task*)
            malloc(n_threads*sizeof(struct parallel_compute_task)); 
        if (tasks == NULL) { mg_error("Allocation error."); mg_quit(EXIT_FAILURE); }                       
		
		for (i = 0; i < n_threads; i++) {
			tasks[i].pop = pop;
			tasks[i].rel = rel;
			tasks[i].index = i;
			ret = pthread_create(threads+i, NULL, slice_alignment_compute, (void*)(tasks+i));
            if (ret != 0) {
                mg_error( "Error creating thread %d. Too many threads?", i);
                mg_quit(EXIT_FAILURE);
            }
		}
        
		for (i = 0; i < n_threads; i++)
			pthread_join(threads[i], NULL);

        free(tasks);
    }

    /*
	if (SORT == 0) {
		if (rel->rel == 0)
			qsort(alignment_set, pop_size, sizeof(struct alignment*), alignment_compare_ec);
		else if (rel->rel == 1)
			qsort(alignment_set, pop_size, sizeof(struct alignment*), alignment_compare_ics);
		else if (rel->rel == 2)
			qsort(alignment_set, pop_size, sizeof(struct alignment*), alignment_compare_3s);
	}
	else if (SORT == 1) */
    quick_locally_insertion(alignment_set, rel, pop_size);
}

void population_print(struct population* pop) {
	struct alignment** alignment_set = pop->alignment_set;
	int pop_size = pop->pop_size;
	int pop_temp = pop->pop_temp;
	int i;
	for (i = 0; i < pop_size + pop_temp; i++)
		printf("EC %p %d: %d\n", alignment_set[i], i, alignment_set[i]->n_edges_preserved);
}

void population_step_roulette(struct population* pop, struct carrier* rel, int cur_gen) {
	int pop_size = pop->pop_size;
	int pop_elite = pop->pop_elite;
	int pop_temp = pop->pop_temp;
    int n_threads = pop->n_threads;
	
	struct alignment** alignment_set = pop->alignment_set;
		
	/* Compute the weight of the population */
	int i;
	float pop_weight = 0;
	for (i = 0; i < pop_size; i++)
		pop_weight += alignment_set[i]->score;
	
	/* The roulette */
    if (n_threads == 1) {
        int crosses = 0;
        struct tensor_aux_space** tauxs = pop->tauxs;
        while (crosses < pop_temp) {
            int indiv1 = roulette(alignment_set, pop_size, pop_weight);
            int indiv2 = indiv1;
            while (indiv2 == indiv1)
                indiv2 = roulette(alignment_set, pop_size, pop_weight);
					
            struct alignment* a1 = alignment_set[indiv1];
            struct alignment* a2 = alignment_set[indiv2];
            struct alignment* a3 = alignment_set[pop_size + crosses];

            //srand((int)time(NULL));            
		
            alignment_tensor(a3, a1, a2, tauxs[0]);
            a3->is_computed = 0;
			
            crosses += 1;
        }
    } else if (n_threads >= 1) {
		pthread_t threads[n_threads];
        //		struct parallel_tensor_task tasks[n_threads];
		struct parallel_tensor_task *tasks;
        tasks = (struct parallel_tensor_task*)
            malloc(n_threads*sizeof(struct parallel_tensor_task)); 
        if (tasks == NULL) { mg_error("Allocation error."); mg_quit(EXIT_FAILURE); }       
		
		for (i = 0; i < n_threads; i++) {
			tasks[i].pop = pop;
			tasks[i].index = i;
			tasks[i].pop_weight = pop_weight;
            tasks[i].cur_gen = cur_gen;
			pthread_create(threads+i, NULL, slice_alignment_tensor, (void*)(tasks+i));
		}
		
		for (i = 0; i < n_threads; i++)
			pthread_join(threads[i], NULL);

        free(tasks);
	}

	/* Swap temp with new members */
	struct alignment* alignment_temp[pop_temp];
	memcpy(alignment_temp, alignment_set + pop_size, pop_temp*(sizeof *alignment_set));
	memcpy(alignment_set + pop_size, alignment_set + pop_elite, pop_temp*(sizeof *alignment_set));
	memcpy(alignment_set + pop_elite, alignment_temp, pop_temp*(sizeof *alignment_set));
	
	population_sort(pop, rel);
	
	if (DEBUG_CROSS == 1)
		population_print(pop);
}

void* slice_alignment_tensor(void *p) {
    struct parallel_tensor_task* task = (struct parallel_tensor_task*)p;    
	struct population* pop = task->pop;
	float pop_weight = task->pop_weight;
	int index = task->index;
	struct alignment** alignment_set = pop->alignment_set;
	struct tensor_aux_space** tauxs = pop->tauxs;
	int n_threads = pop->n_threads;
	int pop_size = pop->pop_size;
	int pop_temp = pop->pop_temp;

    srand(((int)time(0)+task->cur_gen) ^ index);
	
	int i;
	for (i = pop_size + index; i < pop_size + pop_temp; i += n_threads) {
		int indiv1 = roulette(alignment_set, pop_size, pop_weight);
		int indiv2 = indiv1;
		while (indiv2 == indiv1)
			indiv2 = roulette(alignment_set, pop_size, pop_weight);
		
		struct alignment* a1 = alignment_set[indiv1];
		struct alignment* a2 = alignment_set[indiv2];
		struct alignment* a3 = alignment_set[i];
		
		alignment_tensor(a3, a1, a2, tauxs[index]);
		a3->is_computed = 0;
	}
	
	return 0;
}
    
void* slice_alignment_compute(void *p) {
    struct parallel_compute_task* task = (struct parallel_compute_task*)p;
	struct population* pop = task->pop;
	struct carrier* rel = task->rel;
	int index = task->index;
	struct alignment** alignment_set = pop->alignment_set;
	int n_threads = pop->n_threads;
	int pop_size = pop->pop_size;
    int pop_temp = pop->pop_temp;
    
	int i;

	for (i = index; i < pop_size + pop_temp; i += n_threads) {
		if (i >= pop_size) break;
		struct alignment* a = alignment_set[i];
		if (a->is_computed == 0)
			alignment_compute(a, rel, pop->cauxs[index]);
	}
    
	return 0;
}

void run_simulation(char* graph_file1, char* graph_file2, char* init_pop_file, char* output_file_name, int rel_rel, int pop_size, int pop_elite, int n_gen, int freq, float alpha, char *cmpdata_file, int n_threads) {

    struct graph* g1=NULL,*g2=NULL;
    struct carrier *rel=NULL;
	struct population* pop=NULL;    

	srand(time(0));

    g1 = graph_read(graph_file1);
	g2 = graph_read(graph_file2);

    if (g1->n_vertices > g2->n_vertices) {
        mg_error("The number of nodes in Network 1 need to be less or equal to to the nodes in Network 2");
        mg_quit(EXIT_FAILURE);
    }

	rel = carrier_create(g1, g2, rel_rel, alpha, cmpdata_file);

	// Random or initialize?
	if (init_pop_file == NULL)
		pop = population_random(rel, pop_size, pop_elite, g1, g2, n_threads);
	else
		pop = population_read(rel, pop_size, pop_elite, g1, g2, init_pop_file, n_threads);
	
	// Optimizing what?
	char opt_str[10];
	if (rel->rel == 0)
		sprintf(opt_str, "EC");
	else if (rel->rel == 1)
		sprintf(opt_str, "ICS");
	else if (rel->rel == 2)
		sprintf(opt_str, "S3");
	
	int period;
    if (freq!=0) period = n_gen / freq;
    else period = -1;
	char alignment_file[256];
	
	// save 0th generation
    if (freq!=0) {
        sprintf(alignment_file, "%s_%s_%d_%d_%d.txt", output_file_name, opt_str, pop_size, n_gen, 0);
        population_save_best(pop, alignment_file); 
        sprintf(alignment_file, "%s_stats.txt", output_file_name);
        population_save_best_stats(0, pop, alignment_file);
    }
    
	// run, save stats with each generation
	int i;
	for (i = 0; i < n_gen; i++) {
		if (DEBUG_CROSS == 1)
			printf("GENERATION %d\n", i);
		population_step_roulette(pop, rel, i+1);
		if ((freq!=0) && ((i+1) % period == 0)) {
			sprintf(alignment_file, "%s_%s_%d_%d_%d.txt", output_file_name, opt_str, pop_size, n_gen, i+1);
			population_save_best(pop, alignment_file);
			sprintf(alignment_file, "%s_stats.txt", output_file_name);
            population_save_best_stats(i+1, pop, alignment_file);
		}
	}

    char tmpstr[1024];
    mg_finalstr[0] = '\0';
    strcat(mg_finalstr, "MAGNA++ has finished running.\n");
    sprintf(alignment_file, "%s_final_stats.txt", output_file_name);
    population_save_best_stats(-1, pop, alignment_file);    
    sprintf(tmpstr,"Final alignment statistics saved to %s\n",alignment_file);
    printf(tmpstr); strcat(mg_finalstr,tmpstr);
    
    sprintf(alignment_file, "%s_final_alignment.txt", output_file_name);
    population_save_best(pop, alignment_file);    
    sprintf(tmpstr,"Final alignment saved to %s\n",alignment_file);
    printf(tmpstr); strcat(mg_finalstr,tmpstr);    

    sprintf(alignment_file, "%s_final_visualize.sif", output_file_name);
    alignment_common_subgraph_write(pop->alignment_set[0],alignment_file,pop->cauxs[0]);
    sprintf(tmpstr,"Common conserved subgraph saved to %s\n",alignment_file);
    printf(tmpstr); strcat(mg_finalstr,tmpstr);

	carrier_delete(rel);
	graph_delete(g1);
	graph_delete(g2);
	population_delete(pop);
}

void population_save_best_stats(int geni, struct population* pop, char* stats_file) {
	struct alignment* a = pop->alignment_set[0];
    char genstr[10];
    if (geni!=-1) sprintf(genstr,"%d",geni);
    else sprintf(genstr,"final");
    #if 0
    printf("generation %s\nscore %f\nnode_score %f\nEC_score %f\nICS_score %f\nS3_score %f\nnode_correctness %f\ndom_verts %d\ndom_edges %d\nran_verts %d\nran_edges %d\n\n",
           genstr,
           a->score,
           a->node_score,
           (float)a->n_edges_preserved/a->domain->n_edges,
           (float)a->n_edges_preserved/a->n_edges_induced,
           (float)a->n_edges_preserved/a->n_edges_unique,
           alignment_node_correctness(a),
           a->domain->n_vertices, a->domain->n_edges,
           a->range->n_vertices, a->range->n_edges);
#else
    printf("generation %s\nscore %f\nnode_score %f\nEC_score %f\nICS_score %f\nS3_score %f\nnode_correctness %f\n\n",
           genstr,
           a->score,
           a->node_score,
           (float)a->n_edges_preserved/a->domain->n_edges,
           (float)a->n_edges_preserved/a->n_edges_induced,
           (float)a->n_edges_preserved/a->n_edges_unique,
           alignment_node_correctness(a));
#endif    
	FILE* output_fd = NULL;    
    if (geni==0 || geni==-1) output_fd = fopen(stats_file,"w");
    else output_fd = fopen(stats_file, "a");
    if (output_fd == NULL) {
        printf("population_save_best_stats: Couldn't open file: %s\n", stats_file);
        mg_quit(EXIT_FAILURE);
    }
#if 0
    fprintf(output_fd, "generation %s\nscore %f\nnode_score %f\nEC_score %f\nICS_score %f\nS3_score %f\nnode_correctness %f\ndom_verts %d\ndom_edges %d\nran_verts %d\nran_edges %d\n\n",
            genstr,
            a->score,
            a->node_score,
           (float)a->n_edges_preserved/a->domain->n_edges,
           (float)a->n_edges_preserved/a->n_edges_induced,
           (float)a->n_edges_preserved/a->n_edges_unique,
            alignment_node_correctness(a),
            a->domain->n_vertices, a->domain->n_edges,
            a->range->n_vertices, a->range->n_edges);            
#else
    fprintf(output_fd, "generation %s\nscore %f\nnode_score %f\nEC_score %f\nICS_score %f\nS3_score %f\nnode_correctness %f\n\n",
            genstr,
            a->score,
            a->node_score,
           (float)a->n_edges_preserved/a->domain->n_edges,
           (float)a->n_edges_preserved/a->n_edges_induced,
           (float)a->n_edges_preserved/a->n_edges_unique,
            alignment_node_correctness(a));
#endif    
    fclose(output_fd);
}    

void population_save_best(struct population* pop, char* alignment_file) {	
	struct alignment* best_alignment = pop->alignment_set[0];
	alignment_write(best_alignment, alignment_file);
}

void population_delete(struct population* pop) {
	int pop_size = pop->pop_size;
	int pop_temp = pop->pop_temp;
	int n_threads = pop->n_threads;
	
	struct alignment** alignment_set = pop->alignment_set;
	int i;
	for (i = 0; i < pop_size + pop_temp; i++)
		if (alignment_set[i] != NULL)
			alignment_delete(alignment_set[i]);
	struct tensor_aux_space** tauxs = pop->tauxs;
    struct compute_aux_space **cauxs = pop->cauxs;
	for (i = 0; i < n_threads; i++)
		if (tauxs[i] != NULL)
			tensor_aux_space_delete(tauxs[i]);
	for (i = 0; i < n_threads; i++)
        if (cauxs[i] != NULL)
            compute_aux_space_delete(cauxs[i]);    
	free(pop->alignment_set);
	free(pop->tauxs);
	free(pop->cauxs);
	free(pop);
}

inline void swap_align(struct alignment** a1, struct alignment** a2) {
	struct alignment* temp = *a1;
	*a1 = *a2;
	*a2 = temp;
}

void insertion_sort(struct alignment** alignment_set, struct carrier* rel, int first, int last) {
	struct alignment** as = alignment_set;

	int i;
	// Move through interval
	for(i = first + 1; i < last; i++) {
		int j;
		// Move the next element in
		for(j = i; j > first; j--) {
			if (alignment_compare(as[j], as[j-1], rel) <= 0)
				swap_align(alignment_set + j, alignment_set + (j-1));
			// If no swaps made, element is in its place
			else break;
		}
	}
}

int partition(struct alignment** alignment_set, struct carrier* rel, int first, int last) {
	struct alignment** as = alignment_set;
	
	// Layout of interval: p xxxxx... xxx e
	// p - pivot, x - element, e - end
	int left = first + 1;
	int right = last - 1;
	
	// Keep going till left and right cross paths
	while (left < right) {
		
		// Move right index
		while ((right > first) && (alignment_compare(as[first], as[right], rel) <= 0))
			right--;
		// Pathological case: right reaches the beginning
		// In this case, no swaps needed
		if (right == first)
			return first;
		
		// Move left index
		// No segfaults here, by the magic of lazy evaluation
		while ((left < last) && (alignment_compare(as[left], as[first], rel) <= 0))
			left++;
		// Pathological case: left reaches the end
		// Move pivot to the end
		if (left == last) {
			swap_align(alignment_set + first, alignment_set + (last-1));
			return last - 1;
		}
		
		// Swap appropriate elements
		if (left < right)
			swap_align(alignment_set + left, alignment_set + right);
		
	}
	// Move pivot
	swap_align(alignment_set + first, alignment_set + right);
	return right;
}

void sub_quick_locally_insertion(struct alignment** alignment_set, struct carrier* rel, int first, int last) {
	struct alignment** as = alignment_set;
	
	// Trivially sorted
	if (last - first <= 1)
		return;
	// Swap two element if necessary
	else if (last - first <= 2) {
		if (alignment_compare(as[last-1], as[first], rel) <= 0)
			swap_align(alignment_set + first, alignment_set + (last-1));
	}
	// Call Insertion Sort on small enough interval
	else if (last - first <= 10)
		insertion_sort(alignment_set, rel, first, last);
	// Otherwise, partition the interval
	else {
		// Randomize the selected pivot,
		// no deliberately triggering worse-case performance!!!
		int randIndex = first + rand() % (last - first);
		swap_align(alignment_set + first, alignment_set + randIndex);
		
		// Create partitions
		int mid = partition(alignment_set, rel, first, last);
		
		// Recurse on the partitions
		sub_quick_locally_insertion(alignment_set, rel, first, mid);
		sub_quick_locally_insertion(alignment_set, rel, mid + 1, last);
	}
}

void quick_locally_insertion(struct alignment** alignment_set, struct carrier* rel, int n) {
	sub_quick_locally_insertion(alignment_set, rel, 0, n);
}

int roulette(struct alignment** alignment_set, int n, float total_weight) {
	float w = ((float)rand() / RAND_MAX) * (total_weight + n);
	float sum = 0;
	int i;
	for (i = 0; i < n; i++) {
		sum += alignment_set[i]->score + 1;
		if (sum > w)
			return i;
	}
	return n - 1;
}
