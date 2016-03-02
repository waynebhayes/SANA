#include <vector>
#include <iostream>
#include <sstream>
#include "MIGRAALWrapper.hpp"

using namespace std;

const string MIGRAALDIR     = "wrappedAlgorithms/MI-GRAAL";
const string CONVERTER      = "ncount";
const string GHOSTBinary  = "MI-GRAAL";
const string MIGRAALProgram = "./MI-GRAALRunner.py";

MIGRAALWrapper::MIGRAALWrapper(Graph* G1, Graph* G2, string args): WrappedMethod(G1, G2, "MIGRAAL", args) {
	wrappedDir = "wrappedAlgorithms/MI-GRAAL";
    outputName = g1FileName + "_" + g2FileName;
}

void MIGRAALWrapper::loadDefaultParameters() {
	parameters = "-p 3";
}

string MIGRAALWrapper::convertAndSaveGraph(Graph* graph, string name) {
	graph->saveInGWFormat(name);
	return name;
}

string MIGRAALWrapper::generateAlignment() {
	exec("cd " + MIGRAALDIR + "; chmod +x " + CONVERTER + " " + GHOSTBinary + " " + MIGRAALProgram);

    string cmd = "cd wrappedAlgorithms/MI-GRAAL; " +
    				MIGRAALProgram + " " + g1File + " " + g2File + " " + outputName + " " + parameters;
    execPrintOutput(cmd);

    return outputName + ".aln";
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

void MIGRAALWrapper::deleteAuxFiles() {
    exec("cd " + MIGRAALDIR + ";rm " + g1File + " " + g2File);
    exec("cd " + MIGRAALDIR + "; rm " + outputName + ".* " + "tmp*");
}
