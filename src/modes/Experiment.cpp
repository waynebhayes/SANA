#include "Experiment.hpp"
#include <iostream>
#include "../utils/utils.hpp"
#include "../utils/Timer.hpp"
#include "../measures/SymmetricSubstructureScore.hpp"
#include "../measures/EdgeCorrectness.hpp"
#include "../measures/LargestCommonConnectedSubgraph.hpp"
#include "../measures/NodeCorrectness.hpp"
#include "../measures/GoCoverage.hpp"
#include "../measures/ShortestPathConservation.hpp"
#include "../measures/InvalidMeasure.hpp"
#include "../measures/InducedConservedStructure.hpp"
#include "../measures/WeightedEdgeConservation.hpp"
#include "../measures/localMeasures/GoSimilarity.hpp"
#include "../measures/localMeasures/Importance.hpp"
#include "../measures/localMeasures/Sequence.hpp"
#include "../measures/localMeasures/NodeDensity.hpp"
#include "../measures/localMeasures/EdgeDensity.hpp"
#include "../measures/localMeasures/Graphlet.hpp"
#include "../Alignment.hpp"
#include "../utils/Timer.hpp"


const string Experiment::methodArgsFile = "experiments/methods.cnf";
const string Experiment::datasetsFile = "experiments/datasets.cnf";

void Experiment::run(ArgumentParser& args) {
    checkFileExists(methodArgsFile);
    checkFileExists(datasetsFile);

    experName = args.strings["-experiment"];
    experFolder = "experiments/"+experName+"/";
    experFile = experFolder+experName+".exp";
    checkFileExists(experFile);
    resultsFolder = experFolder+"results/";
    createFolder(resultsFolder);
    init();

    bool dbg = args.bools["-dbg"];
    if (not dbg) {
        makeSubmissions();        
    } else {
        printSubmissions();
    }

}

void Experiment::init() {
    vector<vector<string>> data = fileToStringsByLines(experFile);
    t = stod(data[0][0]);
    nSubs = stoi(data[1][0]);
    subPolicy = data[1][1];
    experArgs = data[2];
    methods = data[3];
    dataset = data[4][0];
    networkPairs = getNetworkPairs(dataset);
    measures = data[5];
}

void Experiment::makeSubmissions() {
    for (string method: methods) {
        for (const auto& pair: networkPairs) {
            for (uint i = 0; i < nSubs; i++) {
                string cmd = subCommand(method, pair[0], pair[1], i);
                string outFile = getOutputFileName(method, pair[0], pair[1], i);
                if (not fileExists(outFile)) {
                    cerr << "SUBMIT "+outFile << endl;
                    exec(cmd);
                } else {
                    cerr << "OMIT   "+outFile << endl;
                }
            }
        }
    }
}

void Experiment::printSubmissions() {
    for (string method: methods) {
        for (const auto& pair: networkPairs) {
            for (uint i = 0; i < nSubs; i++) {
                string cmd = subCommand(method, pair[0], pair[1], i);
                string outFile = getOutputFileName(method, pair[0], pair[1], i);
                if (not fileExists(outFile)) {
                    cout << "SUBMIT "+outFile << endl;
                } else {
                    cout << "OMIT   "+outFile << endl;
                }
                cout << cmd << endl;
            }
        }
    }
}

string Experiment::getOutputFileName(string method, string G1Name, string G2Name, uint subNum) {
    return resultsFolder+method+"_"+G1Name+"_"+G2Name+"_"+intToString(subNum);
}

string Experiment::subCommand(string method, string G1Name, string G2Name, uint subNum) {
    string command = "./sana -mode cluster -qmode normal";
    command += " -g1 " + G1Name + " -g2 " + G2Name;
    command += " -t " + to_string(t);
    for (string arg: getMethodArgs(method)) command += " " + arg;
    for (string arg: experArgs) command += " " + arg;
    string outFile = getOutputFileName(method, G1Name, G2Name, subNum);
    command += " -o " + outFile;
    return command;
}

