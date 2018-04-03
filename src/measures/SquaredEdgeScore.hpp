#ifndef SQUAREDEDGESCORE_HPP
#define SQUAREDEDGESCORE_HPP
#include <vector>
#include <iostream>
#include "Measure.hpp"
#include "localMeasures/LocalMeasure.hpp"
#include "../Graph.hpp"

extern double SES_DENOM;

class SquaredEdgeScore : public Measure {
public:
    SquaredEdgeScore(Graph* G1, Graph* G2);
    virtual ~SquaredEdgeScore();
    double eval(const Alignment& A);
    static double getDenom(void);
private:
};
#endif

