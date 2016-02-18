#include <vector>
#include <iostream>
#include <sstream>
#include "MIGRAALWrapper.hpp"

using namespace std;

const string MIGRAALDIR = "wrappedAlgorithms/MI-GRAAL";
const string NCOUNT = "ncount";
const string MIGRAALBinary = "MI-GRAAL";
const string MIGRAALProgram = "./MI-GRAALRunner.py";

MIGRAALWrapper::MIGRAALWrapper(Graph* G1, Graph* G2): Method(G1, G2, "MIGRAAL"),
    g1Name(G1->getName()), g2Name(G2->getName()) {

	//rand int used to avoid collision between parallel runs
    //these files cannot be moved to the tmp/ folder
    g1GWFile = "migraaltmp1_"+g1Name+"_"+g2Name+"_"+intToString(randInt(0, 999999));
    g2GWFile = "migraaltmp2_"+g1Name+"_"+g2Name+"_"+intToString(randInt(0, 999999));

    outputName = "res";

    //this file cannot be moved to the tmp/ folder
    alignmetFile = MIGRAALDIR + "/" + outputName + ".aln";
}

void MIGRAALWrapper::generateAlignment() {
	exec("cd " + MIGRAALDIR + "; chmod +x " + NCOUNT + " " + MIGRAALBinary + " " + MIGRAALProgram);

    string cmd = "cd wrappedAlgorithms/MI-GRAAL; " + MIGRAALProgram
    			+ " ../../" + g1GWFile + " ../../" + g2GWFile + " " + outputName + " " + "-p" + " 3";
    cerr << "Executing " << cmd << endl;
    execPrintOutput(cmd);
    cerr << "Done" << endl;
}

void MIGRAALWrapper::deleteAuxFiles() {
    exec("rm " + g1GWFile + " " + g2GWFile);
    exec("cd " + MIGRAALDIR + "; rm " + outputName + ".* " + "tmp*");
}

Alignment MIGRAALWrapper::run() {
	G1->saveInGWFormat(g1GWFile);
	G2->saveInGWFormat(g2GWFile);

    generateAlignment();

    Alignment A = loadAlignment(G1, G2, alignmetFile);
    deleteAuxFiles();
    return A;
}

Alignment MIGRAALWrapper::loadAlignment(Graph* G1, Graph* G2, string fileName) {
    vector<string> words = fileToStrings(fileName);
    vector<ushort> mapping(G1->getNumNodes(), G2->getNumNodes());

    for (uint i = 0; i < words.size(); i+=2) {
    	string node1 = words[i];
    	string node2 = words[i+1];
    	node1 = node1.substr(4, node1.length()); // The word "node" is listed before every node
    	node2 = node2.substr(4, node2.length());
    	cout << node1 << " " << node2 << endl;
        mapping[atoi(node1.c_str()) - 1] = atoi(node2.c_str()) - 1;
    }
    return Alignment(mapping);
}

void MIGRAALWrapper::describeParameters(ostream& stream) {
}

string MIGRAALWrapper::fileNameSuffix(const Alignment& A) {
    return "";
}
