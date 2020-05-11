#include "SimilarityMode.hpp"
#include <cassert>
#include <utility>
#include "../utils/utils.hpp"
#include "../arguments/measureSelector.hpp"
#include "../arguments/methodSelector.hpp"
#include "../arguments/GraphLoader.hpp"

void SimilarityMode::run(ArgumentParser& args) {
    pair<Graph, Graph> graphs = GraphLoader::initGraphs(args);
    Graph G1 = graphs.first;
    Graph G2 = graphs.second;
    
    MeasureCombination M;
    measureSelector::initMeasures(M, G1, G2, args);
    if(M.getNumberOfLocalMeasures() == 0) {
        cout << "ERROR: need at least one local measure to produce similarity matrix." << endl;
        exit(-1);
    }

    vector<vector <float>> sim = M.getAggregatedLocalSims();

    assert(args.doubleVectors["-simFormat"].size() == 1);
    saveSimilarityMatrix(sim, G1, G2, args.strings["-o"] + ".sim", args.doubleVectors["-simFormat"][0]);

    cout << "Finished. Saved similarity file as " << (args.strings["-o"] + ".sim") << endl;
}

void SimilarityMode::saveSimilarityMatrix(const vector<vector<float>>& sim, 
        const Graph& G1, const Graph& G2, const string& file, int format) {
    ofstream outfile;
    outfile.open(file.c_str());
    if (not outfile.is_open()) {
        cout << "Problem saving similarity to specified location. Saving to sana program file." << endl;
        exit(-1);
    }
    switch(format) {
        case 0:
            for(uint i = 0; i < sim.size(); ++i) {
                for(uint j = 0; j < sim[i].size(); ++j) if(sim[i][j]) {
                    outfile << i << " " << j << " " << sim[i][j] << endl;
                }
            }
        break;
        case 1:
            for(uint i = 0; i < sim.size(); ++i) {
                for(uint j = 0; j < sim[i].size(); ++j) if(sim[i][j]) {
                    outfile << G1.getNodeName(i) << " " << G2.getNodeName(j) << " " << sim[i][j] << endl;
                }
            }
        break;
    }
    outfile.close();
}

string SimilarityMode::getName() {
    return "SimilarityMode";
}
