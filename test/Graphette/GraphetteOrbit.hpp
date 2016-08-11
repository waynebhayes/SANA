#include <algorithm>
#include <iostream>
#include <vector>
#include <cassert>

//Just declaring a namespace to surround my functions for now
//Will be removed after the code is integrated with SANA

namespace GraphetteOrbit{
	/*
	This function is the purpose of all those lines of code.
	You can call it from outside.
	It takes ANY k-graphette and determines its automorphism orbits.
	parameters:
		num_nodes = number of nodes = k
		decimal_number = the k-graphette as decimal number
		saveOrbits = where the orbits will be saved
	*/
	void getOrbits(int num_nodes, int decimal_number, std::vector<std::vector<uint> >& saveOrbits);
	
	//self-explanatory
	int toDecimal(std::vector<bool> &bits);
	
	//takes a graphette as decimal number and determines its upper half triangle of AdjMatrix
	//as a linear vector 
	std::vector<bool> bits_vector(int decimal_number, int num_nodes);
	
	//from the upper half of the AdjMatrix as a linear vector, it creates
	//a real AdjMatrix
	void setAdjMatrix(std::vector<bool>& bits, std::vector<std::vector<bool> >& AdjMatrix);
	
	//Reverse of the previous function
	//From AdjMatrix, it creates the upper half triangle as a linear vector  
	void getBits(std::vector<bool>& bits, std::vector<std::vector<bool> >& AdjMatrix);
	
	//checks if the permutation meets a certain criterion.
	//Note: This function just for speed up. You can comment out its usage 
	bool suitable(std::vector<uint>& permutation, std::vector<std::vector<bool> >& AdjMatrix);
	
	//self-explanatory
	uint getDegree(uint node, std::vector<std::vector<bool> >& AdjMatrix);
	
	//gets the cycles from the given permutations applies them to orbits
	void getCycles(std::vector<uint>& permutation, std::vector<uint>& orbit);

	//Follows trail of the permutation of a node to determine a cycle
	void followTrail(std::vector<uint>& permutation, std::vector<uint>& cycle, uint seed, uint current, std::vector<bool>& visited);
}
