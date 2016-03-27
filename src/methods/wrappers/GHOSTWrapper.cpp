#include <vector>
#include <iostream>
#include <sstream>
#include "GHOSTWrapper.hpp"

using namespace std;

const string CONVERTER = "./bio-graph";
const string GHOSTBinary = "./GHOST";

GHOSTWrapper::GHOSTWrapper(Graph* G1, Graph* G2, string args): WrappedMethod(G1, G2, "GHOST", args) {
	wrappedDir = "wrappedAlgorithms/GHOST";
}

// -wrappedArgs "matcher nneighbors nneighbors beta ratio searchiter"
void GHOSTWrapper::loadDefaultParameters() {
	parameters = "linear all 1.0 8.0 10";
}

string GHOSTWrapper::convertAndSaveGraph(Graph* graph, string name) {
	string gwFile   = name + ".gw";
	string gexfFile = name + ".gexf";

	graph->saveInGWFormat(gwFile);

	exec("mv " + gwFile + " " + wrappedDir);
	exec("cd " + wrappedDir + "; chmod +x " + CONVERTER);
	exec("cd " + wrappedDir + "; " + CONVERTER + " --convert " + gwFile + " --output " + gexfFile);
	exec("mv " + wrappedDir + "/" + gexfFile + " " + gexfFile);

	return gexfFile;
}

void GHOSTWrapper::createCfgFile(string cfgFileName) {
	vector<string> params = split(parameters, ' ');

	matcher    = params[0];
	nneighbors = params[1];
	beta       = params[2];
	ratio      = params[3];
	searchiter = params[4];

	ofstream outfile;
	outfile.open(cfgFileName.c_str());

	outfile << "[main]" << endl;
	outfile << "network1: " << g1File << endl;
	outfile << "network2: " << g2File << endl;
	outfile << "matcher: "  << matcher << endl;
	outfile << "nneighbors: "  << nneighbors << endl;
	outfile << "beta: " << beta << endl;
	outfile << "ratio: " << ratio << endl;
	outfile << "searchiter: " << searchiter << endl;

	outfile.close();
}

string GHOSTWrapper::generateAlignment() {
	cfgFile = g1FileName + ".cfg";
	createCfgFile(wrappedDir + "/" + cfgFile);

	execPrintOutput("cd " + wrappedDir + "; " + GHOSTBinary + " -c " + cfgFile);
	return " ";
}

Alignment GHOSTWrapper::loadAlignment(Graph* G1, Graph* G2, string fileName) {
    return Alignment::loadEdgeList(G1,G2,fileName);
}

void GHOSTWrapper::deleteAuxFiles() {
//    exec("cd " + wrappedDir + "; rm " + g1File + " " + g2File);
}
