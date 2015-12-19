#ifndef SHORTESTPATHCONSERVATION_HPP
#define SHORTESTPATHCONSERVATION_HPP
#include "Measure.hpp"

class ShortestPathConservation: public Measure {
public:
    ShortestPathConservation(Graph* G1, Graph* G2);
    virtual ~ShortestPathConservation();
    double eval(const Alignment& A);

private:
    vector<vector<short> > distMatrixG1, distMatrixG2;
    uint maxDist;

    static const bool NORMALIZE = false;
};

#endif

