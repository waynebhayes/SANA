#include <string>
#include <vector>
#include <utility>
#include <iostream>
#include <fstream>
#include <sstream>
#include <cstdlib>
#include <stdexcept>
#include <unordered_set>
#include <algorithm>
#include <random>
#include <queue>
#include <iomanip>
#include <set>
#include <cmath>
#include <cassert>
#include <signal.h>
#include <stdio.h>
#include <unistd.h>

#include "SANA.hpp"
#include "../measures/SymmetricSubstructureScore.hpp"
#include "../measures/EdgeCorrectness.hpp"
#include "../measures/WeightedEdgeConservation.hpp"
#include "../measures/NodeCorrectness.hpp"
#include "../measures/localMeasures/Sequence.hpp"
#include "../NormalDistribution.hpp"
using namespace std;

SANA::SANA(Graph* G1, Graph* G2,
	double T_initial, double T_decay, double t, MeasureCombination* MC):
	Method(G1, G2, "SANA_"+MC->toString())
{
	//data structures for the networks
	n1 = G1->getNumNodes();
	n2 = G2->getNumNodes();
	g1Edges = G1->getNumEdges();
	G1->getAdjMatrix(G1AdjMatrix);
	G2->getAdjMatrix(G2AdjMatrix);
	G1->getAdjLists(G1AdjLists);
	G2->getAdjLists(G2AdjLists);


	//random number generation
	random_device rd;
	gen = mt19937(getRandomSeed());
	G1RandomNode = uniform_int_distribution<>(0, n1-1);
	G2RandomUnassignedNode = uniform_int_distribution<>(0, n2-n1-1);
	randomReal = uniform_real_distribution<>(0, 1);


	//temperature schedule
	this->T_initial = T_initial;
	this->T_decay = T_decay;
	minutes = t;


	//data structures for the solution space search
	uint ramificationChange = n1*(n2-n1);
	uint ramificationSwap = n1*(n1-1)/2;
	uint totalRamification = ramificationSwap + ramificationChange;
	changeProbability = (double) ramificationChange/totalRamification;


	//objective function
	this->MC = MC;
	ecWeight = MC->getWeight("ec");
	s3Weight = MC->getWeight("s3");
	try {
		wecWeight = MC->getWeight("wec");
	} catch(...) {
		wecWeight = 0;
	}
	localWeight = MC->getSumLocalWeight();


	//restart scheme
	restart = false;


	//to evaluate EC incrementally
	needAligEdges = ecWeight > 0 or s3Weight > 0 or wecWeight > 0;


	//to evaluate S3 incrementally
	needInducedEdges = s3Weight > 0;


	//to evaluate WEC incrementally
	needWec = wecWeight > 0;
	if (needWec) {
		Measure* wec = MC->getMeasure("wec");
		LocalMeasure* m = ((WeightedEdgeConservation*) wec)->getNodeSimMeasure();
		vector<vector<float> >* wecSimsP = m->getSimMatrix();
		wecSims = vector<vector<float> > (n1, vector<float> (n2, 0));
		for (uint i = 0; i < n1; i++) {
			for (uint j = 0; j < n2; j++) {
				wecSims[i][j] = (*wecSimsP)[i][j];
			}
		}
	}


	//to evaluate local measures incrementally
	needLocal = localWeight > 0;
	if (needLocal) sims = MC->getAggregatedLocalSims();


	//other execution options
	constantTemp = false;
	enableTrackProgress = true;
	iterationsPerStep = 10000000;


	//this does not need to be initialized here,
	//but the space has to be reserved in the
	//stack. it is initialized properly before
	//running the algorithm
	assignedNodesG2 = vector<bool> (n2);
	unassignedNodesG2 = vector<ushort> (n2-n1);
	A = vector<ushort> (n1);


	//to track progress
	//vector<double> eIncs = energyIncSample();
	//avgEnergyInc = vectorMean(eIncs);
	avgEnergyInc = -0.00001;
}

SANA::~SANA() {
}

Alignment SANA::run() {
	if (restart) return runRestartPhases();
	else {
		long long unsigned int iter = 0;
		return simpleRun(Alignment::random(n1, n2), minutes*60, iter);
	}
}

