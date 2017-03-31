// Written by Vikram Saraph

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include <iostream>
#include "graph.h"

// Graph
// Sort vertices by node name, dictionary order on names
// Sort edges by node number, dictionary order on pairs of integers

struct vertex* vertex_malloc(char* name, int number) {
	struct vertex* v;
	v = (struct vertex*)malloc(sizeof *v);
    if(v==NULL){mg_error("Error allocating vertex. Not enough memory?");mg_quit(EXIT_FAILURE);}
	
	int len = strlen(name) + 1;
	v->name = (char*)malloc(len * sizeof(char));
    if(v->name==NULL){mg_error("Error allocating vertex name. Not enough memory?");mg_quit(EXIT_FAILURE);}
	strcpy(v->name, name);
	v->number = number;
	return v;
}

int vertex_cmp(struct vertex* v1, struct vertex* v2) {
	char* name1 = v1->name;
	char* name2 = v2->name;
	
	return strcmp(name1, name2);
}

void vertex_print(struct vertex* v) {
	printf("%s, %d\n", v->name, v->number);
}

void vertex_delete(struct vertex* v) {
	free(v->name);
	free(v);
}

struct edge* edge_malloc(int vertex1, int vertex2) {
	struct edge* e;
	e = (struct edge*)malloc(sizeof *e);
    if(e==NULL){mg_error("Error allocating edge. Not enough memory?");mg_quit(EXIT_FAILURE);}    
	
	e->vertex1 = vertex1;
	e->vertex2 = vertex2;
	
	return e;
}

int edge_cmp(struct edge* e1, struct edge* e2) {
	if (e1->vertex1 > e2->vertex1)
		return 1;
	else if (e1->vertex1 == e2->vertex1) {
		if (e1->vertex2 == e2->vertex2)
			return 0;
		else if (e1->vertex2 > e2->vertex2)
			return 1;
	}
	return -1;	
}

void edge_print(struct edge* e) {
	printf("(%d, %d)\n", e->vertex1, e->vertex2);
}

void edge_delete(struct edge* e) {
	free(e);
}

struct graph* graph_malloc(void) {
	struct graph* g;
	g = (struct graph*)malloc(sizeof *g);
    if(g==NULL){mg_error("Error allocating graph. Not enough memory?");mg_quit(EXIT_FAILURE);}    
	
	g->vertex_set_by_name = NULL;
	g->n_vertices = 0;
	g->edge_set = NULL;
	g->n_edges = 0;
	
	return g;
}

void graph_vertex_malloc(struct graph* g, int n_vertices) {	
	struct vertex** vertex_set_by_name;
	struct vertex** vertex_set_by_number;
	vertex_set_by_name = (struct vertex**)malloc(n_vertices * (sizeof *vertex_set_by_name));
	vertex_set_by_number = (struct vertex**)malloc(n_vertices * (sizeof *vertex_set_by_name));

    if(vertex_set_by_name==NULL || vertex_set_by_number==NULL){
        mg_error("Error allocating vertex set by name/number. Not enough memory?"); mg_quit(EXIT_FAILURE);}    
		
	g->vertex_set_by_name = vertex_set_by_name;
	g->vertex_set_by_number = vertex_set_by_number;
	g->n_vertices = n_vertices;
}

void graph_vertex_realloc(struct graph* g, int n_vertices) {	
	struct vertex** vertex_set_by_name;
	struct vertex** vertex_set_by_number;
	vertex_set_by_name = (struct vertex**)realloc(g->vertex_set_by_name, n_vertices * (sizeof *vertex_set_by_name));
	vertex_set_by_number = (struct vertex**)realloc(g->vertex_set_by_number, n_vertices * (sizeof *vertex_set_by_name));

    if(vertex_set_by_name==NULL || vertex_set_by_number==NULL){
        mg_error("Error allocating vertex set by name/number. Not enough memory?"); mg_quit(EXIT_FAILURE);}    
		
	g->vertex_set_by_name = vertex_set_by_name;
	g->vertex_set_by_number = vertex_set_by_number;
	g->n_vertices = n_vertices;
}

void graph_edge_malloc(struct graph* g, int n_edges) {
	struct edge** edge_set;
	edge_set = (struct edge**)malloc(n_edges * (sizeof *edge_set));
    if(edge_set==NULL){mg_error("Error allocating edge set. Not enough memory?");mg_quit(EXIT_FAILURE);}    
	
	g->edge_set = edge_set;
	g->n_edges = n_edges;
}

void graph_edge_realloc(struct graph* g, int n_edges) {
	struct edge** edge_set;
	edge_set = (struct edge**)realloc(g->edge_set, n_edges * (sizeof *edge_set));
    if(edge_set==NULL){mg_error("Error allocating edge set. Not enough memory?");mg_quit(EXIT_FAILURE);}	
	g->edge_set = edge_set;
	g->n_edges = n_edges;
}

