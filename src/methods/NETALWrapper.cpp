#include <vector>
#include <iostream>
#include <sstream>
#include "NETALWrapper.hpp"
#include "../measures/localMeasures/Sequence.hpp"

using namespace std;

const string NETALWrapper::NETALProgram = "./NETAL/NETAL";

NETALWrapper::NETALWrapper(Graph* G1, Graph* G2): Method(G1, G2, "NETAL"),
    g1Name(G1->getName()), g2Name(G2->getName()) {

    createFolder("sequence");
    createFolder("sequence/bitscores");
    similarityFile = "sequence/bitscores/" + g1Name + "_" + g2Name + ".bitscores";

    //rand int used to avoid collision between parallel runs
    //these files cannot be moved to the tmp/ folder
    g1EdgeListFile = "netaltmp1_"+g1Name+"_"+g2Name+"_"+intToString(randInt(0, 999999));
    g2EdgeListFile = "netaltmp2_"+g1Name+"_"+g2Name+"_"+intToString(randInt(0, 999999));

    //this file cannot be moved to the tmp/ folder
    alignmetFile = g1EdgeListFile + "-" + g2EdgeListFile + ".alignment";
}

void NETALWrapper::generateAlignment() {
    exec("chmod +x "+NETALProgram);
    string cmd = NETALProgram + " " + g1EdgeListFile + " " + g2EdgeListFile;
    cerr << "Executing " << cmd << endl;
    execPrintOutput(cmd);
    cerr << "Done" << endl;
    exec("mv \\(" + g1EdgeListFile + "-" + g2EdgeListFile + "*.alignment " + g1EdgeListFile + "-" + g2EdgeListFile + ".alignment" );
}

void NETALWrapper::deleteAuxFiles() {
    string evalFile = "\\(" + g1EdgeListFile + "-" + g2EdgeListFile + "*.eval";
    exec("rm " + g1EdgeListFile + " " + g2EdgeListFile + " " + evalFile + " " + alignmetFile);
}

Alignment NETALWrapper::run() {
    G1->writeGraphEdgeListFormatNETAL(g1EdgeListFile);
    G2->writeGraphEdgeListFormatNETAL(g2EdgeListFile);

    generateAlignment();

    Alignment A = loadAlignment(G1, G2, alignmetFile);
    deleteAuxFiles();
    return A;
}


Alignment NETALWrapper::loadAlignment(Graph* G1, Graph* G2, string fileName) {
    vector<string> lines = fileToStrings(fileName, true);
    string word;
    vector<ushort> mapping(G1->getNumNodes(), G2->getNumNodes());

    for (uint i = 0; i < lines.size(); ++i) {
    	istringstream line(lines[i]);
        vector<string> words;
        while (line >> word) words.push_back(word);

        if (words.size() == 3) {
        	mapping[atoi(words[0].c_str())] = atoi(words[2].c_str());
        }
    }
    return Alignment(mapping);
}

void NETALWrapper::describeParameters(ostream& stream) {
}

string NETALWrapper::fileNameSuffix(const Alignment& A) {
    return "";
}
