#ifndef EDGEDIFFERENCE_HPP
#define EDGEDIFFERENCE_HPP
#include "Measure.hpp"

class EdgeDifference: public Measure {
public:
    EdgeDifference(const Graph* G1, const Graph* G2);
    virtual ~EdgeDifference();
    double eval(const Alignment& A);

    static double adjustSumToTargetScore(double edgeDifferenceSum, uint pairsCount);
    static double getEdgeDifferenceSum(const Graph *G1, const Graph *G2, const Alignment &A);
};

#endif //EDGEDIFFERENCE_HPP