void graph_edge_list_read(struct graph* g, FILE *input_file, char* file_name) {
    struct vertex* new_vertex;
    int n_alloc_vertices = 1024, n_alloc_edges = 1024;
    graph_vertex_malloc(g,n_alloc_vertices); g->n_vertices = 0;    
    graph_edge_malloc(g,n_alloc_edges); g->n_edges = 0;

    int ret;
    char input_buffer[1024];
    char str1[512], str2[512];
    int num1,num2;
    int i = 0;
    while(fgets(input_buffer, sizeof input_buffer, input_file)) {
        //        if (input_buffer[0]=='#' || input_buffer[0]=='\0') continue;
        ret = sscanf(input_buffer, " %s ", str1);
        if (ret!=1 || str1[0]=='#') continue;

        ret = sscanf(input_buffer," %s %s ", str1, str2);
        if(ret!=2) {mg_error("Error reading network file: %s", file_name); mg_quit(EXIT_FAILURE);}
        num1 = graph_find_vertex_number_unsorted(g,str1);
        if(num1==-1) { // add new vertex to vertex list
            num1 = g->n_vertices;
            g->n_vertices++;
            new_vertex = vertex_malloc(str1,num1);
            if (n_alloc_vertices < g->n_vertices) {
                graph_vertex_realloc(g,2*n_alloc_vertices);
                g->n_vertices = n_alloc_vertices+1;
                n_alloc_vertices *= 2;
            }
            g->vertex_set_by_name[num1] = new_vertex;
            g->vertex_set_by_number[num1] = new_vertex;
        }            
        num2 = graph_find_vertex_number_unsorted(g,str2);
        if(num2==-1) { // add new vertex to vertex list
            num2 = g->n_vertices;
            g->n_vertices++;
            new_vertex = vertex_malloc(str2,num2);
            if (n_alloc_vertices < g->n_vertices) {
                graph_vertex_realloc(g,2*n_alloc_vertices);
                g->n_vertices = n_alloc_vertices+1;
                n_alloc_vertices *= 2;
            }
            g->vertex_set_by_name[num2] = new_vertex;
            g->vertex_set_by_number[num2] = new_vertex;
        }
        if (num1 != num2) g->n_edges++;
        if (n_alloc_edges < g->n_edges) {
            graph_edge_realloc(g,2*n_alloc_edges);
            g->n_edges = n_alloc_edges+1;
            n_alloc_edges *= 2;
        }
        if (num1 < num2) { g->edge_set[i] = edge_malloc(num1,num2); i++; }
        else if (num1 > num2) { g->edge_set[i] = edge_malloc(num2,num1); i++; }
    }
    if (ferror(input_file)) {
        mg_error("Error reading network file: %s", file_name); mg_quit(EXIT_FAILURE);
    }
    graph_vertex_realloc(g,g->n_vertices);
    graph_edge_realloc(g,g->n_edges);
    graph_sort_vertices(g);
    graph_sort_edges(g);
}

struct graph* graph_read(char* file_name) {
	FILE* input_file = fopen(file_name, "r");
	if (input_file == NULL) {
		mg_error("Couldn't open network file: %s", file_name);
		mg_quit(EXIT_FAILURE);
	}
    
//	char input_buffer[16];
//	fgets(input_buffer, sizeof input_buffer, input_file);
//    rewind(input_file);

	struct graph* g = graph_malloc();
    
    //    if(strncmp(input_buffer,"LEDA.GRAPH",10)==0) {
    if(strcmp(".gw",file_name+strlen(file_name)-3) == 0) {
        graph_read_vertices(g, input_file, file_name);
        graph_read_edges(g, input_file, file_name);
    }
    else if(strcmp(".sif",file_name+strlen(file_name)-4) == 0) {
        graph_sif_read(g,input_file,file_name);
    }
    else { // assume it's a edge list file if it's not a LEDA graph
        graph_edge_list_read(g, input_file, file_name);
    }

	fclose(input_file);
	printf("Graph %s read. %d vertices, %d edges.\n", file_name, g->n_vertices, g->n_edges);
    //    graph_print_edges(g);
	return g;
}

void graph_sif_read(struct graph *g, FILE *input_file, char *file_name) {
    struct vertex* new_vertex;
    int n_alloc_vertices = 1024, n_alloc_edges = 1024;
    graph_vertex_malloc(g,n_alloc_vertices); g->n_vertices = 0;    
    graph_edge_malloc(g,n_alloc_edges); g->n_edges = 0;

    char *input_buffer=NULL;
    char *str1, *str2, *strint;
    int num1,num2;
    int i = 0;
    while((input_buffer = read_line(input_file))) {
        //        if (input_buffer[0]=='#' || input_buffer[0]=='\0') continue;
        //ret = sscanf(input_buffer, " %s ", str1);
        str1 = strtok(input_buffer, " \r\t\n");
        if (str1==NULL || str1[0]=='#') continue;

        //        ret = sscanf(input_buffer," %s %s ", str1, str2);
        strint = strtok(NULL," \r\t\n");
        str2 = strtok(NULL," \r\t\n");
        if (strint==NULL || str2==NULL) {mg_error("Error reading network file: %s", file_name); mg_quit(EXIT_FAILURE);}
        num1 = graph_find_vertex_number_unsorted(g,str1);
        if(num1==-1) { // add new vertex to vertex list
            num1 = g->n_vertices;
            g->n_vertices++;
            new_vertex = vertex_malloc(str1,num1);
            if (n_alloc_vertices < g->n_vertices) {
                graph_vertex_realloc(g,2*n_alloc_vertices);
                g->n_vertices = n_alloc_vertices+1;
                n_alloc_vertices *= 2;
            }
            g->vertex_set_by_name[num1] = new_vertex;
            g->vertex_set_by_number[num1] = new_vertex;
        }
        do {
            num2 = graph_find_vertex_number_unsorted(g,str2);
            if(num2==-1) { // add new vertex to vertex list
                num2 = g->n_vertices;
                g->n_vertices++;
                new_vertex = vertex_malloc(str2,num2);
                if (n_alloc_vertices < g->n_vertices) {
                    graph_vertex_realloc(g,2*n_alloc_vertices);
                    g->n_vertices = n_alloc_vertices+1;
                    n_alloc_vertices *= 2;
                }
                g->vertex_set_by_name[num2] = new_vertex;
                g->vertex_set_by_number[num2] = new_vertex;
            }
            if (num1 != num2) g->n_edges++;
            if (n_alloc_edges < g->n_edges) {
                graph_edge_realloc(g,2*n_alloc_edges);
                g->n_edges = n_alloc_edges+1;
                n_alloc_edges *= 2;
            }
            if (num1 < num2) { g->edge_set[i] = edge_malloc(num1,num2); i++; }
            else if (num1 > num2) { g->edge_set[i] = edge_malloc(num2,num1); i++; }
        } while((str2 = strtok(NULL," \r\t\n")));
        free(input_buffer);
    }
    if (ferror(input_file)) {
        mg_error("Error reading network file: %s", file_name); mg_quit(EXIT_FAILURE);
    }
    graph_vertex_realloc(g,g->n_vertices);
    graph_edge_realloc(g,g->n_edges);
    graph_sort_vertices(g);
    graph_sort_edges(g);
}

