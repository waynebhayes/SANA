#include <cassert>
#include <map>
#include <sstream>
#include <algorithm>
#include "Experiment.hpp"
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
#include "../Timer.hpp"

const int Experiment::NUM_RANDOM_RUNS = 10;
const int Experiment::PRECISION_DECIMALS = 6;

void Experiment::run(ArgumentParser& args) {
    string exper = args.strings["-experiment"];
    string experFile = "experiments/"+exper+".exp";
    assert(fileExists(experFile));
    init(experFile);
    printData("experiments/"+exper+".txt");
    printDataCSV("experiments/"+exper+".csv");
}

Experiment::Experiment() {}

Experiment::Experiment(string experimentFile) {
    init(experimentFile);
}

void Experiment::init(string experimentFile) {
    vector<vector<string> > content = fileToStringsByLines(experimentFile);
    uint nlines = content.size();
    measures = content[0];
    methods = content[1];
    uint nmethods = methods.size();

    folderFormat = content[2].size() == 1;
    if (not folderFormat) {
        uint npairs = (nlines-2)/(nmethods+1);
        networkPairs = vector<vector<string> > (npairs, vector<string> (2));
        alignmentFiles = vector<vector<string> > (npairs, vector<string> (nmethods));
        for (uint i = 0; i < npairs; i++) {
            uint line = 2+i*(nmethods+1);
            networkPairs[i] = content[line];
            for (uint j = 0; j < nmethods; j++) {
                alignmentFiles[i][j] = content[line+1+j][0];
            }
        }
    }
    else {
        vector<string> methodDirs(nmethods);
        for (uint i = 0; i < nmethods; i++) {
            methodDirs[i] = content[2+i][0];
        }
        uint npairs = nlines-2-nmethods;
        networkPairs = vector<vector<string> > (npairs, vector<string> (2));
        alignmentFiles = vector<vector<string> > (npairs, vector<string> (nmethods));
        for (uint i = 0; i < npairs; i++) {
            uint line = 2+nmethods+i;
            networkPairs[i] = content[line];
            for (uint j = 0; j < nmethods; j++) {
                if (methodDirs[j] == "*") alignmentFiles[i][j] = "*";
                else {
                    string dir = methodDirs[j];
                    string filePrefix = networkPairs[i][0]+"_"+networkPairs[i][1];
                    alignmentFiles[i][j] = dir + "/" + autocompleteFileName(dir, filePrefix);
                }
            }
        }
    }

    Timer t;
    t.start();
    cerr << "Collecting data..." << endl;
    collectData();
    cerr << "done (" << t.elapsedString() << ")" << endl;
}

struct RankComp {
    RankComp(vector<double> const *scores) {
        this->scores = scores;
    }

    bool operator() (uint i, uint j) {
        return (*scores)[i] > (*scores)[j];
    }

    vector<double> const *scores;
};

void Experiment::scoresToRankings(vector<string>& row) {
    uint n = row.size()-2;
    vector<double> scores(n);
    for (uint i = 0; i < n; i++) scores[i] = stod(row[i+2]);
    vector<uint> indices(n);
    for (uint i = 0; i < n; i++) indices[i] = i;
    sort(indices.begin(), indices.end(), RankComp(&scores));
    int nextRank = 1;
    for (uint i = 0; i < n; i++) {
        row[indices[i]+2] = intToString(nextRank);
        if (i < n-1 and scores[indices[i]] != scores[indices[i+1]]) ++nextRank;
    }
}

void addAverageToLastRow(vector<vector<string> >& table) {
    table.push_back(vector<string> (table[0].size()));
    uint n = table.size();
    table[n-1][0] = "AVG";
    table[n-1][1] = "";
    for (uint j = 2; j < table[0].size(); j++) {
        double sum = 0;
        uint invalidCount = 0;
        for (uint i = 1; i < n-1; i++) {
            double value = stod(table[i][j]);
            if (value == -1) {
                invalidCount++;
            }
            else {
                sum += value;
            }
        }
        table[n-1][j] = to_string(sum/(n-2-invalidCount));
    }
}

