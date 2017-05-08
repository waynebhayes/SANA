#include "Graph.hpp"
using namespace std;

Graph::Graph()
	:numNodes_(0)
	,numEdges_(0)
{
	adjMatrix_.clear();
	degree_.clear();
}
Graph::Graph(uint numNodes)
	:numNodes_(numNodes)
	,numEdges_(0)
	,adjMatrix_(numNodes)
	,degree_(numNodes, false)
{

}

Graph::Graph(vector<pair<uint, uint>>& edgeList)
{
	if(not edgeList.size()){
		numNodes_ = 0;
		numEdges_= 0;
		adjMatrix_.clear();
		degree_.clear();
	}
	else{
		uint n = 0;
		for(auto edge: edgeList){
			if(max(edge.first, edge.second) > n){
				n = max(edge.first, edge.second);
			}
		}
		n++;
		numNodes_ = n;
		numEdges_ = 0;
		adjMatrix_ = HalfMatrix(n);
		degree_.resize(n, 0);
		for(auto edge: edgeList){
			this->addEdge(edge.first, edge.second);
		}
	}
}

Graph::Graph(HalfMatrix& adjMatrix)
	: numNodes_(adjMatrix.length())
	, adjMatrix_(adjMatrix)
{
	uint n = numNodes_;
	degree_.clear();
	degree_.resize(n, 0);
	uint sum = 0;
	for(uint i = 0; i < n; i++){
		for(uint j = i+1; j < n; j++){
			if(adjMatrix_(i, j)){
				sum++;
				degree_[i]++;
				degree_[j]++;
			}
		}
	}
	numEdges_ = sum;
}

Graph::~Graph(){
	adjMatrix_.clear();
	degree_.clear();
}

void Graph::addEdge(uint node1, uint node2){
	//To ensure that each edge is added no more than once
	if(not this->hasEdge(node1, node2)){
		adjMatrix_(node1, node2) = true;
		degree_[node1]++;
		degree_[node2]++;
		numEdges_++;
	}
}

void Graph::removeEdge(uint node1, uint node2){
	//To ensure that each edge is removed no more than once
	if(this->hasEdge(node1, node2)){
		adjMatrix_(node1, node2) = false;
		degree_[node1]--;
		degree_[node2]--;
		numEdges_--;
	}
}

bool Graph::hasEdge(uint node1, uint node2){
	return adjMatrix_(node1, node2);
}

uint Graph::numNodes(){
	return numNodes_;
}

uint Graph::numEdges(){
	return numEdges_;
}

uint Graph::degree(uint node){
	return degree_[node];
}

void Graph::printAdjMatrix()
{
	adjMatrix_.print();
}

vector<uint> Graph::neighbors(uint node){
	vector<uint> nbors;
	for(uint i = 0; i < numNodes_; i++){
		if(i == node) continue;
		else if(this->hasEdge(i, node))
			nbors.push_back(i);
	}
	return nbors;
}

Graphette* Graph::sampleGraphette(uint k){
	if(k == 0 or k > numNodes_){
		throw out_of_range("Graph::sampleGraphette(k): k is outside (0, numNodes_]");
	}
	else{
		vector<bool> visited(false, numNodes_);
		vector<uint> nodes(1, xrand(0, numNodes_));
		set<uint> candidates;
		while(nodes.size() < k){
			auto n = *(nodes.rbegin());
			visited[n] = true;
			for(auto nbor : this->neighbors(n)){
				if(not visited[nbor])
					candidates.insert(nbor);
			}
			if(candidates.size() > 0){
				auto i = xrand(0, candidates.size());
				auto it = candidates.begin();
				while(i > 0){
					it++;
					i--;
				}
				nodes.push_back(*it);
				candidates.erase(it);
			}
			else{
				//Select a random node
				uint node = xrand(0, numNodes_);
				if(not visited[node]){
					nodes.push_back(node);	
				}
			}
		}
		return this->createGraphette(nodes);
	}
}

set<uint> Graph::explore(uint seed, uint radius, vector<bool>& visited){
	set<uint> nbors;
	nbors.insert(seed);
	visited[seed] = true;
	for(uint i=0; i<numNodes_; i++){
		if(seed == i) continue;
		if(this->hasEdge(seed, i)){
			if(radius == 1) 
				nbors.insert(i);
			else if(radius > 1 and visited[i] == false){
				auto temp = this->explore(i, radius-1, visited);
				nbors.insert(temp.begin(), temp.end());
			}
		}
	}
	return nbors;
}

Graphette* Graph::createGraphette(vector<uint>& nodes){
	vector<bool> bitVector;
	for(uint i=0; i<nodes.size(); i++){
		for(uint j=i+1; j<nodes.size(); j++){
			bitVector.push_back(adjMatrix_(nodes[i], nodes[j]));
		}
	}
	Graphette* g = new Graphette(nodes.size(), bitVector);
	g->setLabels(nodes);
	return g;
}