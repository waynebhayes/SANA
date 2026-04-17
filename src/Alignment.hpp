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
    Alignment &operator=(const Alignment &); //TODO
    Alignment(const vector<uint>& mapping, uint n2);
    Alignment(const Graph& G1, const Graph& G2, const vector<array<string, 2>>& edgeList);

    static Alignment loadEdgeList(const Graph& G1, const Graph& G2, const string& fileName);
    //list of pairs of aligned node names, but first node in each pair may be of G2
    static Alignment loadEdgeListUnordered(const Graph& G1, const Graph& G2, const string& fileName);
    static Alignment loadPartialEdgeList(const Graph& G1, const Graph& G2, const string& fileName, bool byName);
    static Alignment loadMapping(const string& fileName, unsigned n2);
    static Alignment randomColorRestrictedAlignment(const Graph& G1, const Graph& G2);

    // void loadAllowedPartners(const Graph& G1, const Graph& G2, const string& fileName);

    //returns a random alignment from a graph with n1 nodes to a graph with nodes n2 >= n1 nodes
    static Alignment random(uint n1, uint n2);
    static Alignment empty();

    //returns an alignment of size n2 that is the inverse of this
    //value 'n1' is used as invalid mapping
    Alignment reverse(uint n2) const; //TODO

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

    uint numOfPegs() const {return n1;}
    void compose(const Alignment& other); // TODO

    uint computeNumAlignedEdges(const Graph& G1, const Graph& G2) const;

    bool isCorrectlyDefined(const Graph& G1, const Graph& G2) const; // TODO
    void printDefinitionErrors(const Graph& G1, const Graph& G2) const;

    // This is for relaxed and casual access. No write access!
    uint operator[](uint peg) const {return A[peg].load(memory_order_relaxed);}

#ifdef PREFERRED_HOLES
    uint numOfHoles() const {return n2;}
    uint pegToHole(uint peg) const {return A.at(peg).load();}
    uint holeToPeg(uint hole) const {return invA.at(hole).load();}
    void movePeg(uint peg, uint newHole) {
        assert(invA[newHole].load() == n1);
        const unsigned oldHole = A[peg].load();
        A[peg].store(newHole);
        invA[newHole].store(peg);
        invA[oldHole].store(n1);
    }
    void swapPegs(uint peg1, uint peg2) {
        const unsigned hole1 = A[peg1];
        A[peg2].store(A[peg1].exchange(A[peg2].load()));
    }
#else
    uint pegToHole(uint peg) const {return A[peg].load();}
    void movePeg(uint peg, uint newHole) {A[peg].store(newHole);}
    void swapPegs(uint peg1, uint peg2) {A[peg2].store(A[peg1].exchange(A[peg2].load()));}
#endif

private:
    unsigned n1;
    unsigned n2;
    vector<atomic_uint> A;

#ifdef PREFERRED_HOLES
    vector<atomic_uint> invA;
#endif
};

#endif /* ALIGNMENT_HPP */
