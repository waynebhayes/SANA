#ifndef SEQUENCE_HPP
#define SEQUENCE_HPP
#include "LocalMeasure.hpp"
#include <unordered_map>

class Sequence: public LocalMeasure {
public:
    Sequence(Graph* G1, Graph* G2);
    virtual ~Sequence();
    void generateBitscoresFile(string bitscoresFile);

private:

    unordered_map<string,string> initNameMap(string curatedFastaFile);
    void initSimMatrix();
};

#endif