void SANA::describeParameters(ostream& sout) {
    sout << "Temperature schedule:" << endl;
    sout << "T_initial: " << T_initial << endl;
    sout << "T_decay: " << T_decay << endl;
    
    sout << "Optimize: " << endl;
    MC->printWeights(sout);
    
    sout << "Execution time: ";
    if (restart) sout << minutesNewAlignments + minutesPerCandidate*numCandidates + minutesFinalist;
    else sout << minutes;
    sout << "m" << endl;
    
    if (restart) {
    	sout << "Restart scheme:" << endl;
	    sout << "- new alignments: " << minutesNewAlignments << "m" << endl;
	    sout << "- each candidate: " << minutesPerCandidate << "m" << endl;
	    sout << "- finalist: " << minutesFinalist << "m" << endl;
	    sout << "number candidates: " << numCandidates << endl;
	    sout << "number new alignments: " << newAlignmentsCount << endl;
	    sout << "iterations per new alignment: " << iterationsPerStep << endl;
    }
}

string SANA::fileNameSuffix(const Alignment& A) {
	return "_" + extractDecimals(eval(A),3);
}

void SANA::enableRestartScheme(double minutesNewAlignments, uint iterationsPerStep, uint numCandidates, double minutesPerCandidate, double minutesFinalist) {
	restart = true;
	this->minutesNewAlignments = minutesNewAlignments;
	this->iterationsPerStep = iterationsPerStep;
	this->numCandidates = numCandidates;
	this->minutesPerCandidate = minutesPerCandidate;
	this->minutesFinalist = minutesFinalist;

	candidates = vector<Alignment> (numCandidates, Alignment::random(n1, n2));
	candidatesScores = vector<double> (numCandidates, 0);
	for (uint i = 0; i < numCandidates; i++) {
		candidatesScores[i] = eval(candidates[i]);
	}
}

double SANA::temperatureFunction(double iter, double T_initial, double T_decay) {
	return T_initialScaling * T_initial * (constantTemp ? 1 : exp(-T_decay*T_decayScaling*iter));
}

double SANA::acceptingProbability(double energyInc, double T) {
	return energyInc >= 0 ? 1 : exp(energyInc/T);
}

void SANA::initDataStructures(const Alignment& startA) {
	A = startA.getMapping();

	assignedNodesG2 = vector<bool> (n2, false);
	for (uint i = 0; i < n1; i++) {
		assignedNodesG2[A[i]] = true;
	}
	
	unassignedNodesG2 = vector<ushort> (n2-n1); 
	int j = 0;
	for (uint i = 0; i < n2; i++) {
		if (not assignedNodesG2[i]) {
			unassignedNodesG2[j] = i;
			j++;
		}
	}

	if (needAligEdges) {
		aligEdges = startA.numAlignedEdges(*G1, *G2);
	}

	if (needInducedEdges) {
		inducedEdges = G2->numNodeInducedSubgraphEdges(A);
	}

	if (needLocal) {
		localScoreSum = 0;
		for (uint i = 0; i < n1; i++) {
			localScoreSum += sims[i][A[i]];
		}
	}

	if (needWec) {
		wecSum = 0;
		for (uint i = 0; i < n1; i++) {
			wecSum += wecSims[i][A[i]];
		}
	}
	
	currentScore = eval(startA);

	timer.start();
}

double SANA::eval(const Alignment& Al) {
	return MC->eval(Al);
}

void SANA::setInterruptSignal() {
	interrupt = false;
	struct sigaction sigInt;
	sigInt.sa_handler = sigIntHandler;
	sigemptyset(&sigInt.sa_mask);
	sigInt.sa_flags = 0;
	sigaction(SIGINT, &sigInt, NULL);
}

Alignment SANA::simpleRun(const Alignment& startA, double maxExecutionSeconds,
	long long unsigned int& iter) {
	
	initDataStructures(startA);

	setInterruptSignal();

	for (; ; iter++) {
		T = temperatureFunction(iter, T_initial, T_decay);
		if (interrupt) {
			return A;
		}
		if (iter%iterationsPerStep == 0) {
			trackProgress(iter);
			if (iter != 0 and timer.elapsed() > maxExecutionSeconds) {
				return A;
			}
		}
		SANAIteration();
	}
	return A; //dummy return to shut compiler warning
}

