#ifndef EDGEDIFFERENCE_HPP
#define EDGEDIFFERENCE_HPP
#include "WeightedMeasure.hpp"

class EdgeDifference: public WeightedMeasure {
public:
    EdgeDifference(const Graph* G1, const Graph* G2);
    virtual ~EdgeDifference();
    double eval(const Alignment& A);

    // Keep static methods for backward compatibility with existing static calls
    static double adjustSumToTargetScore(const Graph *G1, const Graph *G2, const double edgeDifferenceSum);
    static double getEdgeDifferenceSum(const Graph *G1, const Graph *G2, const Alignment &A);

private:
    double getEdgeScore(double w1, double w2);
};

#endif //EDGEDIFFERENCE_HPP
