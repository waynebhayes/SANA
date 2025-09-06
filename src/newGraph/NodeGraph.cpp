#include "NodeGraph.hpp"
#include <queue>
#include <set>
#include <unordered_set>
#include <iterator>
#include <cmath>
#include <cassert>
#include <sstream>
#include <typeinfo> //typeid
#include <fcntl.h>
#include <unistd.h>
#include <sys/file.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <unistd.h>
#include <regex>

#ifdef MULTI_MPI
#include "Alignment.hpp" // alignment needed for pruning
#endif

using namespace std;

//static attributes
const string NodeGraph::DEFAULT_COLOR_NAME = "__default";
const unsigned NodeGraph::INVALID_COLOR_ID = 9999999;

NodeGraph::NodeGraph(const bool directed, const string& graphName, const string& optionalFilePath,
                     const vector<array<uint, 2>>& edgeList,
                     const vector<string>& optionalNodeNames,
                     const vector<EDGE_T>& optionalEdgeWeights,
                     const vector<array<string, 2>>& partialNodeColorPairs):
    directed(directed),
    name(graphName),
    filePath(optionalFilePath) {
    numEdges = 0;
    
    vector<string> nodeNames;
    uint numNodes;
    if (optionalNodeNames.empty()) numNodes = optionalNodeNames.size();
    else {
        //if names are not given, derive the number of nodes from the edge list
        //and give them dummy names
        if (edgeList.empty()) {
            numNodes = 0;
        } else {
            uint maxInd = 0;
            for (const auto& edge : edgeList)
                for (uint node : edge)
                    if (node > maxInd) maxInd = node;
            numNodes = maxInd+1;
        }
        nodeNames.reserve(numNodes);
        for (uint i = 0; i < numNodes; i++) nodeNames.push_back(to_string(i));
    }

    nodeNameToIndexMap.reserve(numNodes);
    for (uint i = 0; i < numNodes; i++) {
        if (nodeNameToIndexMap.count(nodeNames[i]))
            throw runtime_error("repeated node name "+nodeNames[i]+" passed to graph constructor");
        nodeNameToIndexMap.at(nodeNames.at(i)) = i;
    }

    bool uniformWeights = optionalEdgeWeights.size() == 0;
    assert(uniformWeights or optionalEdgeWeights.size() == edgeList.size());

    vector<MAP_TYPE> adjLists(numNodes, MAP_TYPE{});
    vector<MAP_TYPE> injLists(numNodes, MAP_TYPE{});
    auto nodeWeights = vector(numNodes, 0.0);
    totalGraphWeight = 0;
    for (uint i = 0; i < edgeList.size(); i++) {
        unsigned node1 = edgeList[i][0], node2 = edgeList[i][1];
        assert(node1 < numNodes and node2 < numNodes);
        EDGE_T weight;
        if (uniformWeights)
            weight = 1;
        else
            weight = optionalEdgeWeights[i];
        if (weight == 0)
            throw runtime_error("edges with weight 0 are not supported");
        if((directed && adjLists[node1].count(node2)) || (!directed && (adjLists[node1].count(node2) || adjLists[node2].count(node1))))
            throw runtime_error("Repeated edge ("+nodeNames[node1]+","+nodeNames[node2]+") in edge list passed to graph constructor; did you mean to specify \"-directed\"?");
        numEdges++;
        adjLists[node1].emplace(node2, weight);
        if (node1 == node2) continue;
        if(!directed)
            adjLists[node2].emplace(node1, weight);
        else
            injLists[node2].emplace(node1, weight);
        nodeWeights[node1] += weight;
        if(!directed && node2!=node1) {
            nodeWeights[node2] += weight;
        }
        totalGraphWeight += weight;
    }


    vector<unsigned> nodeColors(numNodes, INVALID_COLOR_ID);
    vector<string> nodeColorNames(numNodes, "");
    initColorDataStructs(partialNodeColorPairs, nodeColors, nodeColorNames);

    // We do not want fragmented data. Therefore, we will be ditching all of these temporary structures in favor
    // of a fresh start and hopefully linear data that is easy to cash.
    nodes.reserve(numNodes);
    for (uint i = 0; i < numNodes; i++) {
        nodes.emplace_back(i, nodeColors.at(i), nodeWeights.at(i), nodeNames.at(i),
               nodeColorNames.at(i), adjLists.at(i), injLists.at(i));
    }
    nodes.shrink_to_fit();
}

