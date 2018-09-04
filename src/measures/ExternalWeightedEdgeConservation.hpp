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
    //double simScore(uint source, uint target, const Alignment& A);
    //double changeOp(uint source, uint oldTarget, uint newTarget, const Alignment& A);
    //double swapOp(uint source1, uint source2, uint target1, uint target2, const Alignment& A);
    int getColIndex(uint n1, uint n2); //make these private before you push 
    int getRowIndex(uint n1, uint n2);
    double getScore(int colNum, int rowNum); //returns sim score given the indices of the col and row

private:
    std::vector<std::vector<double>> simScores;
    std::vector<std::vector<int>> colIndex;
    std::vector<std::vector<int>> rowIndex;
    std::vector<std::vector<uint> > adjListG1;
    std::vector<std::vector<uint> > adjListG2;
    Matrix<MATRIX_UNIT> matrixG1;
    Matrix<MATRIX_UNIT> matrixG2;
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


