#ifndef EDGERATIO_HPP
#define EDGERATIO_HPP
#include "Measure.hpp"

class EdgeRatio: public Measure {
public:
    EdgeRatio(const Graph* G1, const Graph* G2);
    virtual ~EdgeRatio();

    double eval(const Alignment& A);
    static double scoreOnePegSlow(const uint peg1, const uint avoidPeg, const uint hole, const Alignment& A); // NOTE: UNIMPLEMENTED
    static double getAligEdgeScore(const uint u1, const uint v1, const uint u2, const uint v2);               // NOTE: UNIMPLEMENTED
    static double getIncChangeOp(uint peg, uint oldHole, uint newHole, const Alignment &A);
    static double getIncSwapOp(uint peg1, uint peg2, uint hole1, uint hole2, const Alignment &A);
    static double getSum(const Alignment &A);

private:
    static const Graph *G1, *G2;
    static double denominator;
    const int kErrorScore = -2;

    static double getRatio(double w1, double w2);
    static double computeIncChangeOp(uint peg, uint oldHole, uint newHole, const Alignment &A);
    static double computeIncSwapOp(uint peg1, uint peg2, uint hole1, uint hole2, const Alignment &A);
    static double computeSum(const Alignment &A);
    double computeDenom();
};

#endif