void graph_read_vertices(struct graph* g, FILE* input_file, char* file_name) {
	char input_buffer[1024];
    int ret;
	
	fgets(input_buffer, sizeof input_buffer, input_file);
	fgets(input_buffer, sizeof input_buffer, input_file);
	fgets(input_buffer, sizeof input_buffer, input_file);
	fgets(input_buffer, sizeof input_buffer, input_file);
	
	int n_vertices;
	ret = fscanf(input_file, "%d\n", &n_vertices);
    if (ret!=1) {
		mg_error("File format error in network file: %s", file_name);
        mg_quit(EXIT_FAILURE);
    }
	
	graph_vertex_malloc(g, n_vertices);
	struct vertex** vertex_set_by_name = g->vertex_set_by_name;
	struct vertex** vertex_set_by_number = g->vertex_set_by_number;

	int number;
	char* name;
	struct vertex* new_vertex;
	for (number = 0; number < n_vertices; number++) {
		if (fgets(input_buffer, sizeof input_buffer, input_file)==NULL) {
            mg_error("File format error in network file: %s", file_name);
            mg_quit(EXIT_FAILURE);
        }
		name = strtok(input_buffer, "|{}");
		new_vertex = vertex_malloc(name, number);
		vertex_set_by_name[number] = new_vertex;
		vertex_set_by_number[number] = new_vertex;
	}
	
	graph_sort_vertices(g);
}

void graph_read_edges(struct graph* g, FILE* input_file, char* file_name) {
	char input_buffer[1024];
    int ret;

	int n_edges;
	ret = fscanf(input_file, "%d\n", &n_edges);
    if (ret!=1) {
        mg_error("File format error in network file: %s", file_name);
        mg_quit(EXIT_FAILURE);
    }
	
	graph_edge_malloc(g, n_edges);
	struct edge** edge_set = g->edge_set;
	
	int i;
	char* vertex1;
	char* vertex2;
	for (i = 0; i < n_edges; i++) {
		if (fgets(input_buffer, sizeof input_buffer, input_file) == NULL) {
            mg_error("File format error in network file: %s", file_name);
            mg_quit(EXIT_FAILURE);
        }
		vertex1 = strtok(input_buffer, " ");
		vertex2 = strtok(NULL, " ");
		// Add things here to retrieve other information
		edge_set[i] = edge_malloc(atoi(vertex1) - 1, atoi(vertex2) - 1);
	}
	
	graph_sort_edges(g);
}

void graph_print_vertices(struct graph* g) {
	struct vertex** vertex_set_by_name = g->vertex_set_by_name;
	int n_vertices = g->n_vertices;
	
	int i;
	for (i = 0; i < n_vertices; i++)
		vertex_print(vertex_set_by_name[i]);
}

void graph_print_edges(struct graph* g) {
	struct edge** edge_set = g->edge_set;
	int n_edges = g->n_edges;
	
	int i;
	for (i = 0; i < n_edges; i++)
		edge_print(edge_set[i]);
}

void graph_sort_vertices(struct graph* g) {
	struct vertex** vertex_set_by_name = g->vertex_set_by_name;
	
	int i;
	// Move through interval
	for (i = 1; i < g->n_vertices; i++) {
		int j;
		// Move the next element in
		for (j = i; j > 0; j--) {
			int cmp = vertex_cmp(vertex_set_by_name[j], vertex_set_by_name[j-1]);
			struct vertex* temp;
			if (cmp > 0) {
				temp = vertex_set_by_name[j-1];
				vertex_set_by_name[j-1] = vertex_set_by_name[j];
				vertex_set_by_name[j] = temp;
			}
			// If no swaps made, element is in its place
			else break;
		}
	}
}

void graph_sort_edges(struct graph* g) {
	struct edge** edge_set = g->edge_set;
	
	int i;
	// Move through interval
	for (i = 1; i < g->n_edges; i++) {
		int j;
		// Move the next element in
		for (j = i; j > 0; j--) {
			int cmp = edge_cmp(edge_set[j], edge_set[j-1]);
			struct edge* temp;
			if (cmp == 1) {
				temp = edge_set[j-1];
				edge_set[j-1] = edge_set[j];
				edge_set[j] = temp;
			}
			// If no swaps made, element is in its place
			else break;
		}
	}
}

int graph_find_vertex_number(struct graph* g, char* name) {
	struct vertex** vertex_set_by_name = g->vertex_set_by_name;
	int n_vertices = g->n_vertices;
	
	struct vertex* v = vertex_malloc(name, -1);
	
	int left = 0;
	int right = n_vertices - 1;
	int mid = 0;
	while (right >= left) {
		mid = (left + right) / 2;
		int cmp = vertex_cmp(vertex_set_by_name[mid], v);
		if (cmp > 0)
			left = mid + 1;
		else if (cmp < 0)
			right = mid - 1;
		else
			break;
	}
	vertex_delete(v);

	if (right >= left)
		return vertex_set_by_name[mid]->number;
	else
		return -1;
}

int graph_find_vertex_number_unsorted(struct graph* g, char* name) {
	struct vertex** vertex_set_by_name = g->vertex_set_by_name;
	int n_vertices = g->n_vertices;
	
	struct vertex* v = vertex_malloc(name, -1);

    int i;
    for(i=0;i<n_vertices;i++) {
        if (vertex_cmp(vertex_set_by_name[i], v) == 0) {
            vertex_delete(v);            
            return vertex_set_by_name[i]->number;
        }
    }
    return -1;
}

