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

#define invalidPeg pegNum
#define invalidHole holeNum

using namespace std;

/* Please make it a priority not to modify this class. This is a very general/abstract/core class
   that should not know anything about any of the measures/methods/modes that use it.
   Do not add anything specific to, or used only by, a particular measure/method/mode.
   Instead of adding a function here, add it to the would-be-caller class with an alignment as parameter. */
// I do not know who said above, but I second this! -ML
class Alignment {
public:

    // CONSTRUCTORS
    Alignment();
    Alignment(const Alignment& other);
    Alignment &operator=(Alignment);
    Alignment(const vector<uint>& mapping, unsigned holeNum);
    Alignment(const Graph& G1, const Graph& G2, const vector<array<string, 2>>& edgeList);

    static Alignment loadEdgeList(const Graph& G1, const Graph& G2, const string& fileName);

    //list of pairs of aligned node names, but first node in each pair may be of G2
    static Alignment loadEdgeListUnordered(const Graph& G1, const Graph& G2, const string& fileName);
    static Alignment loadPartialEdgeList(const Graph& G1, const Graph& G2, const string& fileName, bool byName);
    static Alignment loadMapping(const string& fileName, const Graph& G1, const Graph& G2); // TODO
    static Alignment randomColorRestrictedAlignment(const Graph& G1, const Graph& G2);
    
    //returns a random alignment from a graph with n1 nodes to a graph with nodes n2 >= n1 nodes
    static Alignment random(uint pegNum, uint holeNum);
    static Alignment empty();
    static Alignment identity(uint n);

    //returns an alignment of size n2 that is the inverse of this
    //value 'n1' is used as invalid mapping
    Alignment reverse() const;

    //returns the correct alignment between G1 and G2 by looking at
    //their node names. it assumes that they have the same node names
    //this is useful when aligning a network with itself but with
    //shuffled node order
    static Alignment correctMapping(const Graph& G1, const Graph& G2);

    // OPERATORS

    vector<uint> copyPegsToHoles() const;
    vector<uint> copyHolesToPegs() const;

    void movePeg(uint peg, uint newHole);
    void movePeg(uint peg, uint oldHole, uint newHole);
    void swapPegs(uint peg1, uint peg2);
    void swapPegs(uint peg1, uint peg2, uint hole1, uint hole2);

    uint pegToHole(uint peg) const {return pegsToHoles[peg].load(std::memory_order_relaxed);}
    uint holeToPeg(uint hole) const {return holesToPegs[hole].load(std::memory_order_relaxed);}
    uint operator[](uint peg) const {return pegsToHoles[peg].load(memory_order_relaxed);} // No write access.
    uint numOfPegs() const {return pegNum;}
    uint numOfHoles() const {return holeNum;}

    void compose(const Alignment& other);

    uint computeNumAlignedEdges(const Graph& G1, const Graph& G2) const;

    bool isCorrectlyDefined(const Graph& G1, const Graph& G2) const;
    void printDefinitionErrors(const Graph& G1, const Graph& G2) const;

private:
    unsigned pegNum; // Size of A, used as unassigned value for invA
    unsigned holeNum; // Size of invA, used as unassigned value for A
    vector<atomic_uint> pegsToHoles;
    vector<atomic_uint> holesToPegs;
};

#endif /* ALIGNMENT_HPP */
