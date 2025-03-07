#ifndef EDGEGEOMEAN_HPP
#define EDGEGEOMEAN_HPP
#include "Measure.hpp"

class EdgeGeoMean: public Measure {
public:
    EdgeGeoMean(const Graph* G1, const Graph* G2);
    virtual ~EdgeGeoMean();
    double eval(const Alignment& A);

    static double getEdgeScore(double w1, double w2);
    static double getEdgeGeoMeanSum(const Graph *G1, const Graph *G2, const Alignment &A);
    static double getIncChangeOp(const uint peg, const uint oldHole, const uint newHole, const Alignment &A);
    static double getIncSwapOp(const uint peg1, const uint peg2, const uint hole1, const uint hole2, const Alignment &A);

private:
    static double denominator;
    static const Graph *G1, *G2;
    static double computeIncChangeOp(const uint peg, const uint oldHole, const uint newHole, const Alignment &A);
    static double computeIncSwapOp(const uint peg1, const uint peg2, const uint hole1, const uint hole2, const Alignment &A);
    double computeDenom(const Graph* G1, const Graph* G2);
};

#endif //EDGEGEOMEAN_HPP
