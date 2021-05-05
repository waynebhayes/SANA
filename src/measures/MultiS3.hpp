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
    enum NumeratorType{numer_default, ra_k, la_k, la_global, ra_global};
    enum DenominatorType{denom_default, rt_k, mre_k, ee_k, ee_global, rt_global};
    static NumeratorType numerator_type;
    static DenominatorType denominator_type;

    MultiS3(const  Graph* G1, const Graph* G2, NumeratorType _numerator_type, DenominatorType _denominator_type);
    virtual ~MultiS3();
    double eval(const Alignment& A);
    static vector<uint> shadowDegree; // sum of neighboring edge weights including G1

    //denom is the number of ladders under G1
    void setDenom(const Alignment& A);
    uint computeNumer(const Alignment& A) const;
    uint computeDenom(const Alignment& A) const;
    void getNormalizationFactor() const;

    //these don't belong here, they should be members in SANA -Nil
    // WH: Not sure I agree, these are MS3 specific... OTOH measure *values* are associated with a specific alignment,
    // and putting these in *either* SANA:: or here breaks when there are multiple Alignment instances.
    static uint numer, denom, ER_k, EL_k, RA_k, RU_k, RO_k; // used for inc eval
    static double Normalization_factor;
    static double _type; //0 default ; 1 ee

private:
    static void initDegrees(const Alignment& A, const Graph& G1, const Graph& G2);
    static bool degreesInit;
};
#endif

