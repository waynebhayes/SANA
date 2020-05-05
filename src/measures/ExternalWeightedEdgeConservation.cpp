#include "ExternalWeightedEdgeConservation.hpp"
#include <sstream>
#include <fstream>
#include <algorithm>

using namespace std;

ExternalWeightedEdgeConservation::ExternalWeightedEdgeConservation(
    const Graph* G1, const Graph* G2, string scoresFile):
        Measure(G1, G2, "ewec"),
        matrixG2(G2->getAdjMatrix()),
        nodeNamesG1(G1->getNodeNames()),
        nodeNamesG2(G2->getNodeNames()) {
    loadMatrix(scoresFile);
}

double ExternalWeightedEdgeConservation::eval(const Alignment& A){
    double score = 0;
    for (const auto& edge: *(G1->getEdgeList())) {
        uint node1 = edge[0], node2 = edge[1];
        if (matrixG2->get(A[node1],A[node2])) {
            string n1s = (*nodeNamesG1)[node1], n2s = (*nodeNamesG1)[node2];
            string an1s = (*nodeNamesG2)[A[node1]], an2s = (*nodeNamesG2)[A[node2]];
            int e1 = getRowIndex(n1s, n2s); //Row for G1 and Col for G2
            int e2 = getColIndex(an1s, an2s);
            score += getScore(e2, e1);
        }
    }
    //normalizing
    score /= (2*G1->getNumEdges());
    return score;
}

void ExternalWeightedEdgeConservation::breakEdge(string e, string &n1, string &n2){
    int breakPoint = e.find('-');
    e[breakPoint] = ' ';
    stringstream ss(e);
    ss >> n1;
    ss >> n2;
}

void ExternalWeightedEdgeConservation::addEdgeToCol(string e, int ind){
    string n1;
    string n2;
    breakEdge(e, n1, n2);
    int n1Index = distance(nodeNamesG2->begin(), find(nodeNamesG2->begin(), nodeNamesG2->end(), n1));
    int n2Index = distance(nodeNamesG2->begin(), find(nodeNamesG2->begin(), nodeNamesG2->end(), n2));
    if(n1Index > n2Index){
        int n3Index = n1Index;
        n1Index = n2Index;
        n2Index = n3Index;
    }
    colIndex[n1Index][n2Index] = ind;
}

void ExternalWeightedEdgeConservation::addEdgeToRow(string e, int ind){
    string n1;
    string n2;
    breakEdge(e, n1, n2);
    int n1Index = distance(nodeNamesG1->begin(), find(nodeNamesG1->begin(), nodeNamesG1->end(), n1));
    int n2Index = distance(nodeNamesG1->begin(), find(nodeNamesG1->begin(), nodeNamesG1->end(), n2));
    if(n1Index > n2Index){
        int n3Index = n1Index;
        n1Index = n2Index;
        n2Index = n3Index;
    }
    rowIndex[n1Index][n2Index] = ind;
}

void ExternalWeightedEdgeConservation::loadMatrix(string scoresFile){
    simScores = vector<vector<double>>();
    colIndex = vector<vector<int>>(nodeNamesG2->size(), vector<int>(nodeNamesG2->size()));
    rowIndex = vector<vector<int>>(nodeNamesG1->size(), vector<int>(nodeNamesG1->size()));

    ifstream infile(scoresFile);
    if( !infile  ) {
        cout << "Can't open file " << scoresFile << endl;
        exit( -1 );
    }
    string line;
    int r = 0;
    int c = 0;

    //process first row from file
    getline(infile, line);
    stringstream colHeadersStream(line);
    string edge1;
    while(colHeadersStream >> edge1){
        addEdgeToCol(edge1, c);
        simScores.push_back(vector<double>());
        ++c;
    }
    
    //process rest of the file
    while(getline(infile, line)){
        c = 0;
        stringstream lnStream(line);
        string edge2;
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

int ExternalWeightedEdgeConservation::getColIndex(string n1, string n2){
    int n1Index = distance(nodeNamesG2->begin(), find(nodeNamesG2->begin(), nodeNamesG2->end(), n1));
    int n2Index = distance(nodeNamesG2->begin(), find(nodeNamesG2->begin(), nodeNamesG2->end(), n2));
    return getColIndex(n1Index, n2Index);
}

int ExternalWeightedEdgeConservation::getRowIndex(string n1, string n2){
    int n1Index = distance(nodeNamesG1->begin(), find(nodeNamesG1->begin(), nodeNamesG1->end(), n1));
    int n2Index = distance(nodeNamesG1->begin(), find(nodeNamesG1->begin(), nodeNamesG1->end(), n2));
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
