#ifndef GRAPH_H
#define GRAPH_H

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
#include "utils/utils.hpp"
#include "utils/Timer.hpp"
#include "computeGraphlets.hpp"
#include "utils/Matrix.hpp"

using namespace std;

//EDGE_T: macro specifying the type of the edge weights
#ifdef MULTI_PAIRWISE
  #ifdef FLOAT_WEIGHTS
    #define EDGE_T float
    #error currently, multi_pairwise is not designed for float edges
  #else
    #define EDGE_T unsigned char //change to unsigned short for >256 networks
  #endif
#else
  #ifdef FLOAT_WEIGHTS
    #define EDGE_T float
  #else
    #define EDGE_T bool //unweighted graphs -- the normal/traditional setting
    //even though true/false act very much like 1 and 0, sometimes we need to
    //write things slightly different for bools than for number types.
    //We use the following flag to create an alternative code path in such cases
    #define BOOL_EDGE_T 1
  #endif
#endif

class Graph {
public:
    /* All-purpose constructor
    - optionalFilePath can be left empty if not relevant
    - self-loops are allowed in the edge list
    - the edge list should not contain repeated entries (the constructor
      will not crash if it does, but 'isWellDefined()' will)
    - if optionalNodeNames is empty, the number of nodes is deduced from
      the edge list and name "i" is given to the i-th node
    - if optionalEdgeWeights is empty, all nodes get weight 1 (i.e., unweighted graph).
      Otherwise, edgeWeights should have the same size as edgeList.
      Edges with weight 0 are not supported and will end in runtime_error
    - partialNodeColorPairs is a list of node/color name pairs.
      Any node not in any pair gets a special default color */
    Graph(const string& name,
          const string& optionalFilePath,
          const vector<array<uint, 2>>& edgeList,
          const vector<string>& optionalNodeNames,
          const vector<EDGE_T>& optionalEdgeWeights,
          const vector<array<string, 2>>& partialNodeColorPairs);

    //there is no default constructor intentionally to disable the option of
    //creating empty graphs and populating the data structures directly.

    //derived graphs: they all call the same constructor above, ensuring internal consistency.
    //prefer this "functional" way of creating new graphs rather than modifying the current graph
    //do not add I/O functions to the Graph class. Use GraphLoader.
    Graph nodeInducedSubgraph(const vector<uint>& nodes) const;
    Graph randomNodeInducedSubgraph(uint numNodes) const;
    Graph shuffledGraph(vector<uint>& newToOldMap) const; //the parameter is a return value
    Graph graphPower(uint power) const; //graph obtained by multiplying the adj matrix by itself 'power' times
    Graph graphWithAddedRandomEdges(double addedEdgesProportion) const;
    Graph graphWithRemovedRandomEdges(double removedEdgesProportion) const;
    Graph graphWithRewiredRandomEdges(double rewiredEdgesProportion) const;
    //keeps only the edges that are also in other. the result is unweighted
    Graph graphIntersection(const Graph& other, const vector<uint>& thisToOtherNodeMap) const;
    //return a graph with the edge weights in 'this' minus the weights in 'other'.
    //Crashes if any edge weight would be negative 
    //it does *not* modify 'this' graph. It is not marked const solely for technical reasons
    Graph subtractGraph(const Graph& other, const vector<uint>& otherToThisNodeMap); 

    //calling this function is the same as passing the parameter directly to the constructor
    //(which is recommended). This function only exists in case you need to construct
    //the (rest of the) graph before you can compute the colors
    //(e.g., if the colors depend on the node names, like with -lock-same-names)
    void initColorDataStructs(const vector<array<string, 2>>& partialNodeColorPairs);


    //O(1) getters (defined in header for efficiency -- allows inlining)
    string getName() const { return name; }
    string getFilePath() const { return filePath; }
    uint getNumNodes() const { return adjLists.size(); }
    uint getNumEdges() const { return edgeList.size(); }
    //note: edges with weight 0 are not supported
    bool hasEdge(uint node1, uint node2) const { return adjMatrix.get(node1, node2) != 0; }
    //returns 0 if there is no edge; the order of the arguments is irrelevant
    EDGE_T getEdgeWeight(uint node1, uint node2) const { return adjMatrix.get(node1, node2); }
    bool hasNodeName(const string& nodeName) const { return nodeNameToIndexMap.count(nodeName); }
    string getNodeName(uint node) const { return nodeNames.at(node); }
    uint getNameIndex(const string& nodeName) const { return nodeNameToIndexMap.at(nodeName); } //reverse of getNodeName
    uint getNumNbrs(uint node) const { return adjLists[node].size(); }
    double getTotalEdgeWeight() const { return totalEdgeWeight; }
    bool hasSelfLoop(uint node) const { return adjMatrix.get(node, node) != 0; }
    
