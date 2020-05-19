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

Graphette::Graphette(ullint n, ullint decimalNumber)
    : numNodes_(n)
    , decimalNumber_(decimalNumber)
    , adjMatrix_(n, decimalNumber)
    , degree_(n, 0)
{
    this->init();
}

Graphette::Graphette(ullint n, vector<bool>& bitVector)
    : numNodes_(n)
    , adjMatrix_(n, bitVector)
    , degree_(n, 0)
{
    this->init();
    decimalNumber_ = this->decodeHalfMatrix();
}

Graphette::Graphette(ullint n, HalfMatrix adjMatrix)
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
    ullint sum = 0;
    for(ullint i = 0; i < numNodes_; i++){
        for(ullint j = i+1; j < numNodes_; j++){
            sum+= adjMatrix_(i,j);
            degree_[i]+= adjMatrix_(i,j);
            degree_[j]+= adjMatrix_(i,j);
        }
        label_.push_back(i);
    }
    numEdges_ = sum;
}

ullint Graphette::decodeHalfMatrix(){
    ullint num = 0, k = 0, bitVectorSize = (numNodes_*(numNodes_-1))/2;
    for(ullint i = 0; i < numNodes_; i++)
        for(ullint j=i+1; j < numNodes_; j++){
            num += adjMatrix_(i, j) << (bitVectorSize-1-k);
            k++;
        }
    return num;
}

vector<bool> Graphette::bitVector(){
    vector<bool> bitVector;
    for(ullint i = 0; i < numNodes_; i++)
        for(ullint j=i+1; j < numNodes_; j++){
            bitVector.push_back(adjMatrix_(i, j));
        }
    return bitVector;
}

ullint Graphette::decimalNumber(){
    return decimalNumber_;
}

ullint Graphette::numNodes(){
    return numNodes_;
}

ullint Graphette::numEdges(){
    return numEdges_;
}

ullint Graphette::degree(ullint node){
    return degree_[node];
}

ullint Graphette::label(ullint node){
    return label_[node];
}

vector <ullint> Graphette::labels(){
    return label_;
}

void Graphette::setLabel(ullint node, ullint label){
    label_[node] = label;
}

void Graphette::setLabels(vector<ullint>& label){
    label_ = label;
}

void Graphette::printAdjMatrix(){
    adjMatrix_.print();
}

vector<Graphette*> Graphette::generateAll(ullint n){
    vector<Graphette*> graphetteCopy(0);
    if(n > 0){
        ullint num_edges = (n*(n-1))/2;
        for(ullint num = 0; num < pow(2, num_edges); num++){
            Graphette* g = new Graphette(n, num);
            graphetteCopy.push_back(g);
        }
    }
    return graphetteCopy;
}

vector<vector<ullint>> Graphette::orbits(){
    vector<ullint> permutation, orbit;
    
    //initially,the permutation is (0, 1, ..., numNodes_-1)
    //and each node is in its own orbit. we'll merge orbits later 
    for(ullint i = 0; i < numNodes_; i++){
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
    vector<vector<ullint>> output;
    for(ullint orbitId = 0; orbitId < numNodes_; orbitId++){
        vector<ullint> temp;
        for(ullint node = 0; node < numNodes_; node++){
            if(orbit[node] == orbitId)
                temp.push_back(node);
        }
        if(not temp.empty())
            output.push_back(temp);
    }
    return output;
}

bool Graphette::suitable(vector<ullint>& permutation){
    for(ullint node = 0; node < numNodes_; node++){
        if(this->degree(node) != this->degree(permutation[node])){
            return false;
        }
    }
    return true;
}

Graphette* Graphette::permuteNodes(vector<ullint>& permutation){
    //To store adjMatrix_ after permutation
    HalfMatrix pAdjMatrix(numNodes_);
    //Apply the permutation
    for(ullint i = 0; i < numNodes_; i++){
        for(ullint j = i+1; j < numNodes_; j++){
            ullint pi = permutation[i], pj = permutation[j]; //new_i, new_j
            if(pi == pj) 
                throw invalid_argument("Graphette::permuteNodes(): Bad permutation: distict nodes map to the same node");
            else
                pAdjMatrix(pi, pj) = adjMatrix_(i, j);
        }
    }

    Graphette* g = new Graphette(numNodes_, pAdjMatrix);
    return g;
}

void Graphette::captureCycles(vector<ullint>& permutation, vector<ullint>& orbit){
    vector<bool> visited(numNodes_, false);
    for(ullint i = 0; i < numNodes_; i++){
        if(not visited[i]){
            //finding out each cycle at a time
            vector<ullint> cycle;
            followTrail(permutation, cycle, i, i, visited);
            ullint minOrbit = orbit[cycle[0]];
            for(ullint j: cycle)
                minOrbit = min(orbit[j], minOrbit);
            for(ullint j: cycle)
                orbit[j] = minOrbit;
        }
    }
}

void Graphette::followTrail(vector<ullint>& permutation, vector<ullint>& cycle,
    ullint seed, ullint current, vector<bool>& visited){
    cycle.push_back(current);
    visited[current] = true;
    if(permutation[current] != seed)
        followTrail(permutation, cycle, seed, permutation[current], visited);
}