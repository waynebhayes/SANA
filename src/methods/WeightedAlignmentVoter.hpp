#ifndef WEIGHTEDALIGNMENTVOTER_HPP
#define WEIGHTEDALIGNMENTVOTER_HPP
#include <iostream>
#include "Method.hpp"
#include "../measures/localMeasures/LocalMeasure.hpp"
using namespace std;

class WeightedAlignmentVoter: public Method {
public:
    WeightedAlignmentVoter(Graph* G1, Graph* G2, LocalMeasure* nodeSim);
    Alignment run();
    void describeParameters(ostream& stream);
    string fileNameSuffix(const Alignment& A);
private:
    LocalMeasure* nodeSim;
    vector<uint> A;
    void updateNeighbors(const vector<bool>& alreadyAlignedG1, const vector<bool>& alreadyAlignedG2, uint node, const vector<vector<double> >& nodeSimMatrix, vector<vector<double> >& simMatrix);
    uint addBestPair(const vector<vector<double> >& simMatrix, vector<bool>& alreadyAlignedG1, vector<bool>& alreadyAlignedG2);
};

#endif