void SANA::SANAIteration() {
	if (randomReal(gen) <= changeProbability) performChange();
	else performSwap();
}

void SANA::performChange() {
	ushort source = G1RandomNode(gen);
	ushort oldTarget = A[source];
	uint newTargetIndex = G2RandomUnassignedNode(gen);
	ushort newTarget = unassignedNodesG2[newTargetIndex];

	int newAligEdges = -1; //dummy initialization to shut compiler warnings
	if (needAligEdges) {
		newAligEdges = aligEdges + aligEdgesIncChangeOp(source, oldTarget, newTarget);
	}

	int newInducedEdges = -1; //dummy initialization to shut compiler warnings
	if (needInducedEdges) {
		newInducedEdges = inducedEdges + inducedEdgesIncChangeOp(source, oldTarget, newTarget);
	}

	double newLocalScoreSum = -1; //dummy initialization to shut compiler warnings
	if (needLocal) {
		newLocalScoreSum = localScoreSum + localScoreSumIncChangeOp(source, oldTarget, newTarget);
	}

	double newWecSum = -1; //dummy initialization to shut compiler warning
	if (needWec) {
		newWecSum = wecSum + WECIncChangeOp(source, oldTarget, newTarget);
	}

	double newCurrentScore = 0;
	newCurrentScore += ecWeight * (newAligEdges/g1Edges);
	newCurrentScore += s3Weight * (newAligEdges/(g1Edges+newInducedEdges-newAligEdges));
	newCurrentScore += localWeight * (newLocalScoreSum/n1);
	newCurrentScore += wecWeight * (newWecSum/(2*g1Edges));

	energyInc = newCurrentScore-currentScore;

	if (energyInc >= 0 or randomReal(gen) <= exp(energyInc/T)) {
		A[source] = newTarget;
		unassignedNodesG2[newTargetIndex] = oldTarget;
		assignedNodesG2[oldTarget] = false;
		assignedNodesG2[newTarget] = true;
		aligEdges = newAligEdges;
		inducedEdges = newInducedEdges;
		localScoreSum = newLocalScoreSum;
		wecSum = newWecSum;
		currentScore = newCurrentScore;
	}
}

void SANA::performSwap() {
	ushort source1 = G1RandomNode(gen), source2 = G1RandomNode(gen);
	ushort target1 = A[source1], target2 = A[source2];

	int newAligEdges = -1; //dummy initialization to shut compiler warnings
	if (needAligEdges) {
		newAligEdges = aligEdges + aligEdgesIncSwapOp(source1, source2, target1, target2);
	}
	
	double newLocalScoreSum = -1; //dummy initialization to shut compiler warnings
	if (needLocal) {
		newLocalScoreSum = localScoreSum + localScoreSumIncSwapOp(source1, source2, target1, target2);
	}

	double newWecSum = -1; //dummy initialization to shut compiler warning
	if (needWec) {
		newWecSum = wecSum + WECIncSwapOp(source1, source2, target1, target2);
	}

	double newCurrentScore = 0;
	newCurrentScore += ecWeight * (newAligEdges/g1Edges);
	newCurrentScore += s3Weight * (newAligEdges/(g1Edges+inducedEdges-newAligEdges));
	newCurrentScore += localWeight * (newLocalScoreSum/n1);
	newCurrentScore += wecWeight * (newWecSum/(2*g1Edges));
	
	energyInc = newCurrentScore-currentScore;
	if (energyInc >= 0 or randomReal(gen) <= exp(energyInc/T)) {
		A[source1] = target2;
		A[source2] = target1;
		aligEdges = newAligEdges;
		localScoreSum = newLocalScoreSum;
		wecSum = newWecSum;
		currentScore = newCurrentScore;
	}
}

