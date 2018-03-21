#include "WAVEWrapper.hpp"

using namespace std;

const string PROGRAM = "./run-WAVE.sh";

WAVEWrapper::WAVEWrapper(Graph* G1, Graph* G2, string args): WrappedMethod(G1, G2, "WAVE", args) {
    wrappedDir = "wrappedAlgorithms/WAVE";
}

void WAVEWrapper::loadDefaultParameters() {
    cout << "ERROR: WAVE needs a similarity file.  Use -wrappedArgs \"similarityFile\" to specify the file" << endl;
    exit(-1);
    parameters = "";
}

string WAVEWrapper::convertAndSaveGraph(Graph* graph, string name) {
    graph->saveInGWFormatWithNames(name + ".gw");
    return name+".gw";
}

string WAVEWrapper::generateAlignment() {
    // exec("cd " + wrappedDir + "; chmod +x " + PROGRAM);
    exec("cd " + wrappedDir + "; " + PROGRAM + " " + g1File + " " + g2File + " " + "../../" + parameters + " " + alignmentTmpName);

    return wrappedDir + "/" + alignmentTmpName + ".align";
}

Alignment WAVEWrapper::loadAlignment(Graph* G1, Graph* G2, string fileName) {
    return Alignment::loadPartialEdgeList(G1, G2, fileName, true);
}

void WAVEWrapper::deleteAuxFiles() {
    //exec("cd " + wrappedDir + "; rm " + g1File + " " + g2File);
}
