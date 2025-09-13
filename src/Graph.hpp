#ifndef GRAPH_HPP
#define GRAPH_HPP

#include <string>
#include <vector>
#include <array>
#include <iostream>
#include <unordered_map>
#include <unordered_set>
#include <memory>
#include <random>
#include "utils/utils.hpp"

#if __APPLE__ || __MACOS__
typedef unsigned char uchar;
typedef unsigned short ushort;
typedef unsigned int unsigned;
typedef unsigned long ulong;
#endif



using namespace std;

//EDGE_T: macro specifying the type of the edge weights
#if defined(EDGE_T)
    // do nothing, assume user knows what they're doing
#else // EDGE_T is not currently defined
    #ifdef WEIGHT
	    #error "WEIGHT requires EDGE_T to be defined; use float unless your edges are integer weight"
	#else
	    #define EDGE_T bool
	#endif
#endif


#ifdef SPARSE
typedef unordered_map<unsigned, EDGE_T> MAP_TYPE;
#else
typedef map<unsigned, EDGE_T> MAP_TYPE;
#endif

class Graph {
public:
    static const string DEFAULT_COLOR_NAME;
    static const unsigned INVALID_COLOR_ID;

    typedef struct _node {
        const unsigned nodeID;
        const unsigned colorID;
        const double totalWeight;
        const string nodeName;
        const string colorName;
        const MAP_TYPE adjList;
        const MAP_TYPE injList;

        // Normal constructor
        // We are not using move for a reason. We want these lists to be copied.
        _node(unsigned nodeID, unsigned colorID, double totalWeight, const string &nodeName, const string &colorName,
            const MAP_TYPE &adjList, const MAP_TYPE &injList):
            nodeID(nodeID),
            colorID(colorID),
            totalWeight(totalWeight),
            nodeName(nodeName),
            colorName(colorName),
            adjList(adjList),
            injList(injList) {}

        _node(const _node &oldNode, const unsigned newColorID, const string &newColorName):
            nodeID(oldNode.nodeID),
            colorID(newColorID),
            totalWeight(oldNode.totalWeight),
            nodeName(oldNode.nodeName),
            colorName(newColorName),
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
    Graph(bool directed,
          const string& name,
          const string& optionalFilePath,
          const vector<array<unsigned, 2>>& edgeList,
          const vector<string>& optionalNodeNames,
          const vector<EDGE_T>& optionalEdgeWeights,
          const vector<array<string, 2>>& partialNodeColorPairs);

    // Note: you should not be doing this. But I require it as a legacy function for GraphLoader. - Marcus
    void reinitializeColors(const vector<array<string, 2>>& partialNodeColorPairs);

private:

    // This should NOT BE PUBLIC and cannot be used to change node colors. Use the above instead.
    void initColorDataStructs(const vector<array<string, 2>>& partialNodeColorPairs, vector<unsigned> &nodeColors);

    friend class GraphLoader;

public:

    //derived graphs: they all call the same constructor above, ensuring internal consistency.
    //prefer this "functional" way of creating new graphs rather than modifying the current graph
    //do not add I/O functions to the Graph class. Use GraphLoader.
    Graph nodeInducedSubgraph(const vector<unsigned>& nodes) const;
    Graph randomNodeInducedSubgraph(unsigned numNodes) const;
    Graph shuffledGraph(vector<unsigned>& newToOldMap) const; //the parameter is a return value
    Graph graphPower(unsigned power) const; //graph obtained by multiplying the adj matrix by itself 'power' times
    Graph graphWithAddedRandomEdges(double addedEdgesProportion) const;
    Graph graphWithRemovedRandomEdges(double removedEdgesProportion) const;
    Graph graphWithRewiredRandomEdges(double rewiredEdgesProportion) const;
    //keeps only the edges that are also in other. the result is unweighted
    Graph graphIntersection(const Graph& other, const vector<unsigned>& thisToOtherNodeMap) const;

    //O(1) getters (defined in header for efficiency -- allows inlining)
    string getName() const { return name; }
    string getFilePath() const { return filePath; }
    unsigned getNumNodes() const { return nodes.size(); }
    unsigned getNumEdges() const { return numEdges; }
    bool isDirected() const { return directed; }

    // Your bread and butter getter for interacting with the graph.
    const Node &deliverNode(unsigned nodeID) const {return nodes.at(nodeID);}

    // Marcus: these should only ever be used rarely and only ever when you need ONE random access.
    // Whenever possible, use deliverNode to receive and store the Node reference.
    EDGE_T getEdgeWeight(unsigned node1, unsigned node2) const {
        const MAP_TYPE &adjList = nodes.at(node1).adjList;
        const auto it = adjList.find(node2);
        return it != adjList.end() ? it->second : static_cast<EDGE_T>(0);
    };
    bool hasNodeName(const string& nodeName) const { return nodeNameToIndexMap.count(nodeName); }
    string getNodeName(unsigned node) const { return nodes.at(node).nodeName; }
    unsigned getNameIndex(const string& nodeName) const { return nodeNameToIndexMap.at(nodeName); } //reverse of getNodeName
    unsigned getNumNbrs(unsigned node) const { return nodes.at(node).adjList.size(); }
    double getTotalEdgeWeight() const { return totalGraphWeight; }
    double getTotalNodeWeight(unsigned node) const { return nodes.at(node).totalWeight; }
    //note: edges with weight 0 are not supported
    bool hasEdge(unsigned node1, unsigned node2) const {
        const MAP_TYPE &adjList = nodes.at(node1).adjList;
        return adjList.find(node2) != adjList.end();
    }
    bool hasSelfLoop(unsigned node) const { return hasEdge(node, node); }


    // large data structures are returned as unique pointers, legacy functions.
    // recommendation: don't use at all.
    unique_ptr<vector<unsigned>> getAdjList(unsigned node) const;
    unique_ptr<vector<vector<unsigned>>> getAdjLists() const; // THIS REALLY SHOULD NOT BE USED
    unique_ptr<vector<unsigned>> getInjList(unsigned node) const;
    const vector<array<unsigned, 2>>* getEdgeList() const;
    const unordered_map<string,unsigned>* getNodeNameToIndexMap() const;

    //things that are computed when called
    vector<string> getNodeNames() const {
        vector<string> nodeNames(getNumNodes(), "");
        for (const auto &node : nodes) {
            nodeNames.at(node.nodeID) = node.nodeName;
        }
        return nodeNames;
    }
    unsigned randomNode() const {return randIndex(nodes.size());}
    unsigned maxDegree() const;
    vector<unsigned> degreeDistribution() const;
    vector<vector<unsigned>> connectedComponents() const; //nodes grouped by CCs, sorted from larger to smaller
    unsigned numEdgesInNodeInducedSubgraph(const vector<unsigned>& subgraphNodes) const;
    vector<unsigned> numEdgesAroundByLayers(unsigned node, unsigned maxDist) const;
    vector<unsigned> numNodesAroundByLayers(unsigned node, unsigned maxDist) const;
    vector<unsigned> nodesAround(unsigned node, unsigned maxDist) const;
    bool hasSameNodeNamesAs(const Graph& other) const;
    vector<string> commonNodeNames(const Graph& other) const;

    // COLOR SYSTEM
    //colors have arbitrary strings as names. internally, they also have a numeric
    //id starting from 0, used as index for data structures
    //transform back and forth between ids and names:
    unsigned getColorId(const string &colorName) const {return colorNameToId.at(colorName);}
    string getColorName(unsigned colorId) const {return colorNames.at(colorId);}
    const vector<string>* getColorNames() const {return &colorNames;} //all names sorted by id

    //we say the graph "has" a color if there is at least one node with that color.
    //getColorId assumes that the graph has the passed color
    //if that assumption is not certain, it should be checked first:
    bool hasColor(const string &colorName) const {return colorNameToId.find(colorName) != colorNameToId.end();}
    unsigned getNodeColor(unsigned node) const {return nodes.at(node).colorID;}
    unsigned numColors() const {return colorNames.size();}

    //functions that are part of SANA's main loop. Defined here to allow inlining
    unsigned numNodesWithColor(unsigned colorId) const { return nodeGroupsByColor.at(colorId).size(); }
    const vector<unsigned>* getNodesWithColor(unsigned colorId) const { return &nodeGroupsByColor.at(colorId); }

    //color ids are internal to each graph (i.e., color i in G1 may not have the same name as color i in G2)
    //this maps the color ids of this graph to the color ids of the other graph with the
    //same name (or to the special value INVALID_COLOR_ID if the other graph has no color with that name)
    vector<unsigned> myColorIdsToOtherGraphColorIds(const Graph& other) const;

    //useful when creating a derived graph with the same nodes and colors
    //creates the vector of pairs that the constructor needs
    vector<array<string, 2>> colorsAsNodeColorNamePairs() const;

    //check for internal consistency. good practice to keep it in an "assert"
    //after constructing or modifying a graph
    bool isWellDefined() const;
    void debugPrint() const; //print info about the internal state of graph

    const bool directed;

private:
    const uint64_t numEdges;
    //each edge has a weight in the range of type EDGE_T, but their sum may be beyond that range
    //double can contain the sum of EDGE_T values for any EDGE_T.
    double totalGraphWeight;
    const string name, filePath;

    const vector<array<unsigned, 2>> edgeList;
    vector<Node> nodes;
    unordered_map<string, unsigned> nodeNameToIndexMap;
    //data structures for the color system
    vector<string> colorNames; //color index to color name
    unordered_map<string, unsigned> colorNameToId; //color name to color index
    vector<vector<unsigned>> nodeGroupsByColor; //color index to list of node indices
};

#endif /* GRAPH_HPP */
