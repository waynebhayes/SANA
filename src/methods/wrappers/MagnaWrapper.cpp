#include <vector>
#include <iostream>
#include "MagnaWrapper.hpp"

using namespace std;

const string MAGNADIR     = "wrappedAlgorithms/MAGNA++";
const string MAGNABinary = "./MAGNA++";

MagnaWrapper::MagnaWrapper(Graph* G1, Graph* G2, string args): WrappedMethod(G1, G2, "MAGNA", args) {
    wrappedDir = "wrappedAlgorithms/MAGNA++";
    outputName = g1TmpName + "_" + g2TmpName;
}

void MagnaWrapper::loadDefaultParameters() {
	parameters = "-p 15000 -n 2000 -m S3";
}

string MagnaWrapper::convertAndSaveGraph(Graph* graph, string name) {
	graph->saveInGWFormat(name + ".gw");
	return name + ".gw";
}

string MagnaWrapper::generateAlignment() {
	exec("cd " + MAGNADIR + "; chmod +x " + MAGNABinary);
	cout << g1File << " " << g2File << endl;
    string cmd = "cd wrappedAlgorithms/MAGNA++; " +
    				MAGNABinary + " -G " + g1File + " -H " + g2File + " -o " + outputName + " " + parameters;
    execPrintOutput(cmd);
    return outputName + "_final_alignment.txt";
}

Alignment MagnaWrapper::loadAlignment(Graph* G1, Graph* G2, string fileName) {
	vector<string> words = fileToStrings(MAGNADIR + "/" + fileName, false);
    vector<ushort> mapping(G1->getNumNodes(), G2->getNumNodes());
    
    for (uint i = 0; i < words.size(); i+=2) {
    	string node1 = words[i];
    	string node2 = words[i+1];
    	cout << node1 << " " << node2 << endl;
    	node1 = node1.substr(4, node1.length()-4);
    	node2 = node2.substr(4, node2.length()-4);
    	mapping[stoul(node1,nullptr)] = stoul(node2,nullptr);
    }
    return Alignment(mapping);
}

void MagnaWrapper::deleteAuxFiles() {
    exec("cd " + MAGNADIR + ";rm " + g1File + " " + g2File );
    exec("cd " + MAGNADIR + "; rm " + outputName + "*");
}
