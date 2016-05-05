#ifndef GRAPHLETCOSINE_HPP
#define GRAPHLETCOSINE_HPP
#include "LocalMeasure.hpp"

class GraphletCosine: public LocalMeasure {
public:
    GraphletCosine(Graph* G1, Graph* G2);
    virtual ~GraphletCosine();

private:
    void initSimMatrix();

    const uint NUM_ORBITS = 73;
};

#endif
