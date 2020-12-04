#include "PINALOGWrapper.hpp"


const string PROGRAM = "./pinalog1.0";

PINALOGWrapper::PINALOGWrapper(Graph* G1, Graph* G2, string args): WrappedMethod(G1, G2, "PINALOG", args) {
	wrappedDir = "wrappedAlgorithms/PINALOG";
}

void PINALOGWrapper::loadDefaultParameters(){
	parameters = "";
}


string PINALOGWrapper::convertAndSaveGraph(Graph* graph, string name){
	name = name + ".pin";
	graph->writeGraphEdgeListFormatPINALOG(name);
	return name;
}


// Two ways of running PINALOG
// [this one is used] ./pinalog1.0 network1 network2 blastdata
//                    ./pinalog1.0 network1 network2 blastdata gene_association

string PINALOGWrapper::generateAlignment(){
    exec("cd " + wrappedDir + "; chmod +x " + PROGRAM);

//    string outFile = g1Name + "_" + g2Name + ".pinalog.nodes_algn";
    string outFile = "net1_net2.pinalog.nodes_algn";
    string cmd = g1File + " " + g2File + " " + parameters;

    execPrintOutput("cd " + wrappedDir + "; " + PROGRAM + " " + cmd);

    return wrappedDir + "/" + outFile;
}

Alignment PINALOGWrapper::loadAlignment(Graph* G1, Graph* G2, string fileName) {
    return Alignment::loadPartialEdgeList(G1, G2, fileName, true);
}

void PINALOGWrapper::deleteAuxFiles() {
    // TODO remove other generated files
    exec("cd " + wrappedDir + "; rm " + g1File + " " + g2File);
}
