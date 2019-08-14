#include <vector>
#include <iostream>
#include <cmath>
#include "GraphletCosine.hpp"
using namespace std;

GraphletCosine::GraphletCosine(Graph* G1, Graph* G2) : LocalMeasure(G1, G2, "graphletcosine") {
    string subfolder = autogenMatricesFolder+getName()+"/";
    createFolder(subfolder);
    string fileName = subfolder+G1->getName()+"_"+G2->getName()+"_graphletcosine.bin";
    loadBinSimMatrix(fileName);
}

GraphletCosine::~GraphletCosine() {}

double GraphletCosine::magnitude(vector<uint> &vector) {
    double res = 0;
    for(uint i = 0; i < vector.size(); ++i) {
        res += vector[i] * static_cast<double>(vector[i]);
    }

    return sqrt(res);
}

double GraphletCosine::dot(vector<uint> &v1, vector<uint> &v2) {
    double res = 0;
    for(uint i = 0; i < v1.size(); ++i) {
        res += v1[i] * static_cast<double>(v2[i]);
    }

    return res;
}

double GraphletCosine::cosineSimilarity(vector<uint> &v1, vector<uint> &v2) {
    return dot(v1, v2) / (magnitude(v1) * magnitude(v2));
}

vector<uint> GraphletCosine::reduce(vector<uint> &v) {
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

static bool shouldReduce = false;

void GraphletCosine::initSimMatrix() {
    uint n1 = G1->getNumNodes();
    uint n2 = G2->getNumNodes();
    sims = vector<vector<float> > (n1, vector<float> (n2, 0));
    vector<vector<uint> > gdvs1 = G1->loadGraphletDegreeVectors();
    vector<vector<uint> > gdvs2 = G2->loadGraphletDegreeVectors();

    for (uint i = 0; i < n1; i++) {
        for (uint j = 0; j < n2; j++) {


            if (shouldReduce) {
                vector<uint> v1 = reduce(gdvs1[i]);
                vector<uint> v2 = reduce(gdvs2[j]);

                sims[i][j] = cosineSimilarity(v1, v2);
            } else {
                sims[i][j] = cosineSimilarity(gdvs1[i], gdvs2[j]);
            }
        }
    }
}
