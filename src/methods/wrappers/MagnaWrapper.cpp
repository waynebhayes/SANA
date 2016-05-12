#include <vector>
#include <iostream>
#include "MagnaWrapper.hpp"

using namespace std;

const string MAGNADIR     = "wrappedAlgorithms/MAGNA++";
const string MAGNABinary = "./MAGNA++";

MagnaWrapper::MagnaWrapper(Graph* G1, Graph* G2, string args): WrappedMethod(G1, G2, "MAGNA", args) {
	wrappedDir = "wrappedAlgorithms/MAGNA++";
    outputName = g1FileName + "_" + g2FileName;
}

void MagnaWrapper::loadDefaultParameters() {
	parameters = "-p 15000 -n 2000 -m S3";
}

string MagnaWrapper::convertAndSaveGraph(Graph* graph, string name) {
	string gwFile = name + ".gw";
	graph->saveInGWFormat(gwFile);
	exec("mv " + gwFile + " " + wrappedDir);
	return name;
}

string MagnaWrapper::generateAlignment() {
	exec("cd " + MAGNADIR + "; chmod +x " + MAGNABinary);
	cout << g1File << " " << g2File << endl;
    string cmd = "cd wrappedAlgorithms/MAGNA++; " +
    				MAGNABinary + " -G" + g1File + ".gw -H" + g2File + ".gw -o" + outputName + " " + parameters;
    execPrintOutput(cmd);

    return outputName + ".aln";
}

Alignment MagnaWrapper::loadAlignment(Graph* G1, Graph* G2, string fileName) {
	vector<string> words = fileToStrings(fileName);
    vector<ushort> mapping(G1->getNumNodes(), G2->getNumNodes());

    for (uint i = 0; i < words.size(); i+=2) {
    	string node1 = words[i];
    	string node2 = words[i+1];
    	cout << node1 << " " << node2 << endl;
        mapping[atoi(node1.c_str()) - 1] = atoi(node2.c_str()) - 1;
    }
    return Alignment(mapping);
}

void MagnaWrapper::deleteAuxFiles() {
    exec("cd " + MAGNADIR + ";rm " + g1File + " " + g2File);
    exec("cd " + MAGNADIR + "; rm " + outputName + ".* " + "tmp*");
}
