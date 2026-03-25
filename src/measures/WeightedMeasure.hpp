#ifndef WEIGHTEDMEASURE_HPP
#define WEIGHTEDMEASURE_HPP
#include "Measure.hpp"

class WeightedMeasure: public Measure {
public:
    WeightedMeasure(const Graph* G1, const Graph* G2, const string& name);
    virtual ~WeightedMeasure();
    virtual double eval(const Alignment& A) = 0;

    virtual double getIncChangeOp(uint peg, uint oldHole, uint newHole, const Alignment& A);
    virtual double getIncSwapOp(uint peg1, uint peg2, uint hole1, uint hole2, const Alignment& A);

    // Made public so SANA can call getEdgeScore directly in fused neighbor loops,
    // avoiding redundant neighbor-list iterations when multiple measures are active.
    virtual double getEdgeScore(double w1, double w2) = 0;

protected:
    double computeIncChangeOp(uint peg, uint oldHole, uint newHole, const Alignment& A);
    double computeIncSwapOp(uint peg1, uint peg2, uint hole1, uint hole2, const Alignment& A);
};

#endif

