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
    double simScore(ushort source, ushort target, const Alignment& A);
    double changeOp(ushort source, ushort oldTarget, ushort newTarget, const Alignment& A);
    double swapOp(ushort source1, ushort source2, ushort target1, ushort target2, const Alignment& A);

private:
    std::vector<std::vector<float>> simScores;
    std::map<std::string, std::map<std::string, int>> colIndex;
    std::map<std::string, std::map<std::string, int>> rowIndex;
    std::vector<std::vector<ushort> > adjListG1;
    std::vector<std::vector<ushort> > adjListG2;
    std::vector<std::vector<bool> > adjMatrixG1;
    std::vector<std::vector<bool> > adjMatrixG2;
    std::vector<std::string> nodeNamesG1;
    std::vector<std::string> nodeNamesG2;

    void breakEdge(std::string e, std::string &n1, std::string &n2); //breaks edge e into nodes and assign them to n1 and n2

    void addEdgeToCol(std::string e, int ind);
    void addEdgeToRow(std::string e, int ind);

    //initialize member variables in loadMatrix
    void loadMatrix(std::string scoresFile); //loads simScores from input file while indexing edges
    float getScore(int colNum, int rowNum); //returns sim score given the indices of the col and row
    int getColIndex(std::string n1, std::string n2); 
    int getRowIndex(std::string n1, std::string n2);
    int getColIndex(ushort n1, ushort n2); 
    int getRowIndex(ushort n1, ushort n2);
};

#endif


