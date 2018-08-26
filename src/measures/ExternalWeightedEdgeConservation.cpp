#include "ExternalWeightedEdgeConservation.hpp"
#include <sstream>
#include <fstream>
#include <algorithm>

ExternalWeightedEdgeConservation::ExternalWeightedEdgeConservation(Graph* G1, Graph* G2, std::string scoresFile) : Measure(G1, G2, "ewec"){
    G1->getAdjLists(adjListG1);
    G2->getAdjLists(adjListG2);
    G1->getMatrix(matrixG1);
    G2->getMatrix(matrixG2);
    nodeNamesG1 = G1->getNodeNames();
    nodeNamesG2 = G2->getNodeNames();
    loadMatrix(scoresFile);
}

double ExternalWeightedEdgeConservation::eval(const Alignment& A){
    std::vector<std::vector<uint> > edgeListG1;
    G1->getEdgeList(edgeListG1);
    //std::vector<std::vector<bool> > adjMatrixG2;
    //G2->getAdjMatrix(adjMatrixG2);
    double score = 0;
    for (const auto& edge: edgeListG1) {
        uint node1 = edge[0], node2 = edge[1];
        if (matrixG2[A[node1]][A[node2]]) {
            std::string n1s = nodeNamesG1[node1], n2s = nodeNamesG1[node2];
            std::string an1s = nodeNamesG2[A[node1]], an2s = nodeNamesG2[A[node2]];
            int e1 = getRowIndex(n1s, n2s); //Row for G1 and Col for G2
            int e2 = getColIndex(an1s, an2s);
            score += getScore(e2, e1);
        }
    }
    //normalizing
    score /= (2*G1->getNumEdges());
    return score;
}

void ExternalWeightedEdgeConservation::breakEdge(std::string e, std::string &n1, std::string &n2){
    int breakPoint = e.find('-');
    e[breakPoint] = ' ';
    std::stringstream ss(e);
    ss >> n1;
    ss >> n2;
}

void ExternalWeightedEdgeConservation::addEdgeToCol(std::string e, int ind){
    std::string n1;
    std::string n2;
    breakEdge(e, n1, n2);
    int n1Index = std::distance(nodeNamesG2.begin(), std::find(nodeNamesG2.begin(), nodeNamesG2.end(), n1));
    int n2Index = std::distance(nodeNamesG2.begin(), std::find(nodeNamesG2.begin(), nodeNamesG2.end(), n2));
    if(n1Index > n2Index){
        int n3Index = n1Index;
        n1Index = n2Index;
        n2Index = n3Index;
    }
    colIndex[n1Index][n2Index] = ind;
}

void ExternalWeightedEdgeConservation::addEdgeToRow(std::string e, int ind){
    std::string n1;
    std::string n2;
    breakEdge(e, n1, n2);
    int n1Index = std::distance(nodeNamesG1.begin(), std::find(nodeNamesG1.begin(), nodeNamesG1.end(), n1));
    int n2Index = std::distance(nodeNamesG1.begin(), std::find(nodeNamesG1.begin(), nodeNamesG1.end(), n2));
    if(n1Index > n2Index){
        int n3Index = n1Index;
        n1Index = n2Index;
        n2Index = n3Index;
    }
    rowIndex[n1Index][n2Index] = ind;
}

void ExternalWeightedEdgeConservation::loadMatrix(std::string scoresFile){
    simScores = std::vector<std::vector<double>>();
    colIndex = std::vector<std::vector<int>>(nodeNamesG2.size(), vector<int>(nodeNamesG2.size()));
    rowIndex = std::vector<std::vector<int>>(nodeNamesG1.size(), vector<int>(nodeNamesG1.size()));

    std::ifstream infile(scoresFile);
    if( !infile  ) {
        std::cout << "Can't open file " << scoresFile << std::endl;
        std::exit( -1 );
    }
    std::string line;
    int r = 0;
    int c = 0;

    //process first row from file
    std::getline(infile, line);
    std::stringstream colHeadersStream(line);
    std::string edge1;
    while(colHeadersStream >> edge1){
        addEdgeToCol(edge1, c);
        simScores.push_back(vector<double>());
        ++c;
    }
    
    //process rest of the file
    while(std::getline(infile, line)){
        c = 0;
        std::stringstream lnStream(line);
        std::string edge2;
        lnStream >> edge2;
        addEdgeToRow(edge2, r);
        
        double value;
        while(lnStream >> value){
            simScores[c].push_back(value);
            ++c;
        }
        ++r;
    }
}

double ExternalWeightedEdgeConservation::getScore(int colNum, int rowNum){
    return simScores[colNum][rowNum];
}

int ExternalWeightedEdgeConservation::getColIndex(std::string n1, std::string n2){
    int n1Index = std::distance(nodeNamesG2.begin(), std::find(nodeNamesG2.begin(), nodeNamesG2.end(), n1));
    int n2Index = std::distance(nodeNamesG2.begin(), std::find(nodeNamesG2.begin(), nodeNamesG2.end(), n2));
    return getColIndex(n1Index, n2Index);
}

