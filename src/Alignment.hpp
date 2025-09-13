#ifndef ALIGNMENT_HPP
#define ALIGNMENT_HPP

#include <string>
#include <vector>
#include <sstream>
#include <array>
#include <iostream>
#include <cassert>
#include <algorithm>
#include "Graph.hpp"
#include "utils/utils.hpp"
#include <atomic>

using namespace std;

/* Please make it a priority not to modify this class. This is a very general/abstract/core class
   that should not know anything about any of the measures/methods/modes that use it.
   Do not add anything specific to, or used only by, a particular measure/method/mode.
   Instead of adding a function here, add it to the would-be-caller class with an alignment as parameter. */
class Alignment {
public:
    Alignment();
    Alignment(const Alignment& alig);
    Alignment &operator=(Alignment);
    Alignment(const vector<uint>& mapping);
    Alignment(const Graph& G1, const Graph& G2, const vector<array<string, 2>>& edgeList);

    static Alignment loadEdgeList(const Graph& G1, const Graph& G2, const string& fileName);

    //list of pairs of aligned node names, but first node in each pair may be of G2
    static Alignment loadEdgeListUnordered(const Graph& G1, const Graph& G2, const string& fileName);
    static Alignment loadPartialEdgeList(const Graph& G1, const Graph& G2, const string& fileName, bool byName);
    static Alignment loadMapping(const string& fileName);
    static Alignment randomColorRestrictedAlignment(const Graph& G1, const Graph& G2);
    
    //returns a random alignment from a graph with n1 nodes to a graph with nodes n2 >= n1 nodes
    static Alignment random(uint n1, uint n2);
    static Alignment empty();
    static Alignment identity(uint n);

    //returns an alignment of size n2 that is the inverse of this
    //value 'n1' is used as invalid mapping
    Alignment reverse(uint n2) const;

    //returns the correct alignment between G1 and G2 by looking at
    //their node names. it assumes that they have the same node names
    //this is useful when aligning a network with itself but with
    //shuffled node order
    static Alignment correctMapping(const Graph& G1, const Graph& G2);

    vector<uint> asVector() const {
        vector<uint> v;
        for (const auto& e : A) {
            v.push_back(e.load());
        }
        return v;
    }

    void set(uint node, uint value) {A[node].store(value);}
    const uint getSafe(uint node) const {return A[node].load();}
    void swap(uint node1, uint node2){A[node2].store(A[node1].exchange(A[node2].load()));}
    const uint operator[](uint node) const {return A[node].load(memory_order_relaxed);} // This is for relaxed and casual access. No write access, nerds.
    uint size() const {return A.size();}
    void compose(const Alignment& other);

    uint computeNumAlignedEdges(const Graph& G1, const Graph& G2) const;

    bool isCorrectlyDefined(const Graph& G1, const Graph& G2) const;
    void printDefinitionErrors(const Graph& G1, const Graph& G2) const;

private:
    vector<atomic_uint> A;
};

#endif /* ALIGNMENT_HPP */
