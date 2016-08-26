#ifndef GRAPHETTE_HPP
#define GRAPHETTE_HPP

#include <vector>
#include <iostream>
#include <cmath>
#include <set>
#include <assert.h>
#include <stdlib.h>
#include <time.h>
#include "Graph.hpp"

// This function takes a decimal number and a number of nodes
// to find the binary representation of the decimal number and
// store it in a vector. Length of vector depends on number of nodes.

// Ex:
// num_nodes = 4       --->    Length of vector must be 6
// decimal_number = 3  --->    011 in binary
// vector = [0, 1, 1]  --->    vector = [0, 0, 0, 0, 1, 1]

std::vector<bool> bits_vector(int decimal_number, int num_nodes);


// This function generates all the possible combinations of bits vectors
// using number of nodes as input. Think of it like generating all 
//  possible combinations of graphs with n nodes.

// Ex:
// num_nodes = 4
// Because there are 4 nodes, there are at most 6 possible
// edges meaning the length of a bit vector will be 6.
// So think of it like this: _ _ _ _ _ _
// Since each _ can be either 0 or 1, there will be 2^6 = 64 number of graphs.

// And this function's job is to generate 64 bits vectors with all of the 
// possible combinations of binary values store in them.
// v0 = [0, 0, 0, 0, 0, 0]
// v1 = [0, 0, 0, 0, 0, 1]
// v2 = [0, 0, 0, 0, 1, 0]
// .
// .
// .
// .
// .
// .
// v63 = [1, 1, 1 ,1 ,1, 1]
std::vector<std::vector<bool>> generate_all_bits_vectors(int num_nodes);

// This functions is similar to generate_all_bits_vectors function.
// With a given number of nodes, we can generte all the possible 
// graph combinations with n nodes.

// I have defined a member function in SANA Graph class called setAdjMatrix().
// This member function uses a bit vector as input and put each bit inside
// that bit vector into the correct position of the Graph's adjMatrix, only
// in the upper right triangle of the matrix.

// Ex:
// v = [1, 1, 1, 0, 0, 1]
// Take v and put each bit in v to a Graph's adjMatrix.
// So the result will be
// 
// x 1 1 1
// x x 0 0
// x x x 1
// 
// x means we don't care about values in that position.

// So with 4 nodes, generate_all_graphs will generate 64 graphs.
std::vector<Graph*> generate_all_graphs(int node_number);

std::vector<Graph*> generate_canonical(const std::vector<Graph*>& graph_vectors);

Graph* get_canonical(Graph* g, std::vector<Graph*> graph_canonical);

bool GraphAreConnected(Graph *G, int i, int j);
bool _permutationIdentical(int n, int perm[]);
bool CombinAllPermutations(int n, bool (*fcn)(int, int *));
bool graphIsomorphic(Graph& G1, Graph& G2);

vector<ushort> canonicalMapping(vector<Graph*>& graph_vectors,vector<Graph*>& graph_canonical);

// Take a decimal input and put the its binary form into a matrix
// of n nodes
std::vector<std::vector<bool>> decimal_to_matrix(int decimal_number, int num_nodes);

// Print only the upper half a matrix
void print_matrix(std::vector<std::vector<bool>> matrix);

// Check if an edge is in edge_vector
bool contain_edge(std::vector<ushort>& edge, std::vector<std::vector<ushort>>& edge_vector);

// Generate a random_Graph
Graph random_Graph(int num_nodes);

#endif // GRAPHETTE_HPP