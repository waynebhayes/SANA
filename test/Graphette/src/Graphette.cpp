#include "Graphette.hpp"

using namespace std;

Graphette::Graphette()
	: numNodes_(0)
	, numEdges_(0)
{
	adjMatrix_.clear();
	degree_.clear();
}

Graphette::Graphette(ushort n, uint decimalNumber)
	: numNodes_(n)
	, decimalNumber_(decimalNumber)
	, adjMatrix_(n, decimalNumber)
	, degree_(n, 0)
{
	this->init();
}

Graphette::Graphette(ushort n, vector<bool>& bitVector)
	: numNodes_(n)
	, adjMatrix_(n, bitVector)
	, degree_(n, 0)
{
	this->init();
	decimalNumber_ = this->decodeHalfMatrix();
}

Graphette::Graphette(ushort n, HalfMatrix adjMatrix)
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
}

void Graphette::init(){
	uint sum = 0;
	for(ushort i = 0; i < numNodes_; i++){
		for(ushort j = i+1; j < numNodes_; j++){
			sum+= adjMatrix_(i,j);
			degree_[i]+= adjMatrix_(i,j);
			degree_[j]+= adjMatrix_(i,j);
		}
	}
	numEdges_ = sum;
}

uint Graphette::decodeHalfMatrix(){
	uint num = 0, k = 0, bitVectorSize = (numNodes_*(numNodes_-1))/2;
	for(ushort i = 0; i < numNodes_; i++)
		for(ushort j=i+1; j < numNodes_; j++){
			num += adjMatrix_(i, j) << (bitVectorSize-1-k);
			k++;
		}
	return num;
}

vector<bool> Graphette::getBitVector(){
	vector<bool> bitVector;
	for(ushort i = 0; i < numNodes_; i++)
		for(ushort j=i+1; j < numNodes_; j++){
			bitVector.push_back(adjMatrix_(i, j));
		}
	return bitVector;
}

uint Graphette::getDecimalNumber(){
	return decimalNumber_;
}

ushort Graphette::getNumNodes(){
	return numNodes_;
}

uint Graphette::getNumEdges(){
	return numEdges_;
}

ushort Graphette::getDegree(ushort node){
	return degree_[node];
}

void Graphette::printAdjMatrix(){
	for (ushort i = 0; i < numNodes_; i++){
		for (ushort j = 0; j < numNodes_; j++){
			if (i < j)
				cout << adjMatrix_(i, j) << " ";
			else
				cout << "  ";
		}
		cout << "\n";
	}
}

vector<Graphette*> Graphette::generateAll(ushort n){
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

vector<vector<ushort>> Graphette::getOrbits(){
	vector<ushort> permutation, orbit;
	
	//initially,the permutation is (0, 1, ..., numNodes_-1)
	//and each node is in its own orbit. we'll merge orbits later 
	for(ushort i = 0; i < numNodes_; i++){
		permutation.push_back(i);
		orbit.push_back(i);
	} 
	while( next_permutation(permutation.begin(), permutation.end()) ){
		
		//Just a speed up; ruling out searching into unnecessary permutations
		if(not this->suitable(permutation)) continue;

		Graphette* pGraph = this->permuteNodes(permutation);
		//Check for automorphism
		if(this->getDecimalNumber() == pGraph->getDecimalNumber()){
			this->captureCycles(permutation, orbit);
		}
		delete pGraph;
	}
	//saving orbits
	vector<vector<ushort>> output;
	for(ushort orbitId = 0; orbitId < numNodes_; orbitId++){
		vector<ushort> temp;
		for(ushort node = 0; node < numNodes_; node++){
			if(orbit[node] == orbitId)
				temp.push_back(node);
		}
		if(not temp.empty())
			output.push_back(temp);
	}
	return output;
}

bool Graphette::suitable(vector<ushort>& permutation){
	for(ushort node = 0; node < numNodes_; node++){
		if(this->getDegree(node) != this->getDegree(permutation[node])){
			return false;
			break;
		}
	}
	return true;
}

Graphette* Graphette::permuteNodes(vector<ushort>& permutation){
	//To store adjMatrix_ after permutation
	HalfMatrix pAdjMatrix(numNodes_);
	//Apply the permutation
	for(ushort i = 0; i < numNodes_; i++){
		for(ushort j = i+1; j < numNodes_; j++){
			ushort pi = permutation[i], pj = permutation[j]; //new_i, new_j
			if(pi == pj) 
				assert("Bad permutation: distict nodes map to the same node");
			else
				pAdjMatrix(pi, pj) = adjMatrix_(i, j);
		}
	}

	Graphette* g = new Graphette(numNodes_, pAdjMatrix);
	return g;
}

void Graphette::captureCycles(vector<ushort>& permutation, vector<ushort>& orbit){
	vector<bool> visited(numNodes_, false);
	for(ushort i = 0; i < numNodes_; i++){
		if(not visited[i]){
			//finding out each cycle at a time
			vector<ushort> cycle;
			followTrail(permutation, cycle, i, i, visited);
			ushort minOrbit = orbit[cycle[0]];
			for(ushort j: cycle)
				minOrbit = min(orbit[j], minOrbit);
			for(ushort j: cycle)
				orbit[j] = minOrbit;
		}
	}
}

void Graphette::followTrail(vector<ushort>& permutation, vector<ushort>& cycle,
	ushort seed, ushort current, vector<bool>& visited){
	cycle.push_back(current);
	visited[current] = true;
	if(permutation[current] != seed)
		followTrail(permutation, cycle, seed, permutation[current], visited);
}