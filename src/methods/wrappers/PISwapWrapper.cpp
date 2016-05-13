#include <vector>
#include <iostream>
#include <sstream>
#include "PISwapWrapper.hpp"

using namespace std;

const string CONVERTER = "";
const string PISwapBinary = "python piswap.py";
const string alignmentconverter = "python convertOutput.py";

PISwapWrapper::PISwapWrapper(Graph* G1, Graph* G2, string args): WrappedMethod(G1, G2, "PISWAP", args) {
	wrappedDir = "wrappedAlgorithms/PISWAP";
}

void PISwapWrapper::loadDefaultParameters() {
	cerr << "ERROR: PISWAP needs a similarity file.  Use -wrappedArgs \"similarityFile\" to specify the file" << endl;
	exit(-1);
	parameters = "";
}

string PISwapWrapper::convertAndSaveGraph(Graph* graph, string name) {
	graph->writeGraphEdgeListFormat(name + ".tab");
	return name + ".tab";
}

string PISwapWrapper::generateAlignment() {
	exec("cd " + wrappedDir + "; " + PISwapBinary + " " +  g1File + " " + g2File + " " + parameters);
	exec("cd " + wrappedDir + "; " + alignmentconverter + " match_output.txt " + alignmentTmpName);

	return wrappedDir + "/" + alignmentTmpName;
}

Alignment PISwapWrapper::loadAlignment(Graph* G1, Graph* G2, string fileName) {
    return 	Alignment::loadEdgeList(G1, G2, fileName);
}

void PISwapWrapper::deleteAuxFiles() {
    exec("cd " + wrappedDir + "; rm " + g1File + " " + g2File);
}
