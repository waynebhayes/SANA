#ifndef EXTERNALWEIGHTEDEDGECONSERVATION_HPP
#define EXTERNALWEIGHTEDEDGECONSERVATION_HPP
#include <vector>
#include <iostream>
#include <map>
#include <string>
#include "Measure.hpp"
#include "localMeasures/LocalMeasure.hpp"
#include "../Graph.hpp"

using namespace std;

//assuming input file is egdvs file generated from GREAT

class ExternalWeightedEdgeConservation: public Measure {
public:
    ExternalWeightedEdgeConservation(Graph* G1, Graph* G2, string scoresFile);
    //virtual ~ExternalWeightedEdgeConservation();
    double eval(const Alignment& A);
    //double simScore(uint source, uint target, const Alignment& A);
    //double changeOp(uint source, uint oldTarget, uint newTarget, const Alignment& A);
    //double swapOp(uint source1, uint source2, uint target1, uint target2, const Alignment& A);
    int getColIndex(uint n1, uint n2); //make these private before you push 
    int getRowIndex(uint n1, uint n2);
    double getScore(int colNum, int rowNum); //returns sim score given the indices of the col and row

private:
    vector<vector<double>> simScores;
    vector<vector<int>> colIndex;
    vector<vector<int>> rowIndex;
    const Matrix<EDGE_T> * matrixG2; //these things are not needed as private attributes here. Just use G1 and G2
    const vector<string> * nodeNamesG1;
    const vector<string> * nodeNamesG2;

    void breakEdge(string e, string &n1, string &n2); //breaks edge e into nodes and assign them to n1 and n2

    void addEdgeToCol(string e, int ind);
    void addEdgeToRow(string e, int ind);

    //initialize member variables in loadMatrix
    void loadMatrix(string scoresFile); //loads simScores from input file while indexing edges
    int getColIndex(string n1, string n2); 
    int getRowIndex(string n1, string n2);
};

#endif


