#include "MIGRAALWrapper.hpp"

using namespace std;

const string MIGRAALDIR     = "wrappedAlgorithms/MI-GRAAL";
const string CONVERTER      = "ncount";
const string GHOSTBinary  = "MI-GRAAL";
const string MIGRAALProgram = "./MI-GRAALRunner.py";

MIGRAALWrapper::MIGRAALWrapper(Graph* G1, Graph* G2, string args): WrappedMethod(G1, G2, "MIGRAAL", args) {
    wrappedDir = "wrappedAlgorithms/MI-GRAAL";
}

void MIGRAALWrapper::loadDefaultParameters() {
    parameters = "-p 3";
}

string MIGRAALWrapper::convertAndSaveGraph(Graph* graph, string name) {
    graph->saveInGWFormatWithNames(name);
    return name;
}

string MIGRAALWrapper::generateAlignment() {

    exec("cd " + MIGRAALDIR + "; chmod +x " + CONVERTER + " " + GHOSTBinary + " " + MIGRAALProgram);

    string cmd = "cd wrappedAlgorithms/MI-GRAAL; " +
                    MIGRAALProgram + " " + g1File + " " + g2File + " " + alignmentTmpName + " " + parameters;
    execPrintOutput(cmd);

    return wrappedDir + "/" + alignmentTmpName + ".aln";
}

Alignment MIGRAALWrapper::loadAlignment(Graph* G1, Graph* G2, string fileName) {
    vector<string> words = fileToStrings(fileName);
    vector<uint> mapping(G1->getNumNodes(), G2->getNumNodes());
    unordered_map<string, uint> g1nodeMap = G1->getNodeNameToIndexMap();
    unordered_map<string, uint> g2nodeMap = G2->getNodeNameToIndexMap();

    for (uint i = 0; i < words.size(); i+=2) {
        string node1 = words[i];
        string node2 = words[i+1];
        cout << node1 << " " << node2 << endl;
        mapping[g1nodeMap[node1]] = g2nodeMap[node2];
    }
    return Alignment(mapping);
}

void MIGRAALWrapper::deleteAuxFiles() {
    //output name appears to be empty
    //exec("cd " + MIGRAALDIR + ";rm " + g1File + " " + g2File);
    //exec("cd " + MIGRAALDIR + "; rm " + outputName + ".* " + "tmp*");
}