int SANA::aligEdgesIncChangeOp(ushort source, ushort oldTarget, ushort newTarget) {
	int res = 0;
	for (uint i = 0; i < G1AdjLists[source].size(); i++) {
		ushort neighbor = G1AdjLists[source][i];
		res -= G2AdjMatrix[oldTarget][A[neighbor]];
		res += G2AdjMatrix[newTarget][A[neighbor]];
	}
	return res;
}

int SANA::aligEdgesIncSwapOp(ushort source1, ushort source2, ushort target1, ushort target2) {
	int res = 0;
	for (uint i = 0; i < G1AdjLists[source1].size(); i++) {
		ushort neighbor = G1AdjLists[source1][i];
		res -= G2AdjMatrix[target1][A[neighbor]];
		res += G2AdjMatrix[target2][A[neighbor]];
	}
	for (uint i = 0; i < G1AdjLists[source2].size(); i++) {
		ushort neighbor = G1AdjLists[source2][i];
		res -= G2AdjMatrix[target2][A[neighbor]];
		res += G2AdjMatrix[target1][A[neighbor]];
	}
	//address case swapping between adjacent nodes with adjacent images:
	res += 2*(G1AdjMatrix[source1][source2] & G2AdjMatrix[target1][target2]);
	return res;
}

int SANA::inducedEdgesIncChangeOp(ushort source, ushort oldTarget, ushort newTarget) {
	int res = 0;
	for (uint i = 0; i < G2AdjLists[oldTarget].size(); i++) {
		ushort neighbor = G2AdjLists[oldTarget][i];
		res -= assignedNodesG2[neighbor];
	}
	for (uint i = 0; i < G2AdjLists[newTarget].size(); i++) {
		ushort neighbor = G2AdjLists[newTarget][i];
		res += assignedNodesG2[neighbor];
	}
	//address case changing between adjacent nodes:
	res -= G2AdjMatrix[oldTarget][newTarget];
	return res;
}

double SANA::localScoreSumIncChangeOp(ushort source, ushort oldTarget, ushort newTarget) {
	return sims[source][newTarget] - sims[source][oldTarget];
}

double SANA::localScoreSumIncSwapOp(ushort source1, ushort source2, ushort target1, ushort target2) {
	return sims[source1][target2] -
		   sims[source1][target1] +
		   sims[source2][target1] -
		   sims[source2][target2];
}

double SANA::WECIncChangeOp(ushort source, ushort oldTarget, ushort newTarget) {
	double res = 0;
	for (uint j = 0; j < G1AdjLists[source].size(); j++) {
		ushort neighbor = G1AdjLists[source][j];
		if (G2AdjMatrix[oldTarget][A[neighbor]]) {
			res -= wecSims[source][oldTarget];
			res -= wecSims[neighbor][A[neighbor]];
		}
		if (G2AdjMatrix[newTarget][A[neighbor]]) {
			res += wecSims[source][newTarget];
			res += wecSims[neighbor][A[neighbor]];
		}
	}
	return res;
}

double SANA::WECIncSwapOp(ushort source1, ushort source2, ushort target1, ushort target2) {
	double res = 0;
	for (uint j = 0; j < G1AdjLists[source1].size(); j++) {
		ushort neighbor = G1AdjLists[source1][j];
		if (G2AdjMatrix[target1][A[neighbor]]) {
			res -= wecSims[source1][target1];
			res -= wecSims[neighbor][A[neighbor]];
		}
		if (G2AdjMatrix[target2][A[neighbor]]) {
			res += wecSims[source1][target2];
			res += wecSims[neighbor][A[neighbor]];						
		}
	}
	for (uint j = 0; j < G1AdjLists[source2].size(); j++) {
		ushort neighbor = G1AdjLists[source2][j];
		if (G2AdjMatrix[target2][A[neighbor]]) {
			res -= wecSims[source2][target2];
			res -= wecSims[neighbor][A[neighbor]];
		}
		if (G2AdjMatrix[target1][A[neighbor]]) {
			res += wecSims[source2][target1];
			res += wecSims[neighbor][A[neighbor]];						
		}
	}
	//address case swapping between adjacent nodes with adjacent images:
	if (G1AdjMatrix[source1][source2] and G2AdjMatrix[target1][target2]) {
		res += 2*wecSims[source1][target1];
		res += 2*wecSims[source2][target2];
	}
	return res;
}

