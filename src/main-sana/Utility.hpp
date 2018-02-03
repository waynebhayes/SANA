#include "Graph.hpp"
#include "BinaryGraph.hpp"
#include <string>

using namespace std;

class Utility {
public:
    static BinaryGraph LoadBinaryGraphFromLEDAFile(const string &);
    static BinaryGraph LoadBinaryGraphFromEdgeList(const string &);
};


