#ifndef DIJKSTRA_HPP
#define DIJKSTRA_HPP

#include <string>
#include <vector>
#include <iostream>
#include <unordered_set>
#include <utility>
#include <algorithm>
#include <random>

//#include "skip_list.hpp"

#include "Method.hpp"
#include "../measures/localMeasures/LocalMeasure.hpp"
#include "../measures/Measure.hpp"
#include "../measures/MeasureCombination.hpp"
#include "../utils/randomSeed.hpp"

using namespace std;

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
	Alignment outputA = Alignment::empty();
	string outputAName;
	unsigned int max_nodes;
	
	std::unordered_set<ushort> G1_exclude;
	std::unordered_set<ushort> G2_exclude;	
	std::unordered_set<ushort> current_neighbors;

	void * seed_queue;
	void * neighbor_queue;
	
	/* Member functions */

	std::pair <ushort, ushort> get_seed(Graph* G1, Graph* G2);
	std::pair <ushort, ushort> best_pair(void * pq);
	void update_neighbors(std::pair <ushort, ushort> & seed_pair,
		vector<ushort> & seed1_adj, vector<ushort> & seed2_adj );
	vector<std::pair<double, std::pair<ushort,ushort>>> best_neighbors(vector<ushort> & G1_neighbors, vector<ushort> & G2_neighbors);
	vector<ushort> exclude_nodes(vector<ushort> & v_in, std::unordered_set<ushort> & exclusion_set);

};

#endif
