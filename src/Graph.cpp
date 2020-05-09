#include "Graph.hpp"
#include <queue>
#include <set>
#include <unordered_set>
#include <iterator>
#include <cmath>
#include <cassert>
#include <fstream>
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

using namespace std;

//static attributes
const string Graph::DEFAULT_COLOR_NAME = "__default"; 
const uint Graph::INVALID_COLOR_ID = 9999999;

Graph::Graph(const string& graphName, const string& optionalFilePath,
             const vector<array<uint, 2>>& edgeList,
             const vector<string>& optionalNodeNames,
             const vector<EDGE_T>& optionalEdgeWeights,
             const vector<array<string, 2>>& partialNodeColorPairs):
        name(graphName), filePath(optionalFilePath),
        edgeList(edgeList), nodeNames(optionalNodeNames) {

    uint numNodes;
    if (optionalNodeNames.size() != 0) numNodes = optionalNodeNames.size();
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
    for (uint i = 0; i < numNodes; i++) nodeNameToIndexMap[nodeNames[i]] = i;

    bool uniformWeights = optionalEdgeWeights.size() == 0;
    assert(uniformWeights or optionalEdgeWeights.size() == edgeList.size());

    adjLists.resize(numNodes);
    adjMatrix = Matrix<EDGE_T>(numNodes);
    totalEdgeWeight = 0;
    for (uint i = 0; i < edgeList.size(); i++) {
        uint node1 = edgeList[i][0], node2 = edgeList[i][1];
        assert(node1 < numNodes and node2 < numNodes);
        adjLists[node1].push_back(node2);
        if (node1 != node2) { //don't duplicate self-loop
            adjLists[node2].push_back(node1); 
        }
        EDGE_T weight;
        if (uniformWeights) weight = 1;
        else weight = optionalEdgeWeights[i];
        //throwing an error would be more appropriate, but choosing assert for effiency
        assert(weight != 0 and "edges with weight 0 are not supported");
        adjMatrix[node1][node2] = adjMatrix[node2][node1] = weight;
        totalEdgeWeight += weight;
    }
    initConnectedComponents();
    initColorDataStructs(partialNodeColorPairs);
}   

//predicate for sorting CCs
bool _isBigger(const vector<uint>& a, const vector<uint>& b) {return a.size()>b.size();}
void Graph::initConnectedComponents() {
    uint n = getNumNodes();
    vector<bool> nodesAreChecked(n, false);
    vector<uint> nodes;
    nodes.reserve(n);
    for (uint i = 0; i < n; ++i) nodes.push_back(i);
    while (not nodes.empty()) {
        uint startOfConnected = nodes.back();
        nodes.pop_back();
        if (nodesAreChecked[startOfConnected]) continue;
        vector<uint> connected;
        queue<uint> neighbors;
        neighbors.push(startOfConnected);
        while (not neighbors.empty()) {
            uint node = neighbors.front();
            neighbors.pop();
            if (nodesAreChecked[node]) continue;
            connected.push_back(node);
            nodesAreChecked[node] = true;
            for (uint nbr: adjLists[node]) {
                if (not nodesAreChecked[nbr]) neighbors.push(nbr);
            }
        }
        connectedComponents.push_back(connected);
    }
    sort(connectedComponents.begin(), connectedComponents.end(), _isBigger);
}

