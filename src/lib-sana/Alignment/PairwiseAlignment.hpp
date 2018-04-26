#ifndef PAIRWISE_ALIGNMENT_H
#define PAIRWISE_ALIGNMENT_H
#include "Alignment.hpp"

class PairwiseAlignment: public Alignment {

public:
    PairwiseAlignment() = default;
    ~PairwiseAlignment() = default;
    PairwiseAlignment(const PairwiseAlignment &rhs);
    unsigned int numAlignedEdges(const Graph &G1, const Graph &G2) const;
    void dumpEdgeList(const Graph &G1, const Graph &G2, ostream& edgeListStream) const;

    const ushort& operator[](const ushort &idx) const {
        return A[idx];
    }
    vector <ushort> getMapping() const;
    void setVector(vector<ushort> &x) {
        A = x;
    }
    vector<ushort> &getVec() {
        return A;
    }
    uint size() const {
        return A.size();
    }
    bool printDefinitionErrors(const Graph &G1, const Graph &G2);

private:
    vector <ushort> A = {};
};

#endif
