#ifndef EDGECORRECTNESS_HPP
#define EDGECORRECTNESS_HPP
#include "Measure.hpp"

class EdgeCorrectness: public Measure {
public:
    EdgeCorrectness(const Graph* G1, const Graph* G2, int graphNum);
    virtual ~EdgeCorrectness();
    double eval(const Alignment& A);

private:
    int denominatorGraph;
    static const Graph *G1, *G2;
    unsigned denominator;
};

#endif

