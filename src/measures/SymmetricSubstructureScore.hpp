#ifndef SYMMETRICSUBSTRUCTURESCORE_HPP
#define SYMMETRICSUBSTRUCTURESCORE_HPP
#include "Measure.hpp"

#define DEBUG_S3 0

class SymmetricSubstructureScore: public Measure {
public:
    SymmetricSubstructureScore(const Graph* G1, const Graph* G2);
    virtual ~SymmetricSubstructureScore();

    double eval(const Alignment& A);
    static double getAligEdgeScore(const uint u1, const uint v1, const uint u2, const uint v2);
    static double getIncChangeOp(const uint peg, const uint oldHole, const uint newHole, const Alignment &A);
    static double getIncSwapOp(const uint peg1, const uint peg2, const uint hole1, const uint hole2, const Alignment &A);
    static double getSum(const Alignment &A);

private:
    static const Graph *G1, *G2;
};

#endif //SYMMETRICSUBSTRUCTURESCORE_HPP

