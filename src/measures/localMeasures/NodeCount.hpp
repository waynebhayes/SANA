#ifndef NodeCount_HPP
#define NodeCount_HPP
#include "LocalMeasure.hpp"

class NodeCount: public LocalMeasure {
public:
    NodeCount(Graph* G1, Graph* G2, const vector<double>& distWeights);
    virtual ~NodeCount();
private:
    vector<double> distWeights;
    
    void initSimMatrix();
};

#endif

