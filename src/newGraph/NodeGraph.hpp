#ifndef NODEGRAPH_H
#define NODEGRAPH_H

#include <string>
#include <vector>
#include <array>
#include <utility>
#include <algorithm>
#include <iostream>
#include <stdexcept>
#include <cstdlib>
#include <time.h>
#include <random>
#include "../utils/utils.hpp"
#include "../utils/Timer.hpp"
#include "../utils/Matrix.hpp"
#include "../BadGraph.hpp"

#ifdef MULTI_MPI
class Alignment;
#endif

#if __APPLE__ || __MACOS__
typedef unsigned char uchar;
typedef unsigned short ushort;
typedef unsigned int uint;
typedef unsigned long ulong;
#endif



using namespace std;

//EDGE_T: macro specifying the type of the edge weights
#if defined(EDGE_T)
    // do nothing, assume user knows what they're doing
#else // EDGE_T is not currently defined
    #if defined(MULTI_PAIRWISE) || defined(MULTI_MPI)
	#ifdef WEIGHT
	    #error currently, MULTI_* is not designed for float edges
	#else
	    #define EDGE_T unsigned char // number of "rungs" in Shadow Network; change to unsigned short for >256 networks
	#endif
    #else // EDGE_T not defined, and not MULTI
	#ifdef WEIGHT
	    #error "WEIGHT requires EDGE_T to be defined; use float unless your edges are integer weight"
	#else
	    #define EDGE_T bool
	#endif
    #endif
#endif

#ifdef SPARSE
typedef unordered_map<unsigned, EDGE_T> MAP_TYPE
#else
typedef map<unsigned, EDGE_T> MAP_TYPE;
#endif

class NodeGraph {
public:
    static const string DEFAULT_COLOR_NAME;
    static const unsigned INVALID_COLOR_ID;

    typedef const struct _node {
        unsigned nodeID;
        unsigned colorID;
        double totalWeight;
        string nodeName;
        const string &colorName; // Reference for memory reasons, but it prevents trivial copying of a graph.
        MAP_TYPE adjList;
        MAP_TYPE injList;

        // Normal constructor
        _node(unsigned nodeID, unsigned colorID, double totalWeight, const string &nodeName, string &colorName,
        const MAP_TYPE &adjList, const MAP_TYPE &injList):
            nodeID(nodeID),
            colorID(colorID),
            totalWeight(totalWeight),
            nodeName(nodeName),
            colorName(colorName),
            adjList(adjList),
            injList(injList) {}

        // Copy constructor while providing new color reference
        _node(const _node &oldNode, const string &colorName):
            nodeID(oldNode.nodeID),
            colorID(oldNode.colorID),
            totalWeight(oldNode.totalWeight),
            nodeName(oldNode.nodeName),
            colorName(colorName),
            adjList(oldNode.adjList),
            injList(oldNode.injList) {}
    } Node;

    /* All-purpose constructor
    - optionalFilePath can be left empty if not relevant
    - self-loops are allowed in the edge list
    - the edge list should not contain repeated entries
    - if optionalNodeNames is empty, the number of nodes is deduced from
      the edge list and name "i" is given to the i-th node
      the name list should not contain repeated names
    - if optionalEdgeWeights is empty, all nodes get weight 1 (i.e., unweighted graph).
      Otherwise, edgeWeights should have the same size as edgeList.
      Edges with weight 0 are not supported and will end in runtime_error
    - partialNodeColorPairs is a list of node/color name pairs.
      Any node not in any pair gets a special default color */
    NodeGraph(bool directed,
          const string& name,
          const string& optionalFilePath,
          const vector<array<uint, 2>>& edgeList,
          const vector<string>& optionalNodeNames,
          const vector<EDGE_T>& optionalEdgeWeights,
          const vector<array<string, 2>>& partialNodeColorPairs);

    explicit NodeGraph(const BadGraph &badGraph);

    // Copy constructor (non-trivial because of color names).
    NodeGraph(const NodeGraph& oldGraph):
        directed(oldGraph.directed),
        name(oldGraph.name + "_copy"),
        filePath(oldGraph.filePath) {
        totalGraphWeight = oldGraph.totalGraphWeight;
        nodeNameToIndexMap = oldGraph.nodeNameToIndexMap;
        colorNames = oldGraph.colorNames;
        colorNameToId = oldGraph.colorNameToId;
        nodeGroupsByColor = oldGraph.nodeGroupsByColor;

        for (unsigned node = 0; node < nodeNameToIndexMap.size(); node++) {
            nodes.emplace_back(oldGraph.nodes.at(node), colorNames.at(oldGraph.nodes.at(node).colorID));
        }
    }

    NodeGraph(const NodeGraph &oldGraph, const vector<array<string, 2>> &partialNodeColorPairs);

    const Node &deliverNode(unsigned nodeID) const {return nodes.at(nodeID);}

private:
    // This should NOT BE PUBLIC. GraphLoader has to be fixed so that it does not require this.
    void NodeGraph::initColorDataStructs(const vector<array<string, 2>>& partialNodeColorPairs,
                                     vector<unsigned> &nodeColors, vector<string> &nodeColorNames);

    const bool directed;
    uint64_t numEdges;
    //each edge has a weight in the range of type EDGE_T, but their sum may be beyond that range
    //double can contain the sum of EDGE_T values for any EDGE_T.
    double totalGraphWeight;
    const string name, filePath;

    vector<Node> nodes;
    unordered_map<string, uint> nodeNameToIndexMap;
    //data structures for the color system
    vector<string> colorNames; //color index to color name
    unordered_map<string, uint> colorNameToId; //color name to color index
    vector<vector<uint>> nodeGroupsByColor; //color index to list of node indices
};

#endif /* NODEGRAPH_H */
