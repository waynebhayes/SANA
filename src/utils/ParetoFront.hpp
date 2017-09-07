#ifndef PARETOFRONT_HPP
#define PARETOFRONT_HPP

#include <iostream>
#include <vector>
#include <map>
#include <unordered_map>
#include <stdlib.h>
#include <time.h>
#include <algorithm>

using namespace std;

struct greaterThan {
	bool operator()(const double &a, const double &b) { return a > b; }
};

class ParetoFront {
	private:
		unordered_map<int, vector<double>> findScoresByAlignment;
		vector<map<double, int, greaterThan>> paretoFront;
		vector<string> measureNames;
		int paretoCapacity;
		int paretoCurrent;
		int numberOfMeasures;

		void handleDuplicate(int position, double &value);
		int removeByAlignmentPosition(int alignmentPosition);
		int removeLowest();
		int removeRandom();
	
		//This member function should most likely not be used, but just in case it's desired.
		//The idea here is we find one of the alignments whose scores is lowest across the board.
		int removeAverageLowest();
		//---------------------------------------------------------------------------------------
		
		bool isDominatingAlignment(int alignmentPosition);
	public:
		ParetoFront(const int paretoCap, const int numbOfMeas, const vector<string> &names) : paretoCapacity(paretoCap), numberOfMeasures(numbOfMeas) {
			paretoCurrent = 0;
			paretoFront = vector<map<double, int, greaterThan>>(numberOfMeasures);
			measureNames = vector<string>(names);
			sort(measureNames.begin(), measureNames.end());
		};
		
		int addAlignmentScore(int alignmentPosition, vector<double> &newScores);
		
		//This member function should most likely not be used, but just in case it's desired.
		void updateAlignmentScore(int alignmentPosition, vector<double> &newScores);
		
		ostream& printAllScoresByMeasures(ostream &os);
		ostream& printAllParetoContainerNames(ostream &os);
		ostream& printAllScoresByAlignments(ostream &os);
};

#endif
