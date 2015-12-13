#ifndef ALPHAESTIMATION_H
#define ALPHAESTIMATION_H
#include <vector>
#include <string>
#include <map>
#include "../measures/Measure.hpp"
#include "../methods/Method.hpp"
#include "../Graph.hpp"

using namespace std;

class AlphaEstimation {
public:
	AlphaEstimation(string alphaFile);
	void printData(string outputFile);

	static double getAlpha(string alphaFile, string methodName, string G1Name, string G2Name);
private:
	vector<string> methods;

	//first index: network pair
	//second index: 0: g1 1: g2
	vector<vector<string> > networkPairs;

	map<string, Graph> graphs;

	//first index: method
	//second index: network pair
	vector<vector<double> > alphas;

	double computeAlpha(Graph& G1, Graph& G2, string methodName, Measure* topMeasure);
	double computeAlphaSANA(Graph& G1, Graph& G2, Measure* topMeasure);
	void computeAlphas();
};


#endif
