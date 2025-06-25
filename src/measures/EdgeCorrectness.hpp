#ifndef EDGECORRECTNESS_HPP
#define EDGECORRECTNESS_HPP
#include "Measure.hpp"

#define DEBUG_EC 0

class EdgeCorrectness: public Measure {
public:
    EdgeCorrectness(const Graph* G1, const Graph* G2);
    virtual ~EdgeCorrectness();

    double eval(const Alignment& A);
    static double getAligEdgeScore(const uint u1, const uint v1, const uint u2, const uint v2);
    static double getIncChangeOp(const uint peg, const uint oldHole, const uint newHole, const Alignment &A);
    static double getIncSwapOp(const uint peg1, const uint peg2, const uint hole1, const uint hole2, const Alignment &A);
    static double getSum(const Alignment &A);

private:
    static const Graph *G1, *G2;
    static double denominator;

};

#endif //EDGECORRECTNESS_HPP

