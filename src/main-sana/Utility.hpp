#include "Graph.hpp"
#include "BinaryGraph.hpp"
#include <string>
#include "PairwiseAlignment.hpp"

using namespace std;

class Utility {
public:
    static BinaryGraph LoadBinaryGraphFromLEDAFile(const string &);
    static BinaryGraph LoadBinaryGraphFromEdgeList(const string &);
    static PairwiseAlignment LoadPairwiseAlignmentFromEdgeList(Graph *G1, Graph *G2, const string &filename);
};


