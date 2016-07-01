#ifndef DIJKSTRA_HPP
#define DIJKSTRA_HPP

#include <string>
#include <vector>
#include <iostream>
#include <unordered_set>
#include <utility>
#include <algorithm>
#include <random>

#include "Method.hpp"
#include "../measures/localMeasures/LocalMeasure.hpp"
#include "../measures/Measure.hpp"
#include "../measures/MeasureCombination.hpp"
#include "../utils/randomSeed.hpp"

using namespace std;
typedef unsigned short ushort;

class Dijkstra: public Method {

public:
	Dijkstra(Graph* G1, Graph* G2, MeasureCombination* MC);
	
	~Dijkstra();

	Alignment run();
	void describeParameters(ostream& stream);
	string fileNameSuffix(const Alignment& A);

private:
	Graph* G1;
	Graph* G2;
	MeasureCombination* MC;

	uint n1;
	uint n2;

	vector<vector<bool> > G1AdjMatrix;
	vector<vector<bool> > G2AdjMatrix;
	vector<vector<ushort> > G1AdjLists;
	vector<vector<ushort> > G2AdjLists;

	vector<ushort> A;

	vector<vector<float> > sims;

	bool implementsLocking(){ return false; }
	
	//beginning of Grady's private variables
	Alignment* outputA;
	string outputAName;
	int max_nodes;
	
	std::unordered_set<ushort> G1_exclude;
	std::unordered_set<ushort> G2_exclude;	
	std::unordered_set<ushort> current_neighbors;
	
	vector<vector<float> >* sim_matrix;
	//SkipList seed_queue;
	//SkipList neighbor_queue;	
};

#endif
