#include "Graphette.hpp"

using namespace std;

Graphette::Graphette()
	: numNodes_(0)
	, numEdges_(0)
{
	adjMatrix_.clear();
	degree_.clear();
	label_.clear();
}

Graphette::Graphette(uint n, uint decimalNumber)
	: numNodes_(n)
	, decimalNumber_(decimalNumber)
	, adjMatrix_(n, decimalNumber)
	, degree_(n, 0)
{
	this->init();
}

Graphette::Graphette(uint n, vector<bool>& bitVector)
	: numNodes_(n)
	, adjMatrix_(n, bitVector)
	, degree_(n, 0)
{
	this->init();
	decimalNumber_ = this->decodeHalfMatrix();
}

Graphette::Graphette(uint n, HalfMatrix adjMatrix)
	: numNodes_(n)
	, adjMatrix_(adjMatrix)
	, degree_(n, 0)
{
	this->init();
	decimalNumber_ = this->decodeHalfMatrix();
}

Graphette::~Graphette(){
	adjMatrix_.clear();
	degree_.clear();
	label_.clear();
}

void Graphette::init(){
	uint sum = 0;
	for(uint i = 0; i < numNodes_; i++){
		for(uint j = i+1; j < numNodes_; j++){
			sum+= adjMatrix_(i,j);
			degree_[i]+= adjMatrix_(i,j);
			degree_[j]+= adjMatrix_(i,j);
		}
		label_.push_back(i);
	}
	numEdges_ = sum;
}

uint Graphette::decodeHalfMatrix(){
	uint num = 0, k = 0, bitVectorSize = (numNodes_*(numNodes_-1))/2;
	for(uint i = 0; i < numNodes_; i++)
		for(uint j=i+1; j < numNodes_; j++){
			num += adjMatrix_(i, j) << (bitVectorSize-1-k);
			k++;
		}
	return num;
}

vector<bool> Graphette::bitVector(){
	vector<bool> bitVector;
	for(uint i = 0; i < numNodes_; i++)
		for(uint j=i+1; j < numNodes_; j++){
			bitVector.push_back(adjMatrix_(i, j));
		}
	return bitVector;
}

uint Graphette::decimalNumber(){
	return decimalNumber_;
}

uint Graphette::numNodes(){
	return numNodes_;
}

uint Graphette::numEdges(){
	return numEdges_;
}

uint Graphette::degree(uint node){
	return degree_[node];
}

uint Graphette::label(uint node){
	return label_[node];
}

vector <uint> Graphette::labels(){
	return label_;
}

void Graphette::setLabel(uint node, uint label){
	label_[node] = label;
}

void Graphette::setLabels(vector<uint>& label){
	label_ = label;
}

void Graphette::printAdjMatrix(){
	adjMatrix_.print();
}

vector<Graphette*> Graphette::generateAll(uint n){
	vector<Graphette*> graphetteCopy(0);
	if(n > 0){
		uint num_edges = (n*(n-1))/2;
		for(uint num = 0; num < pow(2, num_edges); num++){
			Graphette* g = new Graphette(n, num);
			graphetteCopy.push_back(g);
		}
	}
	return graphetteCopy;
}

vector<vector<uint>> Graphette::orbits(){
	vector<uint> permutation, orbit;
	
	//initially,the permutation is (0, 1, ..., numNodes_-1)
	//and each node is in its own orbit. we'll merge orbits later 
	for(uint i = 0; i < numNodes_; i++){
		permutation.push_back(i);
		orbit.push_back(i);
	} 
	while( next_permutation(permutation.begin(), permutation.end()) ){
		
		//Just a speed up; ruling out searching into unnecessary permutations
		if(not this->suitable(permutation)) continue;

		Graphette* pGraph = this->permuteNodes(permutation);
		//Check for automorphism
		if(this->decimalNumber() == pGraph->decimalNumber()){
			this->captureCycles(permutation, orbit);
		}
		delete pGraph;
	}
	//saving orbits
	vector<vector<uint>> output;
	for(uint orbitId = 0; orbitId < numNodes_; orbitId++){
		vector<uint> temp;
		for(uint node = 0; node < numNodes_; node++){
			if(orbit[node] == orbitId)
				temp.push_back(node);
		}
		if(not temp.empty())
			output.push_back(temp);
	}
	return output;
}

bool Graphette::suitable(vector<uint>& permutation){
	for(uint node = 0; node < numNodes_; node++){
		if(this->degree(node) != this->degree(permutation[node])){
			return false;
		}
	}
	return true;
}

Graphette* Graphette::permuteNodes(vector<uint>& permutation){
	//To store adjMatrix_ after permutation
	HalfMatrix pAdjMatrix(numNodes_);
	//Apply the permutation
	for(uint i = 0; i < numNodes_; i++){
		for(uint j = i+1; j < numNodes_; j++){
			uint pi = permutation[i], pj = permutation[j]; //new_i, new_j
			if(pi == pj) 
				throw invalid_argument("Graphette::permuteNodes(): Bad permutation: distict nodes map to the same node");
			else
				pAdjMatrix(pi, pj) = adjMatrix_(i, j);
		}
	}

	Graphette* g = new Graphette(numNodes_, pAdjMatrix);
	return g;
}

void Graphette::captureCycles(vector<uint>& permutation, vector<uint>& orbit){
	vector<bool> visited(numNodes_, false);
	for(uint i = 0; i < numNodes_; i++){
		if(not visited[i]){
			//finding out each cycle at a time
			vector<uint> cycle;
			followTrail(permutation, cycle, i, i, visited);
			uint minOrbit = orbit[cycle[0]];
			for(uint j: cycle)
				minOrbit = min(orbit[j], minOrbit);
			for(uint j: cycle)
				orbit[j] = minOrbit;
		}
	}
}

void Graphette::followTrail(vector<uint>& permutation, vector<uint>& cycle,
	uint seed, uint current, vector<bool>& visited){
	cycle.push_back(current);
	visited[current] = true;
	if(permutation[current] != seed)
		followTrail(permutation, cycle, seed, permutation[current], visited);
}