char* graph_find_vertex_name(struct graph* g, int number) {
	struct vertex** vertex_set_by_number = g->vertex_set_by_number;
	
	return vertex_set_by_number[number]->name;
}

int graph_find_edge(struct graph* g, struct edge* e) {
	int left = 0;
	int right = g->n_edges - 1;
	struct edge** edge_set = g->edge_set;
	while (right >= left) {
		int mid = (left + right) / 2;
		int cmp = edge_cmp(edge_set[mid], e);
		if (cmp == 1)
			left = mid + 1;
		else if (cmp == -1)
			right = mid - 1;
		else
			return mid;
	}
	return -1;
}

int graph_is_edge(struct graph* g, int vertex1, int vertex2, struct edge* e_temp) {
	// Alloc edge
	e_temp->vertex1 = vertex1;
	e_temp->vertex2 = vertex2;
	
	int index = graph_find_edge(g, e_temp);
	if (index >= 0)
		return 1;
	
	// Alloc edge
	e_temp->vertex1 = vertex2;
	e_temp->vertex2 = vertex1;
	
	index = graph_find_edge(g, e_temp);
	if (index >= 0)
		return 1;
	
	return 0;
}

void graph_delete(struct graph* g) {
	struct vertex** vertex_set_by_name = g->vertex_set_by_name;
	struct vertex** vertex_set_by_number = g->vertex_set_by_number;
	int n_vertices = g->n_vertices;
	struct edge** edge_set = g->edge_set;
	int n_edges = g->n_edges;
	
	int i;
	for (i = 0; i < n_vertices; i++)
		vertex_delete(vertex_set_by_name[i]);
	free(vertex_set_by_name);
	free(vertex_set_by_number);
	
	for (i = 0; i < n_edges; i++)
		edge_delete(edge_set[i]);
	free(edge_set);
	
	free(g);
}

struct alignment* alignment_calloc(struct graph* g1, struct graph* g2) {
	int m = g1->n_vertices;
	int n = g2->n_vertices;
	if (m > n) {
		printf("alignment_malloc: Size of domain is larger than size of range\n");
		return NULL;
	}
	
	struct alignment* a;
	a = (struct alignment*)malloc(sizeof *a);
    if(a==NULL){mg_error("Error allocating alignment. Not enough memory?");mg_quit(EXIT_FAILURE);}    
	
	struct permutation* p;
	p = permutation_calloc(n);
	
	a->domain = g1;
	a->range = g2;
	a->perm = p;
    //	a->invperm = permutation_calloc(n);
	
	a->n_edges_preserved = -1;
	a->n_edges_induced = -1;
	a->score = -1.0;
	a->is_computed = 0;
	
	return a;
}

void alignment_randomize(struct alignment* a) {
	struct permutation* p = a->perm;
	permutation_randomize(p);
}

int alignment_read(struct alignment* a, char* file_name) {
	struct graph* g1 = a->domain;
	struct graph* g2 = a->range;
	int m = g1->n_vertices;
	int n = g2->n_vertices;
	
	FILE* input_file = fopen(file_name, "r");
	if (input_file == NULL) {
		mg_error( "Couldn't open alignment file: %s", file_name);
		mg_quit(EXIT_FAILURE);
	}

	struct permutation* p = a->perm;
	int* sequence = p->sequence;

    int ret;
	int i;
	char name1[1000];
	char name2[1000];
	for (i = 0; i < n; i++)
		sequence[i] = -1;
	for (i = 0; i < m; i++) {
		ret = fscanf(input_file, "%s %s", name1, name2);
        if (ret!=2) {
            mg_error( "File format error in alignment file: %s", file_name);
            mg_quit(EXIT_FAILURE);
        }
		int vertex1 = graph_find_vertex_number(g1, name1);
		int vertex2 = graph_find_vertex_number(g2, name2);
		
		sequence[vertex1] = vertex2;
	}
	
	// Randomize the rest
	int* mapped_to = (int*)calloc(n, sizeof(int));
    if(mapped_to==NULL){mg_error("Error allocating vertex map. Not enough memory?");mg_quit(EXIT_FAILURE);}
    
	for (i = 0; i < n; i++)
		if (sequence[i] >= 0 && sequence[i] < n)
			mapped_to[sequence[i]] = 1;
	int j = 0;
	for (i = 0; i < n; i++)
		if (mapped_to[i] == 0) {
			mapped_to[j] = i;
			j++;
		}
	struct permutation* shuffle_perm = permutation_calloc(n - m);
	knuth_shuffle(shuffle_perm);
	for (i = 0; i < n - m; i++)
		sequence[m+i] = mapped_to[evaluate(shuffle_perm, i)];
	
	free(mapped_to);
	permutation_delete(shuffle_perm);
	
	printf("Alignment %s read.\n", file_name);
	
	return 0;
}

void alignment_tensor(struct alignment* a3, struct alignment* a1, struct alignment* a2, struct tensor_aux_space* taux) {
	tensor(a3->perm, a1->perm, a2->perm, taux);
}

// template <typename T> int sgn(T val) {
//     return (T(0) < val) - (val < T(0));
// }
int alignment_compare(struct alignment* a1, struct alignment* a2, struct carrier* rel) {
    float del = a2->score - a1->score;
    return ((del>0)-(del<0));
}

//#define CG_DEBUG

