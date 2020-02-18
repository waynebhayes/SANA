#ifndef JACCARDSIMILARITYSCORE_HPP
#define JACCARDSIMILARITYSCORE_HPP
#include "Measure.hpp"

class JaccardSimilarityScore: public Measure {
public:
    JaccardSimilarityScore(Graph* G1, Graph* G2);
    virtual ~JaccardSimilarityScore();
    double eval(const Alignment& A);
    vector<uint> getAlignedByNode(const Alignment& A);

private:

};

#endif