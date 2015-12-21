#ifndef EDGEDENSITY_HPP
#define EDGEDENSITY_HPP
#include "LocalMeasure.hpp"

class EdgeDensity: public LocalMeasure {
public:
    EdgeDensity(Graph* G1, Graph* G2, const vector<double>& distWeights);
    virtual ~EdgeDensity();
private:
    vector<double> distWeights;
    void initSimMatrix();
};

#endif

