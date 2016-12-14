#include <iostream>
#include <chrono>
#include <time.h>
#include <sys/time.h>
#include <stdio.h>
#include <execinfo.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#include <fstream>
#include <sstream>
#include <string>
#include "graphette.hpp"


int main(int arg, char* argv[])
{
	int num_nodes = atoi(argv[1]);
	int variation = atoi(argv[2]);
	int block_size = atoi(argv[3]);
	int start = atoi(argv[4]);
	int end = atoi(argv[5]);
	std::vector<string> graph_canonical;
	std::vector<std::vector<unsigned int>> mapping;
	std::vector<string> permutations(block_size);
	
	graph_canonical = generate_canonical(num_nodes, mapping, permutations, block_size, start, end);

	ofstream fcanon("canonical_decimal_representation"+to_string(num_nodes)+"_"+std::to_string(variation)+".txt"), 
			fmap("canon_map"+to_string(num_nodes)+"_"+std::to_string(variation)+".txt"), 
			fpermutation("permutation_map"+to_string(num_nodes)+"_"+std::to_string(variation)+".txt");
	
	// For writing to file
	// This will show the decimal representations of adjMatrix of each Graph
	std::cout << "Canonical Graph adjMatrix dec_rep: ";
	for (string g : graph_canonical)
	{
		std::cout << "G" << g << " ";
		fcanon << g.substr(1,g.length()) << " ";
	}

	int canonical_node = 0, non_canonical = 0;
	for (std::vector<unsigned int> s: mapping) {
		canonical_node = s[0];
		for (unsigned int m: s) {
			fmap << m << " ";
			fpermutation << m << "\t" << canonical_node << "\t" << permutations[m - variation*10000] << "\n";
		}
		fmap << "\n";
	}
	
	std::cout << "\n\n";                                 
	
	return 0;
}




