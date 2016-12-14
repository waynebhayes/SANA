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
	
	int num_nodes = atoi(argv[1]);
	int num_graphs = (num_nodes == 0) ? 0 : pow(2, (num_nodes*(num_nodes - 1)) / 2);
	
	int num_children = floor(num_graphs/10000) + 1;
	pid_t pids[num_children];
	
	int i = 0;
	std::cout << "Number of Graphs: " << num_graphs << " split into children of size 10000\n";
	for (i; i < num_graphs; i+= 10000) {
		if ((pids[i/10000] = fork()) == 0) {
			char* args[] = {
				(char*)"graphetteS", 
				(char*)std::to_string(num_nodes).c_str(), 
				(char*)std::to_string(i/10000).c_str(), 
				(char*)std::to_string(((i+10000 < num_graphs) ? 10000 : num_graphs - i)).c_str(), 
				(char*)std::to_string(i).c_str(),  
				(char*)std::to_string(((i+10000 < num_graphs) ? i+10000 : num_graphs)).c_str(),
				NULL
			};
			std::cout << "Executing: NumNodes: " << args[1] << " Variation: " << args[2] << " Loop Size: " << args[3] << " Start: " << args[4] << " End: " << args[5] << "...\n";
			execve(args[0], args, NULL);
			exit(0);
		}
	}
	
	for (int i = 0; i < num_children; ++i) {
		int status;
		while (-1 == waitpid(pids[i], &status, 0));
		if (!WIFEXITED(status) || WEXITSTATUS(status) != 0) {
			std::cerr << "Process " << i << " (pid " << pids[i] << ") failed" << std::endl;
			exit(1);
		}
	}
	
	std::vector<unsigned int> p_canonicals;
	std::map <unsigned int, std::map<unsigned int, std::string> > canonical_map;
	std::string p_canonical;
	std::vector<std::string> line;
	int current;
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
	
	final_canonicals(p_canonicals, canonical_map, num_nodes);
	
	return 0;
}