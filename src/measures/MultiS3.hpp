#ifndef MULTIS3_HPP
#define MULTIS3_HPP
#include <vector>
#include <iostream>
#include "utils.hpp"
#include "Measure.hpp"
#include "localMeasures/LocalMeasure.hpp"
#include "../Graph.hpp"

extern unsigned NUM_GRAPHS;

class MultiS3 : public Measure {
public:
    MultiS3(Graph* G1, Graph* G2);
    virtual ~MultiS3();
    double eval(const Alignment& A);
    static vector<uint> totalDegrees; // sum of neighboring edge weights including G1
    unsigned computeDenom(const Alignment& A, const Graph& G1, const Graph& G2);
    
    static unsigned denom; // used for inc eval 
    unsigned LaddersUnderG1;
private:
    void initDegrees(const Alignment& A, const Graph& G1, const Graph& G2);
    bool degreesInit;
};
#endif

