#ifndef JACCARDSIMILARITYSCORE_HPP
#define JACCARDSIMILARITYSCORE_HPP
#include <vector>
#include "../Graph.hpp"
#include "Measure.hpp"

class JaccardSimilarityScore: public Measure {
public:
    JaccardSimilarityScore(Graph* G1, Graph* G2);
    virtual ~JaccardSimilarityScore();
    double eval(const Alignment& A);
    static  vector<uint> getAlignedByNode(Graph *G1, Graph *G2, const Alignment& A);
    vector<uint> getAlignedByNode(const Alignment& A);

private:

};

#endif