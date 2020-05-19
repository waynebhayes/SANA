#include "Graph.hpp"
using namespace std;

Graph::Graph()
    : Graph(0)
{

}
Graph::Graph(ullint numNodes)
    : numNodes_(numNodes)
    , numEdges_(0)
    , adjMatrix_(numNodes)
    , adjList_(numNodes)
{

}

Graph::Graph(vector<pair<ullint, ullint>>& edgeList)
    : numNodes_(0)
    , numEdges_(0)
    , adjList_(0)
{
    for(auto& edge: edgeList)
        numNodes_ = max(numNodes_, max(edge.first, edge.second));
    numNodes_+= (edgeList.size() > 0);
    adjMatrix_ = HalfMatrix(numNodes_);
    adjList_.resize(numNodes_);
    for(auto& edge: edgeList){
        this->addEdge(edge.first, edge.second);
    }
    for(auto& vec : adjList_)
        sort(vec.begin(), vec.end());
}

Graph::Graph(HalfMatrix& adjMatrix)
    : numNodes_(adjMatrix.length())
    , numEdges_(0)
    , adjMatrix_(adjMatrix)
{
    ullint n = numNodes_;
    adjList_ = vector<vector<ullint>> (n);
    for(ullint i = 0; i < n; i++) for(ullint j = i+1; j < n; j++)
        if(adjMatrix_(i, j)) 
            this->addEdge(i, j);
    for(auto& vec : adjList_)
        sort(vec.begin(), vec.end());
}

Graph::~Graph(){
    adjMatrix_.clear();
}

void Graph::addEdge(ullint node1, ullint node2){
    //To ensure that each edge is added no more than once
    if(not this->hasEdge(node1, node2)){
        adjMatrix_(node1, node2) = true;
        adjList_[node1].push_back(node2);
        adjList_[node2].push_back(node1);
        numEdges_++;
    }
}

void Graph::removeEdge(ullint node1, ullint node2){
    //To ensure that each edge is removed no more than once
    if(this->hasEdge(node1, node2)){
        adjMatrix_(node1, node2) = false;
        adjList_[node1].erase(lower_bound(adjList_[node1].begin(), adjList_[node1].end(), node2));
        adjList_[node2].erase(lower_bound(adjList_[node2].begin(), adjList_[node2].end(), node1));
        numEdges_--;
    }
}

bool Graph::hasEdge(ullint node1, ullint node2){
    return adjMatrix_(node1, node2);
}

ullint Graph::numNodes(){
    return numNodes_;
}

ullint Graph::numEdges(){
    return numEdges_;
}

ullint Graph::degree(ullint node){
    return adjList_[node].size();
}

void Graph::printAdjMatrix()
{
    adjMatrix_.print();
}

vector<ullint> Graph::neighbors(ullint node){
    return adjList_[node];
}

Graphette* Graph::sampleGraphette(ullint k, ullint node1, ullint node2){
    if(k == 0 or k > numNodes_){
        throw out_of_range("Graph::sampleGraphette(): k is outside [0, numNodes_)");
    }
    else if(node1 < 0 or node1 >= numNodes_){
        throw out_of_range("Graph::sampleGraphette(): node1 is outside [0, numNodes_)");
    }
    else if(node2 < 0 or node2 >= numNodes_){
        throw out_of_range("Graph::sampleGraphette(): node2 is outside [0, numNodes_)");
    }
    else {
        vector<bool> visited(numNodes_, false);
        //nodes contains the selected 
        vector<long> candidates(numNodes_);
        vector<ullint> nodes;
        long len = 0, gone = 0;
        nodes.push_back(node1);
        nodes.push_back(node2);
        visited[node1] = true;
        visited[node2] = true;
        for(auto nbor : this->neighbors(node1)){
            if(not visited[nbor]){
                candidates[len++] = nbor;
                visited[nbor] = true;
            }
        }
        for(auto nbor : this->neighbors(node2)){
            if(not visited[nbor]){
                candidates[len++] = nbor;
                visited[nbor] = true;
            }

        }
        while(nodes.size() < k){
            ullint i, node;
            if(len > gone){ // insist on graphlet before accepting a disconnect
                do{ 
                    i = xrand(0, len);
                }while(candidates[i] == -1);
                node = candidates[i];
                candidates[i] = -1;
                gone++;
            }
            else{
                //Select a random node
                do{ 
                    node = xrand(0, numNodes_);
                }while(visited[node]);
                visited[node] = true;
            }
            nodes.push_back(node);
            for(auto nbor : this->neighbors(node)){
                if(not visited[nbor]) {
                    candidates[len++] = nbor;
                    visited[nbor] = true;
                }
            }
        }
        return this->createGraphette(nodes);
    }
}

Graphette* Graph::createGraphette(vector<ullint>& nodes){
    vector<bool> bitVector;
    sort(nodes.begin(), nodes.end());
    for(ullint i=0; i<nodes.size(); i++){
        for(ullint j=i+1; j<nodes.size(); j++){
            bitVector.push_back(adjMatrix_(nodes[i], nodes[j]));
        }
    }
    Graphette* g = new Graphette(nodes.size(), bitVector);
    g->setLabels(nodes);
    return g;
}
