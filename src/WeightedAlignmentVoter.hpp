#ifndef WEIGHTEDALIGNMENTVOTER_HPP
#define	WEIGHTEDALIGNMENTVOTER_HPP
#include "utils.hpp"
#include "Graph.hpp"
#include "LocalMeasure.hpp"
#include "Method.hpp"
#include "Alignment.hpp"
#include <iostream>
using namespace std;

class WeightedAlignmentVoter: public Method {
public:
	WeightedAlignmentVoter(Graph* G1, Graph* G2, LocalMeasure* nodeSim);
    Alignment run();
    void describeParameters(ostream& stream);
    string fileNameSuffix(const Alignment& A);
private:
	LocalMeasure* nodeSim;
	vector<ushort> A;
	void updateNeighbors(const vector<bool>& alreadyAlignedG1, const vector<bool>& alreadyAlignedG2, ushort node, const vector<vector<double> >& nodeSimMatrix, vector<vector<double> >& simMatrix);
	ushort addBestPair(const vector<vector<double> >& simMatrix, vector<bool>& alreadyAlignedG1, vector<bool>& alreadyAlignedG2);
};

#endif