void alignment_composite_graph_old(struct alignment *a, struct compute_aux_space *caux) {
    struct graph *domain = a->domain;
    struct graph *range = a->range;    
    int nl,nk = range->n_vertices;
    int nnz = domain->n_edges + range->n_edges;
    int *spA = caux->spA; // values
    int *spIA = caux->spIA; // ind first elt of each row
    int *spJA = caux->spJA; // col inds
    int *spJR = caux->spJR; // col inds
    int *spJC = caux->spJC; // row inds
    int *spINZ = caux->spINZ; // nnz of each row
    int *spWI = caux->spWI; // for the accum. could be done w/ spINZ
    int *v_weights = caux->v_weights;
    int i,j,k;
    for(i=0;i<nk;i++) v_weights[i] = 1; //init cluster sizes
    // create row and col inds
    int v1,v2,u = 0;
    for(j=0;j<range->n_edges;j++) {
        v1 = range->edge_set[j]->vertex1;
        v2 = range->edge_set[j]->vertex2;
        if (v1<v2) { spJR[u] = v1; spJC[u] = v2; spA[u] = 1; }
        else { spJR[u] = v2; spJC[u] = v1; spA[u] = 1; }
        u++;
    }
    nl = domain->n_vertices;
    for(i=0;i<nl;i++) v_weights[evaluate(a->perm,i)]++;
    for(j=0;j<domain->n_edges;j++) {
        v1 = evaluate(a->perm,domain->edge_set[j]->vertex1);
        v2 = evaluate(a->perm,domain->edge_set[j]->vertex2); 
        if (v1<v2) { spJR[u] = v1; spJC[u] = v2; spA[u] = 1; }
        else { spJR[u] = v2; spJC[u] = v1; spA[u] = 1; }
        u++;
    }
    // create uncompressed spmat
    spIA[0] = 0;
    for(i=0;i<nk;i++) spINZ[i] = 0;
    for(i=0;i<nnz;i++) spINZ[spJR[i]]++;
    for(i=0;i<nk;i++) spIA[i+1] = spIA[i]+spINZ[i]; // spIA = [0;cumsum(spINZ)]
    for(i=0;i<nk;i++) spINZ[i] = 0;
    for(i=0;i<nnz;i++) {
        j = spIA[spJR[i]] + spINZ[spJR[i]]++;
        spJA[j] = spJC[i];
        // btw, if spA is different for each i, then we have to reorder it here        
    }

#ifdef CG_DEBUG
    printf("A: "); for(i=0;i<nnz;i++) printf("%d ", spA[i]); printf("\n");
    printf("JA: "); for(i=0;i<nnz;i++) printf("%d ", spJA[i]); printf("\n");
    printf("JR: "); for(i=0;i<nnz;i++) printf("%d ", spJR[i]); printf("\n");
    printf("JC: "); for(i=0;i<nnz;i++) printf("%d ", spJC[i]); printf("\n");
    printf("IA: "); for(i=0;i<nk+1;i++) printf("%d ", spIA[i]); printf("\n");
    printf("INZ: "); for(i=0;i<nk;i++) printf("%d ", spINZ[i]); printf("\n");
    printf("v_weights: "); for(i=0;i<nk;i++) printf("%d ", v_weights[i]); printf("\n");
#endif

    // accumulate duplicates
    for(i=0;i<nk;i++) spWI[i] = -1;
    int start,oldend,count = 0;
    for(j=0;j<nk;j++) {
        start = count;
        oldend = spIA[j] + spINZ[j];
        for(k=spIA[j]; k<oldend; k++) {
            i = spJA[k];
            if (spWI[i] >= start) {
                spA[spWI[i]] += spA[k];
                spINZ[j]--;
            }
            else {
                spA[count] = spA[k];
                spJA[count] = spJA[k];
                spWI[i] = count;
                count++;
            }
        }
        spIA[j] = start;
    }
    spIA[nk] = count;

#ifdef CG_DEBUG
    printf("A: "); for(i=0;i<nnz;i++) printf("%d ", spA[i]); printf("\n");
    printf("JA: "); for(i=0;i<nnz;i++) printf("%d ", spJA[i]); printf("\n");
    printf("JR: "); for(i=0;i<nnz;i++) printf("%d ", spJR[i]); printf("\n");
    printf("JC: "); for(i=0;i<nnz;i++) printf("%d ", spJC[i]); printf("\n");
    printf("IA: "); for(i=0;i<nk+1;i++) printf("%d ", spIA[i]); printf("\n");
    printf("INZ: "); for(i=0;i<nk;i++) printf("%d ", spINZ[i]); printf("\n");
    printf("v_weights: "); for(i=0;i<nk;i++) printf("%d ", v_weights[i]); printf("\n");
#endif
}