void SANA::trackProgress(long long unsigned int i) {
	if (not enableTrackProgress) return;
	bool printDetails = false;
	bool printScores = false;
	bool checkScores = true;
	cerr << i/iterationsPerStep << " (" << timer.elapsed() << "s): score = " << currentScore;
	cerr <<  " P(" << avgEnergyInc << ", " << T << ") = " << acceptingProbability(avgEnergyInc, T) << endl;
	if (not (printDetails or printScores or checkScores)) return;
	Alignment Al(A);
	if (printDetails) cerr << " (" << Al.numAlignedEdges(*G1, *G2) << ", " << G2->numNodeInducedSubgraphEdges(A) << ")";
	if (printScores) {
		SymmetricSubstructureScore S3(G1, G2);
		EdgeCorrectness EC(G1, G2);
		cerr << "S3: " << S3.eval(Al) << "  EC: " << EC.eval(Al) << endl;
	}
	if (checkScores) {
		double realScore = eval(Al);
		if (realScore-currentScore > 0.000001) {
			cerr << "internal error: incrementally computed score (" << currentScore;
			cerr << ") is not correct (" << realScore << ")" << endl;
		}
	}
}

Alignment SANA::runRestartPhases() {
	cerr << "new alignments phase" << endl;
	Timer T;
	T.start();
	newAlignmentsCount = 0;
	while (T.elapsed() < minutesNewAlignments*60) {
		long long unsigned int iter = 0;
		Alignment A = simpleRun(Alignment::random(n1, n2), 0, iter);
		uint i = getLowestIndex();
		double lowestScore = candidatesScores[i];
		if (currentScore > lowestScore) {
			candidates[i] = A;
			candidatesScores[i] = currentScore;
		}
		newAlignmentsCount++;
	}
	cerr << "candidates phase" << endl;
	vector<long long unsigned int> iters(numCandidates, iterationsPerStep);
	for (uint i = 0; i < numCandidates; i++) {
		candidates[i] = simpleRun(candidates[i], minutesPerCandidate*60, iters[i]);
		candidatesScores[i] = currentScore;
	}
	cerr << "finalist phase" << endl;
	uint i = getHighestIndex();
	return simpleRun(candidates[i], minutesFinalist*60, iters[i]);
}

uint SANA::getLowestIndex() const {
	double lowestScore = candidatesScores[0];
	uint lowestIndex = 0;
	for (uint i = 1; i < numCandidates; i++) {
		if (candidatesScores[i] < lowestScore) {
			lowestScore = candidatesScores[i];
			lowestIndex = i;
		}
	}
	return lowestIndex;
}

uint SANA::getHighestIndex() const {
	double highestScore = candidatesScores[0];
	uint highestIndex = 0;
	for (uint i = 1; i < numCandidates; i++) {
		if (candidatesScores[i] > highestScore) {
			highestScore = candidatesScores[i];
			highestIndex = i;
		}
	}
	return highestIndex;
}


void SANA::setTemperatureScheduleAutomatically() {
	setT_INITIALAutomatically();
	setT_DECAYAutomatically();
}

void SANA::setT_INITIALAutomatically() {
	T_initial = searchT_INITIAL();
}

void SANA::setT_DECAYAutomatically() {
	T_decay = searchT_decay(T_initial, minutes);
}

