#ifndef EDGECORRECTNESS_HPP
#define EDGECORRECTNESS_HPP
#include "Measure.hpp"

class EdgeCorrectness: public Measure {
public:
    EdgeCorrectness(const Graph* G1, const Graph* G2, int graphNum);
    virtual ~EdgeCorrectness();
    double eval(const Alignment& A);
    static double getIncChangeOp(uint peg, uint oldHole, uint newHole, const Alignment &A);
    static double getIncSwapOp(uint peg1, uint peg2, uint hole1, uint hole2, const Alignment &A);

private:
    static const Graph *G1, *G2;
    static double denominator;
};

#endif

