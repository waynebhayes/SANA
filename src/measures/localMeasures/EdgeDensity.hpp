#ifndef EDGEDENSITY_HPP
#define EDGEDENSITY_HPP
#include "LocalMeasure.hpp"

class EdgeDensity: public LocalMeasure {
public:
    EdgeDensity(Graph* G1, Graph* G2, uint maxDist);
    virtual ~EdgeDensity();
private:
    void initSimMatrix();
    float compare(double n1, double n2);
    double calcEdgeDensity(vector<vector<uint> > adjList, uint originNode, uint numNodes, uint maxDist);
    vector<double> generateVector(Graph* g, uint maxDist);
    vector<uint> numEdgesAround(uint node, uint maxDist) const;
    vector<double> edged1;
    vector<double> edged2;
    vector<vector<uint> > edgeList; //edges in no particular order
    uint maxDist;
};

#endif

