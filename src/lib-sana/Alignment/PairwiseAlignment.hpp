#ifndef PAIRWISE_ALIGNMENT_H
#define PAIRWISE_ALIGNMENT_H

#include "Alignment.hpp"

class PairwiseAlignment: public Alignment {

public:
    PairwiseAlignment(){}
    ~PairwiseAlignment(){}

private:
    vector<ushort> A;

};

#endif
