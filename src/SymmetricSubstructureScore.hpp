#ifndef SYMMETRICSUBSTRUCTURESCORE_HPP
#define	SYMMETRICSUBSTRUCTURESCORE_HPP
#include "Measure.hpp"
#include "Alignment.hpp"

class SymmetricSubstructureScore: public Measure {
public:
    SymmetricSubstructureScore(Graph* G1, Graph* G2);
    virtual ~SymmetricSubstructureScore();
    double eval(const Alignment& A);

private:

};

#endif

