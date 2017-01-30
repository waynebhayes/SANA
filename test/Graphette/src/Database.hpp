#ifndef DATABASE_HPP
#define DATABASE_HPP

#include "Graphette.hpp"
#include "Graph.hpp"
#include <vector>
#include <iostream>
#include <fstream>
#include <string>
#include <utility>
#include <stdexcept>

class Database
{
public:
	Database();
	Database(uint k, uint radius, uint limit);
	void addGraph(std::string filename);

	
private:
	uint k_, numOrbitId_, radius_, limit_;
	std::vector<uint> canonDec_; //canonDec[i] = decimal repr. of the canonical isomorph of graphette i;
	std::vector<uint> canonList_; //canonList[i] = ith canonical graphette in decimal
	std::vector<std::string> canonPerm_; //canonPerm[i] = Permutation of the nodes from  graphette i to canonical isomorph;
	std::vector<std::vector<uint>> orbitId_; //orbitId[i][j] = orbit id of jth node from ith canon. graphette in ascending order

	uint getIndex(std::vector<uint>& vec, uint num);
	bool queryGraphette(uint decimal);
};
#endif