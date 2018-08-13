#ifndef UTILITY_HPP
#define UTILITY_HPP
#include <string>
#include "BinaryGraph.hpp"
#include "PairwiseAlignment.hpp"

using namespace std;

class Utility {
public:
    static BinaryGraph LoadBinaryGraphFromLEDAFile(const string &);
    static BinaryGraph LoadBinaryGraphFromEdgeList(const string &);
    static PairwiseAlignment LoadPairwiseAlignmentFromEdgeList(Graph *G1, Graph *G2, const string &filename);
};

#endif


