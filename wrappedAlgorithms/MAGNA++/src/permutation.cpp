// Written by Vikram Saraph

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "permutation.h"

struct permutation* permutation_calloc(int degree) {
	struct permutation* p;
	p = (struct permutation*)malloc(sizeof *p);
	int* sequence = (int*)malloc(degree * sizeof(int));
    if (sequence == NULL) {
        mg_error("Error allocating alignment. Not enough memory?");
        mg_quit(EXIT_FAILURE);
    }
	
	int i;
	for (i = 0; i < degree; i++)
		sequence[i] = i;
	
	p->degree = degree;
	p->sequence = sequence;
	
	return p;
}

void permutation_randomize(struct permutation* p) {
	knuth_shuffle(p);
}

void permutation_delete(struct permutation* p) {
	free(p->sequence);
	free(p);
}

struct cycle_representation* cycle_representation_calloc(int degree) {
	struct cycle_representation* cr;
	cr = (struct cycle_representation*)malloc(sizeof *cr);
	int* cycle_data = (int*)malloc(degree * sizeof(int));
    if(cycle_data==NULL) { mg_error("Error allocating cycle data. Not enough memory?"); mg_quit(EXIT_FAILURE); }
	
	int i;
	for (i = 0; i < degree; i++)
		cycle_data[i] = i;
	
	cr->degree = degree;
	cr->cycle_data = cycle_data;
	
	return cr;
}

void cycle_representation_delete(struct cycle_representation* cr) {
	free(cr->cycle_data);
	free(cr);
}

int cycle_decomposition(struct cycle_representation* cr, struct permutation* p) {
	if (cr->degree != p->degree) {
		printf("Mismatch in degree\n");
		return 1;
	}
	
	int n = p->degree;
	int* sequence = p->sequence;	
	int* cycle_data = cr->cycle_data;
	
	int* mapped = (int*)calloc(n, sizeof(int));
    if(mapped==NULL) { mg_error("Error allocating cycle map. Not enough memory?"); mg_quit(EXIT_FAILURE); }    
	
	int i;
	int heap_index = n;
	for (i = 0; i < n; i++) {
		if (mapped[i] == 1)
			continue;
		
		int next_i = i;
		int stack_index = 0;
		do {
			mapped[next_i] = 1;
			cycle_data[stack_index] = next_i;
			stack_index += 1;
			heap_index -= 1;
			next_i = sequence[next_i];
		} while (next_i > i);
		
		memmove(cycle_data + heap_index, cycle_data, stack_index*sizeof(int));
	}
	
	free(mapped);
	
	return 0;
}

void cycle_print(struct cycle_representation* cr) {
	int* cycle_data = cr->cycle_data;
	int start = 0;
	int index = 0;
	while (1) {
		printf("(");
		while (1) {
			printf("%d ", cycle_data[index]);
			index += 1;
			if (index >= cr->degree) {
				printf(")\n");
				return;
			}
			else if (cycle_data[index] < cycle_data[start]) {
				start = index;
				break;
			}
		}
		printf(")");
	}
}

void permutation_print(struct permutation* p) {
	int i;
	for (i = 0; i < p->degree; i++)
		printf("%d ", p->sequence[i]);
	printf("\n");
}

struct tensor_aux_space* tensor_aux_space_malloc(int degree) {
	struct tensor_aux_space* aux;
	aux = (struct tensor_aux_space*)malloc(sizeof *aux); 
    if(aux==NULL) { mg_error("Error allocating auxiliary space. Not enough memory?"); mg_quit(EXIT_FAILURE); }   
	struct permutation* path = permutation_calloc(degree);
	struct cycle_representation* cr = cycle_representation_calloc(degree);
	struct permutation* half_path = permutation_calloc(degree);
	
	aux->path = path;
	aux->cr = cr;
	aux->half_path = half_path;
	
	return aux;
}

void tensor_aux_space_delete(struct tensor_aux_space* aux) {
	permutation_delete(aux->path);
	cycle_representation_delete(aux->cr);
	permutation_delete(aux->half_path);
	free(aux);
}

int random_range(int bound) {
	return rand() % bound;
}

void swap(int* x, int* y) {
	int t = *x;
	*x = *y;
	*y = t;
}

