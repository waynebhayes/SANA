#ifndef GOCOVERAGE_HPP
#define	GOCOVERAGE_HPP
#include "localMeasures/LocalMeasure.hpp"
#include "Measure.hpp"

class GoCoverage: public Measure {
public:
    GoCoverage(Graph* G1, Graph* G2);
    virtual ~GoCoverage();
    double eval(const Alignment& A);
private:
	bool NORMALIZE;
    double permutationInverse(uint M, uint N);

	double downweightedScore(uint M, uint N);
	double scoreUpperBound();
};

#endif

