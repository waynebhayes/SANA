#include <chrono>
#include "graphette.hpp"

//UNCOMMENT EACH STEP TO EXECUTE AND RECOMMENT THE STEPS TO MOVE ON TO NEXT STEP

//To execute parallelization use below command 
// cat [executable_TYPE.txt] | ~wayne/bin/bin.x86_64/parallel -s sh [num_processors]


int main(int arg, char* argv[]) 
{    
    int num_nodes = atoi(argv[1]);
    
//STEP ONE:
    // Uncomment this part if you want to generate the sub executable calls for larger node values 
    // Be sure to set the BLOCK_SIZE to something decent    (we used 22000 for node 8)

    //NAMING CONVENTION: ./generate_intial_canonical_commands

    // int BLOCK_SIZE = atoi(argv[2]);
    
    // generate_intial_canonical_commands (num_nodes, BLOCK_SIZE);

    //Run parallelization script on executable output file 
    
//STEP TWO:
    //NAMING CONVENTION: ./generate_intermediate_canonical_commands

    // std::string file_name(argv[3]); 
    
    // generate_intermediate_canonical_commands(num_nodes, BLOCK_SIZE, file_name);

//STEP THREE:
    //Loop through all of the files of intermediate canonicals and
    //  put them into a map of intermediate canonicals to their non-canonical forms

    //NAMING CONVENTION: ./generate_intermediate_canonicals

    // int start_file_interval = atoi(argv[2]);
    // int end_file_interval = atoi(argv[3]);
    // std::string file_name(argv[4]);
    
    // generate_intermediate_canonicals (num_nodes, start_file_interval, end_file_interval, file_name);
    
    //Run parallelization script on executable output file 
    
//STEP FOUR:
    //NAMING CONVENTION: ./generate_final_canonicals

    // std::string file_name(argv[2]);
    
    // generate_final_canonicals(num_nodes, file_name);
        
//STEP FIVE:
    //Run this to generate the node permutations for canonical -> non-canonical mappings

    // std::string file_name(argv[2]);
    
    //NAMING CONVENTION: ./generate_permutations_commands

    // generate_node_permutation_commands(num_nodes, file_name);
    
//STEP SIX:
    //Call split -l [x lines] [canonical_mapN_final.txt] [new_file names]
    //SAMPLE: split -l 100 canonical_map7_final.txt map

    //NAMING CONVENTION: ./generate_permutations

    // generate_node_permutations(num_nodes, file_name);
    
    //Run parallelization script on executable output file 
    
    //Run cat [file_name]*_permutation_map_[num_nodes].txt > permutation_map_[num_nodes].txt
    //    afterwwards to combine the separate node permutation files together
    
    return 0;
}