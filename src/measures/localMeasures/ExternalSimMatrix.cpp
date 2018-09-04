#include <vector>
#include <iostream>
#include "ExternalSimMatrix.hpp"
#include "../../utils/utils.hpp"
#include <string>
using namespace std;

ExternalSimMatrix::ExternalSimMatrix(Graph* G1, Graph* G2, string file, int format): LocalMeasure(G1, G2, "esim") {
    this->file = file;
    this->format =format;
    string fileName = autogenMatricesFolder+G1->getName()+"_"+
        G2->getName()+"_esim_"+file;
    loadBinSimMatrix(fileName);
}

void ExternalSimMatrix::initSimMatrix() {
    uint n1 = G1->getNumNodes();
    uint n2 = G2->getNumNodes();
    sims = vector<vector<float> > (n1, vector<float> (n2, 0));

    switch(format) {
    case 0:    loadFormat0();    break;
    case 1:    loadFormat1();    break;
    case 2: loadFormat2();  break;
    default:                break;
    }

}

void ExternalSimMatrix::loadFormat0() {
    vector<string> words = fileToStrings(file, false);

    for(uint i = 0; i < words.size(); i +=3) {
        int n = atoi(words[i].c_str());
        int m = atoi(words[i+1].c_str());
        sims[n][m] = stod(words[i+2]);
     }
}

void ExternalSimMatrix::loadFormat1() {
    unordered_map<string,uint> g1Map = G1->getNodeNameToIndexMap();
    unordered_map<string,uint> g2Map = G2->getNodeNameToIndexMap();
    vector<string> words = fileToStrings(file, false);

    for(uint i = 0; i < words.size(); i +=3) {
        uint n = g1Map[words[i]];
        uint m = g2Map[words[i+1]];
        sims[n][m] = stod(words[i+2]);
    }
}

void ExternalSimMatrix::loadFormat2() {
    vector<string> words = fileToStrings(file, false);
    for(uint i = 0; i < G1->getNumNodes(); i++){
        for(uint j = 0; j < G2->getNumNodes(); j++){
            double sim = stod(words[i*G2->getNumNodes() + j]);
            sims[i][j] = sim;
        }
    }
}

ExternalSimMatrix::~ExternalSimMatrix() {
}
