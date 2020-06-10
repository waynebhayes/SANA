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
    MultiS3(Graph* G1, Graph* G2, int _numerator_type, int _denominator_type);
    virtual ~MultiS3();
    double eval(const Alignment& A);
    static vector<uint> shadowDegree; // sum of neighboring edge weights including G1
    unsigned computeDenom(const Alignment& A, const Graph& G1, const Graph& G2);
    
    static unsigned numer, denom; // used for inc eval
    static double _type; //0 default ; 1 ee
    unsigned LaddersUnderG1;
    static unsigned numerator_type,denominator_type,_default,ra_i,la_i,la_global,ra_global,rt_i,ee_i,ee_global,rt_global;


private:
    void initDegrees(const Alignment& A, const Graph& G1, const Graph& G2);
    bool degreesInit;
};
#endif

