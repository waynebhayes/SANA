#ifndef PARETOFRONT_HPP
#define PARETOFRONT_HPP

#include <iostream>
#include <map>
#include <unordered_map>
#include <vector>
#include <stdlib.h>
#include <time.h>
#include <algorithm>
#include <fstream>

using namespace std;

struct greaterThan {
        bool operator()(const double &a, const double &b) { return a > b; }
};

typedef vector<double>* alignmentPtr;
typedef pair<multimap<double, alignmentPtr, greaterThan>::iterator, multimap<double, alignmentPtr, greaterThan>::iterator> multiValueIterator;
typedef multimap<double, alignmentPtr, greaterThan>::iterator singleValueIterator;

class ParetoFront {
        private:
                unordered_map<alignmentPtr, vector<double>> findScoresByAlignment;
                vector<multimap<double, alignmentPtr, greaterThan>> paretoFront;
                vector<string> measureNames;
                
                unsigned int capacity;
                unsigned int currentSize;
                unsigned int numberOfMeasures;

                bool isDominating(vector<double> &scores);
                char whoDominates(vector<double> &otherScores, vector<double> &newScores);
        bool initialPass(vector<double> &newScores);
            
        alignmentPtr removeAlignment(alignmentPtr alignmentPosition, vector<double> &scores);
                alignmentPtr removeRandom();
        vector<alignmentPtr> emptyVector();
            
        vector<alignmentPtr> removeNewlyDominiated(singleValueIterator &iterIN, unsigned int i, vector<double>& newScores);
        vector<alignmentPtr> tryToInsertAlignmentScore(alignmentPtr algmtPtr, vector<double> &newScores);
        vector<alignmentPtr> insertDominatingAlignmentScore(alignmentPtr algmtPtr, vector<double> &newScores);
        vector<alignmentPtr> insertAlignmentScore(alignmentPtr algmtPtr, vector<double> &newScores);
        public:
                ParetoFront(const unsigned int paretoCap, const unsigned int numbOfMeas, const vector<string> &names) : capacity(paretoCap), numberOfMeasures(numbOfMeas) {
                        currentSize = 1;
                        paretoFront = vector<multimap<double, alignmentPtr, greaterThan>>(numberOfMeasures);
            for(unsigned int i = 0; i < numberOfMeasures; i++)
                paretoFront[i].insert(pair<double, alignmentPtr>(0, NULL));
            findScoresByAlignment.insert(pair<alignmentPtr, vector<double>>(NULL, vector<double>(numberOfMeasures, 0)));
                        measureNames = vector<string>(names);
                        sort(measureNames.begin(), measureNames.end());
            srand(time(NULL));
                };

                vector<alignmentPtr> addAlignmentScores(alignmentPtr algmtPtr, vector<double> &newScores);

                //ostream& printAllScoresByMeasures(ostream &os);
                //ostream& printAllParetoContainerNames(ostream &os);
                //ostream& printAllScoresByAlignments(ostream &os);
        ostream& printAlignmentScores(ostream &os);
};
#endif
