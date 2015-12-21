#ifndef GRAPHLET_HPP
#define GRAPHLET_HPP
#include "LocalMeasure.hpp"

class Graphlet: public LocalMeasure {
public:
    Graphlet(Graph* G1, Graph* G2);
    virtual ~Graphlet();

private:
    void initSimMatrix();

    vector<double> getNumbersOfAffectedOrbits();
    vector<double> getOrbitWeights();
    double getOrbitWeightSum();

    const uint NUM_ORBITS = 73;
};

#endif
