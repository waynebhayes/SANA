#include <iostream>
#include <stdio.h>
#include <execinfo.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#include <fstream>
#include <sstream>
#include <sys/wait.h>
#include <string>
#include <map>

#include <chrono>
#include <vector>
#include "graphette.hpp"



int main(int arg, char* argv[]) 
{	
	int num_nodes = atoi(argv[1]);
	
// Uncomment this part if you want to generate the sub executable calls for larger node values 
// Be sure to set the BLOCK_SIZE to something decent	(we used 22000 for node 8)
	// int BLOCK_SIZE = atoi(argv[2]);
	// std::string file_name(argv[3]); 
	
	//NAMING CONVENTION: ./generate_intial_canonical_commands
	// generate_intial_canonical_commands (int num_nodes, int BLOCK_SIZE);

	//NAMING CONVENTION: ./generate_intermediate_canonical_commands
	// generate_intermediate_canonical_commands(num_nodes, BLOCK_SIZE, file_name);

	//Loop through all of the files of intermediate canonicals and
	//  put them into a map of intermediate canonicals to their non-canonical forms
	// int start_file_interval = atoi(argv[2]);
	// int end_file_interval = atoi(argv[3]);
	
	//NAMING CONVENTION: ./generate_intermediate_canonicals
	// generate_intermediate_canonicals (int num_nodes, int start_file_interval, int end_file_interval);
		
	//Run this to generate the node permutations for canonical -> non-canonical mappings
	std::string file_name(argv[2]);
	
	//NAMING CONVENTION: ./multiprocess_permutations_commands
	// generate_node_permutation_commands(num_nodes, file_name);
	
	//NAMING CONVENTION: ./multiprocess_permutations
	generate_node_permutations(num_nodes, file_name);
	
	return 0;
}