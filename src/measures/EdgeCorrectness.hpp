#ifndef EDGECORRECTNESS_HPP
#define EDGECORRECTNESS_HPP
#include "Measure.hpp"

class EdgeCorrectness: public Measure {
public:
    EdgeCorrectness(const Graph* G1, const Graph* G2);
    virtual ~EdgeCorrectness();
    double eval(const Alignment& A);

private:

};

#endif