void Graph::initColorDataStructs(const vector<array<string, 2>>& partialNodeColorPairs) {
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
            throw runtime_error("node name '"+nodeName+"' appears twice in node-color pairs");
        if (not nodeNameToIndexMap.count(nodeName))
            throw runtime_error("unknown node name '"+nodeName+"' in node-color pairs");
        nodeNameToColorName[nodeName] = colorName;
        colorSet.insert(colorName);
    }

    if (nodeNameToColorName.size() < getNumNodes()) {
        colorNames.push_back(DEFAULT_COLOR_NAME); //default color gets index 0, if present
    }
    colorNames.insert(colorNames.end(), colorSet.begin(), colorSet.end());
    
    for (uint i = 0; i < colorNames.size(); i++) {
        colorNameToId[colorNames[i]] = i;
    }

    //nodes initialized with color id 0 (which corresponds to the default color, if any node has it)
    //nodes not in the passed map will keep it
    nodeColors = vector<uint> (getNumNodes(), 0);
    for (auto& nodeToColor : nodeNameToColorName) {
        uint nodeId = nodeNameToIndexMap[nodeToColor.first];
        uint colorId = colorNameToId[nodeToColor.second];
        nodeColors[nodeId] = colorId;
    }

    nodeGroupsByColor = vector<vector<uint>> (colorNames.size(), vector<uint> (0));
    for (uint i = 0; i < getNumNodes(); i++) {
        nodeGroupsByColor[nodeColors[i]].push_back(i);
    }    
}

Graph Graph::nodeInducedSubgraph(const vector<uint>& nodes) const {
    uint oldN = getNumNodes();
    uint newN = nodes.size();
    const uint INVALID_NEW_INDEX = newN; //arbitrary value outside range 0..newN-1
    vector<uint> oldToNewIndex(oldN, INVALID_NEW_INDEX);
    for (uint i = 0; i < newN; i++) oldToNewIndex[nodes[i]] = i;
    
    vector<array<uint, 2>> newEdgeList;
    vector<EDGE_T> newEdgeWeights;
    for (const auto& edge: edgeList) {
        uint newNode1 = oldToNewIndex[edge[0]];
        uint newNode2 = oldToNewIndex[edge[1]];
        if (newNode1 != INVALID_NEW_INDEX and newNode2 != INVALID_NEW_INDEX) {
            newEdgeList.push_back({newNode1, newNode2});
            newEdgeWeights.push_back(edgeWeight(edge[0], edge[1]));
        }
    }

    vector<string> newNodeNames;
    newNodeNames.reserve(newN);
    for (uint node : nodes) newNodeNames.push_back(nodeNames[node]);
    
    vector<array<string, 2>> newNodeNameToColorName;
    bool hasDefColor = colorNames[0] == DEFAULT_COLOR_NAME; //if present, the default color is at index 0
    for (const uint node : nodes) {
        if (hasDefColor and nodeColors[node] == 0) continue;
        newNodeNameToColorName.push_back({nodeNames[node], colorNames[nodeColors[node]]});
    }
    return Graph(name+"_subgraph", "", newEdgeList, newNodeNames, newEdgeWeights, newNodeNameToColorName);
}

Graph Graph::randomNodeInducedSubgraph(uint numNodes) const {
    if (numNodes > getNumNodes()) cerr << "the subgraph cannot have more nodes" << endl;
    vector<uint> v;
    v.reserve(getNumNodes());
    for (uint i = 0; i < getNumNodes(); i++) v.push_back(i);
    randomShuffle(v);
    v.resize(numNodes);
    return nodeInducedSubgraph(v);
}

Graph Graph::shuffledGraph(vector<uint> &newToOldMap) const {
    newToOldMap.clear(); //this is a return argument by reference
    newToOldMap.reserve(getNumNodes());
    for (uint i = 0; i < getNumNodes(); i++) newToOldMap.push_back(i);
    randomShuffle(newToOldMap);
    return nodeInducedSubgraph(newToOldMap);
}

//this function is not tested a lot, use with care?
Graph Graph::graphPower(uint power) const {
    if (power == 0) throw runtime_error("graphs don't have 0 powers");
    if (power == 1) cerr<<"Warning: first power of a graph is just the graph itself"<<endl;
    uint n = getNumNodes();
    SparseMatrix<uint> oriAdjMat(n);
    for (const auto& edge : edgeList) {
        uint node1 = edge[0], node2 = edge[1];
        oriAdjMat[node1][node2] = oriAdjMat[node2][node1] = 1;
    }
    SparseMatrix<uint> newAdjMat = oriAdjMat;

    for(uint i = 1; i < power; i++) {
        //the number of multiplications can be reduced from O(power) to O(log power) 
        //by multiplying the new matrix with itself instead of the original
        //I left this optimization unimplemented since I don't think this function is used much -Nil
        SparseMatrix<uint> newAdjMat = newAdjMat.multiply(oriAdjMat);
    }
    vector<array<uint, 2>> newEdgeList;
    //there is probably an O(m) way to do this instead of O(n^2) -Nil
    for(uint i=0;i<n;i++){
        for(uint j=i;j<n;j++){
            if(newAdjMat.get(i,j) > 0){
                newEdgeList.push_back({i,j});
            }
        }
    }
    return Graph(name+"_power_"+to_string(power), "", newEdgeList, 
                 nodeNames, {}, colorsAsNodeColorNamePairs()); //unweighted result
}

