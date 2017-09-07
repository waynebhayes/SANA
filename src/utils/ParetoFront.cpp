#include "ParetoFront.hpp"

//using namespace std;

//struct greaterThan {
//	bool operator()(const double &a, const double &b) { return a > b; }
//};

void ParetoFront::handleDuplicate(int position, double &value) {
	if(paretoCurrent > 0)
		while(paretoFront[position].find(value) != paretoFront[position].end())
			value += 0.000000001; //This serves to avoid possible duplicate hits in the map.
}

int ParetoFront::removeByAlignmentPosition(int alignmentPosition) {
	for(int i = 0; i < numberOfMeasures; i++)
		paretoFront[i].erase(prev(paretoFront[i].end()));
	//findScoresByAlignment.erase(alignmentPosition);
	paretoCurrent--;
	return alignmentPosition;
}

int ParetoFront::removeLowest() {
	bool weakestParetoExists = true;
	for(int i = 0, j=i+1; j < numberOfMeasures; i++, j++) {
		if((paretoFront[i].rbegin())->second != (paretoFront[j].rbegin())->second) {
			weakestParetoExists = false;
			break;
		}
	}
	if(weakestParetoExists) {
		int alignmentPosition = (paretoFront[0].rbegin())->second;
		return removeByAlignmentPosition(alignmentPosition);
	}
	return -1;
}

int ParetoFront::removeRandom() {
	if(paretoCurrent <= numberOfMeasures)
		return -1;
	int alignmentPosition;
	do {
		alignmentPosition = rand() % paretoCurrent + 1;
	} while(isDominatingAlignment(alignmentPosition)); //Don't remove the fringe pareto members!
	return removeByAlignmentPosition(alignmentPosition);
}

int ParetoFront::removeAverageLowest() {
	//This member function should most likely not be used, but just in case it's desired.
	//The idea here is we find one of the alignments whose scores is lowest across the board.
	unordered_map<int, int> trackLowest;
	vector<map<double, int, greaterThan>::reverse_iterator> paretoTracker;
	for(int i = 0; i < numberOfMeasures; i++)
		paretoTracker.push_back(paretoFront[i].rbegin());
	for(int i = 0; i < paretoCurrent; i++)
	{
		for(int j = 0; j < numberOfMeasures; j++) {
			if(trackLowest.find(paretoTracker[j]->second) == trackLowest.end())
				trackLowest[paretoTracker[j]->second] = 1;
			else
				trackLowest[paretoTracker[j]->second]++;
			if(trackLowest[paretoTracker[j]->second] == numberOfMeasures)
				return removeByAlignmentPosition(paretoTracker[j]->second);
			paretoTracker[j]++;
		}
	}
	return removeRandom();
}

bool ParetoFront::isDominatingAlignment(int alignmentPosition) {
	for(int i = 0; i < numberOfMeasures; i++)
		if((paretoFront[i].begin())->second == alignmentPosition)
			return true;
	return false;
}

int ParetoFront::addAlignmentScore(int alignmentPosition, vector<double> &newScores) {
	if(alignmentPosition == paretoCurrent || (alignmentPosition < paretoCurrent && paretoCurrent == paretoCapacity)) {
		if(paretoCurrent > 0) {
			int j = 0;
			for(int i = 0; i < numberOfMeasures; i++)
				if(newScores[i] < (paretoFront[i].rbegin())->first)
					j++;
			if(j == numberOfMeasures)
				return -1;
		}

		if(paretoCurrent == paretoCapacity) {
			alignmentPosition = removeLowest();
			if(alignmentPosition == -1)
				alignmentPosition = removeRandom();
		}

		for(int i = 0; i < numberOfMeasures; i++) {
			handleDuplicate(i, newScores[i]);
			paretoFront[i][newScores[i]] = alignmentPosition;
			if(findScoresByAlignment.find(alignmentPosition) == findScoresByAlignment.end())
				findScoresByAlignment[alignmentPosition] = vector<double>(numberOfMeasures);
			findScoresByAlignment[alignmentPosition][i] = newScores[i];
		}

		paretoCurrent++;
		return alignmentPosition;
	}
	cerr << "Alignment Position less than or greater than current number in the Pareto Front.\n"
	     << "Not inserting the scores for this Alignment into the Pareto Front." << endl;
	return -1;
}

void ParetoFront::updateAlignmentScore(int alignmentPosition, vector<double> &newScores) {
//This member function should most likely not be used, but just in case it's desired.
	vector<double>* AlignmentScores = &findScoresByAlignment[alignmentPosition];
	for(unsigned int i = 0; i < AlignmentScores->size(); i++) {
		if(alignmentPosition == paretoFront[i][(*AlignmentScores)[i]])
		{
			paretoFront[i].erase((*AlignmentScores)[i]);
			handleDuplicate(i, newScores[i]);
			paretoFront[i][newScores[i]] = alignmentPosition;
		}
		else {
			cerr << "One of the old scores for the current alignment was over-written."
			     << "\nThere is probably a bug in the updateAlignmentScore OR addAlignmentScore member function."
			     << "\nExiting." << endl;
			exit(1);
		}
	}
}

ostream& ParetoFront::printAllScoresByMeasures(ostream &os) {
	for(int i = 0; i < numberOfMeasures; i++) {
		os << "Measure " << measureNames[i] << ":\n\t";
		for( auto iter = paretoFront[i].begin(); iter != paretoFront[i].end(); iter++ )
			os << iter->first << ' ';
		if(i < numberOfMeasures - 1)
			os << "\n\n";
	}
	return os;
}

ostream& ParetoFront::printAllParetoContainerNames(ostream &os) {
	if(measureNames.size() > 0) {
		os << measureNames[0];
		for(unsigned int i = 1; i < measureNames.size(); i++)
			os << ' ' << measureNames[i];
	}
	return os;
}

ostream& ParetoFront::printAllScoresByAlignments(ostream &os) {
	for(int i = 0; i < paretoCurrent; i++)
	{
		os << "Alignment " << i << ":\n\t{ ";
		for(int j = 0; j < numberOfMeasures; j++) {
			os << "(" << measureNames[j] << ", " << findScoresByAlignment[i][j] << ")";
			if( j < numberOfMeasures - 1)
				os << ", ";
		}
		os << " }";
		if(i < paretoCurrent - 1)
			os << "\n\n";
	}
	return os;
}
