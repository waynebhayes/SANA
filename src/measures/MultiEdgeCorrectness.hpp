#ifndef MULTIEDGECORRECTNESS_HPP
#define MULTIEDGECORRECTNESS_HPP
#include "Measure.hpp"
#include "../Graph.hpp"

class MultiEdgeCorrectness: public Measure {
public:
    MultiEdgeCorrectness(Graph* G1, Graph* G2);
    virtual ~MultiEdgeCorrectness();
    double eval(const Alignment& A);

};

#endif

