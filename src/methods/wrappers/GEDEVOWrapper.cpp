#include <vector>
#include <iostream>
#include <sstream>
#include "GEDEVOWrapper.hpp"

using namespace std;

const string CONVERTER = "python GWtoNTW.py";
const string PROGRAM   = "gedevo";
const string OUTPUT_CONVERTER = "python GEDEVOoutput.py";
const string GLOBAL_PARAMETERS = "--undirected";

GEDEVOWrapper::GEDEVOWrapper(Graph* G1, Graph* G2, string args): WrappedMethod(G1, G2, "GEDEVO", args) {
	wrappedDir = "wrappedAlgorithms/GEDEVO";
}

void GEDEVOWrapper::loadDefaultParameters() {
	parameters = "--maxsame 3000 --threads 24 --pop 400"; // maxsame 3000 is what they recommend, runtime many hours
}

string GEDEVOWrapper::convertAndSaveGraph(Graph* graph, string name) {
	string gwFile  = name + ".gw";
	string ntwFile = name + ".ntw";

	graph->saveInGWFormatWithNames(gwFile);
	exec("mv " + gwFile + " " + wrappedDir + "/" + gwFile);
	exec("cd " + wrappedDir + "; " + CONVERTER + " " + gwFile + " " + ntwFile);
	exec("mv " + wrappedDir + "/" + ntwFile + " " + ntwFile);

	return ntwFile;
}

string GEDEVOWrapper::generateAlignment() {
	string g1Sigs = g1FileName + ".sigs";
	string g2Sigs = g2FileName + ".sigs";
	G1->saveGraphletsAsSigs(wrappedDir + "/" + g1Sigs);
	G2->saveGraphletsAsSigs(wrappedDir + "/" + g2Sigs);

	string newOutputFile = g1FileName + "_" + g2FileName + ".algn";

	string cmd = "--save " + alignmentFileName + " --no-save --groups " +
			g1FileName + " " + g2FileName +
			" --ntw " + g1File +
			" --ntw " + g2File +
			" --grsig " + g1Sigs + " " + g1FileName +
			" --grsig " + g2Sigs + " " + g2FileName +
			" --no-workfiles --undirected" +
			" " + parameters;

	cout << "\n\n\nrunning with: \"" + cmd + "\"" << endl << flush;

	exec("cd " + wrappedDir + "; chmod +x " + PROGRAM);
	execPrintOutput("cd " + wrappedDir + "; " + "./" + PROGRAM + " " + cmd);

	exec("cd " + wrappedDir + ";" + OUTPUT_CONVERTER + " " + alignmentFileName + ".matching " + newOutputFile);

    exec("cd " + wrappedDir + "; rm " + g1Sigs + " " + g2Sigs);
    exec("cd " + wrappedDir + "; rm " + alignmentFileName + ".matching");

	return wrappedDir + "/" + newOutputFile;
}

Alignment GEDEVOWrapper::loadAlignment(Graph* G1, Graph* G2, string fileName) {
    return Alignment::loadPartialEdgeList(G1, G2, fileName, true);
}

void GEDEVOWrapper::deleteAuxFiles() {
    exec("cd " + wrappedDir + "; rm " + g1File + " " + g2File + " *.gw" + " *.algn");
}
