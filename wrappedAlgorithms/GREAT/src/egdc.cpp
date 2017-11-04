#include <string.h>
#include <stdio.h>
#include <cmath>
#include <cstdlib>

#include <iostream>
using namespace std;

int main(int argc, char *argv[]){
    //Check arguments and output usage
    if(argc != 3){
        fprintf(stderr, "Usage: %s <input file> <output name>\n", argv[0]);
        return 1;
        }

    //Open input file
    FILE *fp = fopen(argv[1], "r");
    if(!fp) { perror(argv[1]); return 1; }

    FILE *out;
    
    //If - use standard out
    if(!strcmp(argv[2], "-")){ out = stdout; }
    else{
    //Otherwise, open out file
        out = fopen(argv[2], "w");
        if(!out){ perror(argv[2]); return 1; }
        }

    //Create table of orbit weights
    float weights[] = {1, 2, 2, 3, 3, 3, 3, 3, 3, 4, 4, 3, 3, 4, 5, 4, 5, 5, 4, 5, 6, 6, 4, 5, 6, 4, 5, 6, 4, 5, 5, 5, 6, 5, 6, 6, 7, 4, 6, 5, 6, 5, 7, 5, 5, 6, 7, 6, 6, 4, 4, 6, 5, 7, 7, 7, 6, 5, 7, 5, 6, 7, 5, 5, 6, 6, 6, 6, 4};
    
    int i;

    for( i = 0; i <= 68; i++){
        weights[i] = 1 - log( weights[i])/log(68);     
}

    char line[32600];
    char* value;
    char* node1;
    char* node2;
    int n;
    float sum;

    //For each line, calculate the eGDC
    while( fgets( line, 32600, fp ) != NULL ){
        node1 = strtok( line, " ");
        node2 = strtok( NULL, " ");
        n = 0;
        sum = 0;
        value = strtok( NULL, " ");

    // EXCLUDES ORBIT 0 FOR EDGES - AS EACH EDGE TOUCHES ONE ORBIT 0!!!

        while( value != NULL){
            if( n > 68 ){
                fprintf( stderr, "Error: invalid input file\n");
                return 1;
                }
       sum += weights[n+1]*log( 1 + atoi( value ) );
            n++;
            value = strtok( NULL, " ");
            }
        n--;
        //Check that the file is either just edge orbitals or edges and nonedges
        if( n != 67){
            fprintf( stderr, "%d Error: invalid input file!!!\n", n);
            return 1;
            }
        fprintf( out, "%s %s %f\n", node1, node2, sum);
        }  

    fclose(fp);
    if(out != stdout) fclose(out);

    return 0;
}
