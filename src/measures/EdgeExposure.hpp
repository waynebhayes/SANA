#ifndef EDGEEXPOSURE_HPP
#define EDGEEXPOSURE_HPP
#include <vector>
#include <iostream>
#include "utils.hpp"
#include "Measure.hpp"
#include "localMeasures/LocalMeasure.hpp"
#include "../Graph.hpp"

class EdgeExposure : public Measure {
public:
    EdgeExposure(Graph* G1, Graph* G2);
    virtual ~EdgeExposure();
    double eval(const Alignment& A);

    static uint getMaxEdge();
    static uint numer, denom;

    int numExposedEdges(const Alignment& A) const;
private:
	static uint EDGE_SUM, MAX_EDGE;
};
#endif