string Experiment::getName() {
    return "Experiment";
}

Experiment::Experiment() {

}

vector<string> Experiment::getMethodArgs(string method) {
    vector<vector<string>> data = fileToStringsByLines(methodArgsFile);
    for (uint i = 0; i < data.size(); i++) {
        if (data[i][0] == method) {
            data[i].erase(data[i].begin());
            return data[i];
        }
    }
    throw runtime_error("method not found in "+methodArgsFile+": "+method);
}

vector<vector<string>> Experiment::getNetworkPairs(string dataset) {
    vector<vector<string>> data = fileToStringsByLines(datasetsFile);
    for (uint i = 0; i < data.size(); i++) {
        if (data[i].size() == 1 and data[i][0] == dataset) {
            vector<vector<string>> res(0);
            uint j = i+1;
            while (j < data.size() and data[j].size() == 2) {
                res.push_back(data[j]);
                j++;
            }
            return res;
        }
    }
    throw runtime_error("dataset not found in "+datasetsFile+": "+dataset);
}

Measure* Experiment::loadMeasure(Graph* G1, Graph* G2, string name) {
    if (name == "s3") {
        return new SymmetricSubstructureScore(G1, G2);
    }
    if (name == "ec") {
        return new EdgeCorrectness(G1, G2);
    }
    if (name == "ics") {
        return new InducedConservedStructure(G1, G2);
    }
    if (name == "lccs") {
        return new LargestCommonConnectedSubgraph(G1, G2);
    }
    if (name == "noded") {
        return new NodeDensity(G1, G2, {0.1, 0.25, 0.5, 0.15});
    }
    if (name == "edged") {
        return new EdgeDensity(G1, G2, {0.1, 0.25, 0.5, 0.15});
    }
    if (name == "graphlet") {
        return new Graphlet(G1, G2);
    }
    if (name == "wec") {
        LocalMeasure* wecNodeSim = new NodeDensity(G1, G2, {0.1, 0.25, 0.5, 0.15});
        return new WeightedEdgeConservation(G1, G2, wecNodeSim);
    }
    if (name == "go1") {
        vector<double> weights(1, 1);
        return new GoSimilarity(G1, G2, weights);
    }
    if (name == "go2") {
        vector<double> weights(2, 0);
        weights[1] = 1;
        return new GoSimilarity(G1, G2, weights);
    }
    if (name == "go3") {
        vector<double> weights(3, 0);
        weights[2] = 1;
        return new GoSimilarity(G1, G2, weights);
    }
    if (name == "go4") {
        vector<double> weights(4, 0);
        weights[3] = 1;
        return new GoSimilarity(G1, G2, weights);
    }
    if (name == "go5") {
        vector<double> weights(5, 0);
        weights[4] = 1;
        return new GoSimilarity(G1, G2, weights);
    }
    if (name == "go6") {
        vector<double> weights(6, 0);
        weights[5] = 1;
        return new GoSimilarity(G1, G2, weights);
    }
    if (name == "go7") {
        vector<double> weights(7, 0);
        weights[6] = 1;
        return new GoSimilarity(G1, G2, weights);
    }
    if (name == "go8") {
        vector<double> weights(8, 0);
        weights[7] = 1;
        return new GoSimilarity(G1, G2, weights);
    }
    if (name == "go9") {
        vector<double> weights(9, 0);
        weights[8] = 1;
        return new GoSimilarity(G1, G2, weights);
    }
    if (name == "gocov") {
        return new GoCoverage(G1, G2);
    }
    if (name == "shortestpath") {
        return new ShortestPathConservation(G1, G2);
    }
    if (name == "nc") {
        if (G1->getNumNodes() == G2->getNumNodes()) {
            return new NodeCorrectness(Alignment::identity(G1->getNumNodes()));
        }
        else {
            return new InvalidMeasure();
        }
    }
    if (name == "importance") {
        return new Importance(G1, G2);
    }
    if (name == "sequence") {
        return new Sequence(G1, G2);
    }
    throw runtime_error("Unknown measure: "+name);
}