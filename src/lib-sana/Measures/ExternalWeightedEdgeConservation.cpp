//
// Created by taylor on 3/27/18.
//

#include "ExternalWeightedEdgeConservation.hpp"
#include <typeinfo>


template<typename T>
ExternalWeightedEdgeConservation<T>::ExternalWeightedEdgeConservation(Graph *G1, Graph *G2, std::string scoresFile)
        : Measure(G1, G2, "ewec") {
        G1->getCopyAdjList(adjListG1);
        G2->getCopyAdjList(adjListG2);
        G1->genAdjMatrix(adjMatrixG1); //Graph's don't hold matrices, we need to generate it. Should Graph's have adjMatrices?
        G2->genAdjMatrix(adjMatrixG2);
        nodeNamesG1 = G1->getIndexToNodeNameMap();
        nodeNamesG2 = G2->getIndexToNodeNameMap();
        loadMatrix(scoresFile);
}

template<typename T>
double ExternalWeightedEdgeConservation<T>::eval(const Alignment &A) {
    //Ugly Construction :(
    vector<vector<ushort> > edgeListG1 = vector<vector<ushort> > (G1->getAdjList().begin(), G1->getAdjList().end());
    double score = 0;
    ushort curr_src_node = 0;
    for(const auto &row : edgeListG1) {
        for(const auto &col : row) {
            //Col holds curr_dest_node
            ushort curr_dest_node = col;
            if(adjMatrixG2[A[curr_src_node]][A[curr_dest_node]]) {
                string n1s = nodeNamesG1[curr_src_node], n2s = nodeNamesG1[curr_dest_node];
                string an1s = nodeNamesG2[A[curr_src_node]], an2s = nodeNamesG2[A[curr_dest_node]];
                int e1 = getRowIndex(n1s,n2s);
                int e2 = getColIndex(an1s,an2s);
                score += getScore(e2,e1);
            }
        }
    }
    //Normalizing
    score /= (2*G1->GetNumEdges());
    return score;
}

template<typename T>
int ExternalWeightedEdgeConservation<T>::getColIndex(T n1, T n2) {
    return 0;
}

template<typename T>
int ExternalWeightedEdgeConservation<T>::getRowIndex(T n1, T n2) {
    return 0;
}

template<typename T>
double ExternalWeightedEdgeConservation<T>::getScore(int colNum, int rowNum) {
    return 0;
}

template<typename T>
void ExternalWeightedEdgeConservation<T>::breakEdge(string e, string &n1, string &n2) {

}

template<typename T>
void ExternalWeightedEdgeConservation<T>::addEdgeToCol(string e, int ind) {

}

template<typename T>
void ExternalWeightedEdgeConservation<T>::addEdgeToRow(string e, int ind) {

}

template<typename T>
void ExternalWeightedEdgeConservation<T>::loadMatrix(string scoresFile) {

}
