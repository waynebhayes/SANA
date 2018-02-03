#ifndef PAIRWISE_ALIGNMENT_H
#define PAIRWISE_ALIGNMENT_H

#include "Graph.hpp"
#include "Alignment.hpp"

class PairwiseAlignment: public Alignment {

public:
    PairwiseAlignment(){}
    ~PairwiseAlignment(){}
    PairwiseAlignment(const PairwiseAlignment &rhs);
    PairwiseAlignment(Graph *G1, Graph *G2, const vector < vector<string> > &mapList);

    void printDefinitionErrors(const Graph &G1, const Graph &G2);
private:
    vector<ushort> A;
};

#endif
