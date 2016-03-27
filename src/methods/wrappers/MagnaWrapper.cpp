#include <vector>
#include <iostream>
#include "MagnaWrapper.hpp"

using namespace std;

const string MAGNAPROGRAM = "./MAGNA";

MagnaWrapper::MagnaWrapper(Graph* G1, Graph* G2, string args): WrappedMethod(G1, G2, "MAGNA", args) {
    wrappedDir = "wrappedAlgorithms/MAGNA";
}

// p: Population size - 15000
// n: Number of Generations - 2000
// t: threads - 1
// m: optimizing measure - S3
// o: output file
// d: node comparison data file
// a: alpha value
// f: frequency of output
void MagnaWrapper::loadDefaultParameters() {
    parameters = "-p 15000 -n 2000 -t 1 -m S3";
}

string MagnaWrapper::convertAndSaveGraph(Graph* graph, string name) {
    graph->writeGraphEdgeListFormatMAGNA(name);
    return name;
}

string MagnaWrapper::generateAlignment() {
    // Give the program execute permissions
    exec("cd " + wrappedDir + "; chmod +x "+ MAGNAPROGRAM);

    // Run the program in the wrappedDir with the parameters
    string cmd = "cd " + wrappedDir + ";" + MAGNAPROGRAM + " " + g1File + " " + g2File + " " + parameters;
    execPrintOutput(cmd);

    // Rename the alignment file
    string oldName = "\\(" + g1File + "-" + g2File + "*.alignment";
    string newName = g1File + "-" + g2File + ".alignment";
    exec("cd " + wrappedDir + "; mv " + oldName + " " + newName);

    return wrappedDir + "/" + newName;
}

Alignment MagnaWrapper::loadAlignment(Graph* G1, Graph* G2, string fileName) {
    vector<string> lines = fileToStrings(fileName, true);
    string word;
    vector<ushort> mapping(G1->getNumNodes(), G2->getNumNodes());

    for (uint i = 0; i < lines.size(); ++i) {
        istringstream line(lines[i]);
        vector<string> words;
        while (line >> word) words.push_back(word);

        if (words.size() == 3) {
            mapping[atoi(words[0].c_str())] = atoi(words[2].c_str());
        }
    }
    return Alignment(mapping);
}

void MagnaWrapper::deleteAuxFiles() {
    string evalFile = "\\(" + g1File + "-" + g2File + "*.eval";

    exec("cd " + wrappedDir + "; rm " + g1File + " " + g2File + " " + alignmentFile +
         " " + evalFile + " simLog.txt alignmentDetails.txt" );
}
