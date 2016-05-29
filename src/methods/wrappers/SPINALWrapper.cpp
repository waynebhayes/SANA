#include <vector>
#include <iostream>
#include <sstream>
#include "SPINALWrapper.hpp"

using namespace std;

const string CONVERTER = "python converter_to_gml.py";
const string PROGRAM   = "./spinal";
const string OUTPUT_CONVERT  = "python spinal_output_converter.py";

SPINALWrapper::SPINALWrapper(Graph* G1, Graph* G2, string args): WrappedMethod(G1, G2, "SPINAL", args) {
	wrappedDir = "wrappedAlgorithms/SPINAL";
}

void SPINALWrapper::loadDefaultParameters() {
	parameters = "-I -n";
}

string SPINALWrapper::convertAndSaveGraph(Graph* graph, string name) {
	string gwFile  = name + ".gw";
	string gmlFile = name + ".gml";

	graph->saveInGWFormatWithNames(gwFile);

	exec("mv " + gwFile + " " + wrappedDir);

	execPrintOutput("cd " + wrappedDir + "; " + CONVERTER + " " + gwFile);
	exec("mv " + wrappedDir + "/" + gmlFile + " " + gmlFile);

	return gmlFile;
}

string SPINALWrapper::generateAlignment() {
	exec("cd " + wrappedDir + "; chmod +x " + PROGRAM);
	unsigned int sFile = parameters.find("-ns");
	if(sFile < parameters.size()) {
		sFile += 5;
		string simFile = parameters.substr(sFile, parameters.size() - parameters.find("-ns"));
		parameters = parameters.substr(0, parameters.find("-ns"));
		execPrintOutput("cd " + wrappedDir + "; " + PROGRAM + " " + parameters + " " + g1File + " " + g2File + " " + simFile + " " + alignmentTmpName + ".txt");
	} else {
		execPrintOutput("cd " + wrappedDir + "; " + PROGRAM + " " + parameters + " " + g1File + " " + g2File + " " + alignmentTmpName + ".txt");
	}
	exec("cd " + wrappedDir + "; " + OUTPUT_CONVERT + " " + alignmentTmpName + ".txt");
	return wrappedDir + "/" + alignmentTmpName + ".aln";
}

Alignment SPINALWrapper::loadAlignment(Graph* G1, Graph* G2, string fileName) {
    return Alignment::loadPartialEdgeList(G1, G2, fileName, false);
}

void SPINALWrapper::deleteAuxFiles() {
//    exec("cd " + wrappedDir + "; rm " + g1File + " " + g2File);
}
