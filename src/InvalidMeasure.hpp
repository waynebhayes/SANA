#ifndef INVALIDMEASURE_HPP
#define	INVALIDMEASURE_HPP
#include "Measure.hpp"
#include "Alignment.hpp"

class InvalidMeasure: public Measure {
public:
	InvalidMeasure();
    virtual ~InvalidMeasure();
    double eval(const Alignment& A);

private:

};

#endif

