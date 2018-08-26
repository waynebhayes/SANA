#include "TriangleCorrectness.hpp"
#include <vector>

TriangleCorrectness::TriangleCorrectness(Graph* G1, Graph* G2) : Measure(G1, G2, "tc"){
    if(calculateTriangles(G1) > calculateTriangles(G2)){
        GSmaller = G2;
    }else{
        GSmaller = G1;
    }

    maxTriangles = calculateTriangles(GSmaller);
}

TriangleCorrectness::~TriangleCorrectness() {
}

double TriangleCorrectness::eval(const Alignment& A){
    Graph inducedSubgraph = A.commonSubgraph(*G1, *G2);
    return ((double)calculateTriangles(&inducedSubgraph)/(maxTriangles));
}

int TriangleCorrectness::calculateTriangles(Graph* G){
    int numTriangles = 0;
    vector<vector<uint> > GAdjLists;
    Matrix<MATRIX_UNIT> GMatrix;
    G->getAdjLists(GAdjLists);
    G->getMatrix(GMatrix);
    for(uint i = 0; i < G->getNumNodes(); i++){
        for(uint j = 0; j < GAdjLists[i].size(); j++){
            for(uint k = 0; k < GAdjLists[i].size(); k++){
                if(k != j){
                    uint neighbor1 = GAdjLists[i][j];
                    uint neighbor2 = GAdjLists[i][k];
                    if(GMatrix[neighbor1][neighbor2]){
                        numTriangles++;
                    }
                }
            }
        }
    }
    return numTriangles/6;
}

int TriangleCorrectness::getMaxTriangles(){
    return maxTriangles;
}
