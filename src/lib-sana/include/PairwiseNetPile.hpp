#ifndef PAIRWISE_NETPILE_H
#define PAIRWISE_NETPILE_H

#include "Graph.hpp"
#include "NetPile.hpp"
#include "PairwiseAlignment.hpp"

class PairwiseNetPile: public NetPile {

public:
    PairwiseNetPile(){}
    ~PairwiseNetPile(){}

private:

    Graph *g1, *g2;
    PairwiseAlignment *align;

};


#endif
