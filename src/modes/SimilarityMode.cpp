#include <cassert>
#include "SimilarityMode.hpp"

#include "../utils/utils.hpp"

#include "../arguments/measureSelector.hpp"
#include "../arguments/methodSelector.hpp"
#include "../arguments/graphLoader.hpp"

#include "../report.hpp"

void SimilarityMode::run(ArgumentParser& args) {
    Graph G1, G2;
    initGraphs(G1, G2, args);

    MeasureCombination M;
    initMeasures(M, G1, G2, args);

    if(M.getNumberOfLocalMeasures() == 0) {
    	cerr << "ERROR: need at least one local measure to produce similarity matrix." << endl;
    	exit(-1);
    }

    vector<vector <float> > sim = M.getAggregatedLocalSims();

    saveSimilarityMatrix(sim, args.strings["-o"] + ".sim");

    cout << "Finished. Saved similarity file as " << (args.strings["-o"] + ".sim") << endl;
}

void SimilarityMode::saveSimilarityMatrix(vector<vector <float> > sim, string file) {
	ofstream outfile;
	outfile.open(file.c_str());

	if (not outfile.is_open()) {
		cerr << "Problem saving similarity to specified location. Saving to sana program file." << endl;
		exit(-1);
	}

	for(uint i = 0; i < sim.size(); ++i) {
		for(uint j = 0; j < sim[i].size(); ++j) {
			outfile << i << " " << j << " " << sim[i][j] << endl;
		}
	}
}

string SimilarityMode::getName(void) {
    return "SimilarityMode";
}
