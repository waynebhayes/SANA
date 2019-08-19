//
// Created by taylor on 4/26/18.
//

#ifndef SYMMETRICSUBSTRUCTURESCORE_HPP
#define SYMMETRICSUBSTRUCTURESCORE_HPP


#include "Measure.hpp"

class SymmetricSubstructureScore: public Measure {
public:
    SymmetricSubstructureScore(Graph* G1, Graph* G2);
    virtual ~SymmetricSubstructureScore();
    double eval(const Alignment& A);

private:

};


#endif //SANA_SYMMETRICSUBSTRUCTURESCORE_HPP
