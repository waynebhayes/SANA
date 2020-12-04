#ifndef EDGEDENSITY_HPP
#define EDGEDENSITY_HPP
#include "LocalMeasure.hpp"

class EdgeDensity: public LocalMeasure {
public:
	EdgeDensity(Graph* G1, Graph* G2, ushort maxDist);
    virtual ~EdgeDensity();
private:
    void initSimMatrix();
    float compare(double n1, double n2);
    double calcEdgeDensity(vector<vector<ushort> > adjList, ushort originNode, ushort numNodes, ushort maxDist);
    vector<double> generateVector(Graph* g, ushort maxDist);
    vector<ushort> numEdgesAround(ushort node, ushort maxDist) const;
    vector<double> edged1;
    vector<double> edged2;
    vector<vector<ushort> > edgeList; //edges in no particular order
    ushort maxDist;
};

#endif

