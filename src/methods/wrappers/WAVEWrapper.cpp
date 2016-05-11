#include <vector>
#include <iostream>
#include <sstream>
#include "WAVEWrapper.hpp"

using namespace std;

const string PROGRAM = "./WAVE";

WAVEWrapper::WAVEWrapper(Graph* G1, Graph* G2, string args): WrappedMethod(G1, G2, "WAVE", args) {
	wrappedDir = "wrappedAlgorithms/WAVE";
}

void WAVEWrapper::loadDefaultParameters() {
	cerr << "ERROR: WAVE needs a similarity file.  Use -wrappedArgs \"similarityFile\" to specify the file" << endl;
	exit(-1);
	parameters = "";
}

string WAVEWrapper::convertAndSaveGraph(Graph* graph, string name) {
	graph->writeGraphEdgeListFormat(name);
	return name;
}

string WAVEWrapper::generateAlignment() {
	exec("cd " + wrappedDir + "; chmod +x " + PROGRAM);
	exec("cd " + wrappedDir + "; " + PROGRAM + " " + g1File + " " + g2File + " " + parameters + " " + alignmentFileName);

	return wrappedDir + "/" + alignmentFileName;
}

Alignment WAVEWrapper::loadAlignment(Graph* G1, Graph* G2, string fileName) {
    return Alignment::loadEdgeList(G1, G2, fileName);
}

void WAVEWrapper::deleteAuxFiles() {
    exec("cd " + wrappedDir + "; rm " + g1File + " " + g2File);
}
