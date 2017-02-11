#include "ExternalWeightedEdgeConservation.hpp"
#include <sstream>
#include <fstream>

ExternalWeightedEdgeConservation::ExternalWeightedEdgeConservation(Graph* G1, Graph* G2, std::string scoresFile) : Measure(G1, G2, "ewec"){
    loadMatrix(scoresFile);
    G1->getAdjLists(adjListG1);
    G2->getAdjLists(adjListG2);
    G1->getAdjMatrix(adjMatrixG1);
    G2->getAdjMatrix(adjMatrixG2);
}

double ExternalWeightedEdgeConservation::eval(const Alignment& A){
    std::vector<std::vector<ushort> > edgeListG1;
    G1->getEdgeList(edgeListG1);
    std::vector<std::vector<bool> > adjMatrixG2;
    G2->getAdjMatrix(adjMatrixG2);
    double score = 0;
    for (const auto& edge: edgeListG1) {
        ushort node1 = edge[0], node2 = edge[1];
        if (adjMatrixG2[A[node1]][A[node2]]) {
            std::string n1s = G1->getNodeNames()[node1], n2s = G1->getNodeNames()[node2];
            std::string an1s = G2->getNodeNames()[A[node1]], an2s = G2->getNodeNames()[A[node2]];
            int e1 = getRowIndex(n1s, n2s); //Row for G1 and Col for G2
            int e2 = getColIndex(an1s, an2s);
            if(e1 != -1 && e2 != -1){
                score += getScore(e2, e1);
                //std::cout << score << std::endl;
	    }
            else{
                //print message for debugging
            }
        }
    }

    //normalizing
    score /= (2*G1->getNumEdges());
    std::cout << score << std::endl;
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
    if(colIndex.find(n1) != colIndex.end()){
        if(colIndex[n1].find(n2) != colIndex[n1].end()){
            return colIndex[n1][n2];
        }
    }
    if(colIndex.find(n2) != colIndex.end()){
        if(colIndex[n2].find(n1) != colIndex[n2].end()){
            return colIndex[n2][n1];
        }
    }
    return -1;
}

int ExternalWeightedEdgeConservation::getRowIndex(std::string n1, std::string n2){
    if(rowIndex.find(n1) != rowIndex.end()){
        if(rowIndex[n1].find(n2) != rowIndex[n1].end()){
            return rowIndex[n1][n2];
        }
    }
    if(rowIndex.find(n2) != rowIndex.end()){
        if(rowIndex[n2].find(n1) != rowIndex[n2].end()){
            return rowIndex[n2][n1];
        }
    }
    return -1;
}

int ExternalWeightedEdgeConservation::getColIndex(ushort n1, ushort n2){
    std::string n1s = G2->getNodeNames()[n1], n2s = G2->getNodeNames()[n2];
    if(colIndex.find(n1s) != colIndex.end()){
        if(colIndex[n1s].find(n2s) != colIndex[n1s].end()){
            return colIndex[n1s][n2s];
        }
    }
    if(colIndex.find(n2s) != colIndex.end()){
        if(colIndex[n2s].find(n1s) != colIndex[n2s].end()){
            return colIndex[n2s][n1s];
        }
    }
    return -1;
}

int ExternalWeightedEdgeConservation::getRowIndex(ushort n1, ushort n2){
    std::string n1s = G1->getNodeNames()[n1], n2s = G1->getNodeNames()[n2];
    if(rowIndex.find(n1s) != rowIndex.end()){
        if(rowIndex[n1s].find(n2s) != rowIndex[n1s].end()){
            return rowIndex[n1s][n2s];
        }
    }
    if(rowIndex.find(n2s) != rowIndex.end()){
        if(rowIndex[n2s].find(n1s) != rowIndex[n2s].end()){
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

    return score;
}