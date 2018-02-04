#ifndef PAIRWISE_ALIGNMENT_H
#define PAIRWISE_ALIGNMENT_H

#include "Graph.hpp"
#include "Alignment.hpp"

class PairwiseAlignment: public Alignment {

public:
    PairwiseAlignment() = default;
    ~PairwiseAlignment() = default;
    PairwiseAlignment(const PairwiseAlignment &rhs);

    void setVector(vector<ushort> &x) {
        A = x;
    }
    vector<ushort> &getVec() {
        return A;
    }
    bool printDefinitionErrors(const Graph &G1, const Graph &G2);

private:
    vector <ushort> A;
};

#endif
