#ifndef NODEDENSITY_HPP
#define	NODEDENSITY_HPP
#include "LocalMeasure.hpp"
#include "Measure.hpp"

class NodeDensity: public LocalMeasure {
public:
    NodeDensity(Graph* G1, Graph* G2, const vector<double>& distWeights);
    virtual ~NodeDensity();
private:
	vector<double> distWeights;
	
	void initSimMatrix();
};

#endif