NodeGraph::NodeGraph(const BadGraph& badGraph):
    directed(badGraph.directed),
    name(badGraph.name),
    filePath(badGraph.name) {
    totalGraphWeight = badGraph.totalEdgeWeight;
    nodeNameToIndexMap = badGraph.nodeNameToIndexMap;
    colorNames = badGraph.colorNames;
    colorNameToId = badGraph.colorNameToId;
    nodeGroupsByColor = badGraph.nodeGroupsByColor;

    nodes.reserve(nodeNameToIndexMap.size());
    for (unsigned node = 0; node < nodeNameToIndexMap.size(); node++) {
        const unsigned colorNum = badGraph.nodeColors.at(node);
        MAP_TYPE tempMapAdj{};
        MAP_TYPE tempMapInj{};
        for (auto neighbor: badGraph.adjLists.at(node)) {
            tempMapAdj.emplace(neighbor, badGraph.adjMatrix.get(node, neighbor));
        }
        if (directed)
            for (auto neighbor: badGraph.injLists.at(node)) {
                tempMapInj.emplace(neighbor, badGraph.adjMatrix.get(neighbor, node));
            }

        nodes.emplace_back(node, colorNum, badGraph.totalWeight.at(node), badGraph.nodeNames.at(node),
               colorNames.at(colorNum), tempMapAdj, tempMapInj);
    }
}


void NodeGraph::initColorDataStructs(const vector<array<string, 2>>& partialNodeColorPairs,
                                     vector<unsigned> &nodeColors, vector<string> &nodeColorNames) {
    //data structures initialized here:
    nodeColors.clear();
    colorNames.clear();
    colorNameToId.clear();
    nodeGroupsByColor.clear();

    unordered_map<string, string> nodeNameToColorName;
    nodeNameToColorName.reserve(partialNodeColorPairs.size());
    unordered_set<string> colorSet;
    for (const auto& p : partialNodeColorPairs) {
        string nodeName = p[0], colorName = p[1];
        assert(colorName != DEFAULT_COLOR_NAME);
        if (nodeNameToColorName.count(nodeName))
            throw runtime_error("node name '"+nodeName+
                "' appears twice in node-color pairs while initializing Graph "+name);
        if (not nodeNameToIndexMap.count(nodeName))
            throw runtime_error("unknown node name '"+nodeName+
                "' in node-color pairs while initializing Graph "+name);
        nodeNameToColorName[nodeName] = colorName;
        colorSet.insert(colorName);
    }

    if (nodeNameToColorName.size() < nodeColorNames.size()) {
        colorNames.push_back(DEFAULT_COLOR_NAME); //default color gets index 0, if present
    }
    colorNames.insert(colorNames.end(), colorSet.begin(), colorSet.end());

    for (uint i = 0; i < colorNames.size(); i++) {
        colorNameToId[colorNames[i]] = i;
    }

    //nodes initialized with color id 0 (which corresponds to the default color, if any node has it)
    //nodes not in the passed map will keep it
    nodeColors = vector<uint> (nodeNameToIndexMap.size(), 0);
    for (auto& nodeToColor : nodeNameToColorName) {
        uint nodeId = nodeNameToIndexMap[nodeToColor.first];
        uint colorId = colorNameToId[nodeToColor.second];
        nodeColors[nodeId] = colorId;
    }

    nodeGroupsByColor = vector<vector<uint>> (colorNames.size(), vector<uint> (0));
    for (uint i = 0; i < nodeNameToIndexMap.size(); i++) {
        nodeGroupsByColor[nodeColors[i]].push_back(i);
    }
}