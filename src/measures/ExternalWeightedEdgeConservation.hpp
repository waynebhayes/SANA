#ifndef EXTERNALWEIGHTEDEDGECONSERVATION_HPP
#define EXTERNALWEIGHTEDEDGECONSERVATION_HPP
#include <vector>
#include <iostream>
#include <map>
#include <string>
#include "Measure.hpp"
#include "localMeasures/LocalMeasure.hpp"
#include "../Graph.hpp"

//assuming input file is egdvs file generated from GREAT

class ExternalWeightedEdgeConservation: public Measure {
public:
    ExternalWeightedEdgeConservation(Graph* G1, Graph* G2, std::string scoresFile);
    //virtual ~ExternalWeightedEdgeConservation();
    double eval(const Alignment& A);
    //double simScore(ushort source, ushort target, const Alignment& A);
    //double changeOp(ushort source, ushort oldTarget, ushort newTarget, const Alignment& A);
    //double swapOp(ushort source1, ushort source2, ushort target1, ushort target2, const Alignment& A);
    int getColIndex(ushort n1, ushort n2); //make these private before you push 
    int getRowIndex(ushort n1, ushort n2);
    double getScore(int colNum, int rowNum); //returns sim score given the indices of the col and row

private:
    std::vector<std::vector<double>> simScores;
    std::vector<std::vector<int>> colIndex;
    std::vector<std::vector<int>> rowIndex;
    std::vector<std::vector<ushort> > adjListG1;
    std::vector<std::vector<ushort> > adjListG2;
    Matrix matrixG1;
    Matrix matrixG2;
    std::vector<std::string> nodeNamesG1;
    std::vector<std::string> nodeNamesG2;

    void breakEdge(std::string e, std::string &n1, std::string &n2); //breaks edge e into nodes and assign them to n1 and n2

    void addEdgeToCol(std::string e, int ind);
    void addEdgeToRow(std::string e, int ind);

    //initialize member variables in loadMatrix
    void loadMatrix(std::string scoresFile); //loads simScores from input file while indexing edges
    int getColIndex(std::string n1, std::string n2); 
    int getRowIndex(std::string n1, std::string n2);
};

#endif


