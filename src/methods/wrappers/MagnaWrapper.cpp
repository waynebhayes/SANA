#include "MagnaWrapper.hpp"

using namespace std;

const string MAGNADIR     = "wrappedAlgorithms/MAGNA++";
const string MAGNABinary = "./MAGNA++";
const string GLOBAL_PARAMETERS = " -p 15000 -n 2000 -m S3 ";

MagnaWrapper::MagnaWrapper(Graph* G1, Graph* G2, string args): WrappedMethod(G1, G2, "MAGNA", args) {
    wrappedDir = "wrappedAlgorithms/MAGNA++";
    outputName = g1TmpName + "_" + g2TmpName;
}

void MagnaWrapper::loadDefaultParameters() {
	parameters = "";
}

string MagnaWrapper::convertAndSaveGraph(Graph* graph, string name) {
	graph->saveInGWFormatWithNames(name + ".gw");
	return name + ".gw";
}

string MagnaWrapper::generateAlignment() {
    exec("cd " + MAGNADIR + "; chmod +x " + MAGNABinary);
    cout << g1File << " " << g2File << endl;
    string cmd = "cd wrappedAlgorithms/MAGNA++; " + MAGNABinary + " -G " + g1File + " -H " + g2File +
	    " -o " + outputName + " " + GLOBAL_PARAMETERS + parameters;
    execPrintOutput(cmd);
    return outputName + "_final_alignment.txt";
}

Alignment MagnaWrapper::loadAlignment(Graph* G1, Graph* G2, string fileName) {
    vector<string> words = fileToStrings(MAGNADIR + "/" + fileName, false);
    vector<ushort> mapping(G1->getNumNodes(), G2->getNumNodes());
    map<string,ushort> node1Map = G1->getNodeNameToIndexMap();
    map<string,ushort> node2Map = G2->getNodeNameToIndexMap();
    
    for (uint i = 0; i < words.size(); i+=2) {
    	string node1 = words[i];
    	string node2 = words[i+1];
    	cout << node1 << " " << node2 << endl;
    	mapping[node1Map[node1]] = node2Map[node2];
    }
    return Alignment(mapping);
}

void MagnaWrapper::deleteAuxFiles() {
    exec("cd " + MAGNADIR + ";rm " + g1File + " " + g2File );
    exec("cd " + MAGNADIR + "; rm " + outputName + "*");
}
