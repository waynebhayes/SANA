#ifndef GRAPHLETLGRAAL_HPP
#define GRAPHLETLGRAAL_HPP
#include "LocalMeasure.hpp"

class GraphletLGraal: public LocalMeasure {
public:
    GraphletLGraal(Graph* G1, Graph* G2);
    virtual ~GraphletLGraal();

private:
    void initSimMatrix();
    
    double gdvSim(uint i, uint j, const vector<vector<uint> >& gdvsG1,
        const vector<vector<uint> >& gdvsG2);

};

#endif
