#ifndef EDGEDIFFERENCE_HPP
#define EDGEDIFFERENCE_HPP
#include "Measure.hpp"

class EdgeDifference: public Measure {
public:
    EdgeDifference(Graph* G1, Graph* G2);
    virtual ~EdgeDifference();
    double eval(const Alignment& A);
    static double adjustSumToTargetScore(double edgeDifferenceSum, uint pairsCount);
    static double getEdgeDifferenceSum(Graph *G1, Graph *G2, const Alignment &A);
private:
    uint G1NodesCount;
    const int kErrorScore = -2;
};

#endif
