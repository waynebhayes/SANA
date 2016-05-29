#include <vector>
#include <iostream>
#include <sstream>
#include "NATALIEWrapper.hpp"

using namespace std;

const string CONVERTER = "";
const string PROGRAM = "./natalie2";
const string GLOBAL_PARAMETERS = " -r 1 -t 600 ";

NATALIEWrapper::NATALIEWrapper(Graph* G1, Graph* G2, string args): WrappedMethod(G1, G2, "NATALIE", args) {
	wrappedDir = "wrappedAlgorithms/NATALIE";
}

void NATALIEWrapper::loadDefaultParameters() {
	parameters = ""; // default to 10 minutes
}

string NATALIEWrapper::convertAndSaveGraph(Graph* graph, string name) {
	name = name + ".gw";
	graph->saveInGWFormatWithNames(name);
	return name;
}

string NATALIEWrapper::generateAlignment() {
    exec("cd " + wrappedDir + "; chmod +x " + PROGRAM);

    string outFile = alignmentTmpName + "-1.tsv";
    string cmd = GLOBAL_PARAMETERS + " -g1 " + g1File +
    			 " -g2 " + g2File +
    			 " -o " + alignmentTmpName +
    			 " -if1 5" +
    			 " -if2 5" +
    			 " " + parameters + ";";
    cmd = cmd + "awk '/	typeM	/{print $1,$3}' " + alignmentTmpName + "-1.sif >" + outFile;

    execPrintOutput("cd " + wrappedDir + "; " + PROGRAM + " " + cmd);

    return wrappedDir + "/" + outFile;
}

Alignment NATALIEWrapper::loadAlignment(Graph* G1, Graph* G2, string fileName) {
    return Alignment::loadPartialEdgeList(G1, G2, fileName, true);
}

void NATALIEWrapper::deleteAuxFiles() {
    exec("cd " + wrappedDir + "; rm " + g1File + " " + g2File);
}
