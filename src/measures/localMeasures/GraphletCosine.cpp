#include <vector>
#include <iostream>
#include <cmath>
#include "GraphletCosine.hpp"
using namespace std;

GraphletCosine::GraphletCosine(Graph* G1, Graph* G2) : LocalMeasure(G1, G2, "graphletcosine") {
    string fileName = autogenMatricesFolder+G1->getName()+"_"+G2->getName()+"_graphletcosine.bin";
    loadBinSimMatrix(fileName);
}

GraphletCosine::~GraphletCosine() {
}

vector<double> GraphletCosine::getNumbersOfAffectedOrbits() {
    //how many types of orbits i-th orbit touches
    return {1, 2, 2, 2, 3, 4, 3, 3, 4, 3, 4, 4, 4, 4, 3, 4, 6,
            5, 4, 5, 6, 6, 4, 4, 5, 5, 8, 4, 6, 6, 7, 5, 6, 6,
            6, 5, 6, 7, 7, 5, 7, 7, 7, 6, 5, 5, 6, 8, 7, 6, 6,
            8, 6, 9, 6, 6, 4, 6, 6, 8,10, 7, 6, 8, 8, 6, 7, 8,
            8, 5, 6, 6, 4};
    //how many orbits i-th orbit touches
    return {1, 2, 3, 3, 3, 5, 4, 7, 6, 4, 5, 7, 6, 7, 7, 4, 7,
            8, 5, 6, 9,11,13,15, 6, 8,11, 5, 9, 7,11, 9, 9,15,
            10, 7, 9,10,13, 8,10,11,15, 9,15, 7, 9,14,10,12,14,
            11,10,13,12,15, 8,11,15,11,14,16,12,14,13,12,13,16,
            14,15,14,15,15};
}

vector<double> GraphletCosine::getOrbitWeights() {
    vector<double> res = getNumbersOfAffectedOrbits();
    for (uint i = 0; i < NUM_ORBITS; i++) {
        res[i] = 1 - log10(res[i])/log10(NUM_ORBITS);
    }
    return res;
}

double GraphletCosine::getOrbitWeightSum() {
    vector<double> orbitWeights = getOrbitWeights();
    double res = 0;
    for (uint i = 0; i < NUM_ORBITS; i++) {
        res += orbitWeights[i];
    }
    return res;
}

double magnitude(vector<uint> vector) {
	double res = 0;
	for(uint i = 0; i < vector.size(); ++i) {
		res += vector[i] * vector[i];
	}

	return sqrt(res);
}

double dot(vector<uint> v1, vector<uint> v2) {
	double res = 0;
	for(uint i = 0; i < v1.size(); ++i) {
		res += v1[i] * v2[i];
	}

	return res;
}

double cosineSimilarity(vector<uint> v1, vector<uint> v2) {
	return dot(v1, v2) / (magnitude(v1) * magnitude(v2));
}

vector<uint> reduce(vector<uint> v) {
	vector<uint> res(11);
	res[0] = v[0];
	res[1] = v[1];
	res[2] = v[2];
	res[3] = v[4];
	res[4] = v[5];
	res[5] = v[6];
	res[6] = v[7];
	res[7] = v[8];
	res[8] = v[9];
	res[9] = v[10];
	res[10] = v[11];

	return res;
}

void GraphletCosine::initSimMatrix() {
    uint n1 = G1->getNumNodes();
    uint n2 = G2->getNumNodes();
    sims = vector<vector<float> > (n1, vector<float> (n2, 0));
    vector<vector<uint> > gdvs1 = G1->loadGraphletDegreeVectors();
    vector<vector<uint> > gdvs2 = G2->loadGraphletDegreeVectors();

    vector<double> orbitWeights = getOrbitWeights();
//    double weightSum = getOrbitWeightSum();
    for (uint i = 0; i < n1; i++) {
        for (uint j = 0; j < n2; j++) {

//        	double orbitDistanceSum = 0;

            vector<uint> v1 = reduce(gdvs1[i]);
            vector<uint> v2 = reduce(gdvs2[j]);

            sims[i][j] = cosineSimilarity(v1, v2);
        }
    }
}
