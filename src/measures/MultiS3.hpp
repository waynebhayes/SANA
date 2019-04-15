#ifndef MULTIS3_HPP
#define MULTIS3_HPP
#include <vector>
#include <iostream>
#include "utils.hpp"
#include "Measure.hpp"
#include "localMeasures/LocalMeasure.hpp"
#include "../Graph.hpp"


class MultiS3 : public Measure {
public:
    MultiS3(Graph* G1, Graph* G2);
    virtual ~MultiS3();
    double eval(const Alignment& A);
    static vector<uint> shadowDegrees; // sum of neighboring edge weights
    unsigned getDenom(const Alignment& A, const Graph& G1, const Graph& G2);
    
    static unsigned denom;
    unsigned LaddersUnderG1;
    unsigned EdgesUnderG1;
private:
    void initDegrees(const Graph& G2);
};
#endif

