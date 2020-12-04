#include <vector>
#include <unordered_map>
#include <fstream>
#include <sstream>
#include <iostream>
#include <cassert>
#include "MultiSequence.hpp"

using namespace std;

MultiSequence::MultiSequence(vector<Graph>* GV) : MultiLocalMeasure(GV, "multisequence"), graphs(GV) {
    string fileName = autogenMatricesFolder;
    for (uint i = 0; i < (*GV).size(); i++) {
        fileName = fileName + (*GV)[i].getName();
    }
    fileName = fileName + +"_sequence.bin";
    loadBinSimMatrix(fileName);
}

void MultiSequence::generateBitscoresFile(string bitscoresFile) {
    cerr << "Generating " << bitscoresFile << " ... ";
    for (uint i = 0; i < (*graphs).size() - 1; i++) {
        for (uint j = i + 1; j < (*graphs).size(); j++) {
            uint n1 = (*graphs)[i].getNumNodes();
            uint n2 = (*graphs)[j].getNumNodes();
            map<ushort,string> g1IndexToNodeMap = (*graphs)[i].getIndexToNodeNameMap();
            map<ushort,string> g2IndexToNodeMap = (*graphs)[j].getIndexToNodeNameMap();
            ofstream outfile(bitscoresFile);
            for (uint m = 0; m < n1; m++) {
                for (uint n = 0; n < n2; n++) {
                    if (sims[i][j][m][n] > 0) {
                        outfile << g1IndexToNodeMap[m] << "\t" << g2IndexToNodeMap[n] << "\t" << sims[i][j][m][n] << endl;
                    }
                }
            }
        }
    }
    cerr << "done" << endl;
}

unordered_map<string,string> MultiSequence::initNameMap(string curatedFastaFile) {
    unordered_map<string,string> res;
    ifstream infile(curatedFastaFile);
    string line;
    while (getline(infile, line)) {
        istringstream iss(line);
        string name1;
        iss >> name1;
        name1 = name1.substr(1); //remove leading '>'
        string name2;
        iss >> name2;
        while (name2.substr(0,3) != "GN=") iss >> name2;
        name2 = name2.substr(3); //remove leading 'GN='
        assert(res.count(name1) == 0);
        res[name1] = name2;
        getline(infile, line); //line containing the actual sequence
    }
    return res;
}

void MultiSequence::initSimMatrix() {
    /*
	string g1Name = G1->getName();
    string g2Name = G2->getName();
    string g1CuratedFastaFile = "sequence/"+g1Name+".fasta";
    string g2CuratedFastaFile = "sequence/"+g2Name+".fasta";
    bool g1NeedNameMap = fileExists(g1CuratedFastaFile);
    bool g2NeedNameMap = fileExists(g2CuratedFastaFile);
    unordered_map<string,string> g1NameMap;
    unordered_map<string,string> g2NameMap;

    if (g1NeedNameMap) g1NameMap = initNameMap(g1CuratedFastaFile);
    if (g2NeedNameMap) g2NameMap = initNameMap(g2CuratedFastaFile);

    uint n1 = G1->getNumNodes();
    uint n2 = G2->getNumNodes();
    sims = vector<vector<float> > (n1, vector<float> (n2, 0));

    map<string,ushort> g1NodeToIndexMap = G1->getNodeNameToIndexMap();
    map<string,ushort> g2NodeToIndexMap = G2->getNodeNameToIndexMap();

    string blastFile = "sequence/scores/"+g1Name+"_"+g2Name+"_blast.out";
    if (not fileExists(blastFile)) {
        throw runtime_error("Cannot find sequence scores for "+g1Name+"-"+g2Name);
    }
    ifstream infile(blastFile);
	*/
	
    uint numGraph = (*graphs).size();
    uint maxNumNodes = 0;
    for (uint i = 0; i < numGraph; i++) {
        if (maxNumNodes < (*graphs)[i].getNumNodes()) {
            maxNumNodes = (*graphs)[i].getNumNodes();
        }
    }
	
    sims = vector<vector<vector<vector<float> > > > (numGraph, vector<vector<vector<float> > > (numGraph, vector<vector<float> > (maxNumNodes, vector<float> (maxNumNodes, 0))));

    for (uint i = 0; i < numGraph - 1; i++) {
        for (uint j = i + 1; j < numGraph; j++) {
            string g1Name = (*graphs)[i].getName();
            string g2Name = (*graphs)[j].getName();
            string g1CuratedFastaFile = "sequence/"+g1Name+".fasta";
            string g2CuratedFastaFile = "sequence/"+g2Name+".fasta";
            bool g1NeedNameMap = fileExists(g1CuratedFastaFile);
            bool g2NeedNameMap = fileExists(g2CuratedFastaFile);
            unordered_map<string,string> g1NameMap;
            unordered_map<string,string> g2NameMap;
			
            if (g1NeedNameMap) g1NameMap = initNameMap(g1CuratedFastaFile);
            if (g2NeedNameMap) g2NameMap = initNameMap(g2CuratedFastaFile);
			
            map<string,ushort> g1NodeToIndexMap = (*graphs)[i].getNodeNameToIndexMap();
            map<string,ushort> g2NodeToIndexMap = (*graphs)[j].getNodeNameToIndexMap();

            string blastFile = "sequence/scores/"+g1Name+"_"+g2Name+"_blast.out";
            if (not fileExists(blastFile)) {
                throw runtime_error("Cannot find sequence scores for "+g1Name+"-"+g2Name);
            }
            ifstream infile(blastFile);
            string line;
            float maxim = -1;
            while (getline(infile, line)) {
                istringstream iss(line);
                string node1, node2;
                iss >> node1 >> node2;
                if (g1NeedNameMap) node1 = g1NameMap[node1];
                if (g2NeedNameMap) node2 = g2NameMap[node2];
                ushort index1 = g1NodeToIndexMap.at(node1);
                ushort index2 = g2NodeToIndexMap.at(node2);
                //uses bitscores, which is the last column
                //there are other possibilities, such as
                //-log of e-values (second-to-last value), also used in l-graal
                string bitscore;
                while (iss >> bitscore);
                //assert(sims[index1][index2] == 0); //there ARE many repeated entries, not sure why
                //keep the maximum among all repeated entries
                //this is a rather arbitrary decision...
                sims[i][j][index1][index2] = max(sims[i][j][index1][index2], stof(bitscore));
                if (sims[i][j][index1][index2] > maxim) maxim = sims[i][j][index1][index2];
            }
			//normalize
            for (uint m = 0; m < maxNumNodes; m++) {
                for (uint n = 0; n < maxNumNodes; n++) {
                    sims[i][j][m][n] /= maxim;
                }
            }
        }
    }
}

MultiSequence::~MultiSequence() {
}

/*
string Sequence::blastScoreFile(const string& G1Name, const string& G2Name) {
    return "sequence/scores/"+G1Name+"_"+G2Name+"_blast.out";
}

bool Sequence::fulfillsPrereqs(Graph* G1, Graph* G2) {
    return fileExists(blastScoreFile(G1->getName(), G2->getName()));
}*/
