#include "Graph.hpp"
#include <queue>
#include <iterator>
#include <cassert>
#include <sstream>
#include <fcntl.h>
#include <regex>
#include <set>

#ifdef LINUX
#include <execinfo.h>
#endif

#ifdef MULTI_MPI
#include "Alignment.hpp" // alignment needed for pruning
#endif

using namespace std;

//static attributes
const string Graph::DEFAULT_COLOR_NAME = "__default";
const unsigned Graph::INVALID_COLOR_ID = 9999999;

Graph::Graph(const bool directed, const string& graphName, const string& optionalFilePath,
                     const vector<array<unsigned, 2>>& edgeList,
                     const vector<string>& optionalNodeNames,
                     const vector<EDGE_T>& optionalEdgeWeights,
                     const vector<array<string, 2>>& partialNodeColorPairs):
    directed(directed),
    numEdges(edgeList.size()),
    name(graphName),
    filePath(optionalFilePath),
    edgeList(edgeList) {

    vector<string> nodeNames;
    const vector<string> *nodeNamesPtr;
    unsigned numNodes;
    if (!optionalNodeNames.empty()) {
        numNodes = optionalNodeNames.size();
        nodeNamesPtr = &optionalNodeNames;
    }
    else {
        //if names are not given, derive the number of nodes from the edge list
        //and give them dummy names
        if (edgeList.empty()) {
            numNodes = 0;
        } else {
            unsigned maxInd = 0;
            for (const auto& edge : edgeList)
                for (const unsigned node : edge)
                    if (node > maxInd) maxInd = node;
            numNodes = maxInd+1;
        }
        nodeNames.reserve(numNodes);
        for (unsigned i = 0; i < numNodes; i++) nodeNames.push_back(to_string(i));
        nodeNamesPtr = &nodeNames;
    }

    nodeNameToIndexMap.reserve(numNodes);
    for (unsigned i = 0; i < numNodes; i++) {
        if (nodeNameToIndexMap.count((*nodeNamesPtr)[i]))
            throw runtime_error("repeated node name "+(*nodeNamesPtr)[i]+" passed to graph constructor");
        nodeNameToIndexMap[nodeNamesPtr->at(i)] = i;
    }

    const bool uniformWeights = optionalEdgeWeights.size() == 0;
    assert(uniformWeights or optionalEdgeWeights.size() == edgeList.size());

    vector<MAP_TYPE> adjLists(numNodes, MAP_TYPE{});
    vector<MAP_TYPE> injLists(numNodes, MAP_TYPE{});
    auto nodeWeights = vector<double>(numNodes, 0.0);
    totalGraphWeight = 0;
    uint64_t dummyNumEdges = 0;
    for (unsigned i = 0; i < edgeList.size(); i++) {
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
            throw runtime_error("Repeated edge ("+(*nodeNamesPtr)[node1]+","+(*nodeNamesPtr)[node2]+") in edge list passed to graph constructor; did you mean to specify \"-directed\"?");
        adjLists[node1].emplace(node2, weight);
        nodeWeights[node1] += weight;
        totalGraphWeight += weight;
        ++dummyNumEdges;
        if (node1 == node2) continue; // That is, self-loops do not show up in injList!
        if(!directed) {
            adjLists[node2].emplace(node1, weight);
            nodeWeights[node2] += weight;
        }
        else
            injLists[node2].emplace(node1, weight);
    }
    assert(numEdges == dummyNumEdges);

    vector<unsigned> nodeColors(numNodes, INVALID_COLOR_ID);
    initColorDataStructs(partialNodeColorPairs, nodeColors);

    // We do not want fragmented data. Therefore, we will be ditching all of these temporary structures in favor
    // of a fresh start and hopefully linear data that is easy to cash.
    nodes.reserve(numNodes);
    for (unsigned i = 0; i < numNodes; i++) {
        nodes.emplace_back(i, nodeColors.at(i), nodeWeights.at(i), nodeNamesPtr->at(i),
                  colorNames.at(nodeColors.at(i)), adjLists.at(i), injLists.at(i));
    }
    nodes.shrink_to_fit();
}

