#ifndef EDGECORRECTNESS_HPP
#define EDGECORRECTNESS_HPP
#include "Measure.hpp"

class EdgeCorrectness: public Measure {
public:
    EdgeCorrectness(Graph* G1, Graph* G2);
    virtual ~EdgeCorrectness();
    double eval(const Alignment& A);

private:

};

#endif