void Experiment::printData(string outputFile) {
    ofstream fout(outputFile.c_str());

    vector<vector<vector<string> > > tables;
    for (uint i = 0; i < data.size(); i++) {
        vector<vector<string> > table(networkPairs.size()+1, vector<string> (methods.size()+2));
        fout << "Measure: " << measures[i] << endl;
        table[0][0] = "G1";
        table[0][1] = "G2";
        for (uint j = 0; j < methods.size(); j++) {
            table[0][j+2] = methods[j];
        }
        for (uint j = 0; j < networkPairs.size(); j++) {
            table[j+1][0] = networkPairs[j][0];
            table[j+1][1] = networkPairs[j][1];
            for (uint k = 0; k < methods.size(); k++) {
                stringstream ss;
                ss.setf(ios::fixed);
                ss.precision(PRECISION_DECIMALS);
                ss << data[i][k][j];
                string val;
                ss >> val;
                if (val.substr(val.size()-7) == ".000000") val = val.substr(0, val.size()-7);
                table[j+1][k+2] = val;
            }
        }
        tables.push_back(table);
        addAverageToLastRow(table);
        printTable(table, 1, fout);
        fout << endl << endl;
    }

    fout << endl << "=== rankings ===" << endl;
    for (uint i = 0; i < data.size(); i++) {
        vector<vector<string> > &table = tables[i];
        fout << "Measure: " << measures[i] << endl;
        for (uint j = 1; j < table.size(); j++) {
            scoresToRankings(table[j]);
        }
        addAverageToLastRow(table);
        printTable(table, 1, fout);
        fout << endl << endl;
    }

    fout.close();
}

void Experiment::printDataCSV(string outputFile) {
    cout.setf(ios::fixed);
    cout.precision(PRECISION_DECIMALS);
    ofstream fout(outputFile.c_str());
    //headers
    fout << "names";
    for (uint k = 0; k < measures.size(); k++) {
        for (uint j = 0; j < methods.size(); j++) {
            fout << ", " << measures[k] << "_" << methods[j];
        }
    }
    fout << endl;
    //body
    for (uint i = 0; i < networkPairs.size(); i++) {
        fout << networkPairs[i][0] << "-" << networkPairs[i][1];
        for (uint k = 0; k < measures.size(); k++) {
            for (uint j = 0; j < methods.size(); j++) {
                fout << ", " << data[k][j][i];
            }
        }
        fout << endl;
    }
    fout.close();
}

void Experiment::collectData() {
    uint npairs = networkPairs.size();
    data = vector<vector<vector<double> > > (measures.size(), vector<vector<double> > (methods.size(), vector<double> (npairs, -1)));

    cerr << "Loading graphs...";
    Timer T;
    T.start();
    map<string, Graph> graphs;
    for (auto pair : networkPairs) {
        string g1Name = pair[0];
        string g2Name = pair[1];
        if (graphs.count(g1Name) == 0) {
            graphs[g1Name] = Graph::loadGraph(g1Name);
        }
        if (graphs.count(g2Name) == 0) {
            graphs[g2Name] = Graph::loadGraph(g2Name);
        }
    }
    cerr << "done ("+T.elapsedString()+")" << endl;

    //Generate random alignments
    vector<vector<Alignment> > randomAligs(npairs, vector<Alignment> (10, Alignment::empty()));
    for (uint pair_i = 0; pair_i < npairs; pair_i++) {
        string g1Name = networkPairs[pair_i][0];
        string g2Name = networkPairs[pair_i][1];
        uint n1 = graphs[g1Name].getNumNodes();
        uint n2 = graphs[g2Name].getNumNodes();
        for (uint alig_i = 0; alig_i < NUM_RANDOM_RUNS; alig_i++) {
            randomAligs[pair_i][alig_i] = Alignment::random(n1, n2);
        }
    }

    for (uint pair_i = 0; pair_i < npairs; pair_i++) {
        string g1Name = networkPairs[pair_i][0];
        string g2Name = networkPairs[pair_i][1];
        T.start();
        cerr << "("+g1Name+", "+g2Name+")";
        string folderName = "";
        if (not folderFormat) folderName = "alignments/"+g1Name+"_"+g2Name+"/";
        Graph &G1 = graphs[g1Name];
        Graph &G2 = graphs[g2Name];
        for (uint measure_i = 0; measure_i < measures.size(); measure_i++) {
            if (measures[measure_i] == "sequence" and ((g1Name[0] >= '0' and g1Name[0] <= '5') or (g2Name[0] >= '0' and g2Name[0] <= '5'))) {
                for (uint method_i = 0; method_i < methods.size(); method_i++) {
                    data[measure_i][method_i][pair_i] = -1;
                }
                continue;
            }
            Measure* measure = loadMeasure(&G1, &G2, measures[measure_i]);
            for (uint method_i = 0; method_i < methods.size(); method_i++) {
                string alig = alignmentFiles[pair_i][method_i];
                if (alig != "*") {
                    Alignment A = Alignment::loadMapping(folderName+alig);
                    data[measure_i][method_i][pair_i] = measure->eval(A);
                }
                else {
                    double scoreSum = 0;
                    for (uint alig_i = 0; alig_i < NUM_RANDOM_RUNS; alig_i++) {
                        scoreSum += measure->eval(randomAligs[pair_i][alig_i]);
                    }
                    data[measure_i][method_i][pair_i] = scoreSum/NUM_RANDOM_RUNS;
                }
            }
            delete measure;
        }
        cerr << " ("+T.elapsedString()+")" << endl;
    }
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
    assert(false and "Unknown measure");
    return NULL;
}

std::string Experiment::getName(void) {
    return "Experiment";
}
