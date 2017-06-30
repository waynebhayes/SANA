#ifndef TRIANGLECORRECTNESS_HPP
#define TRIANGLECORRECTNESS_HPP
#include "Measure.hpp"

class TriangleCorrectness: public Measure {
public:
    TriangleCorrectness(Graph* G1, Graph* G2);
    virtual ~TriangleCorrectness();
    double eval(const Alignment& A);
    int getMaxTriangles();
private:
    int calculateTriangles(Graph* G);
    Graph* GSmaller;
    int maxTriangles;
};

#endif
