#ifndef INVALIDMEASURE_HPP
#define INVALIDMEASURE_HPP
#include "Measure.hpp"

class InvalidMeasure: public Measure {
public:
    InvalidMeasure();
    virtual ~InvalidMeasure();
    double eval(const Alignment& A);
    double eval(const MultiAlignment& MA); //dummy declare

private:

};

#endif

