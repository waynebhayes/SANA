#ifndef EDGECORRECTNESS_HPP
#define EDGECORRECTNESS_HPP
#include "Measure.hpp"

class EdgeCorrectness: public Measure {
public:
    EdgeCorrectness(const Graph* G1, const Graph* G2, int graphNum);
    virtual ~EdgeCorrectness();
    double eval(const Alignment& A);

    static int getIncChangeOp(uint peg, uint oldHole, uint newHole, const Alignment& A);
    static int getIncSwapOp(uint peg1, uint peg2, uint hole1, uint hole2, const Alignment& A);

private:
    int denominatorGraph;
    static const Graph *G1, *G2;
    static int computeIncChangeOp(uint peg, uint oldHole, uint newHole, const Alignment& A);
    static int computeIncSwapOp(uint peg1, uint peg2, uint hole1, uint hole2, const Alignment& A);

};

#endif

