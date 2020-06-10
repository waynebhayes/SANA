#ifndef MULTIS3_HPP
#define MULTIS3_HPP
#include <vector>
#include <iostream>
#include "utils.hpp"
#include "Measure.hpp"
#include "localMeasures/LocalMeasure.hpp"
#include "../Graph.hpp"

extern uint NUM_GRAPHS;

class MultiS3 : public Measure {
public:
    MultiS3(const Graph* G1, const Graph* G2, int type);
    virtual ~MultiS3();
    double eval(const Alignment& A);
    static vector<uint> shadowDegree; // sum of neighboring edge weights including G1

    //denom is the number of ladders under G1
    void setDenom(const Alignment& A);
    uint computeNumer(const Alignment& A) const;
    
    //these don't belong here, they should be members in SANA -Nil
    static uint numer, denom; // used for inc eval
    static double _type; //0 default ; 1 ee
private:
    static void initDegrees(const Alignment& A, const Graph& G1, const Graph& G2);
    static bool degreesInit;
};
#endif

