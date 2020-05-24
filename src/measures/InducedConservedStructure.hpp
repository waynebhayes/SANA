#ifndef INDUCEDCONSERVEDSTRUCTURE_HPP
#define INDUCEDCONSERVEDSTRUCTURE_HPP
#include "Measure.hpp"

class InducedConservedStructure: public Measure {
public:
    InducedConservedStructure(const Graph* G1, const Graph* G2);
    virtual ~InducedConservedStructure();
    double eval(const Alignment& A);
private:

};

#endif