    //large data structures are returned as const pointers
    const vector<vector<uint>>* getAdjLists() const { return &adjLists; }
    const vector<array<uint, 2>>* getEdgeList() const { return &edgeList; }
    //recommendation: use hasEdge() and getEdgeWeight() instead of getAdjMatrix()
    const Matrix<EDGE_T>* getAdjMatrix() const { return &adjMatrix; }
    //recommendation: use getNodeName() instead of getNodeNames() 
    const vector<string>* getNodeNames() const { return &nodeNames; }
    //recommendation: use getNameIndex() instead of getNodeNameToIndexMap()
    const unordered_map<string,uint>* getNodeNameToIndexMap() const { return &nodeNameToIndexMap; }

    //things that are computed when called
    uint randomNode() const;
    uint maxDegree() const;
    vector<uint> degreeDistribution() const;
    vector<vector<uint>> connectedComponents() const; //nodes grouped by CCs, sorted from larger to smaller
    uint numEdgesInNodeInducedSubgraph(const vector<uint>& subgraphNodes) const;
    vector<uint> numEdgesAroundByLayers(uint node, uint maxDist) const;
    vector<uint> numNodesAroundByLayers(uint node, uint maxDist) const;
    vector<uint> nodesAround(uint node, uint maxDist) const;
    bool hasSameNodeNamesAs(const Graph& other) const;
    vector<string> commonNodeNames(const Graph& other) const;
    

    // COLOR SYSTEM
    //colors have arbitrary strings as names. internally, they also have a numeric
    //id starting from 0, used as index for data structures
    //transform back and forth between ids and names:
    uint getColorId(string colorName) const;
    string getColorName(uint colorId) const;
    const vector<string>* getColorNames() const; //all names sorted by id

    //we say the graph "has" a color if there is at least one node with that color.
    //getColorId assumes that the graph has the passed color
    //if that assumption is not certain, it should be checked first:
    bool hasColor(string colorName) const;
    uint getNodeColor(uint node) const;
    uint numColors() const;
    
    //functions that are part of SANA's main loop. Defined here to allow inlining
    uint numNodesWithColor(uint colorId) const  {
        return nodeGroupsByColor[colorId].size();
    }
    uint getRandomNodeWithColor(uint colorId) const {
        return nodeGroupsByColor[colorId][randInt(0, nodeGroupsByColor[colorId].size()-1)];
    } 
    static const string DEFAULT_COLOR_NAME; 

    //color ids are internal to each graph (i.e., color i in G1 may not have the same name as color i in G2)
    //this maps the color ids of this graph to the color ids of the other graph with the
    //same name (or to the special value INVALID_COLOR_ID if the other graph has no color with that name)
    vector<uint> myColorIdsToOtherGraphColorIds(const Graph& other) const;
    static const uint INVALID_COLOR_ID;

    //check for internal consistency. good practice to keep it in an "assert"
    //after constructing or modifying a graph
    bool isWellDefined() const;
    void debugPrint() const; //print info about the internal state of graph

private:
    string name, filePath;

    //main data structures
    vector<array<uint, 2>> edgeList; //edges in no particular order, no repetitions
    vector<string> nodeNames;
    vector<vector<uint>> adjLists; //neighbors in no particular order, no repetitions
    Matrix<EDGE_T> adjMatrix;
    unordered_map<string, uint> nodeNameToIndexMap; //reverse of nodeNames

    //each edge has a weight in the range of type EDGE_T, but their sum may be beyond that range
    //double can contain the sum of EDGE_T values for any EDGE_T.
    double totalEdgeWeight;

    //data structures for the color system
    vector<uint> nodeColors; //node index to color index
    vector<string> colorNames; //color index to color name
    unordered_map<string, uint> colorNameToId; //color name to color index
    vector<vector<uint>> nodeGroupsByColor; //color index to list of node indices

    //useful when creating a derived graph with the same nodes and colors
    //creates the vector of pairs that the constructor needs
    vector<array<string, 2>> colorsAsNodeColorNamePairs() const;


    //for convenience and speed(maybe?)
    friend class SANA;
}; 

#endif /* GRAPH_H */