void alignment_composite_graph(struct alignment *a, struct compute_aux_space *caux) {
    struct graph *domain = a->domain;
    struct graph *range = a->range;    
    int nl,nk = range->n_vertices;
    int nnz = domain->n_edges + range->n_edges;
    int *spA = caux->spA; // values
    int *spIA = caux->spIA; // ind first elt of each row
    int *spJA = caux->spJA; // col inds
    int *spJR = caux->spJR; // col inds
    int *spJC = caux->spJC; // row inds
    int *spINZ = caux->spINZ; // nnz of each row
    int *spWI = caux->spWI; // for the accum. could be done w/ spINZ
    int *v_weights = caux->v_weights;
    int i,j,k;
    for(i=0;i<nk;i++) v_weights[i] = 1; //init cluster sizes
    // create row and col inds
    int v1,v2,u = 0;
    for(j=0;j<range->n_edges;j++) {
        v1 = range->edge_set[j]->vertex1;
        v2 = range->edge_set[j]->vertex2;
        if (v1<v2) { spJR[u] = v1; spJC[u] = v2; spA[u] = 1; }
        else { spJR[u] = v2; spJC[u] = v1; spA[u] = 1; }
        u++;
    }
    nl = domain->n_vertices;
    for(i=0;i<nl;i++) v_weights[evaluate(a->perm,i)]++;
    for(j=0;j<domain->n_edges;j++) {
        v1 = evaluate(a->perm,domain->edge_set[j]->vertex1);
        v2 = evaluate(a->perm,domain->edge_set[j]->vertex2); 
        if (v1<v2) { spJR[u] = v1; spJC[u] = v2; spA[u] = 1; }
        else { spJR[u] = v2; spJC[u] = v1; spA[u] = 1; }
        u++;
    }
    // create uncompressed spmat
    spIA[0] = 0;
    for(i=0;i<nk;i++) spINZ[i] = 0;
    for(i=0;i<nnz;i++) spINZ[spJC[i]]++;
    for(i=0;i<nk;i++) spIA[i+1] = spIA[i]+spINZ[i]; // spIA = [0;cumsum(spINZ)]
    for(i=0;i<nk;i++) spINZ[i] = 0;
    for(i=0;i<nnz;i++) {
        j = spIA[spJC[i]] + spINZ[spJC[i]]++;
        spJA[j] = spJR[i];
        // btw, if spA is different for each i, then we have to reorder it here        
    }

#ifdef CG_DEBUG
    printf("A: "); for(i=0;i<nnz;i++) printf("%d ", spA[i]); printf("\n");
    printf("JA: "); for(i=0;i<nnz;i++) printf("%d ", spJA[i]); printf("\n");
    printf("JR: "); for(i=0;i<nnz;i++) printf("%d ", spJR[i]); printf("\n");
    printf("JC: "); for(i=0;i<nnz;i++) printf("%d ", spJC[i]); printf("\n");
    printf("IA: "); for(i=0;i<nk+1;i++) printf("%d ", spIA[i]); printf("\n");
    printf("INZ: "); for(i=0;i<nk;i++) printf("%d ", spINZ[i]); printf("\n");
    printf("v_weights: "); for(i=0;i<nk;i++) printf("%d ", v_weights[i]); printf("\n");
#endif

    // accumulate duplicates
    for(i=0;i<nk;i++) spWI[i] = -1;
    int start,oldend,count = 0;
    for(j=0;j<nk;j++) {
        start = count;
        oldend = spIA[j] + spINZ[j];
        for(k=spIA[j]; k<oldend; k++) {
            i = spJA[k];
            if (spWI[i] >= start) {
                spA[spWI[i]] += spA[k];
                spINZ[j]--;
            }
            else {
                spA[count] = spA[k];
                spJA[count] = spJA[k];
                spWI[i] = count;
                count++;
            }
        }
        spIA[j] = start;
    }
    spIA[nk] = count;

#ifdef CG_DEBUG
    printf("A: "); for(i=0;i<nnz;i++) printf("%d ", spA[i]); printf("\n");
    printf("JA: "); for(i=0;i<nnz;i++) printf("%d ", spJA[i]); printf("\n");
    printf("JR: "); for(i=0;i<nnz;i++) printf("%d ", spJR[i]); printf("\n");
    printf("JC: "); for(i=0;i<nnz;i++) printf("%d ", spJC[i]); printf("\n");
    printf("IA: "); for(i=0;i<nk+1;i++) printf("%d ", spIA[i]); printf("\n");
    printf("INZ: "); for(i=0;i<nk;i++) printf("%d ", spINZ[i]); printf("\n");
    printf("v_weights: "); for(i=0;i<nk;i++) printf("%d ", v_weights[i]); printf("\n");
#endif
}

void alignment_edges_preserved(struct alignment *a, struct compute_aux_space *caux,
                      int *n_edges_preserved, int *n_unique_edges) {
    struct graph *range = a->range;    
    int nk = range->n_vertices;
    int *spA = caux->spA; // values
    int *spIA = caux->spIA; // ind first elt of each row
    int *spJA = caux->spJA; // col inds
    int *spINZ = caux->spINZ; // nnz of each row
    int *v_weights = caux->v_weights;
    int i,j;
    // calculate preserved and unique edges
    int cis = 0;
    int unique_edges = 0;
    int r,w;
    // btw this is the transpose of the composite graph. hence j is col ind, spJA[i] is row ind
    for (j=0;j<nk;j++) {
        for(i=spIA[j]; i<spIA[j] + spINZ[j]; i++) {
            w = std::min(v_weights[j],v_weights[spJA[i]]);
            r = spA[i];
            #ifdef INSPECT_COMPOSITE_GRAPH
                printf("row %d col %d vrow %d vcol %d w %d r %d\n",
                       spJA[i],j,v_weights[spJA[i]],v_weights[j],w,r);
            #endif
            if (w > 1) {
                if (r>0) unique_edges++; // r>0 is required b/c (0,0) gets added regardless; idk why
                if (r>1) cis++; //(float)r / (float)w;
            }
        }
    }
    *n_edges_preserved = cis;
    *n_unique_edges = unique_edges;
}    
    
void alignment_compute(struct alignment* a, struct carrier* rel,
                       compute_aux_space *caux) {

    alignment_composite_graph(a,caux);
    alignment_edges_preserved(a, caux,&(a->n_edges_preserved), &(a->n_edges_unique));
    //    alignment_cg_new(a, caux, &(a->n_edges_preserved), &(a->n_edges_unique));
    a->n_edges_induced = a->n_edges_unique + a->n_edges_preserved - a->domain->n_edges;
#ifdef INSPECT_COMPOSITE_GRAPH
    printf("s3_score: %f\n----\n", (float)a->n_edges_preserved/a->n_edges_unique);
#endif
    //    printf("s3_score_cur %f\n", (float)a->n_edges_preserved/a->n_edges_unique);

//     alignment_composite_graph_new(a,caux);
//     alignment_edges_preserved(a, caux,&(a->n_edges_preserved), &(a->n_edges_unique));
//     printf("s3_score_new %f\n", (float)a->n_edges_preserved/a->n_edges_unique);

    switch (rel->rel) {
    case 0:
        a->edge_score = (float)a->n_edges_preserved/a->domain->n_edges; break;
    case 1:
        a->edge_score = (float)a->n_edges_preserved/a->n_edges_induced; break;
    default:
        a->edge_score = (float)a->n_edges_preserved/a->n_edges_unique; break;
    }

    /* add node score here */
    if (rel->use_alpha) {
        // a->node_score = 1.0/(1+alignment_nodescore_compute(a,rel)); /* if distance compare */
        // a->node_score = -nodescore_compute(a,rel); /* if distance compare !! magna cannot handle negative scores though so don't do this */
        a->node_score = alignment_nodescore_compute(a,rel); /* if similarity scores */
        a->score = (a->edge_score)*(rel->alpha) + a->node_score*(1-rel->alpha);
    }
    else {
        a->score = a->edge_score;
        a->node_score = 0.0;
    }

//    printf("alpha: %f, score: %f\n", rel->alpha, a->score);
//    printf("dom: %d, rge: %d, pdeg: %d\n", a->domain->n_vertices,
//           a->range->n_vertices, a->perm->degree);

	a->is_computed = 1;
}

