#include <vector>
#include <iostream>
#include <sstream>
#include "NATILEWrapper.hpp"

using namespace std;

const string CONVERTER = "";
const string PROGRAM = "./natalie";

NATILEWrapper::NATILEWrapper(Graph* G1, Graph* G2, string args): WrappedMethod(G1, G2, "NATALIE", args) {
	wrappedDir = "wrappedAlgorithms/NATALIE";
}

void NATILEWrapper::loadDefaultParameters() {
	parameters = "-r 1";
}

string NATILEWrapper::convertAndSaveGraph(Graph* graph, string name) {
	name = name + ".gw";
	graph->saveInGWFormat(name);
	return name;
}

string NATILEWrapper::generateAlignment() {
    exec("cd " + wrappedDir + "; chmod +x " + PROGRAM);

    string output = "out";
    string cmd = " -g1 " + g1File +
    			 " -g2 " + g2File +
    			 " -o " + output +
    			 " -if1 5" +
    			 " -if2 5" +
    			 " " + parameters;

    execPrintOutput("cd " + wrappedDir + "; " + PROGRAM + " " + cmd);

	return " ";
}

Alignment NATILEWrapper::loadAlignment(Graph* G1, Graph* G2, string fileName) {
    return Alignment::loadPartialEdgeList(G1, G2, fileName);
}

void NATILEWrapper::deleteAuxFiles() {
    exec("cd " + wrappedDir + "; rm " + g1File + " " + g2File);
}
