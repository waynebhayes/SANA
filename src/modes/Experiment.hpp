#ifndef EXPERIMENT_H
#define EXPERIMENT_H
#include <vector>
#include <string>

#include "Mode.hpp"
#include "../measures/Measure.hpp"
#include "../methods/Method.hpp"
#include "../Graph.hpp"

using namespace std;

class Experiment : public Mode {
public:
	Experiment();
	Experiment(string experimentFile);

	void run(ArgumentParser& args);
	void printData(string outputFile);

	void printDataCSV(string outputFile);

	static Measure* loadMeasure(Graph* G1, Graph* G2, string name);

private:
	vector<string> measures;
	vector<string> methods;

	//first index: network pair
	//second index: 0: g1 1: g2
	vector<vector<string> > networkPairs;

	//first index: network pair
	//second index: method
	vector<vector<string> > alignmentFiles;

	//first index: measure
	//second index: method
	//third index: network pair
	vector<vector<vector<double> > > data;

	void init(string experimentFile);
	void scoresToRankings(vector<string>& row);
	void collectData();

	vector<vector<string> > getNetworkPairs(string experimentFile);

	static const int NUM_RANDOM_RUNS;
	static const int PRECISION_DECIMALS;

	bool folderFormat;
};


#endif
