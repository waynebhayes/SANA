#ifndef WEIGHTEDEDGECONSERVATION_HPP
#define WEIGHTEDEDGECONSERVATION_HPP
#include <vector>
#include <iostream>
#include "Measure.hpp"
#include "localMeasures/LocalMeasure.hpp"
#include "../Graph.hpp"

class WeightedEdgeConservation: public Measure {
public:
    WeightedEdgeConservation(Graph* G1, Graph* G2, LocalMeasure* m);
    virtual ~WeightedEdgeConservation();
    double eval(const Alignment& A);
    LocalMeasure* getNodeSimMeasure();
private:
    LocalMeasure *nodeSim;
};

#endif