void Graph::reinitializeColors(const vector<array<string, 2>>& partialNodeColorPairs) {
    const unsigned numNodes = nodes.size();
    vector<unsigned> nodeColors(numNodes, INVALID_COLOR_ID);
    initColorDataStructs(partialNodeColorPairs, nodeColors);

    vector<Node> newNodes;
    newNodes.reserve(numNodes);
    for (unsigned i = 0; i < numNodes; i++) {
        Node &oldNode = nodes.at(i);
        newNodes.emplace_back(oldNode, nodeColors.at(i), colorNames.at(nodeColors.at(i)));
    }
    newNodes.shrink_to_fit();

    swap(nodes, newNodes);
}


void Graph::initColorDataStructs(const vector<array<string, 2>>& partialNodeColorPairs, vector<unsigned> &nodeColors) {
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

    if (nodeNameToColorName.size() < nodeNameToIndexMap.size()) {
        colorNames.push_back(DEFAULT_COLOR_NAME); //default color gets index 0, if present
    }
    colorNames.insert(colorNames.end(), colorSet.begin(), colorSet.end());

    for (unsigned i = 0; i < colorNames.size(); i++) {
        colorNameToId[colorNames[i]] = i;
    }

    //nodes initialized with color id 0 (which corresponds to the default color, if any node has it)
    //nodes not in the passed map will keep it
    nodeColors = vector<unsigned> (nodeNameToIndexMap.size(), 0);
    for (auto& nodeToColor : nodeNameToColorName) {
        unsigned nodeId = nodeNameToIndexMap[nodeToColor.first];
        unsigned colorId = colorNameToId[nodeToColor.second];
        nodeColors[nodeId] = colorId;
    }

    nodeGroupsByColor = vector<vector<unsigned>> (colorNames.size(), vector<unsigned> (0));
    for (unsigned i = 0; i < nodeNameToIndexMap.size(); i++) {
        nodeGroupsByColor[nodeColors[i]].push_back(i);
    }
}

Graph Graph::nodeInducedSubgraph(const vector<unsigned>& nodes) const {
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
            newEdgeWeights.push_back(getEdgeWeight(edge[0], edge[1]));
        }
    }

    vector<string> newNodeNames;
    newNodeNames.reserve(newN);
    vector<array<string, 2>> newNodeNameToColorName;
    bool hasDefColor = colorNames[0] == DEFAULT_COLOR_NAME; //if present, the default color is at index 0
    for (const Node& node : this->nodes) {
        newNodeNames.push_back(node.nodeName);
        if (hasDefColor and node.colorID == 0) continue;
        newNodeNameToColorName.push_back({node.nodeName, node.colorName});
    }
    return {directed, name+"_subgraph", "", newEdgeList, newNodeNames, newEdgeWeights, newNodeNameToColorName};

}

