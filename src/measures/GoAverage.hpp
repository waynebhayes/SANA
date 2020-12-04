#ifndef GOAVERAGE_HPP
#define GOAVERAGE_HPP
#include "Measure.hpp"

class GoAverage: public Measure {
public:
    GoAverage(Graph* G1, Graph* G2);
    virtual ~GoAverage();
    double eval(const Alignment& A);
    double eval(const MultiAlignment& MA); //dummy declare
private:

};

#endif

