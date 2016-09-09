#ifndef SYMMETRICEDGECOVERAGE_HPP
#define SYMMETRICEDGECOVERAGE_HPP
#include "Measure.hpp"

class SymmetricEdgeCoverage: public Measure {
public:
    SymmetricEdgeCoverage(Graph* G1, Graph* G2);
    virtual ~SymmetricEdgeCoverage();
    double eval(const Alignment& A);

private:

};

#endif