float alignment_nodescore_compute(struct alignment *a, struct carrier *rel) {
    //    int *sequence = a->perm->sequence;
    int n_dom = a->domain->n_vertices;
    int n_ran = a->range->n_vertices;
    float sum = 0;

    int row,col;
    for (row=0; row<n_dom; row++) {
        //        col = sequence[row];
        col = evaluate(a->perm, row);
        sum += rel->cmpdata[row*n_ran+col]; /* row major order */
    }

    return (sum/(float)n_dom);
}


void alignment_write(struct alignment* a, char* file_name) {
	struct graph* domain = a->domain;
	struct graph* range = a->range;
	int n_vertices = domain->n_vertices;
	
	FILE* output_file = NULL;
	if (file_name != NULL) {
		output_file = fopen(file_name, "w");
		if (output_file == NULL) {
			printf("alignment_write: Couldn't open file: %s\n", file_name);
			mg_quit(EXIT_FAILURE);
		}
	}
		
	int i;
	int j;
	char* name1;
	char* name2;
	for (i = 0; i < n_vertices; i++) {
		name1 = graph_find_vertex_name(domain, i);
		j = evaluate(a->perm, i);
		name2 = graph_find_vertex_name(range, j);
		if (file_name == NULL)
			printf("%s %s\n", name1, name2);
		else
			fprintf(output_file, "%s %s\n", name1, name2);
	}
    
    //permutation_print(a->perm);

	fclose(output_file);
}

void alignment_common_subgraph_write(struct alignment* a, char* file_name,
                                     compute_aux_space *caux) {
    //alignment_cg_new(a, caux, &(a->n_edges_preserved), &(a->n_edges_unique));
    alignment_composite_graph(a,caux);
    //    SpMat &composite_graph = caux->cg;
    int *spA = caux->spA; // values
    int *spIA = caux->spIA; // ind first elt of each row
    int *spJA = caux->spJA; // col inds
    int *spINZ = caux->spINZ; // nnz of each row
    int *v_weights = caux->v_weights;    
    struct graph* domain = a->domain;
    struct graph* range = a->range;
    int nk = range->n_vertices;
    int i,j,w,r;
    char* name1, *name2;

    FILE* output_file = NULL;
    output_file = fopen(file_name, "w");
    if (output_file == NULL) {
        printf("alignment_graph_write: Couldn't open file: %s\n", file_name);
        mg_quit(EXIT_FAILURE);
    }

    struct permutation *q = permutation_calloc(a->perm->degree);
    inverse(q,a->perm);

    for (j=0;j<nk;j++) {
        for(i=spIA[j]; i<spIA[j] + spINZ[j]; i++) {
            w = std::min(v_weights[j],v_weights[spJA[i]]);
            r = spA[i];
            if (w > 1 && r>1) {
                name1 = graph_find_vertex_name(range, j);
                name2 = graph_find_vertex_name(domain, evaluate(q,j));
                fprintf(output_file, "%s=%s i ", name2, name1);
                name1 = graph_find_vertex_name(range, spJA[i]);
                name2 = graph_find_vertex_name(domain, evaluate(q,spJA[i]));
                fprintf(output_file, "%s=%s\n", name2, name1);
            }
        }
    }    
    fclose(output_file);
}

void alignment_graph_write(struct alignment* a, char* file_name) {
	struct graph* domain = a->domain;
	struct graph* range = a->range;
	
	FILE* output_file = NULL;
    //	if (file_name != NULL) {
		output_file = fopen(file_name, "w");
		if (output_file == NULL) {
			printf("alignment_graph_write: Couldn't open file: %s\n", file_name);
			mg_quit(EXIT_FAILURE);
		}
    //	}
		
	int i;
	int j;
	char* name1;
	char* name2;

    for(i=0;i<domain->n_edges;i++) {
        name1 = graph_find_vertex_name(domain, domain->edge_set[i]->vertex1);
        name2 = graph_find_vertex_name(domain, domain->edge_set[i]->vertex2);
        fprintf(output_file, "dom_%s i dom_%s\n", name1, name2);
    }
    for(i=0;i<range->n_edges;i++) {
        name1 = graph_find_vertex_name(range, range->edge_set[i]->vertex1);
        name2 = graph_find_vertex_name(range, range->edge_set[i]->vertex2);
        fprintf(output_file, "ran_%s i ran_%s\n", name1, name2);
    }
	for (i = 0; i < domain->n_vertices; i++) {
		name1 = graph_find_vertex_name(domain, i);
		j = evaluate(a->perm, i);
		name2 = graph_find_vertex_name(range, j);
        fprintf(output_file, "dom_%s a ran_%s\n", name1, name2);
	}
    //permutation_print(a->perm);
	fclose(output_file);
}

void alignment_print(struct alignment* a) {
	alignment_write(a, NULL);
}

void alignment_delete(struct alignment* a) {
	permutation_delete(a->perm);
    //	permutation_delete(a->invperm);
	free(a);
}

