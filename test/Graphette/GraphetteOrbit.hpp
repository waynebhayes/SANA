#include <algorithm>
#include <iostream>
#include <vector>
#include <cassert>

namespace GraphetteOrbit{
	void getOrbits(int num_nodes, int decimal_number, std::vector<std::vector<uint> >& saveOrbits);
	int toDecimal(std::vector<bool> &bits);
	std::vector<bool> bits_vector(int decimal_number, int num_nodes);
	void setAdjMatrix(std::vector<bool>& bits, std::vector<std::vector<bool> >& AdjMatrix);
	void getBits(std::vector<bool>& bits, std::vector<std::vector<bool> >& AdjMatrix);
	bool suitable(std::vector<uint>& permutation, std::vector<std::vector<bool> >& AdjMatrix);
	uint getDegree(uint node, std::vector<std::vector<bool> >& AdjMatrix);
	void getCycles(std::vector<uint>& permutation, std::vector<uint>& orbit);
	void followTrail(std::vector<uint>& permutation, std::vector<uint>& cycle, uint seed, uint current, std::vector<bool>& visited);
}
