#include <cmath>
#include <vector>
#include <iostream>
#include "ShortestPathConservation.hpp"

ShortestPathConservation::ShortestPathConservation(Graph* G1, Graph* G2) : Measure(G1, G2, "spc") {
    
    G1->getDistanceMatrix(distMatrixG1);
    G2->getDistanceMatrix(distMatrixG2);
    uint maxDistG1 = matrixMax(distMatrixG1);
    uint maxDistG2 = matrixMax(distMatrixG2);
    maxDist = max(maxDistG1, maxDistG2);
}

ShortestPathConservation::~ShortestPathConservation() {
}

double ShortestPathConservation::eval(const Alignment& A) {
    uint n1 = G1->getNumNodes();
    uint total = 0;
    for (uint i = 0; i < n1; i++) {
        for (uint j = i+1; j < n1; j++) {
            short dist1 = distMatrixG1[i][j];
            short dist2 = distMatrixG2[A[i]][A[j]];
            if (dist1 == -1 and dist2 == -1) {
                total += 0;
            }
            else if (dist1 == -1 or dist2 == -1) {
                total += maxDist;
            }
            else {
                total += abs(dist1-dist2);
            }
        }
    }
    uint numPairs = binomialCoefficient(n1, 2);

    if (NORMALIZE) return 1 - (double) total/(numPairs*maxDist);
    else return (double) total/numPairs;
}