Graph Graph::graphWithAddedRandomEdges(double addedEdgesProportion) const {
	throw runtime_error("didn't reimplement this yet because I suspect this feature is not used. Sorry -Nil");
}
Graph Graph::graphWithRemovedRandomEdges(double removedEdgesProportion) const {
	throw runtime_error("didn't reimplement this yet because I suspect this feature is not used. Sorry -Nil");
}
Graph Graph::graphWithRewiredRandomEdges(double rewiredEdgesProportion) const {
	throw runtime_error("didn't reimplement this yet because I suspect this feature is not used. Sorry -Nil");
}

Graph Graph::graphIntersection(const Graph& other, const vector<uint>& thisToOtherNodeMap) const {
    vector<array<uint, 2>> newEdgeList;
    for (const auto& edge : edgeList) {
        uint on1 = thisToOtherNodeMap[edge[0]], on2 = thisToOtherNodeMap[edge[1]];
        if (other.hasEdge(on1, on2)) newEdgeList.push_back(edge);
    }
    return Graph(name+"_intersection_"+other.name, "", newEdgeList,
                 nodeNames, {}, colorsAsNodeColorNamePairs()); //unweighted result
}

Graph Graph::subtractGraph(const Graph& other, const vector<uint>& otherToThisNodeMap) {
    if (otherToThisNodeMap.size() != other.getNumNodes())
        throw runtime_error("otherToThisNodeMap size ("+to_string(otherToThisNodeMap.size())+
                            ") != other's number of nodes ("+to_string(other.getNumNodes())+")");
    //subtract the weights from the edges in 'other' from the adjacency matrix
    uint numRemovedEdges = 0;
    for (const auto& otherEdge : other.edgeList) {
        uint on1 = otherEdge[0], on2 = otherEdge[1];
        EDGE_T ow = other.edgeWeight(on1, on2);
        assert(ow > 0);
        uint tn1 = otherToThisNodeMap.at(on1), tn2 = otherToThisNodeMap.at(on2);
        assert(tn1 < getNumNodes() and tn2 < getNumNodes());
#ifndef BOOL_EDGE_T //general case:
        adjMatrix[tn1][tn2] -= ow;
        assert(adjMatrix[tn1][tn2] >= 0);            
#else //special case is needed because '-=' does not work on bools
        assert(adjMatrix[tn1][tn2] == 1);
        adjMatrix[tn1][tn2] = 0; //0 comes from 1-1
#endif
        if (adjMatrix[tn1][tn2] == 0) numRemovedEdges++;        
    }
    //keep the edges in this graph's edge list that have positive weight
    vector<array<uint, 2>> newEdgeList;
    vector<EDGE_T> newEdgeWeights;
    uint newNumEdges = getNumEdges() - numRemovedEdges;
    newEdgeList.reserve(newNumEdges);
    newEdgeWeights.reserve(newNumEdges);
    for (const auto& thisEdge : edgeList) {
        uint tn1 = thisEdge[0], tn2 = thisEdge[1];
        EDGE_T newWeight = adjMatrix.get(tn1, tn2);
        assert(newWeight >= 0);
        if (newWeight > 0) {
            newEdgeList.push_back({tn1, tn2});
            newEdgeWeights.push_back(newWeight);
        }
    }

    //restore the original adjacency matrix of this graph
    for (const auto& otherEdge : other.edgeList) {
        uint on1 = otherEdge[0], on2 = otherEdge[1];
        uint tn1 = otherToThisNodeMap.at(on1), tn2 = otherToThisNodeMap.at(on2);
#ifndef BOOL_EDGE_T //general case:
        EDGE_T ow = other.edgeWeight(on1, on2);
        adjMatrix[tn1][tn2] += ow;
#else //special case for bools
        assert(adjMatrix[tn1][tn2] == 0);
        adjMatrix[tn1][tn2] = 1; //1 comes from 0+1
#endif
    }

    return Graph(name, filePath, newEdgeList, nodeNames, 
                 newEdgeWeights, colorsAsNodeColorNamePairs());
    //note: the new graph takes the same name/path as the original graph. this may be confusing
    //since it's no longer the same graph, but I think SANA relies on this -Nil
}

