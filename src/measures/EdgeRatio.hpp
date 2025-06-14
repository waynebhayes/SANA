#ifndef EDGERATIO_HPP
#define EDGERATIO_HPP
#include "Measure.hpp"

#define DEBUG_EDGERATIO 0

class EdgeRatio: public Measure {
public:
    EdgeRatio(const Graph* G1, const Graph* G2);
    virtual ~EdgeRatio();

    double eval(const Alignment& A);
  //static double scoreOnePegSlow(const uint peg1, const uint avoidPeg, const uint hole, const Alignment& A); // UNIMPLEMENTED
    static double getAligEdgeScore(const uint u1, const uint v1, const uint u2, const uint v2);
    static double getIncChangeOp(const uint peg, const uint oldHole, const uint newHole, const Alignment &A);
    static double getIncSwapOp  (const uint peg1, const uint peg2, const uint hole1, const uint hole2, const Alignment &A);
  //static double getIncSwapOp2 (const uint peg1, const uint peg2, const uint hole1, const uint hole2, const Alignment &A);
    static double getSum(const Alignment &A);

private:
    static const Graph *G1, *G2;
    static double denominator;
    const int kErrorScore = -2;
    static double getRatio(const double w1, const double w2);
    static double computeIncChangeOp(const uint peg, const uint oldHole, const uint newHole, const Alignment &A);
    static double computeIncSwapOp  (const uint peg1, const uint peg2, const uint hole1, const uint hole2, const Alignment &A);
  //static double computeIncSwapOp2 (const uint peg1, const uint peg2, const uint hole1, const uint hole2, const Alignment &A);
    static double computeAligEdgeScore(const uint u1, const uint v1, const uint u2, const uint v2);
    static double computeSum(const Alignment &A);
    double computeDenom();
};

#endif
