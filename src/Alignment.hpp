#ifndef ALIGNMENT_HPP
#define	ALIGNMENT_HPP
#include <string>
#include <vector>
#include "Graph.hpp"
#include "utils.hpp"
using namespace std;

class Alignment {
public:

    static Alignment loadEdgeList(Graph* G1, Graph* G2, string fileName);
    static Alignment loadPartialEdgeList(Graph* G1, Graph* G2, string fileName);
	static Alignment loadMapping(string fileName);
	//returns a random alignment from a graph with
	//nodes 0..n1-1 to a graph with nodes 0..n2-1,
	//assuming n1 <= n2
	static Alignment random(uint n1, uint n2);
	static Alignment empty();
    static Alignment identity(uint n);

	Alignment(const Alignment& alig);
    Alignment(const vector<ushort>& mapping);
    Alignment(Graph* G1, Graph* G2, const vector<vector<string> >& mapList);

    vector<ushort> getMapping() const;

    ushort& operator[](ushort node);
    const ushort& operator[](const ushort node) const;
    uint size() const;

    void write(ostream& stream) const;

    void writeEdgeList(const Graph* G1, const Graph* G2, ostream& stream) const;
    
    uint numAlignedEdges(const Graph& G1, const Graph& G2) const;

    //common subgraph: graph with same nodes as G1, but with only the edges preserved by A
    Graph commonSubgraph(const Graph& G1, const Graph& G2) const;

    void compose(const Alignment& other);

    void completeWithArbitraryEdges(const Graph& G1, const Graph& G2);

    bool isCorrectlyDefined(const Graph& G1, const Graph& G2);
    
    void printDefinitionErrors(const Graph& G1, const Graph& G2);

private:

	vector<ushort> A;

};


#endif
