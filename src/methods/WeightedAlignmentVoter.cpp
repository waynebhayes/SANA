#include <iostream>
#include <vector>
#include <cstdlib>
#include "WeightedAlignmentVoter.hpp"
#include "../measures/WeightedEdgeConservation.hpp"
using namespace std;

WeightedAlignmentVoter::WeightedAlignmentVoter(Graph* G1, Graph* G2, LocalMeasure* nodeSim):
    Method(G1, G2, "WAVE"), nodeSim(nodeSim) {

}

uint WeightedAlignmentVoter::addBestPair(const vector<vector<double> >& simMatrix, vector<bool>& alreadyAlignedG1, vector<bool>& alreadyAlignedG2) {
    uint n1 = simMatrix.size();
    uint n2 = simMatrix[0].size();
    uint bestNodeG1 = -1;
    uint bestNodeG2 = -1;
    double bestScore = -1;
    for (uint i = 0; i < n1; i++) {
        if (not alreadyAlignedG1[i]) {
            for (uint j = 0; j < n2; j++) {
                if (not alreadyAlignedG2[j]) {
                    if (simMatrix[i][j] > bestScore) {
                        bestNodeG1 = i;
                        bestNodeG2 = j;
                        bestScore = simMatrix[i][j];
                    }
                }
            }
        }
    }
    A[bestNodeG1] = bestNodeG2;
    alreadyAlignedG1[bestNodeG1] = true;
    alreadyAlignedG2[bestNodeG2] = true;
    return bestNodeG1;
}

void WeightedAlignmentVoter::updateNeighbors(const vector<bool>& alreadyAlignedG1, const vector<bool>& alreadyAlignedG2, uint node, const vector<vector<double> >& nodeSimMatrix, vector<vector<double> >& simMatrix) {
    vector<vector<uint> > adjListsG1, adjListsG2;
    G1->getAdjLists(adjListsG1);
    G2->getAdjLists(adjListsG2);
    for (uint neighborG1 : adjListsG1[node]) {
        if (not alreadyAlignedG1[neighborG1]) {
            for (uint neighborG2 : adjListsG2[A[node]]) {
                if (not alreadyAlignedG2[neighborG2]) {
                    simMatrix[neighborG1][neighborG2] +=
                        nodeSimMatrix[neighborG1][neighborG2] +
                        nodeSimMatrix[node][A[node]];
                }
            }
        }
    }
}

Alignment WeightedAlignmentVoter::run() {
    int n1 = G1->getNumNodes();
    int n2 = G2->getNumNodes();
    vector<vector<double> > nodeSimMatrix(n1, vector<double> (n2));
    vector<vector<double> > simMatrix(n1, vector<double> (n2));
    for (int i = 0; i < n1; i++) {
        for (int j = 0; j < n2; j++) {
            nodeSimMatrix[i][j] = (*(nodeSim->getSimMatrix()))[i][j];
            simMatrix[i][j] = nodeSimMatrix[i][j];
        }
    }
    A = vector<uint> (n1);
    vector<bool> alreadyAlignedG1(n1, false);
    vector<bool> alreadyAlignedG2(n2, false);
    for (int i = 0; i < n1; i++) {
        uint newNode = addBestPair(simMatrix, alreadyAlignedG1, alreadyAlignedG2);
        updateNeighbors(alreadyAlignedG1, alreadyAlignedG2, newNode, nodeSimMatrix, simMatrix);
    }
    return A;
}

void WeightedAlignmentVoter::describeParameters(ostream& stream) {
    stream << "Node similarity measure: " << nodeSim->getName() << endl;
}

string WeightedAlignmentVoter::fileNameSuffix(const Alignment& A) {
    WeightedEdgeConservation wec(G1, G2, nodeSim);
    return "_"+nodeSim->getName()+"_"+extractDecimals(wec.eval(A),3);
}
