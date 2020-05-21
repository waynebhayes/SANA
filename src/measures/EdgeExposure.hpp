#ifndef EDGEEXPOSURE_HPP
#define EDGEEXPOSURE_HPP
#include <vector>
#include <iostream>
#include "utils.hpp"
#include "Measure.hpp"
#include "localMeasures/LocalMeasure.hpp"
#include "../Graph.hpp"

extern unsigned EDGE_SUM;
extern unsigned MAX_EDGE;

class EdgeExposure : public Measure {
public:
    EdgeExposure(Graph* G1, Graph* G2);
    virtual ~EdgeExposure();
    double eval(const Alignment& A);
    static unsigned getNumer();
    static unsigned getDenom();
    static unsigned getMaxEdge();
    static unsigned numer, denom;
private:
};
#endif

