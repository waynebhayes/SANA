#include "CytoGEDEVOWrapper.hpp"

using namespace std;

const string CONVERTER = "";
const string PROGRAM   = "cytogedevo";
const string OUTPUT_CONVERTER = "./CytoGEDEVOoutput.sh";
const string GLOBAL_PARAMETERS = " --undirected --pop 400 ";

//ARGUMENTS: --maxsecs <seconds> --blastpairlist [3 columns] --pop [400] --threads <N> [recommended runtime is "--maxsame 3000"]

CytoGEDEVOWrapper::CytoGEDEVOWrapper(Graph* G1, Graph* G2, string args): WrappedMethod(G1, G2, "GEDEVO", args) {
    wrappedDir = "wrappedAlgorithms/CytoGEDEVO";
}

void CytoGEDEVOWrapper::loadDefaultParameters() {
    parameters = "--maxsame 3000"; // maxsame 3000 is what they recommend, runtime many hours
}

string CytoGEDEVOWrapper::convertAndSaveGraph(Graph* graph, string name) {
    graph->writeGraphEdgeListFormatPISWAP(name + ".txt" );
    return name + ".txt";
}

string CytoGEDEVOWrapper::generateAlignment() {
    /*string g1Sigs = g1TmpName + ".sigs";
    string g2Sigs = g2TmpName + ".sigs";
    G1->saveGraphletsAsSigs(wrappedDir + "/" + g1Sigs);
    G2->saveGraphletsAsSigs(wrappedDir + "/" + g2Sigs);
    */
    string cmd = GLOBAL_PARAMETERS + " --save " + alignmentTmpName +
            ".gedevo --edgelisth " + g1File +
            " --edgelisth " + g2File +
            " " + parameters;

    cout << "\n\n\nrunning with: \"" + cmd + "\"" << endl << flush;

    // exec("cd " + wrappedDir + "; chmod +x " + PROGRAM);
    execPrintOutput("cd " + wrappedDir + "; " + "./" + PROGRAM + " " + cmd);

    exec("cd " + wrappedDir + ";" + OUTPUT_CONVERTER + " " + alignmentTmpName + ".gedevo " + alignmentTmpName);

    //exec("cd " + wrappedDir + "; rm " + g1Sigs + " " + g2Sigs);
    //exec("cd " + wrappedDir + "; rm " + alignmentTmpName + ".matching");

    return wrappedDir + "/" + alignmentTmpName;
}

Alignment CytoGEDEVOWrapper::loadAlignment(Graph* G1, Graph* G2, string fileName) {
    return Alignment::loadPartialEdgeList(G1, G2, fileName, true);
}

void CytoGEDEVOWrapper::deleteAuxFiles() {
    //exec("cd " + wrappedDir + "; rm -f " + g1File + " " + g2File + " *.gw" + " *.algn");
}
