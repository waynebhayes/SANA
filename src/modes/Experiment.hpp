#ifndef EXPERIMENT_H
#define EXPERIMENT_H
#include <vector>
#include <string>

#include "Mode.hpp"
#include "../measures/Measure.hpp"
#include "../Graph.hpp"

using namespace std;

class Experiment : public Mode {
public:
    Experiment();
    string getName();

    void run(ArgumentParser& args);


    //used by other modes
    static Measure* loadMeasure(Graph* G1, Graph* G2, string name);

private:
    const int PRECISION_DECIMALS = 6;
    static const string methodArgsFile;
    static const string datasetsFile;
    string experName;
    string experFolder;
    string experFile;
    string resultsFolder;

    double t;
    uint nSubs;
    string subPolicy;
    vector<string> experArgs;
    vector<string> methods;
    string dataset;
    vector<vector<string>> networkPairs;
    vector<string> measures;

    void init();

    static vector<string> getMethodArgs(string method);
    static vector<vector<string>> getNetworkPairs(string dataset);
    string getOutputFileName(string method, string G1Name, string G2Name, uint subNum);
    string subCommand(string method, string G1Name, string G2Name, uint subNum);
    void makeSubmissions();
    void printSubmissions();
};


#endif
