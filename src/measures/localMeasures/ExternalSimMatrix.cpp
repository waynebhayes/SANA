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
    FILE* fp;
    bool isPipe = false;
    uint fileNameLength = file.size();
    uint n1 = G1->getNumNodes();
    uint n2 = G2->getNumNodes();
    sims = vector<vector<float> > (n1, vector<float> (n2, 0));

    checkFileExists(file);

    if (fileNameLength > 3 && file.substr(fileNameLength-3,3) == ".gz") {
        fp = decompressFile("gunzip", file);
        isPipe = true;
    } else if (fileNameLength > 3 && file.substr(fileNameLength-3,3) == ".xz") {
        fp = decompressFile("xzcat", file);
        isPipe = true;
    } else {
        fp = fopen(file.c_str(), "r");
    }

    if (fp == NULL) {
        throw runtime_error("ExternalSimMatrix: Error opening file");
    }

    switch(format) {
    case 0:     loadFormat0(fp);    break;
    case 1:     loadFormat1(fp);    break;
    case 2: loadFormat2(fp);    break;
    default:                    break;
    }

    if (isPipe) pclose(fp);
    else fclose(fp);
}

void ExternalSimMatrix::loadFormat0(FILE* infile) {
    uint lineCount = 0;
    uint i;
    uint j;
    float value;

    while (fscanf(infile, "%ud %ud %f", &i, &j, &value) == 3) {
        sims[i][j] = value;
        ++lineCount;
    }

    if (lineCount != getNumEntries()) 
        throw runtime_error("ExternalSimMatrix: Did not find the expected number of entries in the sim file.");
}

void ExternalSimMatrix::loadFormat1(FILE* infile) {
    uint lineCount = 0;
    char buf1[1024];
    char buf2[1024];
    float value;

    unordered_map<string,uint> g1Map = G1->getNodeNameToIndexMap();
    unordered_map<string,uint> g2Map = G2->getNodeNameToIndexMap();

    while (fscanf(infile, "%s %s %f", buf1, buf2, &value) == 3) {
        uint i = g1Map[string(buf1)];
        uint j = g2Map[string(buf2)];
        sims[i][j] = value;
        ++lineCount;
    }

    if (lineCount != getNumEntries()) 
        throw runtime_error("ExternalSimMatrix: Did not find the expected number of entries in the sim file.");
}

void ExternalSimMatrix::loadFormat2(FILE* infile) {
    float value;
    uint entryCount = 0;
    for(uint i = 0; i < G1->getNumNodes(); i++){
        for(uint j = 0; j < G2->getNumNodes(); j++){
            entryCount += fscanf(infile, "%f", &value);
            sims[i][j] = value;  
        }
    }

    if ((fscanf(infile, "%f", &value) != EOF) || entryCount != getNumEntries())
        throw runtime_error("ExternalSimMatrix: Did not find the expected number of entries in the sim file.");
}

ExternalSimMatrix::~ExternalSimMatrix() {
}