Graph Graph::randomNodeInducedSubgraph(unsigned numNodes) const {
    if (numNodes > getNumNodes()) {
        throw runtime_error("A subgraph cannot have more nodes than the original graph.");
    }
    vector<unsigned> v;
    v.reserve(numNodes);
    for (unsigned i = 0; i < getNumNodes(); i++) v.push_back(i);
    randomShuffle(v);
    v.resize(numNodes);
    return nodeInducedSubgraph(v);
}
Graph Graph::shuffledGraph(vector<unsigned>& newToOldMap) const {
    newToOldMap.clear(); //this is a return argument by reference
    newToOldMap.reserve(getNumNodes());
    for (uint i = 0; i < getNumNodes(); i++) newToOldMap.push_back(i);
    randomShuffle(newToOldMap);
    return nodeInducedSubgraph(newToOldMap);
}
Graph Graph::graphPower(unsigned power) const {
    cerr << "Sorry, but \"Graph::graphPower\" is currently unimplemented. Please contact Marcus if you absolutely require it." <<endl
    << "This information will help inform us that this functionality should not be retired." << endl;
    throw runtime_error("Function not implemented!");
}
Graph Graph::graphWithAddedRandomEdges(double addedEdgesProportion) const {
    cerr << "Sorry, but \"Graph::graphWithAddedRandomEdges\" is currently unimplemented. Please contact Marcus if you absolutely require it." <<endl
    << "This information will help inform us that this functionality should not be retired." << endl;
    throw runtime_error("Function not implemented!");
}
Graph Graph::graphWithRemovedRandomEdges(double removedEdgesProportion) const {
    cerr << "Sorry, but \"Graph::graphWithRemovedRandomEdges\" is currently unimplemented. Please contact Marcus if you absolutely require it." <<endl
    << "This information will help inform us that this functionality should not be retired." << endl;
    throw runtime_error("Function not implemented!");
}
Graph Graph::graphWithRewiredRandomEdges(double rewiredEdgesProportion) const {
    cerr << "Sorry, but \"Graph::graphWithRewiredRandomEdges\" is currently unimplemented. Please contact Marcus if you absolutely require it." <<endl
    << "This information will help inform us that this functionality should not be retired." << endl;
    throw runtime_error("Function not implemented!");
}
Graph Graph::graphIntersection(const Graph& other, const vector<unsigned>& thisToOtherNodeMap) const {
    vector<array<uint, 2>> newEdgeList;
    for (const auto& edge : edgeList) {
        uint on1 = thisToOtherNodeMap[edge[0]], on2 = thisToOtherNodeMap[edge[1]];
        if (other.hasEdge(on1, on2)) newEdgeList.push_back(edge);
    }
    vector<string> newNodeNames;
    newNodeNames.reserve(getNumNodes());
    for (uint i = 0; i < getNumNodes(); i++) {
        string g1Name = nodes.at(i).nodeName;
        string newName = "("+g1Name+","+other.nodes.at(thisToOtherNodeMap[i]).nodeName+")";
        newNodeNames.push_back(newName);
    }

    vector<array<string, 2>> newNodeColorPairs;
    bool hasDefColor = colorNames[0] == DEFAULT_COLOR_NAME;
    newNodeColorPairs.reserve(getNumNodes() - (hasDefColor ? numNodesWithColor(0) : 0));
    for (uint i = (hasDefColor ? 1 : 0); i < numColors(); i++) {
        string colName = colorNames[i];
        for (uint node : nodeGroupsByColor[i]) {
            string g1Name = nodes.at(node).nodeName;
            string newName = "("+g1Name+","+other.nodes.at(thisToOtherNodeMap[node]).nodeName+")";
            newNodeColorPairs.push_back({newName, colName});
        }
    }

    return Graph(directed, name+"_intersection_"+other.name, "", newEdgeList,
                 newNodeNames, {}, newNodeColorPairs); //unweighted result
}

#ifdef LINUX
void print_stack_trace() {
    const unsigned max_frames = 64;
    void* callstack[max_frames];
    unsigned frames = backtrace(callstack, max_frames);
    char** symbols = backtrace_symbols(callstack, frames);

    for (unsigned i = 0; i < frames; ++i) {
        std::cout << symbols[i] << std::endl;
    }
    free(symbols); // Free the memory allocated by backtrace_symbols
}
#endif

unique_ptr<vector<unsigned>> Graph::getAdjList(unsigned node) const {
    static bool gaveWarning = false;
    if (!gaveWarning) {
        cerr << "Warning, soon to deprecated getAdjList function was used. This should be fixed." <<endl;
        cerr << "It can cause issues if the output is dereferenced as a temporary object." <<endl;
        cerr << "It is also a poorly optimized compatibility function left over from SANA2.0. -Marcus" <<endl;
        cerr << "If you are on Linux, you should shortly receive a stacktrace for this call..." <<endl;
#ifdef LINUX
        print_stack_trace();
#endif
        gaveWarning = true;
    }
    unique_ptr<vector<unsigned>> adjList(new vector<unsigned>());
    for (const auto &pair: nodes.at(node).adjList)
        adjList->push_back(pair.first);
    return adjList;
}

