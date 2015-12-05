#ifndef WEIGHTEDEDGECONSERVATION_HPP
#define	WEIGHTEDEDGECONSERVATION_HPP
#include "Measure.hpp"
#include "LocalMeasure.hpp"
#include "Alignment.hpp"

class WeightedEdgeConservation: public Measure {
public:
    WeightedEdgeConservation(Graph* G1, Graph* G2, LocalMeasure* m);
    virtual ~WeightedEdgeConservation();
    double eval(const Alignment& A);
    LocalMeasure* getNodeSimMeasure();
private:
	LocalMeasure *nodeSim;
};

#endif

