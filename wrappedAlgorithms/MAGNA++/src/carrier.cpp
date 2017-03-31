#include "carrier.h"

char mg_finalstr[2048];

int write_simulation_parameters(struct magnaparams *s) {
    int flen = strlen(s->output_file_prefix);
    char *filename = (char*)malloc(sizeof(char)*(15+flen));
    if (filename==NULL) {
        mg_error("Error allocating space for parameter file name. Not enough memory?");
        mg_quit(EXIT_FAILURE);
    }
    strncpy(filename,s->output_file_prefix,flen+15);
    strcat(filename,"_params.txt");    
    FILE *fp = fopen(filename, "w");
    if (!fp) {
        mg_error("Error creating parameter file: %s", filename);
        return 0;
    }
    fprintf(fp, "source_graph_file = %s\n",s->graph_file_name1);
    fprintf(fp, "target_graph_file = %s\n",s->graph_file_name2);
    fprintf(fp, "output_file_prefix = %s\n",s->output_file_prefix);
    if (s->init_pop_file) {
        fprintf(fp, "initial_population_file = %s\n",s->init_pop_file);
    }
    fprintf(fp, "edge_optimizing_measure = ");
    if (s->edge_optimizing_measure_code == 0)
        fprintf(fp, "EC\n");
    else if (s->edge_optimizing_measure_code == 1)
        fprintf(fp, "ICS\n");
    else if (s->edge_optimizing_measure_code == 2)
        fprintf(fp, "S3\n");
    //else if (s->edge_optimizing_measure_code == 3)
    //    fprintf(fp, "M\n");
    fprintf(fp, "node_optimizing_measure = ");
    if (s->use_nodematrix == 1) {
        fprintf(fp, "M\n");
        fprintf(fp, "node_matrix_comparison_file = %s\n", s->nodematrix_file);
    }
    else
        fprintf(fp, "Z\n");
    fprintf(fp, "edge_node_weight = %f\n",s->alpha);
    fprintf(fp, "population_size = %d\n",s->pop_size);
    fprintf(fp, "number_of_generations = %d\n",s->n_gen);
    fprintf(fp, "fraction_elite = %f\n",s->pop_elite_ratio);
    fprintf(fp, "output_frequency = %d\n",s->freq);
    fprintf(fp, "number_of_threads = %d\n",s->n_threads);
    fclose(fp);
    return 1;
}

struct carrier* carrier_create(struct graph *g1, struct graph *g2,
                               int rel_rel, float alpha, char *matrix_file) {
    struct carrier *rel;
    rel = (struct carrier*)malloc(sizeof(struct carrier));
    if (rel == NULL) {
        mg_error("Error allocating auxiliary space. Not enough memory?");
        mg_quit(EXIT_FAILURE);
    }
    int n1,n2;
    n1 = g1->n_vertices;
    n2 = g2->n_vertices;

    rel->rel = rel_rel;
    rel->use_alpha = (matrix_file != NULL);
    rel->alpha = alpha;
    rel->cmpdata = NULL;

    if (rel->use_alpha) {
        if(strcmp(".csv",matrix_file+strlen(matrix_file)-4) == 0) {
            rel->cmpdata = matrix_read_csv(matrix_file,g1,g2);
        }
        else {
            rel->cmpdata = matrix_read(matrix_file,n1,n2);
        }
    }

//    int i,j;
//    printf("(%d,%d)\n",n1,n2);
//    for(i=0;i<n1;i++) {
//        for(j=0;j<n2;j++) {
//            printf("%f ",rel->cmpdata[i*n2+j]);
//        }
//    }
  
    return rel;
}

void carrier_delete(struct carrier *rel) {
    free(rel->cmpdata);
    free(rel);
}