uint Graph::randomNode() const {
    if (getNumNodes() == 0) throw runtime_error("no nodes");
    return randInt(0, getNumNodes()-1);
}

uint Graph::maxDegree() const {
    if (adjLists.size() == 0) return 0;
    uint res = adjLists[0].size();
    for (uint i = 1; i < getNumNodes(); i++)
        if (adjLists[i].size() > res) res = adjLists[i].size();
    return res;    
}

vector<uint> Graph::degreeDistribution() const {
    vector<uint> res(maxDegree()+1);
    for (uint i = 0; i < getNumNodes(); i++)
        res[adjLists[i].size()]++;
    return res;
}

uint Graph::numEdgesInNodeInducedSubgraph(const vector<uint>& subgraphNodes) const {
    unordered_set<uint> nodeSet(subgraphNodes.begin(), subgraphNodes.end());
    uint count = 0;
    for (uint node1 : subgraphNodes)
        for (uint node2 : adjLists[node1])
            count += nodeSet.count(node2);
    return count/2;
}

vector<uint> Graph::numEdgesAroundByLayers(uint node, uint maxDist) const {
    uint n = getNumNodes();
    vector<uint> distances(n, n);
    vector<bool> visited(n, false);
    distances[node] = 0;
    queue<uint> Q;
    Q.push(node);
    vector<uint> result(maxDist, 0);
    while (not Q.empty()) {
        uint u = Q.front();
        Q.pop();
        uint dist = distances[u];
        if (dist == maxDist) break;
        for (uint i = 0; i < adjLists[u].size(); i++) {
            uint v = adjLists[u][i];
            if (not visited[v]) result[dist]++;
            if (distances[v] < n) continue;
            distances[v] = dist+1;
            Q.push(v);
        }
        visited[u] = true;
    }
    return result;
}

vector<uint> Graph::numNodesAroundByLayers(uint node, uint maxDist) const {
    uint n = getNumNodes();
    vector<uint> distances(n, n);
    distances[node] = 0;
    queue<uint> Q;
    Q.push(node);
    while (not Q.empty()) {
        uint u = Q.front();
        Q.pop();
        uint dist = distances[u];
        if (dist == maxDist) break;
        for (uint i = 0; i < adjLists[u].size(); i++) {
            uint v = adjLists[u][i];
            if (distances[v] < n) continue;
            distances[v] = dist+1;
            Q.push(v);
        }
    }
    vector<uint> result(maxDist, 0);
    uint total = 0;
    for (uint i = 0; i < n; i++)
        if (distances[i] < n and distances[i] > 0) {
        result[distances[i]-1]++; 
        total++;
    }
    assert(total == nodesAround(node, maxDist).size());
    return result;
}

vector<uint> Graph::nodesAround(uint node, uint maxDist) const {
    uint n = getNumNodes();
    vector<uint> distances(n, n);
    distances[node] = 0;
    queue<uint> Q;
    Q.push(node);
    while (not Q.empty()) {
        uint u = Q.front();
        Q.pop();
        uint dist = distances[u];
        if (dist == maxDist) break;
        for (uint i = 0; i < adjLists[u].size(); i++) {
            uint v = adjLists[u][i];
            if (distances[v] < n) continue;
            distances[v] = dist+1;
            Q.push(v);
        }
    }
    vector<uint> result;
    for (uint i = 0; i < n; i++) {
        if (distances[i] < n and distances[i] >= 0) result.push_back(i);
    }
    return result;
}

