#ifndef ALIGNMENT_H
#define ALIGNMENT_H
#include "Graph.hpp"

class Alignment {

public:
    Alignment(){}
    ~Alignment(){}
    virtual void setVector(vector<ushort> &x) = 0;
    virtual vector<ushort> &getVec() = 0;
    virtual unsigned int numAlignedEdges(const Graph &G1, const Graph &G2) const = 0;
    virtual const ushort& operator[](const ushort &idx) const = 0;
    virtual void dumpEdgeList(const Graph &G1, const Graph &G2, ostream& edgeListStream) const = 0;
    virtual vector<ushort> getMapping() const = 0;

private:
    vector <ushort> A = {};
};

#endif
