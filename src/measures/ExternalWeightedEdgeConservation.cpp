#include "ExternalWeightedEdgeConservation.hpp"
#include <sstream>
#include <fstream>

ExternalWeightedEdgeConservation::ExternalWeightedEdgeConservation(Graph* G1, Graph* G2, std::string scoresFile) : Measure(G1, G2, "ewec"){
    loadMatrix(scoresFile);
    G1->getAdjLists(adjListG1);
    G2->getAdjLists(adjListG2);
    G1->getAdjMatrix(adjMatrixG1);
    G2->getAdjMatrix(adjMatrixG2);
    nodeNamesG1 = G1->getNodeNames();
    nodeNamesG2 = G2->getNodeNames();
}

double ExternalWeightedEdgeConservation::eval(const Alignment& A){
    std::vector<std::vector<ushort> > edgeListG1;
    G1->getEdgeList(edgeListG1);
    //std::vector<std::vector<bool> > adjMatrixG2;
    //G2->getAdjMatrix(adjMatrixG2);
    double score = 0;
    for (const auto& edge: edgeListG1) {
        ushort node1 = edge[0], node2 = edge[1];
        if (adjMatrixG2[A[node1]][A[node2]]) {
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
    colIndex[n1][n2] = ind;
}

void ExternalWeightedEdgeConservation::addEdgeToRow(std::string e, int ind){
    std::string n1;
    std::string n2;
    breakEdge(e, n1, n2);
    rowIndex[n1][n2] = ind;
}

void ExternalWeightedEdgeConservation::loadMatrix(std::string scoresFile){
    simScores = std::vector<std::vector<float>>();
    colIndex = std::map<std::string, std::map<std::string, int>>();
    rowIndex = std::map<std::string, std::map<std::string, int>>();

    std::ifstream infile(scoresFile);
    if( !infile  ) {
        std::cerr << "Can't open file " << scoresFile << std::endl;
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
        simScores.push_back(vector<float>());
        ++c;
    }
    
    //process rest of the file
    while(std::getline(infile, line)){
        c = 0;
        std::stringstream lnStream(line);
        std::string edge2;
        lnStream >> edge2;
        addEdgeToRow(edge2, r);
        
        float value;
        while(lnStream >> value){
            simScores[c].push_back(value);
            ++c;
        }
        ++r;
    }
}

float ExternalWeightedEdgeConservation::getScore(int colNum, int rowNum){
    return simScores[colNum][rowNum];
}

int ExternalWeightedEdgeConservation::getColIndex(std::string n1, std::string n2){
    if(colIndex.count(n1)){
        if(colIndex[n1].count(n2)){
            return colIndex[n1][n2];
        }
    }
    if(colIndex.count(n2)){
        if(colIndex[n2].count(n1)){
            return colIndex[n2][n1];
        }
    }
    return -1;
}

int ExternalWeightedEdgeConservation::getRowIndex(std::string n1, std::string n2){
    if(rowIndex.count(n1)){
        if(rowIndex[n1].count(n2)){
            return rowIndex[n1][n2];
        }
    }
    if(rowIndex.count(n2)){
        if(rowIndex[n2].count(n1)){
            return rowIndex[n2][n1];
        }
    }
    return -1;
}

int ExternalWeightedEdgeConservation::getColIndex(ushort n1, ushort n2){
    std::string n1s = nodeNamesG2[n1], n2s = nodeNamesG2[n2];
    if(colIndex.count(n1s)){
        if(colIndex[n1s].count(n2s)){
            return colIndex[n1s][n2s];
        }
    }
    if(colIndex.count(n2s)){
        if(colIndex[n2s].count(n1s)){
            return colIndex[n2s][n1s];
        }
    }
    return -1;
}

int ExternalWeightedEdgeConservation::getRowIndex(ushort n1, ushort n2){
    std::string n1s = nodeNamesG1[n1], n2s = nodeNamesG1[n2];
    if(rowIndex.count(n1s)){
        if(rowIndex[n1s].count(n2s)){
            return rowIndex[n1s][n2s];
        }
    }
    if(rowIndex.count(n2s)){
        if(rowIndex[n2s].count(n1s)){
            return rowIndex[n2s][n1s];
        }
    }
    return -1;
}

double ExternalWeightedEdgeConservation::simScore(ushort source, ushort target, const Alignment& A){
    double score = 0;

    for (uint i = 0; i < adjListG1[source].size(); ++i) {
        ushort neighbor = adjListG1[source][i];
        if (adjMatrixG2[target][A[neighbor]]) {
            int e1 = getRowIndex(source, neighbor);
            int e2 = getColIndex(target, A[neighbor]);
            score+=getScore(e2,e1);
        }   
    }

    score /= (2*G1->getNumEdges()); //normalization
    return score;
}

double ExternalWeightedEdgeConservation::changeOp(ushort source, ushort oldTarget, ushort newTarget, const Alignment& A){
    ushort neighbor;
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

double ExternalWeightedEdgeConservation::swapOp(ushort source1, ushort source2, ushort target1, ushort target2, const Alignment& A){
    ushort neighbor;
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
        ushort neighbor = adjListG1[source2][i];
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
}
