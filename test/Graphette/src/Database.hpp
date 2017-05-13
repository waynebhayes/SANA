#ifndef DATABASE_HPP
#define DATABASE_HPP
#include "Graphette.hpp"
#include "Graph.hpp"
#include <bits/stdc++.h>

typedef struct
{
	std::string canonicalPermutation;
	ullint canonicalDecimal;
}graphette;

class Database
{
public:
	Database();
	Database(ullint k);
	void addGraph(std::string filename, ullint numSamples);
	
private:
	ullint k_, numOrbitId_;
	std::string databaseDir;
	std::vector<graphette> g;
	std::vector<ullint> canonicalGraphette;
	std::vector<std::vector<ullint>> orbitId_; //orbit id of node j from ith canonical graphette
	Graphette* getCanonicalGraphette(Graphette* x);
};

#endif