// This returns the fraction of maps that are mapped correctly
// Correct as in wrt vertex name
float alignment_node_correctness(struct alignment *a) {
    //    int *sequence = a->perm->sequence;
    int n_dom = a->domain->n_vertices;
    //    int n_ran = a->range->n_vertices;
    int sum = 0;

    //    printf("alignment correctness\n");

	int i,j;
	char* name1;
	char* name2;
	for (i = 0; i < n_dom; i++) {
		name1 = graph_find_vertex_name(a->domain, i);
		j = evaluate(a->perm, i);
		name2 = graph_find_vertex_name(a->range, j);
        sum += (strcmp(name1,name2)==0); // this looks at the whole name
        //sum += (strncmp(name1,name2,1)==0); // this looks at only the first character
        //        printf("dom(%d) %s rge(%d) %s\n", i, name1, j, name2);
    }
    return ((float)sum/n_dom);
}

struct compute_aux_space* compute_aux_space_malloc(struct graph *g1, struct graph *g2) {
    //    struct compute_aux_space* caux = (struct compute_aux_space*)
    //        malloc(sizeof(*caux));
    struct compute_aux_space *caux = new compute_aux_space;
    //(caux->cg).resize(g2->n_vertices,g2->n_vertices);
    //(caux->cg_triplets).resize(g1->n_edges+g2->n_edges);
    //caux->v_weights = (int*)malloc(g2->n_vertices*sizeof(int)); // vector of "cluster sizes"
    //if(caux->v_weights==NULL) {mg_error("Allocation error."); mg_quit(EXIT_FAILURE);}
    int nnz = g1->n_edges+g2->n_edges;
    int nk = g2->n_vertices;
    caux->spA = (int*)malloc(nnz*sizeof(int)); // final values
    caux->spIA = (int*)malloc((nk+1)*sizeof(int)); // ind first elt of each row
    caux->spJA = (int*)malloc(nnz*sizeof(int)); // final col inds
    caux->spJR = (int*)malloc(nnz*sizeof(int)); // row inds
    caux->spJC = (int*)malloc(nnz*sizeof(int)); // col inds
    caux->spINZ = (int*)malloc(nk*sizeof(int)); // nnz of each row
    caux->spWI = (int*)malloc(nk*sizeof(int)); // for the accum. could be done w/ spINZ
    caux->v_weights = (int*)malloc(nk*sizeof(int));    
    return(caux);
}

void compute_aux_space_delete(struct compute_aux_space* caux) {
    free(caux->v_weights);
    //    free(caux);
    delete caux;
}

// #define INSPECT_COMPOSITE_GRAPH

// // Creates a sparse matrix, which describes the composite graph of the alignment
// // The edge weights are the number of edges between the nodes
// // The node weights are the "cluster sizes"
// // Creates the composite graph in O(m) time and O(m) space
// // where m is the total number of edges in all the networks
// void alignment_composite_graph(struct alignment *a, struct compute_aux_space *caux) {
//     SpMat &composite_graph = caux->cg;
//     std::vector<EigTriplet> &cg_triplets = caux->cg_triplets;
//     int *v_weights = caux->v_weights;
//     struct graph *domain = a->domain;
//     struct graph *range = a->range;    
//     int nk = range->n_vertices;
//     int nl;
//     int i,j;
// 
//     int u = 0;
// 
//     int v1,v2;
//     for(i=0;i<nk;i++) v_weights[i] = 1;
//     for(j=0;j<range->n_edges;j++) {
//         v1 = range->edge_set[j]->vertex1;
//         v2 = range->edge_set[j]->vertex2;
//         if (v1<v2) cg_triplets[u] = EigTriplet(v1,v2,1);
//         else cg_triplets[u] = EigTriplet(v2,v1,1);
//         u++;
//     }
// 
//     nl = domain->n_vertices;
// 
//     for(i=0;i<nl;i++) v_weights[evaluate(a->perm,i)]++;
//     for(j=0;j<domain->n_edges;j++) {
//         v1 = evaluate(a->perm,domain->edge_set[j]->vertex1);
//         v2 = evaluate(a->perm,domain->edge_set[j]->vertex2);
//         if (v1<v2) cg_triplets[u] = EigTriplet(v1,v2,1);
//         else cg_triplets[u] = EigTriplet(v2,v1,1);
//         u++;
//     }
// 
//     //    composite_graph.resize(nk,nk);
//     composite_graph.setZero();
//     composite_graph.setFromTriplets(cg_triplets.begin(),cg_triplets.end());
// 
// #ifdef INSPECT_COMPOSITE_GRAPH
//     printf("---\n");
//     alignment_write(a, NULL);
//     //permutation_print(a->mp->perms[0]);
//     std::cout << composite_graph << std::endl;
//     printf("cluster sizes: ");
//     for(i=0;i<nk;i++) {
//         printf("%d ",v_weights[i]);
//     }
//     printf("\n");
// #endif
// }
// 
// void alignment_edges_preserved(SpMat& G, int *v_weights,
//                                struct alignment *a,
//                                int *n_edges_preserved, int *n_unique_edges) {
//     int cis = 0;
//     int unique_edges = 0;
//     int i;
//     int r,w;
//     for (i=0;i<G.outerSize();i++) {
//         for(Eigen::SparseMatrix<int>::InnerIterator it(G,i); it; ++it) {
//             w = std::min(v_weights[it.row()],v_weights[it.col()]);
//             r = it.value();
//             #ifdef INSPECT_COMPOSITE_GRAPH
//                 printf("row %d col %d vrow %d vcol %d w %d r %d\n",
//                        it.row(),it.col(),v_weights[it.row()],v_weights[it.col()],w,r);
//             #endif
//             if (w > 1) {
//                 if (r>0) unique_edges++; // r>0 is required b/c (0,0) gets added regardless; idk why
//                 if (r>1) cis++; //(float)r / (float)w;
//             }
//         }
//     }
//     *n_edges_preserved = cis;
//     *n_unique_edges = unique_edges;
// }
