#ifndef NODECORRECTNESS_HPP
#define	NODECORRECTNESS_HPP
#include "Measure.hpp"
#include <vector>
#include "Graph.hpp"
#include "Alignment.hpp"

class NodeCorrectness: public Measure {
public:
	NodeCorrectness(const Alignment& A);
    virtual ~NodeCorrectness();
    double eval(const Alignment& A);

private:
	Alignment trueA;

};

#endif

