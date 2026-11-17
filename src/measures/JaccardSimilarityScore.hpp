#ifndef JACCARDSIMILARITYSCORE_HPP
#define JACCARDSIMILARITYSCORE_HPP
#include "Measure.hpp"
#include <vector>

class JaccardSimilarityScore: public Measure {
public:
    JaccardSimilarityScore(const Graph* G1, const Graph* G2);
    virtual ~JaccardSimilarityScore();
    double eval(const Alignment& A);
    double getIncChangeOp(uint peg, uint oldHole, uint newHole, const Alignment& A);
    double getIncSwapOp(uint peg1, uint peg2, uint hole1, uint hole2, const Alignment& A);
    static vector<uint> getAlignedByNode(const Graph* G1, const Graph* G2, const Alignment& A);
};

#endif