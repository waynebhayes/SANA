#include "Graph.hpp"
using namespace std;

Graph::Graph()
	:numNodes_(0)
	,numEdges_(0)
{
	adjMatrix_.clear();
	degree_.clear();
}
Graph::Graph(ullint numNodes)
	:numNodes_(numNodes)
	,numEdges_(0)
	,adjMatrix_(numNodes)
	,degree_(numNodes, false)
{

}

Graph::Graph(vector<pair<ullint, ullint>>& edgeList)
{
	if(not edgeList.size()){
		numNodes_ = 0;
		numEdges_= 0;
		adjMatrix_.clear();
		degree_.clear();
	}
	else{
		ullint n = 0;
		for(auto edge: edgeList)
			n = max(n, max(edge.first, edge.second));
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
	ullint n = numNodes_;
	degree_.clear();
	degree_.resize(n, 0);
	ullint sum = 0;
	for(ullint i = 0; i < n; i++){
		for(ullint j = i+1; j < n; j++){
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

void Graph::addEdge(ullint node1, ullint node2){
	//To ensure that each edge is added no more than once
	if(not this->hasEdge(node1, node2)){
		adjMatrix_(node1, node2) = true;
		degree_[node1]++;
		degree_[node2]++;
		numEdges_++;
	}
}

void Graph::removeEdge(ullint node1, ullint node2){
	//To ensure that each edge is removed no more than once
	if(this->hasEdge(node1, node2)){
		adjMatrix_(node1, node2) = false;
		degree_[node1]--;
		degree_[node2]--;
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
	return degree_[node];
}

void Graph::printAdjMatrix()
{
	adjMatrix_.print();
}

vector<ullint> Graph::neighbors(ullint node){
	vector<ullint> nbors;
	for(ullint i = 0; i < numNodes_; i++){
		if(i == node) continue;
		else if(this->hasEdge(i, node))
			nbors.push_back(i);
	}
	return nbors;
}

Graphette* Graph::sampleGraphette(ullint k){
	if(k == 0 or k > numNodes_){
		throw out_of_range("Graph::sampleGraphette(k): k is outside (0, numNodes_]");
	}
	else{
		vector<bool> visited(numNodes_, false);
		//nodes contains the selected 
		set<ullint> nodes, candidates;
		while(nodes.size() < k){
			ullint node;
			if(candidates.size() > 0){
				auto i = xrand(0, candidates.size());
				auto it = candidates.begin();
				while(i > 0){
					it++;
					i--;
				}
				node = *it;
			}
			else{
				//Select a random node
				node = xrand(0, numNodes_);
			}
			if(not visited[node]){
				visited[node] = true;
				candidates.erase(node);
				nodes.insert(node);
				for(auto nbor : this->neighbors(node)){
					if(not visited[nbor])
						candidates.insert(nbor);
				}
			}
		}
		vector<ullint> temp(nodes.begin(), nodes.end());
		return this->createGraphette(temp);
	}
}

Graphette* Graph::createGraphette(vector<ullint>& nodes){
	vector<bool> bitVector;
	for(ullint i=0; i<nodes.size(); i++){
		for(ullint j=i+1; j<nodes.size(); j++){
			bitVector.push_back(adjMatrix_(nodes[i], nodes[j]));
		}
	}
	Graphette* g = new Graphette(nodes.size(), bitVector);
	g->setLabels(nodes);
	return g;
}