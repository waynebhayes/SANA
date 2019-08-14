#include "EdgeDensity.hpp"

#include <vector>
#include <queue>
#include <iostream>

using namespace std;

EdgeDensity::EdgeDensity(Graph* G1, Graph* G2, uint maxDist) : LocalMeasure(G1, G2, "edged") {
    string subfolder = autogenMatricesFolder+getName()+"/";
    createFolder(subfolder);
    string fileName = subfolder+G1->getName()+"_"+
        G2->getName()+"_edged";
    fileName += ".bin";
    this->maxDist = maxDist;

    loadBinSimMatrix(fileName);
}

double EdgeDensity::calcEdgeDensity (vector<vector<uint> > adjList, uint originNode, uint numNodes, uint maxDist) {
    uint UNINTIALIZED_DISTANCE = numNodes;
    vector<uint> distanceFromOrigin(numNodes,UNINTIALIZED_DISTANCE);
    vector<bool> visited(numNodes, false);
    queue <uint> Q;

    uint numEdgesWithinMaxDistance = 0;
    uint numNodesWithinMaxDistance = 0;
    distanceFromOrigin[originNode] = 0;
    Q.push(originNode);

    while(not Q.empty()) {
        uint currentNode = Q.front();
        Q.pop();
        uint dist = distanceFromOrigin[currentNode];
        if(dist == maxDist) break;
        numNodesWithinMaxDistance++;

        for(uint neighbor : adjList[currentNode]) {
            if(distanceFromOrigin[neighbor] == UNINTIALIZED_DISTANCE) {
                distanceFromOrigin[neighbor] = dist + 1;
                Q.push(neighbor);
            }
            if(not visited[neighbor] && distanceFromOrigin[neighbor] < maxDist){
                numEdgesWithinMaxDistance++;
            }
        }
        visited[currentNode] = true;
    }
    uint totalEdges = (numNodesWithinMaxDistance * (numNodesWithinMaxDistance -1))/2;
    return numEdgesWithinMaxDistance/(double)totalEdges;
}

vector<double> EdgeDensity::generateVector(Graph* g, uint maxDist) {
    vector<vector<uint> > adjList;
    g->getAdjLists(adjList);
    vector<double> edged(g->getNumNodes());
    for(uint i = 0; i < g->getNumNodes(); ++i) {
        edged[i] = calcEdgeDensity(adjList, i, g->getNumNodes(), maxDist);
    }
    return edged;
}

float EdgeDensity::compare(double n1, double n2) {
    if (n1 > n2) {
        return (n2 / n1);
    } else {
        return (n1 / n2);
    }
}

void EdgeDensity::initSimMatrix() {
    edged1 = generateVector(G1, maxDist);
    edged2 = generateVector(G2, maxDist);
    uint size1 = edged1.size();
    uint size2 = edged2.size();

    sims = vector<vector<float> > (size1, vector<float> (size2, 0));

    for(uint i = 0; i < size1;  ++i) {
        for(uint j = 0; j < size2;  ++j) {
            sims[i][j] = compare(edged1[i], edged2[j]);
        }
    }
}

EdgeDensity::~EdgeDensity() {
}
