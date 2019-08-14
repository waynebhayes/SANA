#ifndef GRAPHLETCOSINE_HPP
#define GRAPHLETCOSINE_HPP
#include "LocalMeasure.hpp"

class GraphletCosine: public LocalMeasure {
public:
    GraphletCosine(Graph* G1, Graph* G2);
    virtual ~GraphletCosine();

private:
    void initSimMatrix();
    vector<uint> reduce(vector<uint> &v);
    const uint NUM_ORBITS = 73;
    double cosineSimilarity(vector<uint> &v1, vector<uint> &v2);
    double dot(vector<uint> &v1, vector<uint> &v2);
    double magnitude(vector<uint> &vector);
};

#endif