void knuth_shuffle(struct permutation* p) {
	int* sequence = p->sequence;
	int n = p->degree;
	
	int i;
	for (i = n - 1; i > 0; i--) {
		int j = random_range(i + 1);
		swap(sequence + i, sequence + j);
	}
}

int mod(int a, int b) {
	int imod = a % b;
	if (imod < 0)
		return abs(b) + imod;
	else
		return imod;
}

// Function composition
int product(struct permutation* r, struct permutation* p, struct permutation* q) {
	if (p->degree != q->degree) {
		printf("Mismatch in degree\n");
		return 1;
	}
	if (p->degree != r->degree) {
		printf("Mismatch in degree\n");
		return 1;
	}
	
	int n = p->degree;
	
	int i;
	for (i = 0; i < n; i++)
		r->sequence[i] = p->sequence[q->sequence[i]];
	return 0;
}

// p * q-inverse
// Eliminates the need to store q-inverse
int right_division(struct permutation* r, struct permutation* p, struct permutation* q) {
	if (p->degree != q->degree) {
		printf("Mismatch in degree\n");
		return 1;
	}
	if (p->degree != r->degree) {
		printf("Mismatch in degree\n");
		return 1;
	}
	
	int n = p->degree;
	
	int i;
	for (i = 0; i < n; i++) {
		r->sequence[q->sequence[i]] = p->sequence[i];
	}
	return 0;
}

int inverse(struct permutation* q, struct permutation* p) {
	if (q->degree != p->degree) {
		printf("Mismatch in degree\n");
		return 1;
	}
	int n = p->degree;
	
	int i;
	for (i = 0; i < n; i++) {
		q->sequence[p->sequence[i]] = i;
	}

	return 0;
}

int half_cycle(struct permutation* p, struct cycle_representation* cr, int cross_point) {
	//cycle_print(cr);
	if (cr->degree != p->degree) {
		printf("Mismatch in degree\n");
		return 1;
	}
	int n = cr->degree;
	int* cycle_data = cr->cycle_data;
	int* sequence = p->sequence;
	
	int parity = 0;
	
	int start = 0;
	while (1) {
		if (start == n)
			break;
		int least = cycle_data[start];
		int end = start;
		while (1) {
			if (end + 1 == n)
				break;
			else if (cycle_data[end+1] < least)
				break;
			end += 1;
		}
		
		int length = end - start;
		
		if (length == 0) {
			sequence[cycle_data[start]] = cycle_data[start];
			start += 1;
			continue;
		}		
		
		int exchange_point = cross_point % (length + 1);
		
		int half = length / 2;
		int odd = length % 2;
		if (odd)
			parity = 1 - parity;
		
		int i;
		int index = mod(exchange_point, length + 1);
		for (i = 1; i <= half + odd*parity; i++) {
			index = mod(exchange_point + i, length + 1);
			int fixpoint = cycle_data[start + index];
			sequence[fixpoint] = fixpoint;
		}
		
		int cycle_start = mod(index + 1, length + 1);
		sequence[cycle_data[start + exchange_point]] = cycle_data[start + cycle_start];
		for (i = 0; i < length - half - odd*parity; i++) {
			index = mod(cycle_start + i, length + 1);
			int current = cycle_data[start + index];
			int next = cycle_data[start + mod(index + 1, length + 1)];
			sequence[current] = next;
		}
				
		start = end + 1;
	}
	
	return 0;
}

int intersection_count(struct permutation* p, struct permutation* q) {
	if (p->degree != q->degree) {
		printf("Mismatch in degree\n");
		return -1;
	}
	
	int n = p->degree;
	int i;
	int count = 0;
	for (i = 0; i < n; i++)
		if (p->sequence[i] == q->sequence[i])
			count += 1;
	
	return count;
}

void tensor(struct permutation* r, struct permutation* p, struct permutation* q, struct tensor_aux_space* aux) {
	struct permutation* path = aux->path;
	struct cycle_representation* cr = aux->cr;
	struct permutation* half_path = aux->half_path;
	
	right_division(path, q, p);
	cycle_decomposition(cr, path);
	half_cycle(half_path, cr, rand());
	product(r, half_path, p);
}

int evaluate(struct permutation* p, int x) {
	return p->sequence[x];
}
