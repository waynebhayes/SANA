#ifndef MULTI_NETPILE_H
#define MULTI_NETPILE_H

#include "Graph.hpp"
#include "NetPile.hpp"
#include "MultiAlignment.hpp"

class MultiNetPile: public NetPile {

public:
    MultiNetPile(){}
    ~MultiNetPile(){}

private:

    vector<Graph*> graphs;
    MultiAlignment *align;

};


#endif