float* matrix_read(char *file_name, int n1, int n2) {
    float num;
    int ret;
    int i;
    FILE *fp;
    int n = n1*n2;
    int m1,m2;

    float *nums = (float*)calloc(n1*n2,sizeof(float));
    if (nums == NULL) {
        mg_error("Error allocating matrix. Not enough memory?");
        mg_quit(EXIT_FAILURE);
    }    
    
    fp = fopen(file_name,"r"); // read mode

    if( fp == NULL ) {
        mg_error("Error opening matrix file: %s",file_name);
        mg_quit(EXIT_FAILURE);
    }

    ret = fscanf(fp," %d %d ",&m1, &m2);
    if (ret != 2) {
        mg_error("Error reading matrix file: %s",file_name);
        mg_quit(EXIT_FAILURE);
    }
    if (m1!=n1 || m2!=n2) {
        mg_error("Need %d = %dx%d, not %dx%d, elements in matrix file: %s",n,n1,n2,m1,m2,file_name);
        mg_quit(EXIT_FAILURE);
    }

    i = 0;

//    // skips over any initial non number lines if they exist
//    int pos;
//    while (1) {
//      pos = ftell(fp);
//      ret = fscanf(fp,"%f",&num);
//      if (ret==0) {
//	while((ret = fgetc(fp))) {
//	  if(ret == '\n') break;
//	}
//      }
//      else {
//	fseek(fp,pos,SEEK_SET);
//	break;
//      }	
//    }

    while( (ret = fscanf(fp," %f ",&num)) != EOF) {

        if (ret == 0) {
            mg_error("Error (i = %d) reading matrix file: %s",i,file_name);
            mg_quit(EXIT_FAILURE);
        }

        if (num < 0) {
            mg_error("Disallowed negative number in matrix file: %s",file_name);
            mg_quit(EXIT_FAILURE);
        }

        nums[i] = num;
        i++;
        
        if (i>n) break;
    }

    if (i != n || ret != EOF) {
        mg_error("Need %d = %d*%d elements in matrix file: %s",n,n1,n2,file_name);
        mg_quit(EXIT_FAILURE);
    }

    fclose(fp);

    return nums;
}


float* matrix_read_csv(char *file_name, struct graph *g1, struct graph *g2) {
    float num;
    FILE *fp;
    int n1 = g1->n_vertices;
    int n2 = g2->n_vertices;
    int n = n1*n2;
    int j1,j2;
    char input_buffer[1024];
    char *str1, *str2, *strno;

    float *nums = (float*)calloc(n1*n2,sizeof(float));
    if (nums == NULL) {
        mg_error("Error allocating matrix. Not enough memory?");
        mg_quit(EXIT_FAILURE);
    }
    for(int i=0; i<n1*n2; i++) nums[i] = -1;    

    fp = fopen(file_name,"r"); // read mode    
    if( fp == NULL ) {
        mg_error("Error opening matrix file: %s",file_name);
        mg_quit(EXIT_FAILURE);
    }

    char *endstrno;
    int i = 0;
    while(fgets(input_buffer, sizeof input_buffer, fp)) {
    
        str1 = strtok(input_buffer," \t\",");
        j1 = graph_find_vertex_number(g1,str1);
        //strtok(NULL," \t\",");
        str2 = strtok(NULL," \t\",");
        j2 = graph_find_vertex_number(g2,str2);        
        //strtok(NULL," \t\",");
        if (j1 < 0 || j2 < 0) {
            mg_error("Unknown vertex name in matrix file (line = %d): %s",i+1,file_name);
            mg_quit(EXIT_FAILURE);
        }            
        strno = strtok(NULL," \t\",\r\n");
        //num = atof(strno);
        if (!strno) {
            mg_error("Error in matrix file (line = %d): %s",i+1,file_name);
            mg_quit(EXIT_FAILURE);
        }            
        num = strtof(strno,&endstrno);
        if (!endstrno) {
            mg_error("Error in matrix file (line = %d): %s",i+1,file_name);
            mg_quit(EXIT_FAILURE);
        }            
        
//        if (ret == 0) {
//            mg_error("Error (line = %d) reading matrix file: %s",i,file_name);
//            mg_quit(EXIT_FAILURE);
//        }
        if (num < 0) {
            mg_error("Disallowed negative number in matrix file (line = %d): %s",i+1,file_name);
            mg_quit(EXIT_FAILURE);
        }

        nums[j1*n2+j2] = num;
        i++;
        
        //if (i>n) break;
    }

    for(int i=0; i<n1*n2; i++) {
        if (nums[i] < 0) {
            mg_error("Need %d = %d*%d elements in matrix file: %s",n,n1,n2,file_name);
            mg_quit(EXIT_FAILURE);
        }
    }

    fclose(fp);

    return nums;
}
