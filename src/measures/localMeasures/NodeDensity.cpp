#include "NodeDensity.hpp"

#include <vector>
#include <queue>
#include <iostream>

using namespace std;

NodeDensity::NodeDensity(Graph* G1, Graph* G2, ushort maxDist) : LocalMeasure(G1, G2, "noded") {
    string fileName = autogenMatricesFolder+G1->getName()+"_"+
        G2->getName()+"_noded";
    fileName += ".bin";
    this->maxDist = maxDist;

    loadBinSimMatrix(fileName);
}

double NodeDensity::calcNodeDensity (vector<vector<ushort> > adjList, ushort originNode, ushort numNodes, ushort maxDist) {
    ushort UNINTIALIZED_DISTANCE = numNodes;
    vector<ushort> distanceFromOrigin(numNodes,UNINTIALIZED_DISTANCE);
    queue <ushort> Q;

    ushort numNodesWithinMaxDistance = 0;
    distanceFromOrigin[originNode] = 0;
    Q.push(originNode);

    while(not Q.empty()) {
        ushort currentNode = Q.front();
        Q.pop();
        ushort dist = distanceFromOrigin[currentNode];
        if(dist == maxDist) break;
        numNodesWithinMaxDistance++;

        for(ushort neighbor : adjList[currentNode]) {
            if(distanceFromOrigin[neighbor] == UNINTIALIZED_DISTANCE) {
                distanceFromOrigin[neighbor] = dist + 1;
                Q.push(neighbor);
            }
        }
    }

    return numNodesWithinMaxDistance/(double)numNodes;
}

vector<double> NodeDensity::generateVector(Graph* g, ushort maxDist) {
    vector<vector<ushort> > adjList;
    g->getAdjLists(adjList);
    vector<double> noded(g->getNumNodes());
    for(uint i = 0; i < g->getNumNodes(); ++i) {
        noded[i] = calcNodeDensity(adjList, i, g->getNumNodes(), maxDist);
    }
    return noded;
}

float NodeDensity::compare(double n1, double n2) {
    if(n1 == 0 && n2 == 0)
        return 1.0;
    if (n1 > n2) {
        return (n2 / n1);
    } else {
        return (n1 / n2);
    }
}

void NodeDensity::initSimMatrix() {
    noded1 = generateVector(G1, maxDist);
    noded2 = generateVector(G2, maxDist);
    uint size1 = noded1.size();
    uint size2 = noded2.size();

    sims = vector<vector<float> > (size1, vector<float> (size2, 0));
    for(uint i = 0; i < size1;  ++i) {
        for(uint j = 0; j < size2;  ++j) {
            sims[i][j] = compare(noded1[i], noded2[j]);
        }
    }
}

NodeDensity::~NodeDensity() {
}