bool Graph::hasSameNodeNamesAs(const Graph& other) const {
    if (getNumNodes() != other.getNumNodes()) return false;
    for (const auto& kv : nodeNameToIndexMap) {
        string name = kv.first;
        if (not other.nodeNameToIndexMap.count(name)) return false;
    }
    return true;
}
vector<string> Graph::commonNodeNames(const Graph& other) const {
    vector<string> res;
    res.reserve(min(getNumNodes(), other.getNumNodes()));
    for (const auto& kv : nodeNameToIndexMap) {
        string name = kv.first;
        if (other.nodeNameToIndexMap.count(name)) res.push_back(name);
    }
    res.shrink_to_fit();
    return res;
}

// NODE COLOR SYSTEM

uint Graph::getNodeColor(uint node) const { return nodeColors[node]; }
uint Graph::getColorId(string name) const { 
    if (!colorNameToId.count(name)) throw runtime_error("color name not found");
    return colorNameToId.at(name);
}
string Graph::getColorName(uint colorId) const { return colorNames.at(colorId); }
const vector<string>* Graph::getColorNames() const { return &colorNames; }
bool Graph::hasColor(string name) const { return colorNameToId.count(name); }
uint Graph::numNodesWithColor(uint colorId) const {
    return nodeGroupsByColor[colorId].size();
}
uint Graph::numColors() const { return colorNames.size(); }
uint Graph::getRandomNodeWithColor(uint colorId) const {
    return nodeGroupsByColor[colorId][randInt(0, nodeGroupsByColor[colorId].size()-1)];
}
vector<uint> Graph::myColorIdsToOtherGraphColorIds(const Graph& other) const {
    vector<uint> res;
    res.reserve(numColors());
    for (const string& colorName: colorNames) {
        if (!other.hasColor(colorName)) {
            res.push_back(INVALID_COLOR_ID);
        } else {
            res.push_back(other.getColorId(colorName));
        }
    }
    return res;
}

vector<array<string, 2>> Graph::colorsAsNodeColorNamePairs() const {
    vector<array<string, 2>> res;
    bool hasDefColor = colorNames[0] == DEFAULT_COLOR_NAME;
    res.reserve(getNumNodes() - (hasDefColor ? numNodesWithColor(0) : 0));
    for (uint i = (hasDefColor ? 1 : 0); i < numColors(); i++) {
        string colName = colorNames[i];
        for (uint node : nodeGroupsByColor[i])
            res.push_back({nodeNames[node], colName});
    }
    return res;
}

