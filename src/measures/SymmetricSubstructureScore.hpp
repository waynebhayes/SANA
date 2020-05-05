#ifndef SYMMETRICSUBSTRUCTURESCORE_HPP
#define SYMMETRICSUBSTRUCTURESCORE_HPP
#include "Measure.hpp"

class SymmetricSubstructureScore: public Measure {
public:
    SymmetricSubstructureScore(const Graph* G1, const Graph* G2);
    virtual ~SymmetricSubstructureScore();
    double eval(const Alignment& A);

private:

};

#endif

