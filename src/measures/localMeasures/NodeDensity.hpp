#ifndef NODEDENSITY_HPP
#define NODEDENSITY_HPP
#include "LocalMeasure.hpp"

class NodeDensity: public LocalMeasure {
public:
    NodeDensity(Graph* G1, Graph* G2, uint maxDist);
    virtual ~NodeDensity();
private:
    void initSimMatrix();
    float compare(double n1, double n2);
    double calcNodeDensity(vector<vector<uint> > adjList, uint originNode, uint numNodes, uint maxDist);
    vector<double> generateVector(Graph* g, uint maxDist);
    vector<double> noded1;
    vector<double> noded2;
    vector<vector<uint> > nodedList; //edges in no particular order
    uint maxDist;
};

#endif