void Graph::debugPrint() const {
    size_t MAX_LEN = 10;        
    cerr<<"DEBUG PRINT "<<name<<endl;
    cerr<<"filePath: "<<filePath<<endl;
    cerr<<"adjLists size: "<<adjLists.size()<<endl;
    cerr<<"neighbor lists sizes: ";
    for(uint i = 0; i < min(adjLists.size(), MAX_LEN); i++) cerr<<adjLists[i].size()<<' ';
    if (MAX_LEN < adjLists.size()) cerr<<"..."; cerr<<endl;
    cerr<<"adjLists[0]: ";
    for (uint i = 0; i < min(adjLists[0].size(), MAX_LEN); i++) cerr<<adjLists[0][i]<<' ';
    if (MAX_LEN < adjLists[0].size()) cerr<<"..."; cerr<<endl;

    cerr<<"adjMatrix size: "<<adjMatrix.size()<<endl;

    cerr<<"nodeNames (size "<<nodeNames.size()<<"): ";
    for (uint i = 0; i < min(nodeNames.size(), MAX_LEN); i++) cerr<<nodeNames[i]<<' ';
    if (MAX_LEN < nodeNames.size()) cerr<<"..."; cerr<<endl;

    cerr<<"nodeColors (size "<<nodeColors.size()<<"): ";
    for (uint i = 0; i < min(nodeColors.size(), MAX_LEN); i++) cerr<<nodeColors[i]<<' ';
    if (MAX_LEN < nodeColors.size()) cerr<<"..."; cerr<<endl;

    cerr<<"nodeNameToIndexMap (size "<<nodeNameToIndexMap.size()<<"): ";
    uint kvi = 0;
    for (auto kv:nodeNameToIndexMap) {
        cerr<<kv.first<<":"<<kv.second<<' ';
        if (kvi++ == MAX_LEN and nodeNameToIndexMap.size() > MAX_LEN) { cerr<<"..."; break; }
    }
    cerr<<endl;

    cerr<<"edge list (size "<<edgeList.size()<<"): ";
    for (uint i = 0; i < min(edgeList.size(), MAX_LEN); i++) cerr<<'{'<<edgeList[i][0]<<", "<<edgeList[i][1]<<"} ";
    if (MAX_LEN < edgeList.size()) cerr<<"..."; cerr<<endl;

    cerr<<"totalEdgeWeight: "<<totalEdgeWeight<<endl;

    cerr<<"connectedComponents size: "<<connectedComponents.size()<<endl;
    cerr<<"CC sizes: ";
    for(uint i = 0; i < min(connectedComponents.size(), MAX_LEN); i++) cerr<<connectedComponents[i].size()<<' ';
    if (MAX_LEN < connectedComponents.size()) cerr<<"..."; cerr<<endl;
    cerr<<"connectedComponents[0]: ";
    for (uint i = 0; i < min(connectedComponents[0].size(), MAX_LEN); i++) cerr<<connectedComponents[0][i]<<' ';
    if (MAX_LEN < connectedComponents[0].size()) cerr<<"..."; cerr<<endl;

    cerr<<"colorNames (size "<<colorNames.size()<<"): ";
    for (uint i = 0; i < min(colorNames.size(), MAX_LEN); i++) cerr<<colorNames[i]<<' ';
    if (MAX_LEN < colorNames.size()) cerr<<"..."; cerr<<endl;

    cerr<<"colorNameToId (size "<<colorNameToId.size()<<"): ";
    kvi = 0;
    for (auto kv:colorNameToId) {
        cerr<<kv.first<<": "<<kv.second<<' ';
        if (kvi++ == MAX_LEN and colorNameToId.size() > MAX_LEN) { cerr<<"..."; break; }
    }
    cerr<<endl;        

    cerr<<"nodeGroupsByColor size: "<<nodeGroupsByColor.size()<<endl;
    cerr<<"color group sizes: ";
    for(uint i = 0; i < min(nodeGroupsByColor.size(), MAX_LEN); i++) cerr<<nodeGroupsByColor[i].size()<<' ';
    if (MAX_LEN < nodeGroupsByColor.size()) cerr<<"..."; cerr<<endl;
    cerr<<"nodeGroupsByColor[0]: ";
    for (uint i = 0; i < min(nodeGroupsByColor[0].size(), MAX_LEN); i++) cerr<<nodeGroupsByColor[0][i]<<' ';
    if (MAX_LEN < nodeGroupsByColor[0].size()) cerr<<"..."; cerr<<endl;
    cerr<<endl;
}

