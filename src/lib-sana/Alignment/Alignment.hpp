#ifndef ALIGNMENT_H
#define ALIGNMENT_H
#include <string>
#include <vector>

class Alignment {

public:
    Alignment(){}
    ~Alignment(){}
    virtual unsigned int numAlignedEdges(const Graph &G1, const Graph &G2) const;
private:

};

#endif
