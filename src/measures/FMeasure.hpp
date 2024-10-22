#ifndef FMEASURE_HPP
#define FMEASURE_HPP
#include "Measure.hpp"

class FMeasure: public Measure {
public:
    FMeasure(const Graph* G1,const Graph* G2);
    virtual ~FMeasure();
    double eval(const Alignment& A);
private:

};

#endif

