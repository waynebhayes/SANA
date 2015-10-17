#ifndef GOAVERAGE_HPP
#define	GOAVERAGE_HPP
#include "LocalMeasure.hpp"
#include "Measure.hpp"
#include <unordered_map>

class GoAverage: public Measure {
public:
    GoAverage(Graph* G1, Graph* G2);
    virtual ~GoAverage();
    double eval(const Alignment& A);
private:

};

#endif