int ExternalWeightedEdgeConservation::getRowIndex(std::string n1, std::string n2){
    int n1Index = std::distance(nodeNamesG1.begin(), std::find(nodeNamesG1.begin(), nodeNamesG1.end(), n1));
    int n2Index = std::distance(nodeNamesG1.begin(), std::find(nodeNamesG1.begin(), nodeNamesG1.end(), n2));
    return getRowIndex(n1Index, n2Index);
}

int ExternalWeightedEdgeConservation::getColIndex(uint n1, uint n2){
    bool orderCorrect = n1 < n2;
    if(!orderCorrect){
        uint n3 = n1;
        n1 = n2;
        n2 = n3;
    }
    return colIndex[n1][n2];
}

int ExternalWeightedEdgeConservation::getRowIndex(uint n1, uint n2){
    bool orderCorrect = n1 < n2;
    if(!orderCorrect){
        uint n3 = n1;
        n1 = n2;
        n2 = n3;
    }
    return rowIndex[n1][n2];
}
/* Deprecated, do not use.
double ExternalWeightedEdgeConservation::simScore(uint source, uint target, const Alignment& A){
    double score = 0;
    for (uint i = 0; i < adjListG1[source].size(); ++i) {
        uint neighbor = adjListG1[source][i];
        if (adjMatrixG2[target][A[neighbor]]) {
            int e1 = getRowIndex(source, neighbor);
            int e2 = getColIndex(target, A[neighbor]);
            score+=getScore(e2,e1);
        }   
    }
    score /= (2*G1->getNumEdges()); //normalization
    return score;
}

double ExternalWeightedEdgeConservation::changeOp(uint source, uint oldTarget, uint newTarget, const Alignment& A){
    uint neighbor;
    int e1, e2;
    double score = 0;
    int size1 = adjListG1[source].size();
    for(int i = 0; i < size1; ++i){
        neighbor = adjListG1[source][i];
        if(adjMatrixG2[oldTarget][A[neighbor]]){
            std::string n1s = nodeNamesG1[source], n2s = nodeNamesG1[neighbor];
            std::string an1s = nodeNamesG2[oldTarget], an2s = nodeNamesG2[A[neighbor]];
            e1 = getRowIndex(n1s, n2s);
            e2 = getColIndex(an1s, an2s);
            score -= getScore(e2,e1);
        }
        if(adjMatrixG2[newTarget][A[neighbor]]){
            std::string n1s = nodeNamesG1[source], n2s = nodeNamesG1[neighbor];
            std::string an1s = nodeNamesG2[newTarget], an2s = nodeNamesG2[A[neighbor]];
            e1 = getRowIndex(n1s, n2s);
            e2 = getColIndex(an1s, an2s);
            score += getScore(e2, e1);
        }
    }
    score /= (2*G1->getNumEdges());
    return score;
}

double ExternalWeightedEdgeConservation::swapOp(uint source1, uint source2, uint target1, uint target2, const Alignment& A){
    uint neighbor;
    int e1, e2;
    double score = 0;
    int size1 = adjListG1[source1].size();
    int size2 = adjListG1[source2].size();
    for(int i = 0; i < size1; ++i){
        neighbor = adjListG1[source1][i];
        if(adjMatrixG2[target1][A[neighbor]]){
            std::string n1s = nodeNamesG1[source1], n2s = nodeNamesG1[neighbor];
            std::string an1s = nodeNamesG2[target1], an2s = nodeNamesG2[A[neighbor]];
            e1 = getRowIndex(n1s, n2s);
            e2 = getColIndex(an1s, an2s);
            score -= getScore(e2,e1);
        }
        if(adjMatrixG2[target2][A[neighbor]]){
            std::string n1s = nodeNamesG1[source1], n2s = nodeNamesG1[neighbor];
            std::string an1s = nodeNamesG2[target2], an2s = nodeNamesG2[A[neighbor]];
            e1 = getRowIndex(n1s, n2s);
            e2 = getColIndex(an1s, an2s);
            score += getScore(e2, e1);
        }
    }

    for(int i = 0; i < size2; ++i){
        uint neighbor = adjListG1[source2][i];
        if(adjMatrixG2[target2][A[neighbor]]){
            std::string n1s = nodeNamesG1[source2], n2s = nodeNamesG1[neighbor];
            std::string an1s = nodeNamesG2[target2], an2s = nodeNamesG2[A[neighbor]];
            e1 = getRowIndex(n1s, n2s);
            e2 = getColIndex(an1s, an2s);
            score -= getScore(e2, e1);
        }
        if(adjMatrixG2[target1][A[neighbor]]){
            std::string n1s = nodeNamesG1[source2], n2s = nodeNamesG1[neighbor];
            std::string an1s = nodeNamesG2[target1], an2s = nodeNamesG2[A[neighbor]];
            e1 = getRowIndex(n1s, n2s);
            e2 = getColIndex(an1s, an2s);
            score += getScore(e2, e1);
        }
    }

    score /= (2*G1->getNumEdges());
    return score;
}*/ 
