#ifndef EDGERATIO_HPP
#define EDGERATIO_HPP
#include "Measure.hpp"

class EdgeRatio: public Measure {
public:
    EdgeRatio(Graph* G1, Graph* G2);
    virtual ~EdgeRatio();
    double eval(const Alignment& A);
    static double adjustSumToTargetScore(double edgeRatioSum, uint pairsCount);
    static double getEdgeRatioSum(Graph *G1, Graph *G2, const Alignment &A);
private:
    uint G1NodesCount;
    const int kErrorScore = -2;
};

#endif
