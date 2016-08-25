#include <algorithm>
#include <iostream>
#include <vector>
#include <cassert>
#include <cmath>
#include "GraphetteOrbit.hpp"

using namespace std;

//see the header file for description of each function

vector<bool> GraphetteOrbit::bits_vector(int decimal_number, int num_nodes)
{
	int num_possible_edges = (num_nodes*(num_nodes-1)) / 2;
	vector<bool> result(num_possible_edges);

	// Convert to binary number and put each bit in result vector.
	int i = result.size() - 1;
	while (decimal_number != 0)
	{
		// This assertion assures that the length of binary in a decimal number
		// will not exceed the number of possible edges.
		// Ex:
		// num_possible_edges = 6
		// Length of bits must be <= num_possible_edges
		assert(i >= 0 && "Binary length exceeds number of possible edges");
		result[i] = decimal_number % 2;
		--i;
		decimal_number /= 2;
	}

	return result;
}
int GraphetteOrbit::toDecimal(vector<bool>&bits){
	uint sum = 0;
	for(uint i = 0; i < bits.size(); i++)
		sum += (bits[i] << (bits.size()-1-i));
	return sum;
}

void GraphetteOrbit::setAdjMatrix(vector<bool>& bits, vector<vector<bool> >& AdjMatrix){
	uint k = 0;
	for(uint i = 0; i < AdjMatrix.size(); i++)for (int j = i+1; j < AdjMatrix[i].size(); j++)
	{
		if(bits[k]){
			AdjMatrix[i][j] = true;
			AdjMatrix[j][i] = true;
		}
		k++;
	}
}

void GraphetteOrbit::getBits(vector<bool>& bits, vector<vector<bool> >& AdjMatrix){
	bits.clear();
	for(uint i = 0; i < AdjMatrix.size(); i++)
		for(int j = i+1; j < AdjMatrix[i].size(); j++)
			bits.push_back(AdjMatrix[i][j]);
}

bool GraphetteOrbit::suitable(vector<uint>& permutation, vector<vector<bool> >& AdjMatrix){
	for(uint i = 0; i < permutation.size(); i++){
		if(GraphetteOrbit::getDegree(i, AdjMatrix) != GraphetteOrbit::getDegree(permutation[i], AdjMatrix)){
			return false;
			break;
		}
	}
	return true;
}

uint GraphetteOrbit::getDegree(uint node, vector<vector<bool> >& AdjMatrix){
	int degree = 0;
	for (uint j = 0; j < AdjMatrix.size(); j++)
		degree += (int)AdjMatrix[node][j];
	return degree;
}

void GraphetteOrbit::getCycles(std::vector<uint>& permutation, std::vector<uint>& orbit){
	vector<bool> visited(permutation.size(), false);
	for(uint i = 0; i < permutation.size(); i++){
		if(not visited[i]){
			//finding out each cycle at a time
			vector<uint> cycle;
			followTrail(permutation, cycle, i, i, visited);
			uint minOrbit = orbit[cycle[0]];
			for(uint j: cycle)
				minOrbit = min(orbit[j], minOrbit);
			for(uint j: cycle)
				orbit[j] = minOrbit;
		}
	}
}

void GraphetteOrbit::followTrail(std::vector<uint>& permutation, std::vector<uint>& cycle,
	uint seed, uint current, std::vector<bool>& visited){
	cycle.push_back(current);
	visited[current] = true;
	if(permutation[current] != seed)
		followTrail(permutation, cycle, seed, permutation[current], visited);
}

void GraphetteOrbit::getOrbits(int num_nodes, int decimal_number, vector<vector<uint> >& saveOrbits){
	
	//setting up the AdjMatrix representation for the graphette
	vector<bool> bits = bits_vector(decimal_number, num_nodes);
	vector<bool> column(num_nodes, 0);
	vector<vector<bool> > AdjMatrix(num_nodes, column);
	GraphetteOrbit::setAdjMatrix(bits, AdjMatrix);
	vector<uint> permutation, orbit;
	
	//initially,the permutation is (0, 1, ..., num_nodes-1)
	//and each node is in its own orbit. we'll merge orbits later 
	for(int i = 0; i < num_nodes; i++){
		permutation.push_back(i);
		orbit.push_back(i);
	}
	while( next_permutation(permutation.begin(), permutation.end()) ){
		
		//Just a speed up; ruling out searching into unnecessary permutations
		if(not GraphetteOrbit::suitable(permutation, AdjMatrix)) continue;

		//setting up a new matrix to apply the permutation
		vector<vector<bool> > NewAdjMatrix(num_nodes, column); //To store AdjMatrix after permutation
		for(uint i = 0; i < AdjMatrix.size(); i++){
			for(int j = i+1; j < AdjMatrix[i].size(); j++){
				uint ni = permutation[i], nj = permutation[j]; //new_i, new_j
				NewAdjMatrix[ni][nj] = AdjMatrix[i][j];
				NewAdjMatrix[nj][ni] = AdjMatrix[j][i];
			}
		}
		vector<bool> nbits;

		//determining the decimal representation of the graphette
		//after applying the permutation
		//if the decimal representation is same as the original,
		//then we have found an automorphism
		GraphetteOrbit::getBits(nbits, NewAdjMatrix);
		int ndecimal = GraphetteOrbit::toDecimal(nbits);
		if(decimal_number == ndecimal)
			//determining and applying permutation cycles of this permutation
			GraphetteOrbit::getCycles(permutation, orbit);
	}
	//saving orbits
	saveOrbits.clear();
	for(int orbitId = 0; orbitId < num_nodes; orbitId++){
		vector<uint> temp;
		for(int node = 0; node < num_nodes; node++){
			if(orbit[node] == orbitId)
				temp.push_back(node);
		}
		if(not temp.empty())
			saveOrbits.push_back(temp);
	}
}