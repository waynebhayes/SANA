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
	: numNodes_(adjMatrix.getLength())
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
	if(not adjMatrix_(node1, node2)){
		adjMatrix_(node1, node2) = true;
		degree_[node1]++;
		degree_[node2]++;
		numEdges_++;
	}
}

void Graph::removeEdge(uint node1, uint node2){
	//To ensure that each edge is removed no more than once
	if(adjMatrix_(node1, node2)){
		adjMatrix_(node1, node2) = false;
		degree_[node1]--;
		degree_[node2]--;
		numEdges_--;
	}
}

uint Graph::getNumNodes(){
	return numNodes_;
}

uint Graph::getNumEdges(){
	return numEdges_;
}

uint Graph::getDegree(uint node){
	return degree_[node];
}

void Graph::printAdjMatrix()
{
	adjMatrix_.print();
}

Graph* Graph::getSubGraph(vector<uint>& nodes){
	uint n = nodes.size();
	vector<bool> bits;
	for(uint i = 0; i < n; i++){
		for(uint j = i+1; j < n; j++){
			bits.push_back(adjMatrix_(nodes[i], nodes[j]));
		}
	}
	HalfMatrix m(n, bits);
	Graph* g = new Graph(m);
	return g;
}
