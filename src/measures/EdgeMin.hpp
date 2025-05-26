#ifndef EDGEMIN_HPP
#define EDGEMIN_HPP
#include "Measure.hpp"

#define DEBUG_EDGEMIN 0

class EdgeMin: public Measure {
public:
    EdgeMin(const Graph* G1, const Graph* G2);
    virtual ~EdgeMin();
    double eval(const Alignment& A);
    static double scoreOnePegSlow(const Graph* G1, const uint peg1, const uint avoidPeg, const Graph* G2, const uint hole, const Alignment& A);
    double getAligEdgeScore(const uint u1, const uint v1, const uint u2, const uint v2);
    static double getIncChangeOp(const uint peg, const uint oldHole, const uint newHole, const Alignment &A);
    static double getIncSwapOp  (const uint peg1, const uint peg2, const uint hole1, const uint hole2, const Alignment &A);
    static double getIncSwapOp2 (const uint peg1, const uint peg2, const uint hole1, const uint hole2, const Alignment &A);
    static double getSum(const Alignment &A);

private:
    static const Graph *G1, *G2;
    static double computeIncChangeOp(const uint peg, const uint oldHole, const uint newHole, const Alignment &A);
    static double computeIncSwapOp  (const uint peg1, const uint peg2, const uint hole1, const uint hole2, const Alignment &A);
    static double computeIncSwapOp2 (uint const peg1, const uint peg2, const uint hole1, const uint hole2, const Alignment &A);
    static double denominator;
    static double computeAligEdgeScore(const uint u1, const uint v1, const uint u2, const uint v2);
    static double computeSum(const Alignment &A);
    double computeDenom(const Graph* G1, const Graph* G2);
};

#endif //EDGEMIN_HPP
