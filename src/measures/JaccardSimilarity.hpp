#ifndef JACCARDSIMILARITY_HPP
#define JACCARDSIMILARITYSCORE_HPP
#include "Measure.hpp"

class JaccardSimilarity: public Measure {
public:
    JaccardSimilarity(Graph* G1, Graph* G2);
    virtual ~JaccardSimilarity();
    double eval(const Alignment& A);

private:

};

#endif