double SANA::searchT_INITIAL() {
	const double NUM_SAMPLES_RANDOM = 100;
	const double HIGH_THRESHOLD_P = 0.999999;	
	const double LOW_THRESHOLD_P = 0.99;

	cerr<<endl;
	//find the threshold score between random and not random temperature
	Timer T;
	T.start();
	cerr << "Computing distribution of scores of random alignments ";
	vector<double> upperBoundKScores(NUM_SAMPLES_RANDOM);
	for (uint i = 0; i < NUM_SAMPLES_RANDOM; i++) {
		upperBoundKScores[i] = scoreRandom();
	}
	cerr << "(" <<  T.elapsedString() << ")" << endl;
	NormalDistribution dist(upperBoundKScores);	
	double highThresholdScore = dist.quantile(HIGH_THRESHOLD_P);
	double lowThresholdScore = dist.quantile(LOW_THRESHOLD_P);
	cerr << "Mean: " << dist.getMean() << endl;
	cerr << "sd: " << dist.getSD() << endl;
	cerr << LOW_THRESHOLD_P << " of random runs have a score <= " << lowThresholdScore << endl;
	cerr << HIGH_THRESHOLD_P << " of random runs have a score <= " << highThresholdScore << endl;

	double lowerBoundT_INITIAL = 0;
	double upperBoundT_INITIAL = 1;
	while (not isRandomT_INITIAL(upperBoundT_INITIAL, highThresholdScore, lowThresholdScore)) {
		cerr << endl;
		upperBoundT_INITIAL *= 2;
	}
	cerr << endl;
	if (upperBoundT_INITIAL > 1) lowerBoundT_INITIAL = upperBoundT_INITIAL/2;

	uint n1 = G1->getNumNodes();
	uint n2 = G2->getNumNodes();
	cerr << "Iterations per run: " << 10000.+100.*n1+10.*n2+n1*n2*0.1 << endl;

	uint count = 0;
	T.start();
	while (lowerBoundT_INITIAL < upperBoundT_INITIAL and
		   count <= 10) {
		//search in log space
		double lowerBoundT_INITIALLog = log2(lowerBoundT_INITIAL+1);
		double upperBoundT_INITIALLog = log2(upperBoundT_INITIAL+1);
		double midT_INITIALLog = (lowerBoundT_INITIALLog+upperBoundT_INITIALLog)/2.;
		double midT_INITIAL = exp2(midT_INITIALLog)-1;

		//we prefer false negatives (random scores classified as non-random)
		//than false positives (non-random scores classified as random)
		cerr << "Test " << count << " (" << T.elapsedString() << "): ";
		count++;
		if (isRandomT_INITIAL(midT_INITIAL, highThresholdScore, lowThresholdScore)) {
			upperBoundT_INITIAL = midT_INITIAL;
			cerr << " (random behavior)";
		}
		else {
			lowerBoundT_INITIAL = midT_INITIAL;
			cerr << " (NOT random behavior)";
		}
		cerr << " New range: (" << lowerBoundT_INITIAL << ", " << upperBoundT_INITIAL << ")" << endl;
	}
	//return the top of the range
	cerr << "Final range: (" << lowerBoundT_INITIAL << ", " << upperBoundT_INITIAL << ")" << endl;
	cerr << "Final T_initial: " << upperBoundT_INITIAL << endl;
	return upperBoundT_INITIAL;
}

//takes a random alignment, lets it run for a certain number
//of iterations (ITERATIONS) with fixed temperature T_initial
//and returns its score
double SANA::scoreForT_INITIAL(double T_initial) {
	uint ITERATIONS = 10000.+100.*n1+10.*n2+n1*n2*0.1; //heuristic value

	double oldIterationsPerStep = this->iterationsPerStep;
	double oldT_INITIAL = this->T_initial;
	bool oldRestart = restart;

	this->iterationsPerStep = ITERATIONS;
	this->T_initial = T_initial;
	constantTemp = true;
	enableTrackProgress = false;
	restart = false;

	long long unsigned int iter = 0;
	simpleRun(Alignment::random(n1, n2), 0, iter);
	this->iterationsPerStep = oldIterationsPerStep;
	this->T_initial = oldT_INITIAL	;
	constantTemp = false;
	enableTrackProgress = true;
	restart = oldRestart;

	return currentScore;
}

bool SANA::isRandomT_INITIAL(double T_initial, double highThresholdScore, double lowThresholdScore) {
	const double NUM_SAMPLES = 5;

	double score = scoreForT_INITIAL(T_initial);
	cerr << "T_initial = " << T_initial << ", score = " << score;
	//quick filter all the scores that are obviously not random
	if (score > highThresholdScore) return false;
	if (score < lowThresholdScore) return true;
	//make sure that alignments that passed the first test are truly random
	//(among NUM_SAMPLES runs, at least one of them has a p-value smaller than LOW_THRESHOLD_P)
	for (uint i = 0; i < NUM_SAMPLES; i++) {
		if (scoreForT_INITIAL(T_initial) <= lowThresholdScore) return true;
	}
	return false;
}