unique_ptr<vector<vector<unsigned>>> Graph::getAdjLists() const {
    static bool gaveWarning = false;
    if (!gaveWarning) {
        cerr << "NO ONE SHOULD BE USING Graph::getAdjLists() EVER! FIX YO CODE -Marcus" <<endl;
        cerr << "If you are on Linux, you should shortly receive a stacktrace for this offensive call..." <<endl;
#ifdef LINUX
        print_stack_trace();
#endif
        gaveWarning = true;
    }
    unique_ptr<vector<vector<unsigned>>> adjLists(new vector<vector<unsigned>>());
    adjLists->reserve(nodes.size());
    for (unsigned i = 0; i < nodes.size(); ++i) {
        adjLists->emplace_back(*getAdjList(i));
    }
    return adjLists;
}

unique_ptr<vector<unsigned>> Graph::getInjList(unsigned node) const {
    static bool gaveWarning = false;
    if (!gaveWarning) {
        cerr << "Warning, soon to deprecated getInjList function was used. This should be fixed." <<endl;
        cerr << "It can cause issues if the output is dereferenced as a temporary object." <<endl;
        cerr << "It is also a poorly optimized compatibility function left over from SANA2.0. -Marcus" <<endl;
        cerr << "If you are on Linux, you should shortly receive a stacktrace for this call..." <<endl;
#ifdef LINUX
        print_stack_trace();
#endif
        gaveWarning = true;
    }
    unique_ptr<vector<unsigned>> injList(new vector<unsigned>());
    for (const auto &pair: nodes.at(node).adjList)
        injList->push_back(pair.first);
    return injList;}

const vector<array<unsigned, 2>>* Graph::getEdgeList() const {
    static bool gaveWarning = false;
    if (!gaveWarning) {
        cerr << "Warning, soon to deprecated getEdgeList function was used. This should be fixed." <<endl;
        cerr << "It is a poorly optimized compatibility function left over from SANA2.0. -Marcus" <<endl;
        cerr << "If you are on Linux, you should shortly receive a stacktrace for this call..." <<endl;
#ifdef LINUX
        print_stack_trace();
#endif
        gaveWarning = true;
    }
    return &edgeList;
}

const unordered_map<string, unsigned>* Graph::getNodeNameToIndexMap() const {
    static bool gaveWarning = false;
    if (!gaveWarning) {
        cerr << "Warning, soon to deprecated getEdgeList function was used. This should be fixed." <<endl;
        gaveWarning = true;
    }
    return &nodeNameToIndexMap;
}

unsigned Graph::maxDegree() const {
    unsigned maxDegree = 0;
    for (const Node &node: nodes) {
        maxDegree = node.adjList.size() > maxDegree ? node.adjList.size() : maxDegree;
    }
    return maxDegree;
}

vector<unsigned> Graph::degreeDistribution() const {
    vector<unsigned> degreeDis(maxDegree()+1);
    for (const Node &node: nodes) {
        const unsigned degree = node.adjList.size();
        ++degreeDis[degree];
    }
    return degreeDis;
}

/* Marcus: borrowed code from the old graph system written by Nil Mamano. I enacted minimal changes where possible. */

// predicate for sorting CCs, but it doesn't measure strongly connected components (and I don't care right now)
static bool _isBiggerCC(const vector<unsigned>& a, const vector<unsigned>& b) { return a.size()>b.size(); }
vector<vector<unsigned>> Graph::connectedComponents() const {
    unsigned nodeNum = getNumNodes();
    vector<bool> nodesAreChecked(nodeNum, false);
    vector<unsigned> nodeList;
    nodeList.reserve(nodeNum);
    for (unsigned i = 0; i < nodeNum; ++i) nodeList.push_back(i);
    vector<vector<unsigned>> res;
    while (not nodeList.empty()) {
        unsigned startOfConnected = nodeList.back();
        nodeList.pop_back();
        if (nodesAreChecked[startOfConnected]) continue;
        vector<unsigned> connected;
        queue<unsigned> neighbors;
        neighbors.push(startOfConnected);
        while (not neighbors.empty()) {
            unsigned node = neighbors.front();
            neighbors.pop();
            if (nodesAreChecked[node]) continue;
            connected.push_back(node);
            nodesAreChecked[node] = true;
            for (const auto& nbrWeightPair: nodes.at(node).adjList) {
                if (not nodesAreChecked[nbrWeightPair.first]) neighbors.push(nbrWeightPair.first);
            }
        }
        res.push_back(connected);
    }
    sort(res.begin(), res.end(), _isBiggerCC);
    return res;
}

