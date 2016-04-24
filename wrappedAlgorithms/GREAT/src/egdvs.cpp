#include <string.h>
#include <stdio.h>
#include <cmath>
#include <cstdlib>
#include <algorithm>
#include <sstream>

int main(int argc, char *argv[]){
	//Check arguments and output usage
    if(argc != 4){
        fprintf(stderr, "Usage: %s <input file 1> <input file 2> <output name>\n", argv[0]);
        return 1;
    	}

	//Open input files
    FILE *fp1 = fopen(argv[1], "r");
    if(!fp1) { perror(argv[1]); return 1; }

    FILE *fp2 = fopen(argv[2], "r");
    if(!fp2) { perror(argv[2]); return 1; }
    
    FILE *out;
	
	//If - use standard out
    if(!strcmp(argv[3], "-")){ out = stdout; }
    else{
    //Otherwise, open out file
        out = fopen(argv[3], "w");
        if(!out){ perror(argv[3]); return 1; }
    	}

	//Initialize the weights for each orbit (edge and non-edge)
	float weights[] = {1, 2, 2, 3, 3, 3, 3, 3, 3, 4, 4, 3, 3, 4, 5, 4, 5, 5, 4, 5,
						6, 6, 4, 5, 6, 4, 5, 6, 4, 5, 5, 5, 6, 5, 6, 6, 7, 4, 6, 5,
					    6, 5, 7, 5, 5, 6, 7, 6, 6, 4, 4, 6, 5, 7, 7, 7, 6, 5, 7, 5,
					    6, 7, 5, 5, 6, 6, 6, 5, 4,    1, 1, 2, 2, 2, 2, 2, 1, 2, 3,
					    3, 2, 3, 3, 4, 3, 2, 3, 4, 2, 3, 4, 3, 4, 3, 2, 4, 4, 4, 4, 
					    2, 4, 3, 2, 4, 4, 4, 3, 5, 5, 4, 3, 4, 5, 4, 4, 4, 4, 3};
	
	int i;
	


	char line[512];
	char line2[512];
	char* value = NULL;
	char* node1 = NULL;
	char* node2 = NULL;
	int n = 1;
	int l = 0;
	int num_col = 0;
	int num_lines = 0;
	float num = 0;
	float sum = 0;
	
	//Find whether the file is just edges
	//Or edges and non-edges
	fgets( line, 512, fp2);
	node1 = strtok( line, " -");
	node2 = strtok( NULL, " -");
    value = strtok( NULL, " -");
	n = 0;
	while( value != NULL){
       if( n > 68 + 49 ){
           fprintf( stderr, "Error: invalid input file\n");
           return 1;
           }
       n++;
       value = strtok( NULL, " -");
       }
    n--;
    if( (n != 68 )&& (n != 68 + 49) ){
            fprintf( stderr, "Error: invalid input file\n");
            return 1;
            }
            
    num_col = n;
    
    //Find total weight
    float total_weight = 0;
	for( i = 0; i <= num_col; i++){
        weights[i] = 1 - log( weights[i])/log( num_col );
        total_weight += weights[i];
        }
    
    //Print out edge names for file 2
    //And find the number of lines
    fprintf( out, "  %s-%s", node1, node2);
    while( fgets( line, 512, fp2 ) != NULL ){
	    node1 = strtok( line, " -");
	    node2 = strtok( NULL, " -");
	    fprintf( out, " %s-%s", node1, node2);
	    num_lines++;
	    }
	fprintf( out, "\n");
	
	//Go back to beginning of file
	fseek ( fp2 , 0 , SEEK_SET );

    if(!fp2) { perror(argv[2]); return 1; }
    l = 0;
    
    
    //For each line, calculate the eGDS with every line in the other file
	while( fgets( line, 512, fp1 ) != NULL ){
	    fseek ( fp2 , 0 , SEEK_SET );
	    char line_copy[512];
	    strcpy( line_copy, line);
	    node1 = strtok( line_copy, " -");
	    node2 = strtok( NULL, " -");
	    fprintf( out,"%s-%s", node1, node2);
	    //Put a line of file 2 into an array
	    while( fgets( line2, 512, fp2) ){
	        int values2[num_col];
	        n = 0;
	        strtok( line2, " -");
	        strtok( NULL, " -");
	        value = strtok( NULL, " -");
	        while( value != NULL){
	            if( n > num_col ){
                    fprintf( stderr, "Error: invalid input file\n");
                    return 1;
                    }
                values2[n] = atoi( value );
                value = strtok( NULL, " -");
                n++;
                }
                
	        n = 0;
	        sum = 0;
	        strcpy( line_copy, line);
	        node1 = strtok( line_copy, " -");
	        node2 = strtok( NULL, " -");
	        value = strtok( NULL, " -");
	        while( value != NULL){
                if( n > num_col ){
                    fprintf( stderr, "Error: invalid input file\n");
                    return 1;
                    }
                num = (log( values2[n] + 1)  - log(atoi( value ) + 1) );
                if( num < 0 ){ num = -num;}
                num *= weights[n];
                num /= log(std::max(values2[n], atoi(value) ) + 2);
                sum += num;
                n++;
                value = strtok( NULL, " -");
                }
            sum /= total_weight;
            fprintf( out, " %f", 1 - sum);
            }
            fprintf( out,"\n");
        }  

    fclose(fp1);
    fclose(fp2);
    if(out != stdout) fclose(out);
    return 0;
}


