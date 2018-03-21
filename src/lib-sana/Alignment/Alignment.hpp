#ifndef ALIGNMENT_H
#define ALIGNMENT_H
#include "Graph.hpp"

class Alignment {

public:
    Alignment(){}
    ~Alignment(){}
    virtual unsigned int numAlignedEdges(const Graph &G1, const Graph &G2) const = 0;

private:

};

#endif