unsigned Graph::numEdgesInNodeInducedSubgraph(const vector<unsigned>& subgraphNodes) const {
    unsigned numEdges = 0;
    const unordered_set<unsigned> subgraphSet(subgraphNodes.begin(), subgraphNodes.end());
    for (const auto nodeID: subgraphNodes) {
        const Node &node = nodes.at(nodeID);
        for (const auto edge: node.adjList) {
            numEdges += subgraphSet.count(edge.first);
        }
    }
    if (!directed) {
        assert(numEdges % 2 == 0);
        numEdges /= 2;
    }
    return numEdges;
}

// Marcus: More borrowed code, thank you Dr. Mamano. Syntax has been cleaned up slightly, I still
// wish we were on C++17 so that I could use structured bindings.
vector<unsigned> Graph::numEdgesAroundByLayers(unsigned node, unsigned maxDist) const {
    unsigned numNodes = getNumNodes();
    vector<unsigned> distances(numNodes, numNodes);
    vector<bool> visited(numNodes, false);
    distances[node] = 0;
    queue<unsigned> Q;
    Q.push(node);
    vector<unsigned> result(maxDist, 0);
    while (not Q.empty()) {
        unsigned uID = Q.front();
        const Node &uNode = nodes.at(uID);
        Q.pop();
        unsigned dist = distances[uID];
        if (dist == maxDist) break;
        for (const auto &edge: uNode.adjList) {
            unsigned vID = edge.first;
            if (not visited[vID]) result[dist]++;
            if (distances[vID] < numNodes) continue;
            distances[vID] = dist+1;
            Q.push(vID);
        }
        visited[uID] = true;
    }
    return result;
}
vector<unsigned> Graph::numNodesAroundByLayers(unsigned node, unsigned maxDist) const {
    unsigned numNodes = getNumNodes();
    vector<unsigned> distances(numNodes, numNodes);
    distances[node] = 0;
    queue<unsigned> Q;
    Q.push(node);
    while (not Q.empty()) {
        unsigned uID = Q.front();
        const Node &uNode = nodes.at(uID);
        Q.pop();
        unsigned dist = distances[uID];
        if (dist == maxDist) break;
        for (const auto &edge: uNode.adjList) {
            unsigned vID = edge.first;
            if (distances[vID] < numNodes) continue;
            distances[vID] = dist+1;
            Q.push(vID);
        }
    }
    vector<unsigned> result(maxDist, 0);
    unsigned total = 0;
    for (unsigned i = 0; i < numNodes; i++)
        if (distances[i] < numNodes and distances[i] > 0) {
            result[distances[i]-1]++;
            total++;
        }
    assert(total == nodesAround(node, maxDist).size());
    return result;
}
vector<unsigned> Graph::nodesAround(unsigned node, unsigned maxDist) const {
    unsigned nodeNum = getNumNodes();
    vector<unsigned> distances(nodeNum, nodeNum);
    distances[node] = 0;
    queue<unsigned> Q;
    Q.push(node);
    while (not Q.empty()) {
        unsigned uID = Q.front();
        const Node &uNode = nodes.at(uID);
        Q.pop();
        unsigned dist = distances[uID];
        if (dist == maxDist) break;
        for (const auto &edge: uNode.adjList) {
            unsigned vID = edge.first;
            if (distances[vID] < nodeNum) continue;
            distances[vID] = dist+1;
            Q.push(vID);
        }
    }
    vector<unsigned> result;
    for (unsigned i = 0; i < nodeNum; i++) {
        if (distances[i] < nodeNum) result.push_back(i);
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
vector<unsigned> Graph::myColorIdsToOtherGraphColorIds(const Graph& other) const {
    vector<unsigned> res;
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
    for (unsigned i = (hasDefColor ? 1 : 0); i < numColors(); i++) {
        string colName = colorNames[i];
        for (unsigned node : nodeGroupsByColor[i])
            res.push_back({nodes.at(node).nodeName, colName});
    }
    return res;
}

bool Graph::isWellDefined() const {
    ostringstream ss;
    //data structures have the right size
    unsigned n = nodes.size();
    if (nodeNameToIndexMap.size() != n)
        ss<<"nodes has size "<<n<<" but nodeNameToIndexMap has size "<<nodeNameToIndexMap.size()<<endl;
    unsigned k = colorNames.size();
    if (colorNameToId.size() != k)
        ss<<"colorNames has size "<<k<<" but colorNameToId has size "<<colorNameToId.size()<<endl;
    if (nodeGroupsByColor.size() != k)
        ss<<"colorNames has size "<<k<<" but nodeGroupsByColor has size "<<nodeGroupsByColor.size()<<endl;

    //adjMatrix is symmetric if !directed, and the sum of edges weights equals totalGraphWeight
    unsigned numEdgesInAdjLists = 0;
    double adjListSum = 0;
    vector<double> nodeSum = vector<double>(n, 0.0);
    for (const Node &node1: nodes) {
        for (const auto &edge: node1.adjList) {
            unsigned node2ID = edge.first;
            EDGE_T weight = edge.second;
            if (!directed && weight != getEdgeWeight(node2ID, node1.nodeID))
                ss<<"matrix is undirected but adjLists are not symmetric at edge ("<<node1.nodeID<<" <-> "<<node2ID<<")"<<endl;
            if (directed || node1.nodeID <= node2ID) {
                numEdgesInAdjLists++;
                adjListSum += weight;
            }
            nodeSum[node1.nodeID] += weight;
        }
    }
    if (adjListSum != totalGraphWeight)
        ss<<"totalGraphWeight attribute is "<<totalGraphWeight<<" but the edges in adjList add up to "<<adjListSum<<endl;
    for (unsigned i = 0; i < n; i++) if(nodeSum[i] != nodes.at(i).totalWeight)
        ss<<"totalWeight["<<getNodeName(i)<<"] attribute is "<<nodes.at(i).totalWeight<<" but in the adjList adds up to "<<nodeSum[i]<<endl;

    //edgeList: all entries appear in adjLists, are not repeated, and every entry in adj matrix is in edge list
    if (edgeList.size() != numEdgesInAdjLists)
        ss<<"edgeList has "<<edgeList.size()<<" edges but adj Lists have "<<numEdgesInAdjLists<<endl;

    vector<unordered_set<unsigned>> nbrSetsInEdgeList(n);
    for (unsigned i = 0; i < n; i++)
        nbrSetsInEdgeList[i].reserve(nodes.at(i).adjList.size()); //to avoid hash table resizings

    for (const auto& edge : edgeList) {
        if (edge[0] < 0 or edge[0] >= n or edge[1] < 0 or edge[1] >= n)
            ss<<"edge {"<<edge[0]<<", "<<edge[1]<<"} in edgeList out of range"<<endl;
        else {
            if (!hasEdge(edge[0], edge[1]))
                ss<<"edge {"<<edge[0]<<", "<<edge[1]<<"} in edgeList missing in adjLists"<<endl;
            if (!directed) {
                unsigned nodeMin = min(edge[0], edge[1]), nodeMax = max(edge[0], edge[1]);
                //nodeMin and nodeMax to impose a canonical order on edges
                if (nbrSetsInEdgeList[nodeMin].count(nodeMax))
                    ss<<"edge {"<<edge[0]<<", "<<edge[1]<<"} repeated in edgeList"<<endl;
                nbrSetsInEdgeList[nodeMin].insert(nodeMax);
            }
        }
    }

    //all names are unique
    unordered_set<string> seenNames;
    for (const Node &node: nodes) {
        if (seenNames.count(node.nodeName))
            ss<<"repeated node name "<<node.nodeName<<endl;
        seenNames.insert(node.nodeName);
        if (!nodeNameToIndexMap.count(node.nodeName))
            ss<<"nodeNameToIndexMap missing node name "<<node.nodeName<<endl;
        else if (nodeNameToIndexMap.at(node.nodeName) != node.nodeID)
            ss<<"nodeNameToIndexMap is not the inverse of nodeNames for "<<node.nodeID<<endl;
        if (node.colorID < 0 or node.colorID >= numColors())
            ss<<"node "<<node.nodeID<<" has color "<<node.colorID<<" but there are "<<numColors()<<" colors"<<endl;
    }

    //colorNames are unique
    unordered_set<string> seenColors;
    seenColors.reserve(colorNames.size());
    for (const string& color : colorNames) {
        if (seenColors.count(color))
            ss<<"repeated color name "<<color;
        seenColors.insert(color);
    }

    //colorNameToId is the inverse map of colorNames
    for (unsigned i = 0; i < colorNames.size(); i++) {
        if (!colorNameToId.count(colorNames[i]))
            ss<<"colorNameToId missing color name "<<colorNames[i]<<endl;
        else if (colorNameToId.at(colorNames[i]) != i)
            ss<<"colorNameToId is not the inverse of colorNames for "<<i<<endl;
    }

    //nodeGroupsByColor contains every node exactly once
    //and the node colors match the nodeColors struct
    //and no color group is empty
    unordered_set<unsigned> nodesInColorGroups;
    nodesInColorGroups.reserve(n);
    for (unsigned c = 0; c < nodeGroupsByColor.size(); c++) {
        if (nodeGroupsByColor[c].size() == 0)
            ss<<"Color "<<c<<" does not have any nodes"<<endl;
        for (unsigned node : nodeGroupsByColor[c]) {
            if (nodesInColorGroups.count(node))
                ss<<"nodeGroupsByColor contains repeated node "<<node<<endl;
            else if (nodes.at(node).colorID != c)
                ss<<"node "<<node<<" in color group "<<c<<" but nodeColors["<<node<<"]="<<nodes.at(node).colorID<<endl;
            nodesInColorGroups.insert(node);
        }
    }
    if (nodesInColorGroups.size() != n)
        ss<<"nodeGroupsByColor contain "<<nodesInColorGroups.size()<<"nodes in total but adjLists has size "<<n<<endl;

    //wrap up
    string errMsg = ss.str();
    if (errMsg.size() != 0) {
        cerr<<"Graph "<<name<<" is not well defined. Has the following issues: "<<endl;
        unsigned maxMsgLen = 2000;
        if (errMsg.size() <= maxMsgLen) cerr<<errMsg<<endl;
        else cerr<<errMsg.substr(0, maxMsgLen)<<" ..."<<endl;
        return false;
    }
    return true;
}


void Graph::debugPrint() const {
    size_t MAX_LEN = 10;
    cerr<<"DEBUG PRINT "<<name<<endl;
    cerr<<"filePath: "<<filePath<<endl;
    cerr<<"directed: "<<directed<<endl;
    cerr<<"nodes vector size: "<<nodes.size()<<endl;

    cerr<<"adj edge list sizes: ";
    for(unsigned i = 0; i < min(nodes.size(), MAX_LEN); i++) cerr<<nodes.at(i).adjList.size()<<' ';
    if (MAX_LEN < nodes.size()) cerr<<"...";
    cerr<<endl;
    cerr<<"nodes[0].adjList: ";
    unsigned count = 0;
    for (const auto& edge: nodes.at(0).adjList) {
        if (count >= MAX_LEN) {
            break;
        }
        std::cerr << "(" << edge.first << ", " << edge.second << ") ";
        count++;
    }
    if (MAX_LEN < nodes.at(0).adjList.size()) cerr<<"...";
    cerr<<endl;

    cerr<<"inj edge list sizes: ";
    for(unsigned i = 0; i < min(nodes.size(), MAX_LEN); i++) cerr<<nodes.at(i).injList.size()<<' ';
    if (MAX_LEN < nodes.size()) cerr<<"...";
    cerr<<endl;
    cerr<<"nodes[0].injList: ";
    count = 0;
    for (const auto& edge: nodes.at(0).injList) {
        if (count >= MAX_LEN) {
            break;
        }
        std::cerr << "(" << edge.first << ", " << edge.second << ") ";
        count++;
    }
    if (MAX_LEN < nodes.at(0).injList.size()) cerr<<"...";
    cerr<<endl;

    cerr<<"nodeNames: ";
    for (unsigned i = 0; i < min(nodes.size(), MAX_LEN); i++) cerr<<nodes.at(i).nodeName<<' ';
    if (MAX_LEN < nodes.size()) cerr<<"...";
    cerr<<endl;

    cerr<<"nodeColors: ";
    for (unsigned i = 0; i < min(nodes.size(), MAX_LEN); i++) cerr<<nodes.at(i).nodeName<<' ';
    if (MAX_LEN < nodes.size()) cerr<<"...";
    cerr<<endl;

    cerr<<"nodeNameToIndexMap (size "<<nodeNameToIndexMap.size()<<"): ";
    unsigned kvi = 0;
    for (auto kv:nodeNameToIndexMap) {
        cerr<<kv.first<<":"<<kv.second<<' ';
        if (kvi++ == MAX_LEN and nodeNameToIndexMap.size() > MAX_LEN) { cerr<<"..."; break; }
    }
    cerr<<endl;

    cerr<<"edge list (size "<<edgeList.size()<<"): ";
    for (unsigned i = 0; i < min(edgeList.size(), MAX_LEN); i++) cerr<<'{'<<edgeList[i][0]<<", "<<edgeList[i][1]<<"} ";
    if (MAX_LEN < edgeList.size()) cerr<<"...";
    cerr<<endl;

    cerr<<"totalGraphWeight: "<<totalGraphWeight<<endl;
    auto CCs = connectedComponents();
    cerr<<"Number of CCs: "<<CCs.size()<<endl;
    cerr<<"CC sizes: ";
    for(unsigned i = 0; i < min(CCs.size(), MAX_LEN); i++) cerr<<CCs[i].size()<<' ';
    if (MAX_LEN < CCs.size()) cerr<<"...";
    cerr<<endl;

    cerr<<"CCs[0]: ";
    for (unsigned i = 0; i < min(CCs[0].size(), MAX_LEN); i++) cerr<<CCs[0][i]<<' ';
    if (MAX_LEN < CCs[0].size()) cerr<<"...";
    cerr<<endl;

    cerr<<"colorNames (size "<<colorNames.size()<<"): ";
    for (unsigned i = 0; i < min(colorNames.size(), MAX_LEN); i++) cerr<<colorNames[i]<<' ';
    if (MAX_LEN < colorNames.size()) cerr<<"...";
    cerr<<endl;

    cerr<<"colorNameToId (size "<<colorNameToId.size()<<"): ";
    kvi = 0;
    for (auto kv:colorNameToId) {
        cerr<<kv.first<<": "<<kv.second<<' ';
        if (kvi++ == MAX_LEN and colorNameToId.size() > MAX_LEN) { cerr<<"..."; break; }
    }
    cerr<<endl;

    cerr<<"nodeGroupsByColor size: "<<nodeGroupsByColor.size()<<endl;
    cerr<<"color group sizes: ";
    for(unsigned i = 0; i < min(nodeGroupsByColor.size(), MAX_LEN); i++) cerr<<nodeGroupsByColor[i].size()<<' ';
    if (MAX_LEN < nodeGroupsByColor.size()) cerr<<"...";
    cerr<<endl;

    cerr<<"nodeGroupsByColor[0]: ";
    for (unsigned i = 0; i < min(nodeGroupsByColor[0].size(), MAX_LEN); i++) cerr<<nodeGroupsByColor[0][i]<<' ';
    if (MAX_LEN < nodeGroupsByColor[0].size()) cerr<<"...";
    cerr<<endl; cerr<<endl;
}