double SANA::scoreRandom() {
	return eval(Alignment::random(n1, n2));
}

double SANA::searchSpaceSizeLog() {
	//the search space size is (n2 choose n1) * n1!
	//we use the stirling approximation
	if (n2 == n1) return n1*log(n1)-n1;
	return n2*log(n2)-(n2-n1)*log(n2-n1)-n1;
}

long long unsigned int SANA::hillClimbingIterations(long long unsigned int idleCountTarget) {
	Alignment startA = Alignment::random(n1, n2);
	long long unsigned int iter = 0;

	cerr << "We consider that SANA has stagnated if it goes ";
	cerr << idleCountTarget << " without improving" << endl;

	initDataStructures(startA);
	T = 0;
	long long unsigned int idleCount = 0;
	for (; ; iter++) {
		if (iter%iterationsPerStep == 0) {
			trackProgress(iter);
		}
		double oldScore = currentScore;
		SANAIteration();
		if (oldScore == currentScore) idleCount++;
		else idleCount = 0;
		if (idleCount == idleCountTarget) {
			return  iter - idleCount;
		}
	}
	return iter; //dummy return to shut compiler warning
}

double SANA::expectedNumAccEInc(double temp, const vector<double>& energyIncSample) {
	double res = 0;
	for (uint i = 0; i < energyIncSample.size(); i++) {
		res += exp(energyIncSample[i]/temp);
	}
	return res;
}

//returns a sample of energy increments, with size equal to the number of iterations per second
//after hill climbing
vector<double> SANA::energyIncSample() {
	double iter = iterPerSecond(); //this runs HC, so it updates the values of A and currentScore
	cout << "Hill climbing score: " << currentScore << endl;
	//generate a sample of energy increments, with size equal to the number of iterations per second
	vector<double> EIncs(0);
	T = 0;
	for (uint i = 0; i < iter; i++) {
		SANAIteration();
		if (energyInc < 0) {
			EIncs.push_back(energyInc);
		}
	}
	return EIncs;
}

double SANA::searchT_decay(double T_initial, double minutes) {

	vector<double> EIncs = energyIncSample();
	cerr << "Total of " << EIncs.size() << " energy increment samples averaging " << vectorMean(EIncs) << endl;

	//find the temperature epsilon such that the expected number of these energy samples accepted is 1
	//by bisection, since the expected number is monotically increasing in epsilon

	//upper bound and lower bound of x
	uint N = EIncs.size();
	double ESum = vectorSum(EIncs);
	double EMin = vectorMin(EIncs);
	double EMax = vectorMax(EIncs);
	double x_left = abs(EMax)/log(N);
	double x_right = min(abs(EMin)/log(N), abs(ESum)/(N*log(N)));
	cerr << "Starting range: (" << x_left << ", " << x_right << ")" << endl;

	const uint NUM_ITER = 100;
	for (uint i = 0; i < NUM_ITER; i++) {
		double x_mid = (x_left + x_right)/2;
		double y = expectedNumAccEInc(x_mid, EIncs);
		if (y < 1) x_left = x_mid;
		else if (y > 1) x_right = x_mid;
		else break;
	}

	double epsilon = (x_left + x_right)/2;
	cerr << "Final range: (" << x_left << ", " << x_right << ")" << endl;
	cerr << "Final epsilon: " << epsilon << endl;
	double iter_t = minutes*60*iterPerSecond();

	double lambda = log((T_initial*T_initialScaling)/epsilon)/(iter_t*T_decayScaling);
	cerr << "Final T_decay: " << lambda << endl;
	return lambda;
}

double SANA::iterPerSecond() {
	long long unsigned int iter = hillClimbingIterations(1000000+searchSpaceSizeLog());
	double res = iter/timer.elapsed();
	cerr << "SANA does " << to_string(res) << " iterations per second" << endl; 
	return res;
}
