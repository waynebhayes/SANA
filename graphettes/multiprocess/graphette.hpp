#ifndef GRAPHETTE_HPP
#define GRAPHETTE_HPP

#include <iostream>
#include <cmath>
#include <set>
#include <assert.h>
#include <stdlib.h>
#include <time.h>
#include <fstream>
#include <sstream>
#include <stdio.h>
#include <execinfo.h>
#include <signal.h>
#include <unistd.h>
#include <algorithm>
#include <cmath>
#include "Graph.hpp"
using namespace std;


int convert(string s);
void canonical_array(int array[], int n);
string s_permutation(int array[], int n);
void generate_bits_vector(int decimal_number, vector<bool>& result);
void split(const std::string &s, char delim, std::vector<std::string> &elems);

void generate_node_permutation_commands(int num_nodes, std::string file_name_variation);
void generate_node_permutations(int num_nodes, std::string file_name);

void generate_final_canonicals(int num_nodes, std::string file_name);
void generate_final_canonicals_map(int num_nodes, std::vector<unsigned int> &p_canonicals, std::map <unsigned int, std::map<unsigned int, string> > &canonical_map);




void generate_intermediate_canonical_commands (int num_nodes, int BLOCK_SIZE, std::string file_name);
void generate_intermediate_canonicals (int num_nodes, int start_file_interval, int end_file_interval, std::string file_name);

void intermediate_canonicals_map (std::vector<unsigned int> &p_canonicals,
    std::map <unsigned int, std::map<unsigned int, string> > &canonical_map, int num_nodes, int variation);
    
std::vector<string> filter_intermediate_canonical(int num_nodes, std::vector<std::vector<unsigned int>> &mapping, 
    std::vector<unsigned int> &p_canonicals);
    
void generate_initial_canonical (int num_nodes, int variation, int block_size, int start, int end);
void generate_intial_canonical_commands (int num_nodes, int BLOCK_SIZE);

std::vector<string> generate_canonical(int num_nodes, std::vector<std::vector<unsigned int>> &mapping, std::vector<string> &permutations, 
    int block_size, int start, int end); 

//Isomorphism checking code
bool GraphAreConnected(Graph *G, int i, int j);
bool _permutationIdentical(int n, int perm[]);
bool CombinAllPermutations(int n, bool (*fcn)(int, int *));
bool graphIsomorphic(Graph& G1, Graph& G2, int array[]);
   
// Take a decimal input and put the its binary form into a matrix
// of n nodes
vector<vector<bool>> decimal_to_matrix(int decimal_number, int num_nodes);

// Print only the upper half a matrix
void print_matrix(std::vector<std::vector<bool>> matrix);

// Check if an edge is in edge_vector
bool contain_edge(std::vector<ushort>& edge, std::vector<std::vector<ushort>>& edge_vector);

#endif // GRAPHETTE_HPP