#include "Graph.hpp"
#include "Utils.hpp"
#include <vector>
#include <fstream>
#include <string>
#include <stdexcept>
#include <utility>
#include <sstream>
#include <unordered_set>

using namespace std;

string Graph::GetName() const {
    return name;
}

void Graph::setName(string name) {
    this -> name = name;
}

unsigned int Graph::GetNumNodes() const {
	return numNodes;
}

unsigned int Graph::GetNumEdges() const {
    return numEdges;
}

int Graph::RandomNode() {
    // TODO change this later so its uses a non-static function
    return Utils::RandInt(0,GetNumNodes()-1);
}

void Graph::AddEdge(const unsigned int &node1, const unsigned int &node2, const unsigned int &weight) throw(GraphInvalidIndexError) {
    if (node1 >= numNodes || node2 >= numNodes)
        throw GraphInvalidIndexError("Invalid node index passed into AddEdge");
    if (node1 > node2) {
        if (adjLists[node2][node1] == 0) {
           adjLists[node2].push_back(node1);
        }
    } else {
        adjLists[node1].push_back(node2);
    }
    ++numEdges;  // Either way we iterate the edge count
}

void Graph::RemoveEdge(const unsigned int &node1, const unsigned int &node2) throw(GraphInvalidIndexError) {
    if (node1 >= numNodes || node2 >= numNodes)
        throw GraphInvalidIndexError("Invalid node index passed into RemoveEdge");

    //update adjacency lists
    for (unsigned int i = 0; i < adjLists[node1].size(); i++) {
        if (adjLists[node1][i] == node2) {
            unsigned int lastNeighbor = adjLists[node1][adjLists[node1].size()-1];
            adjLists[node1][i] = lastNeighbor;
            adjLists[node1].pop_back();
            break;
        }
    }
    for (unsigned int i = 0; i < adjLists[node2].size(); i++) {
        if (adjLists[node2][i] == node1) {
            unsigned int lastNeighbor = adjLists[node2][adjLists[node2].size()-1];
            adjLists[node2][i] = lastNeighbor;
            adjLists[node2].pop_back();
            break;
        }
    }
}


void Graph::AddRandomEdge() {
    int node1 = 0, node2 = 0;
    while(node1 == node2 or adjLists[node1][node2]) {
        node1 = RandomNode();
        node2 = RandomNode();
    }
    AddEdge(node1,node2);
}

void Graph::RemoveRandomEdge() {
    int node1 = 0, node2 = 0;
    while(node1 == node2 or adjLists[node1][node2]) {
        node1 = RandomNode();
        node2 = RandomNode();
    }
    RemoveEdge(node1,node2);
}

void Graph::SetNumNodes(const unsigned int &numNodes) {
    adjLists.resize(numNodes);
    nodeNames.resize(numNodes);
}

void Graph::SetNodeName(const unsigned int &nodeIndex, const string &name) throw(GraphInvalidIndexError) {
    if (nodeIndex >= numNodes)
        throw GraphInvalidIndexError("Invalid node index passed into SetNodeName");
    nodeNames[nodeIndex] = name;
}

string Graph::GetNodeName(const unsigned int &nodeIndex) const throw(GraphInvalidIndexError) {
     if (nodeIndex >= numNodes)
         throw GraphInvalidIndexError("Invalid node index passed into GetNodeName");
     return nodeNames[nodeIndex];
}

void Graph::ClearGraph() {
    adjLists.clear();
    numNodes = numEdges = 0;
}

unordered_map<string,ushort> Graph::getNodeNameToIndexMap() const {
    unordered_map<string,ushort> res;
    for(size_t i = 0; i < GetNumNodes(); ++i) { //Get the number of nodes in the graph
        res[this->GetNodeName(i)] = i; //At that node position, map the name to the number
    }
    return res;
}

unordered_map<ushort,string> Graph::getIndexToNodeNameMap() const {
    unordered_map<string,ushort> reverse = getNodeNameToIndexMap();
    unordered_map<ushort,string> res;
    for (const auto &nameIndexPair : reverse ) {
        res[nameIndexPair.second] = nameIndexPair.first;
    }
    return res;
}

const vector < vector<unsigned int> >& Graph::getAdjList() const {
    return adjLists;
}

void Graph::getCopyAdjList(vector<vector<ushort> > &adjListCopy) const {
    adjListCopy.resize(adjLists.size()); //resize so we have the correct number of rows
    ushort curr_row = 0;
    for(auto r : adjLists) {
        for(auto c : r) {
            adjListCopy[curr_row].push_back(ushort(c)); //So we push back the list at each row of adjLists
            //while casting to ushort
        }
    }
}


void Graph::genAdjMatrix(vector<vector<bool> > &adjMatrixCopy) const {
    //Make a adjMatrix of bools
    adjMatrixCopy.resize(this->GetNumNodes());
    int curr_row = 0;
    for(const auto i : adjLists) { //For every row
        for(const auto j : i) { //For the elements of the row list
            adjMatrixCopy[curr_row][j] = true;
        }
        ++curr_row;
    }
}

unsigned int Graph::NumNodeInducedSubgraphEdges(const vector<ushort> &subgraphNodes) const {
    std::unordered_set<unsigned int> nodeSet(subgraphNodes.begin(),subgraphNodes.end());
    uint count = 0;
    for (uint i = 0; i < subgraphNodes.size(); i++) {
        ushort node1 = subgraphNodes[i];
        for (uint j = 0; j < adjLists[node1].size(); j++) {
            ushort node2 = adjLists[node1][j];
            count += nodeSet.count(node2);
        }
    }
    return count/2;
}
