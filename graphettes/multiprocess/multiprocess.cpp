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
#include <cmath>
#include <map>
#include <vector>
#include "graphette.hpp"

void split(const std::string &s, char delim, std::vector<std::string> &elems) {
    std::stringstream ss;
    ss.str(s);
    std::string item;
    while (std::getline(ss, item, delim))
        elems.push_back(item);
}

int main(int arg, char* argv[]) {

	int MAX_ARGS = 7;
	int BLOCK_SIZE = atoi(argv[2]);
	
	int num_nodes = atoi(argv[1]);
	int num_graphs = (num_nodes == 0) ? 0 : pow(2, (num_nodes*(num_nodes - 1)) / 2);
	
	int num_children = floor(num_graphs/BLOCK_SIZE) + 1;

//Uncomment this part if you want to generate the sub executable calls for larger node values 
//Be sure to set the BLOCK_SIZE to something decent	(we used 22000 for node 8)
	// int i = 0;
	// std::cout << "Number of Graphs: " << num_graphs << " split into children of size BLOCK_SIZE\n";
	// for (i; i < num_graphs; i+= BLOCK_SIZE) {
		// char* args[] = {
			// (char*)"./graphette", 
			// (char*)std::to_string(num_nodes).c_str(), 
			// (char*)std::to_string(i/BLOCK_SIZE).c_str(), 
			// (char*)std::to_string(BLOCK_SIZE).c_str(), 
			// (char*)std::to_string(i).c_str(),  
			// (char*)std::to_string(((i+BLOCK_SIZE < num_graphs) ? i+BLOCK_SIZE : num_graphs)).c_str(),
			// NULL
		// };
		// std::cout << args[0] << " " << args[1] << " " << args[2] << " " << args[3] << " " << args[4] << " " << args[5] << "\n";
	// }
	
	std::vector<unsigned int> p_canonicals;
	std::map <unsigned int, std::map<unsigned int, std::string> > canonical_map;
	std::string p_canonical;
	std::vector<std::string> line;
	int current;
	
	//Loop through all of the files of intermediate canonicals and
	//  put them into a map of intermediate canonicals to their non-canonical forms
	for (int i = 0; i < num_children; ++i) {
		current = 102391;
		std::ifstream file("permutation_map"+std::to_string(num_nodes)+"_"+std::to_string(i)+".txt");
		if (file.is_open()) {
			while (getline(file, p_canonical, '\n')) {
				line.clear();
				split(p_canonical, '\t', line);
				int current_compare = stoi(line[1]);
				int non_c = stoi(line[0]);
				if (current_compare != current ) {
					current = current_compare;
					p_canonicals.push_back(current_compare);
				}
				if (!canonical_map.count(current_compare)) {
					std::map<unsigned int, std::string> permute;
					permute[non_c] = line[2];
					canonical_map[current_compare] = permute;
				}
				else {
					canonical_map[current_compare][non_c] = line[2];
				}
			}
			file.close();
		}
	}
	
	//Run this to generate a file of the true/final canonicals in the 3 text files
	final_canonicals(p_canonicals, canonical_map, num_nodes);
	
	return 0;
}