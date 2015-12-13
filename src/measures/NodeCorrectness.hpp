#ifndef NODECORRECTNESS_HPP
#define	NODECORRECTNESS_HPP
#include <vector>
#include "Measure.hpp"

class NodeCorrectness: public Measure {
public:
	NodeCorrectness(const Alignment& A);
    virtual ~NodeCorrectness();
    double eval(const Alignment& A);

private:
	Alignment trueA;

};

#endif

