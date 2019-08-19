//
// Created by taylor on 3/27/18.
//

#ifndef EXTERNALWEIGHTEDEDGECONSERVATION_HPP
#define EXTERNALWEIGHTEDEDGECONSERVATION_HPP


#include <Graph.hpp>
#include <Alignment.hpp>
#include <Measure.hpp>
#include <string>

template <typename T>
class ExternalWeightedEdgeConservation : public Measure {
public:
    ExternalWeightedEdgeConservation(Graph *G1, Graph *G2, std::string scoresFile);
    double eval(const Alignment& A);

private:
    int getColIndex(T n1, T n2);
    int getRowIndex(T n1, T n2);
    double getScore(int colNum, int rowNum);

    void breakEdge(string e, string &n1, string &n2);
    void addEdgeToCol(string e, int ind);
    void addEdgeToRow(string e, int ind);

    void loadMatrix(string scoresFile);

    std::vector<std::vector<double>> simScores;
    std::vector<std::vector<int>> colIndex;
    std::vector<std::vector<int>> rowIndex;
    std::vector<std::vector<ushort> > adjListG1;
    std::vector<std::vector<ushort> > adjListG2;
    std::vector<std::vector<bool> > adjMatrixG1;
    std::vector<std::vector<bool> > adjMatrixG2;
    std::unordered_map<ushort,string> nodeNamesG1;
    std::unordered_map<ushort,string> nodeNamesG2;
};


#endif //SANA_EXTERNALWEIGHTEDEDGECONSERVATION_HPP