bool Graph::isWellDefined() const {
    ostringstream ss;
    //data structures have the right size
    uint n = adjLists.size();
    if (adjMatrix.size() != n)
        ss<<"adjLists has size "<<n<<" but adjMatrix has size "<<adjMatrix.size()<<endl;
    if (nodeNames.size() != n)
        ss<<"adjLists has size "<<n<<" but nodeNames has size "<<nodeNames.size()<<endl;
    if (nodeNameToIndexMap.size() != n)
        ss<<"adjLists has size "<<n<<" but nodeNameToIndexMap has size "<<nodeNameToIndexMap.size()<<endl;
    if (nodeColors.size() != n)
        ss<<"adjLists has size "<<n<<" but nodeColors has size "<<nodeNameToIndexMap.size()<<endl;
    uint k = colorNames.size();
    if (colorNameToId.size() != k)
        ss<<"colorNames has size "<<k<<" but colorNameToId has size "<<colorNameToId.size()<<endl;
    if (nodeGroupsByColor.size() != k)
        ss<<"colorNames has size "<<k<<" but nodeGroupsByColor has size "<<nodeGroupsByColor.size()<<endl;

    //adjMatrix is symmetric and the sum of edges weights equals totalEdgeWeight
    uint numEdgesInAdjMat = 0;
    double adjMatSum = 0;
    for (uint i = 0; i < n; i++) {
        for (uint j = 0; j <= i; j++) {
            if (adjMatrix.get(i, j) != adjMatrix.get(j, i))
                ss<<"adjMatrix is not symmetric at ("<<i<<", "<<j<<")"<<endl;
            if (hasEdge(i,j)) {
                numEdgesInAdjMat++;
                adjMatSum += edgeWeight(i,j);
            }
        }
    }
    if (adjMatSum != totalEdgeWeight)
        ss<<"totalEdgeWeight attribute is "<<totalEdgeWeight<<" but the edges in adjMatrix add up to "<<adjMatSum<<endl;

    //edgeList: all entries appear in adjMatrix, are not repeated, and every entry in adj matrix is in edge list
    if (edgeList.size() != numEdgesInAdjMat)
        ss<<"edgeList has "<<edgeList.size()<<" edges but adjMatrix has "<<numEdgesInAdjMat<<endl;

    vector<unordered_set<uint>> nbrSetsInEdgeList(n);
    for (uint i = 0; i < n; i++)
        nbrSetsInEdgeList[i].reserve(adjLists[i].size()); //to avoid hash table resizings

    for (const auto& edge : edgeList) {
        if (edge[0] < 0 or edge[0] >= n or edge[1] < 0 or edge[1] >= n) 
            ss<<"edge {"<<edge[0]<<", "<<edge[1]<<"} in edgeList out of range"<<endl;
        else {
            if (!hasEdge(edge[0], edge[1]))
                ss<<"edge {"<<edge[0]<<", "<<edge[1]<<"} in edgeList missing in adjMatrix"<<endl;
            uint nodeMin = min(edge[0], edge[1]), nodeMax = max(edge[0], edge[1]);
            //nodeMin and nodeMax to impose a canonical order on edges
            if (nbrSetsInEdgeList[nodeMin].count(nodeMax))
                ss<<"edge {"<<edge[0]<<", "<<edge[1]<<"} repeated in edgeList"<<endl;
            nbrSetsInEdgeList[nodeMin].insert(nodeMax);
        }
    }

    //adjLists: all entries appear in adjMatrix, are not repeated, and every entry in adj matrix is in adj lists
    uint doubleCountedNumEdges = 0; //once from each endpoint
    for (uint i = 0; i < n; i++) {
        uint numNbrs = adjLists[i].size();
        if (numNbrs > n)
            ss<<"adjLists["<<i<<"] has size "<<numNbrs<<" but adjLists has size "<<n<<endl;
        doubleCountedNumEdges += adjLists[i].size();
        if (hasEdge(i,i)) doubleCountedNumEdges++;
    }
    if (2*edgeList.size() != doubleCountedNumEdges)
        ss<<"edgeList has "<<2*edgeList.size()<<"edge endpoints but adjLists has "<<doubleCountedNumEdges<<endl;
        
    for (uint i = 0; i < n; i++) {
        unordered_set<uint> seenNbrs;
        seenNbrs.reserve(adjLists[i].size());
        for (uint nbr : adjLists[i]) {
            if (nbr < 0 or nbr >= n)
                ss<<"adjLists["<<i<<"] contains node "<<nbr<<" out of range"<<endl;
            else {
                if (!hasEdge(i,nbr))
                ss<<"adjLists["<<i<<"] contains "<<nbr<<" which is missing in adjMatrix"<<endl;
                if (seenNbrs.count(nbr))
                    ss<<"adjLists["<<i<<"] contains repeated node "<<nbr<<endl;
                seenNbrs.insert(nbr);
            }
        }
    }

    //all names are unique
    unordered_set<string> seenNames;
    for (const string& name : nodeNames) {
        if (seenNames.count(name))
            ss<<"repeated node name "<<name<<endl;
        seenNames.insert(name);
    }

    //nodeNameToIndexMap is the inverse of nodeNames
    for (uint i = 0; i < n; i++) {
        if (!nodeNameToIndexMap.count(nodeNames[i]))
            ss<<"nodeNameToIndexMap missing node name "<<nodeNames[i]<<endl;
        else if (nodeNameToIndexMap.at(nodeNames[i]) != i)
            ss<<"nodeNameToIndexMap is not the inverse of nodeNames for "<<i<<endl;
    }

    //connected components contain every node exactly once
    //(we do not check that CCs are connected and maximal)
    unordered_set<uint> nodesInCCs;
    nodesInCCs.reserve(n);
    for (const auto& cc : connectedComponents) {
        for (uint node : cc) {
            if (node < 0 or node >= n)
                ss<<"CCs contain node "<<node<<" but adjLists has size "<<n<<endl;
            if (nodesInCCs.count(node))
                ss<<"CCs contain repeated node "<<node<<endl;
            nodesInCCs.insert(node);
        }
    }
    if (nodesInCCs.size() != n)
        ss<<"CCs contain "<<nodesInCCs.size()<<"nodes in total but adjLists has size "<<n<<endl;

    //nodeColors contains valid color indices
    for (uint i = 0; i < nodeColors.size(); i++)
        if (nodeColors[i] < 0 or nodeColors[i] >= colorNames.size())
            ss<<"node "<<i<<" has color "<<nodeColors[i]<<" but there are "<<colorNames.size()<<" colors"<<endl;

    //colorNames are unique
    unordered_set<string> seenColors;
    seenColors.reserve(colorNames.size());
    for (const string& color : colorNames) {
        if (seenColors.count(color))
            ss<<"repeated color name "<<color;
        seenColors.insert(color);
    }

    //colorNameToId is the inverse map of colorNames
    for (uint i = 0; i < colorNames.size(); i++) {
        if (!colorNameToId.count(colorNames[i]))
            ss<<"colorNameToId missing color name "<<colorNames[i]<<endl;
        else if (colorNameToId.at(colorNames[i]) != i)
            ss<<"colorNameToId is not the inverse of colorNames for "<<i<<endl;
    }

    //nodeGroupsByColor contains every node exactly once
    //and the node colors match the nodeColors struct
    //and no color group is empty
    unordered_set<uint> nodesInColorGroups;
    nodesInColorGroups.reserve(n);
    for (uint c = 0; c < nodeGroupsByColor.size(); c++) {
        if (nodeGroupsByColor[c].size() == 0)
            ss<<"Color "<<c<<" does not have any nodes"<<endl;
        for (uint node : nodeGroupsByColor[c]) {
            if (nodesInColorGroups.count(node))
                ss<<"nodeGroupsByColor contains repeated node "<<node<<endl;
            else if (nodeColors[node] != c)
                ss<<"node "<<node<<" in color group "<<c<<" but nodeColors["<<node<<"]="<<nodeColors[node]<<endl;
            nodesInColorGroups.insert(node);
        }
    }
    if (nodesInColorGroups.size() != n)
        ss<<"nodeGroupsByColor contain "<<nodesInColorGroups.size()<<"nodes in total but adjLists has size "<<n<<endl;

    //wrap up
    string errMsg = ss.str();
    if (errMsg.size() != 0) {
        cerr<<"Graph "<<name<<" is not well defined. Has the following issues: "<<endl;
        uint maxMsgLen = 2000;
        if (errMsg.size() <= maxMsgLen) cerr<<errMsg<<endl;
        else cerr<<errMsg.substr(0, maxMsgLen)<<" ..."<<endl;
        return false;
    }
    return true;
}
