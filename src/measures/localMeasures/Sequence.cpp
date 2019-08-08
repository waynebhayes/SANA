#include <vector>
#include <unordered_map>
#include <fstream>
#include <sstream>
#include <iostream>
#include <cassert>
#include "Sequence.hpp"

extern bool _graphsSwitched;

using namespace std;

Sequence::Sequence(Graph* G1, Graph* G2) : LocalMeasure(G1, G2, "sequence") {
    string subfolder = autogenMatricesFolder+getName()+"/";
    createFolder(subfolder);
    string fileName = subfolder+G1->getName()+"_"+G2->getName()+"_sequence.bin";
    loadBinSimMatrix(fileName);
}

void Sequence::generateBitscoresFile(string bitscoresFile) {
    cout << "Generating " << bitscoresFile << " ... ";
    uint n1 = G1->getNumNodes();
    uint n2 = G2->getNumNodes();
    unordered_map<uint,string> g1IndexToNodeMap = G1->getIndexToNodeNameMap();
    unordered_map<uint,string> g2IndexToNodeMap = G2->getIndexToNodeNameMap();
    ofstream outfile(bitscoresFile);
    for (uint i = 0; i < n1; i++) {
        for (uint j = 0; j < n2; j++) {
            if (sims[i][j] > 0) {
                outfile << g1IndexToNodeMap[i] << "\t" << g2IndexToNodeMap[j] << "\t" << sims[i][j] << endl;
            }
        }
    }
    cout << "done" << endl;
}

unordered_map<string,string> Sequence::initNameMap(string curatedFastaFile) {
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

void Sequence::initSimMatrix() {

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

    unordered_map<string,uint> g1NodeToIndexMap = G1->getNodeNameToIndexMap();
    unordered_map<string,uint> g2NodeToIndexMap = G2->getNodeNameToIndexMap();

    string blastFile = "sequence/scores/"+g1Name+"_"+g2Name+"_blast.out";

	if (_graphsSwitched){
    	blastFile = "sequence/scores/"+g2Name+"_"+g1Name+"_blast.out";
	}

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

		//check to see if the nodes are in the graphs first, if not then just read the rest of the line and continue...
		if (g1NodeToIndexMap.find(node1) != g1NodeToIndexMap.end() && g2NodeToIndexMap.find(node2) != g2NodeToIndexMap.end()){
			uint index1 = g1NodeToIndexMap.at(node1);
       		uint index2 = g2NodeToIndexMap.at(node2);
       		//uses bitscores, which is the last column
        	//there are other possibilities, such as
        	//-log of e-values (second-to-last value), also used in l-graal
        	string bitscore;
        	while (iss >> bitscore);
			//cout << "Node1 : " << node1 << " Node2: " << node2;
			//cout << "BITSCORE : " << bitscore << endl;
        	//assert(sims[index1][index2] == 0); //there ARE many repeated entries, not sure why
        	//keep the maximum among all repeated entries
        	//this is a rather arbitrary decision...
        	sims[index1][index2] = max(sims[index1][index2], stof(bitscore));
        	if (sims[index1][index2] > maxim) maxim = sims[index1][index2];
		}
		else {
        	string bitscore;
        	while (iss >> bitscore);
		}
    }

    //normalize
    for (uint i = 0; i < n1; i++) {
        for (uint j = 0; j < n2; j++) {
            sims[i][j] /= maxim;
        }
    }
}

Sequence::~Sequence() {
}

string Sequence::blastScoreFile(const string& G1Name, const string& G2Name) {
    return _graphsSwitched ? "sequence/scores/"+G2Name+"_"+G1Name+"_blast.out" : "sequence/scores/"+G1Name+"_"+G2Name+"_blast.out"; 
}

bool Sequence::fulfillsPrereqs(Graph* G1, Graph* G2) {
    return fileExists(blastScoreFile(G1->getName(), G2->getName()));
}
