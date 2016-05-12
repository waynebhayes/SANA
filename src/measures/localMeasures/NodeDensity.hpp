#ifndef NODEDENSITY_HPP
#define NODEDENSITY_HPP
#include "LocalMeasure.hpp"

class NodeDensity: public LocalMeasure {
public:
	NodeDensity(Graph* G1, Graph* G2, ushort maxDist);
    virtual ~NodeDensity();
private:
    void initSimMatrix();
    float compare(double n1, double n2);
    double calcNodeDensity(vector<vector<ushort> > adjList, ushort originNode, ushort numNodes, ushort maxDist);
    vector<double> generateVector(Graph* g, ushort maxDist);
    vector<double> noded1;
    vector<double> noded2;
    vector<vector<ushort> > nodedList; //edges in no particular order
    ushort maxDist;
};

#endif

