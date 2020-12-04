#ifndef EDGECOUNT_HPP
#define EDGECOUNT_HPP
#include "LocalMeasure.hpp"
#include <algorithm>

class EdgeCount: public LocalMeasure {
public:
    EdgeCount(Graph* G1, Graph* G2, const vector<string>& strDistWeights);
    EdgeCount(Graph* G1, Graph* G2, const vector<double>& distWeights);
    virtual ~EdgeCount();
private:
    vector<double> distWeights;
    void initSimMatrix();
};

#endif

