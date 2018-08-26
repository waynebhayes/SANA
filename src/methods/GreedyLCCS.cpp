#include <vector>
#include <iostream>
#include "GreedyLCCS.hpp"
#include "../measures/LargestCommonConnectedSubgraph.hpp"
using namespace std;


GreedyLCCS::GreedyLCCS(Graph* G1, Graph* G2, string startAName):
    Method(G1, G2, "GreedyLCCS"), startAName(startAName) {

    uint n1 = G1->getNumNodes();
    if (startAName == "") {
        uint n2 = G2->getNumNodes();
        startA = new Alignment(Alignment::random(n1, n2));
    }
    else {
        startA = new Alignment(Alignment::loadMapping(startAName));
    }
}

Alignment GreedyLCCS::run() {
    uint n1 = G1->getNumNodes();
    Alignment A = *startA;
    vector<vector<uint> > G1AdjLists(n1, vector<uint> (0));
    G1->getAdjLists(G1AdjLists);
    vector<vector<uint> > G2AdjLists;
    G2->getAdjLists(G2AdjLists);

    //init set of already mapped nodes in G1
    Graph cs = A.commonSubgraph(*G1, *G2);
    vector<uint> aux = cs.getConnectedComponents()[0];
    unordered_set<uint> doneNodesG1(aux.begin(), aux.end());
    
    //init set of already mapped nodes in G2
    for (uint i = 0; i < aux.size(); i++) aux[i] = A[aux[i]];
    unordered_set<uint> doneNodesG2(aux.begin(), aux.end());

    //init neighbors of done nodes in G1
    unordered_set<uint> neighbors;
    for (uint i = 0; i < n1; i++) {
        if (not doneNodesG1.count(i)) {
            for (uint j = 0; j < G1AdjLists[i].size(); j++) {
                uint neighbor = G1AdjLists[i][j];
                if (doneNodesG1.count(neighbor)) {
                    neighbors.insert(i);
                    break;
                }
            }
        }
    }

    unordered_set<uint> checkedNeighbors;

    while (not neighbors.empty()) {
        //choose neighbor
        uint node = *neighbors.begin();
        neighbors.erase(neighbors.begin());
        bool done = false;
        for (uint i = 0; not done and i < G1AdjLists[node].size(); i++) {
            uint adjNode = G1AdjLists[node][i];
            if (doneNodesG1.count(adjNode)) {
                uint adjNodeImage = A[adjNode];
                for (uint j = 0; not done and j < G2AdjLists[adjNodeImage].size(); j++) {
                    uint adjNodeImageAdjNode = G2AdjLists[adjNodeImage][j];
                    if (not doneNodesG2.count(adjNodeImageAdjNode)) {
                        doneNodesG1.insert(node);
                        doneNodesG2.insert(adjNodeImageAdjNode);
                        A[node] = adjNodeImageAdjNode;
                        for (uint k = 0; k < G1AdjLists[node].size(); k++) {
                            uint neighbor = G1AdjLists[node][k];
                            if (not doneNodesG1.count(neighbor)) {
                                neighbors.insert(neighbor);
                                if (checkedNeighbors.count(neighbor)) {
                                    checkedNeighbors.erase(neighbor);
                                }
                            }
                        }
                        done = true;
                    }
                }
            }
        }
        if (not done) {
            checkedNeighbors.insert(node);
        }
    }

    //A.completeWithArbitraryEdges(*G1, *G2);
    return A;
}

void GreedyLCCS::describeParameters(ostream& stream) {
    stream << "starting alignment: ";
    if (startAName == "") stream << "random" << endl;
    else stream << startAName << endl;
}

string GreedyLCCS::fileNameSuffix(const Alignment& A) {
    LargestCommonConnectedSubgraph lccs(G1, G2);
    return "_" + extractDecimals(lccs.nodeProportion(A),3);
}
