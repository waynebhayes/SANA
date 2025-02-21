#ifndef EDGEDIFFERENCE_HPP
#define EDGEDIFFERENCE_HPP
#include "Measure.hpp"

class EdgeDifference: public Measure {
public:
    EdgeDifference(const Graph* G1, const Graph* G2);
    virtual ~EdgeDifference();
    double eval(const Alignment& A);

    static double adjustSumToTargetScore(const Graph *G1, const Graph *G2, const double edgeDifferenceSum);
    static double getEdgeDifferenceSum(const Graph *G1, const Graph *G2, const Alignment &A);
    static double getIncChangeOp(const uint peg, const uint oldHole, const uint newHole, const Alignment &A);
    static double getIncSwapOp(const uint peg1, const uint peg2, const uint hole1, const uint hole2, const Alignment &A);

private:
    static const Graph *G1, *G2;
    static double computeIncChangeOp(const uint peg, const uint oldHole, const uint newHole, const Alignment &A);
    static double computeIncSwapOp(const uint peg1, const uint peg2, const uint hole1, const uint hole2, const Alignment &A);
};

#endif //EDGEDIFFERENCE_HPP
