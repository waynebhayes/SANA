#ifndef ALIGNMENT_HPP
#define ALIGNMENT_HPP

#include <string>
#include <vector>
#include <sstream>
#include <iostream>
#include <cassert>
#include <algorithm>

#include "Graph.hpp"
#include "utils/utils.hpp"
class MeasureCombination;
using namespace std;

class Alignment {
public:

    static Alignment loadEdgeList(Graph* G1, Graph* G2, string fileName);
    static Alignment loadEdgeListUnordered(Graph* G1, Graph* G2, string fileName);
    static Alignment loadPartialEdgeList(Graph* G1, Graph* G2, string fileName, bool byName);
    static Alignment loadMapping(string fileName);
    //returns a random alignment from a graph with
    //nodes 0..n1-1 to a graph with nodes 0..n2-1,
    //assuming n1 <= n2
    static Alignment random(uint n1, uint n2);
    static Alignment empty();
    static Alignment identity(uint n);

    //returns the correct alignment between G1 and G2 by looking at
    //their node names. it assumes that they have the same node names
    //this is useful when aligning a network with itself but with
    //shuffled node order
    static Alignment correctMapping(const Graph& G1, const Graph& G2);
    
    Alignment(const Alignment& alig);
    Alignment(const vector<ushort>& mapping);
    Alignment(Graph* G1, Graph* G2, const vector<vector<string> >& mapList);

    vector<ushort> getMapping() const;

    ushort& operator[](ushort node);
    const ushort& operator[](const ushort node) const;
    uint size() const;

    void write(ostream& stream) const;

    void writeEdgeList(Graph const * G1, Graph const * G2, ostream& edgeListStream) const;

    uint numAlignedEdges(const Graph& G1, const Graph& G2) const;
    int numSquaredAlignedEdges(const Graph& G1, const Graph& G2) const;

    //common subgraph: graph with same nodes as G1, but with only the edges preserved by A
    Graph commonSubgraph(const Graph& G1, const Graph& G2) const;

    void compose(const Alignment& other);

    void completeWithArbitraryEdges(const Graph& G1, const Graph& G2);

    bool isCorrectlyDefined(const Graph& G1, const Graph& G2);

    void printDefinitionErrors(const Graph& G1, const Graph& G2);

    static Alignment randomAlignmentWithLocking(Graph* G1, Graph* G2);
    static Alignment randomAlignmentWithNodeType(Graph *G1, Graph *G2);

    // These two reIndex the alignment based on the reIndex Map from G1
    void reIndexBefore_Iterations(map<ushort, ushort> reIndexMap);
    void reIndexAfter_Iterations(map<ushort, ushort> reverseReIndexMap);

    ushort& getBack() { return A.back(); }
private:

    vector<ushort> A;

};


#endif
