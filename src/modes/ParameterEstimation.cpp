#include <cassert>
#include <sstream>
#include <map>
#include "ParameterEstimation.hpp"
#include "Experiment.hpp"
#include "../measures/SymmetricSubstructureScore.hpp"
#include "../measures/JaccardSimilarityScore.hpp"
#include "../measures/EdgeCorrectness.hpp"
#include "../measures/LargestCommonConnectedSubgraph.hpp"
#include "../measures/NodeCorrectness.hpp"
#include "../measures/NetGO.hpp"
#include "../measures/ShortestPathConservation.hpp"
#include "../measures/InvalidMeasure.hpp"
#include "../measures/localMeasures/GoSimilarity.hpp"
#include "../Alignment.hpp"
#include "../utils/Timer.hpp"

const int ParameterEstimation::PRECISION_DECIMALS = 6;
const double ParameterEstimation::minutes = 60;
const string ParameterEstimation::method = "sac";
const string ParameterEstimation::projectFolder = "/extra/wayne0/preserve/nmamano/networkalignment";

void ParameterEstimation::run(ArgumentParser& args) {
    string paramEstimation = args.strings["-paramestimation"];
    string experFile = "experiments/"+paramEstimation+".exp";
    assert(fileExists(experFile));

    init(experFile);

    if (args.bools["-submit"]) {
      submitScriptsToCluster();
    }
    else {
      collectData();
      printData("experiments/"+paramEstimation+".out");
    }
    exit(0);
}

ParameterEstimation::ParameterEstimation(){}

ParameterEstimation::ParameterEstimation(string parameterEstimationFile) {
    init(parameterEstimationFile);
}

void ParameterEstimation::init(string parameterEstimationFile) {
    vector<vector<string> > content = fileToStringsByLines(parameterEstimationFile);
    Graph::loadGraph(content[0][0], G1);
    Graph::loadGraph(content[0][1], G2);
    measureName = content[1][0];
    measure = Experiment::loadMeasure(&G1, &G2, measureName);

    kValues = vector<double> (content[2].size());
    for (uint i = 0; i < kValues.size(); i++) {
        kValues[i] = stod(content[2][i]);
    }
    lValues = vector<double> (content[3].size());
    for (uint i = 0; i < lValues.size(); i++) {
        lValues[i] = stod(content[3][i]);
    }

    experimentFolder = parameterEstimationFile.substr(0,parameterEstimationFile.size()-4); //remove '.exp'
    experimentFolder += "/";
    createFolder(experimentFolder);
}

string ParameterEstimation::getScriptName(double k, double l) {
    return experimentFolder + "k_" + to_string(k) + "_l_" + to_string(l) + ".sh";
}

string ParameterEstimation::getAlignmentFileName(double k, double l) {
    return experimentFolder + "k_" + to_string(k) + "_l_" + to_string(l) + ".txt";
}

void ParameterEstimation::makeScript(double k, double l) {
    string scriptName = getScriptName(k, l);
    ofstream fout(scriptName.c_str());
    fout << "#!/bin/bash" << endl;
    fout << "cd " << projectFolder << endl;
    fout << "./simanneal -g1 " << G1.getName() << " -g2 " << G2.getName();
    fout << " -method " << method << " -restart 60";
    if (measureName == "ec") fout << " -ec 1 -s3 0";
    else if (measureName == "s3") fout << " -ec 0 -s3 1";
    else throw runtime_error("unexpected optimization measure");
    fout << " -k " << k << " -l " << l;
    fout << " -t " << minutes;
    fout << " -o " << getAlignmentFileName(k, l) << endl;
}

void ParameterEstimation::submitScript(double k, double l) {
    string command = "qsub " + getScriptName(k, l);
    execPrintOutput(command);
}

void ParameterEstimation::submitScriptsToCluster() {
    for (double k : kValues) {
        for (double l : lValues) {
            makeScript(k, l);
            submitScript(k, l);
        }
    }
}

double ParameterEstimation::getScore(double k, double l) {
    string aligFileName = getAlignmentFileName(k, l);
    Alignment A = Alignment::loadMapping(aligFileName);
    return measure->eval(A);
}

void ParameterEstimation::collectData() {
    //check that all files exist
    for (double k : kValues) {
        for (double l : lValues) {
            assert(fileExists(getAlignmentFileName(k, l)));
        }
    }

    cout << "Collecting data...";
    Timer t;
    t.start();
    data = vector<vector<double> > (kValues.size(), vector<double> (lValues.size()));
    for (uint i = 0; i < kValues.size(); i++) {
        for (uint j = 0; j < lValues.size(); j++) {
            data[i][j] = getScore(kValues[i], lValues[j]);
        }
    }
    cout << "ParameterEstimation::collectData done (" << t.elapsedString() << ")" << endl;
}

void ParameterEstimation::printData(string outputFile) {
    ofstream fout(outputFile.c_str());
    vector<vector<string> > table(kValues.size()+1, vector<string> (lValues.size()+1));
    table[0][0] = "k\\l";
    for (uint i = 0; i < kValues.size(); i++) {
        table[i+1][0] = to_string(kValues[i]);
    }
    for (uint j = 0; j < lValues.size(); j++) {
        table[0][j+1] = to_string(lValues[j]);
    }
    for (uint i = 0; i < kValues.size(); i++) {
        for (uint j = 0; j < lValues.size(); j++) {
            stringstream ss;
            ss.setf(ios::fixed);
            ss.precision(PRECISION_DECIMALS);
            ss << data[i][j];
            string val;
            ss >> val;
            if (val.substr(val.size()-7) == ".000000") val = val.substr(0, val.size()-7);
            table[i+1][j+1] = val;
        }
    }
    printTable(table, 1, fout);
    fout << endl;
    fout.close();
}

string ParameterEstimation::getName(void) {
    return "ParameterEstimation";
}
