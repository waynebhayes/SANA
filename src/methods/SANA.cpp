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
#include "../measures/SymmetricEdgeCoverage.hpp"
#include "../measures/localMeasures/Sequence.hpp"
#include "../utils/NormalDistribution.hpp"
using namespace std;

static double SANAtime, pBad=1;

SANA::SANA(Graph* G1, Graph* G2,
		double TInitial, double TDecay, double t, MeasureCombination* MC, string& objectiveScore):
    		Method(G1, G2, "SANA_"+MC->toString())
{
	//data structures for the networks
	n1 = G1->getNumNodes();
	n2 = G2->getNumNodes();
	g1Edges = G1->getNumEdges();
	g2Edges = G2->getNumEdges();
	score = objectiveScore;

	G1->getAdjMatrix(G1AdjMatrix);
	G2->getAdjMatrix(G2AdjMatrix);
	G1->getAdjLists(G1AdjLists);
	G2->getAdjLists(G2AdjLists);


	//random number generation
	random_device rd;
	gen = mt19937(getRandomSeed());
	G1RandomNode = uniform_int_distribution<>(0, n1-1);
	uint G1UnLockedCount = n1 - G1->getLockedCount() -1;
	G1RandomUnlockedNodeDist = uniform_int_distribution<>(0, G1UnLockedCount);
	G2RandomUnassignedNode = uniform_int_distribution<>(0, n2-n1-1);
	G1RandomUnlockedGeneDist = uniform_int_distribution<>(0, G1->unlockedGeneCount - 1);
	G1RandomUnlockedmiRNADist = uniform_int_distribution<>(0, G1->unlockedmiRNACount - 1);
	randomReal = uniform_real_distribution<>(0, 1);


	//temperature schedule
	this->TInitial = TInitial;
	this->TDecay = TDecay;
	minutes = t;
	initializedIterPerSecond = false;

	//data structures for the solution space search
	uint ramificationChange = n1*(n2-n1);
	uint ramificationSwap = n1*(n1-1)/2;
	uint totalRamification = ramificationSwap + ramificationChange;
	changeProbability = (double) ramificationChange/totalRamification;
	tau = vector<double> {
	    1.000, 0.985, 0.970, 0.960, 0.950, 0.942, 0.939, 0.934, 0.928, 0.920,
	    0.918, 0.911, 0.906, 0.901, 0.896, 0.891, 0.885, 0.879, 0.873, 0.867,
	    0.860, 0.853, 0.846, 0.838, 0.830, 0.822, 0.810, 0.804, 0.794, 0.784,
	    0.774, 0.763, 0.752, 0.741, 0.728, 0.716, 0.703, 0.690, 0.676, 0.662,
	    0.647, 0.632, 0.616, 0.600, 0.584, 0.567, 0.549, 0.531, 0.514, 0.495,
	    0.477, 0.458, 0.438, 0.412, 0.400, 0.381, 0.361, 0.342, 0.322, 0.303, 
	    0.284, 0.264, 0.246, 0.228, 0.210, 0.193, 0.177, 0.161, 0.145, 0.131,
	    0.117, 0.104, 0.092, 0.081, 0.070, 0.061, 0.052, 0.044, 0.0375, 0.031,
	    0.026, 0.0212, 0.0172, 0.0138, 0.011, 0.008, 0.006, 0.005, 0.004, 0.003,
	    0.002, 0.001, 0.0003, 0.0001, 3e-5, 1e-6, 0, 0, 0, 0,
	    0};


	//objective function
	this->MC = MC;
	ecWeight = MC->getWeight("ec");
	s3Weight = MC->getWeight("s3");
	secWeight = MC->getWeight("sec");
	try {
		wecWeight = MC->getWeight("wec");
	} catch(...) {
		wecWeight = 0;
	}
	localWeight = MC->getSumLocalWeight();


	//restart scheme
	restart = false;
	
	//temperature decay dynamically 
	dynamic_tdecay = false; 


	//to evaluate EC incrementally
	needAligEdges = ecWeight > 0 or s3Weight > 0 or wecWeight > 0 or secWeight > 0;


	//to evaluate S3 incrementally
	needInducedEdges = s3Weight > 0;


	//to evaluate WEC incrementally
	needWec = wecWeight > 0;
	
	//to evaluate SEC incrementally
	needSec = secWeight > 0;
	
	
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
	if (needLocal) {
		sims = MC->getAggregatedLocalSims();
		localWeight = 1; //the values in the sim matrix 'sims'
		//have already been scaled by the weight
	} else {
		localWeight = 0;
	}


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
	vector<double> eIncs = energyIncSample();
}

SANA::SANA(Graph* G1, Graph* G2,
		double TInitial, double TDecay, uint i, MeasureCombination* MC, string& objectiveScore):
    		Method(G1, G2, "SANA_"+MC->toString())
{
	//data structures for the networks
	n1 = G1->getNumNodes();
	n2 = G2->getNumNodes();
	g1Edges = G1->getNumEdges();
	g2Edges = G2->getNumEdges();
	score = objectiveScore;
	
	G1->getAdjMatrix(G1AdjMatrix);
	G2->getAdjMatrix(G2AdjMatrix);
	G1->getAdjLists(G1AdjLists);
	G2->getAdjLists(G2AdjLists);


	//random number generation
	random_device rd;
	gen = mt19937(getRandomSeed());
	G1RandomNode = uniform_int_distribution<>(0, n1-1);
	uint G1UnLockedCount = n1 - G1->getLockedCount() -1;
	G1RandomUnlockedNodeDist = uniform_int_distribution<>(0, G1UnLockedCount);
	G2RandomUnassignedNode = uniform_int_distribution<>(0, n2-n1-1);
    G1RandomUnlockedGeneDist = uniform_int_distribution<>(0, G1->unlockedGeneCount - 1);
    G1RandomUnlockedmiRNADist = uniform_int_distribution<>(0, G1->unlockedmiRNACount - 1);
	randomReal = uniform_real_distribution<>(0, 1);


	//temperature schedule
	this->TInitial = TInitial;
	this->TDecay = TDecay;
	maxIterations = i;
	initializedIterPerSecond = false;

	//data structures for the solution space search
	uint ramificationChange = n1*(n2-n1);
	uint ramificationSwap = n1*(n1-1)/2;
	uint totalRamification = ramificationSwap + ramificationChange;
	changeProbability = (double) ramificationChange/totalRamification;
	tau = vector<double> {0.99, 0.985, 0.96, 0.955, 0.95, 0.942, 0.939, 0.934, 0.928, 0.92, 0.918, 0.911, 0.906, 0.901, 0.896,
			   0.891, 0.885, 0.879, 0.873, 0.867, 0.86, 0.853, 0.846, 0.838, 0.83, 0.822, 0.81, 0.804, 0.794, 0.784,
			   0.774, 0.763, 0.752, 0.741, 0.728, 0.716, 0.703, 0.69, 0.676, 0.662, 0.647, 0.632, 0.616, 0.6, 0.584,
			   0.567, 0.549, 0.531, 0.514, 0.495, 0.477, 0.458, 0.438, 0.412, 0.4, 0.381, 0.361, 0.342, 0.322, 0.303, 
			   0.284, 0.264, 0.246, 0.228, 0.21, 0.193, 0.177, 0.161, 0.145, 0.131, 0.117, 0.104, 0.092, 0.081, 0.07, 
		       0.061, 0.052, 0.044, 0.0375, 0.031, 0.026, 0.0212, 0.0172, 0.0138, 0.0109, 0.0085, 0.0065, 0.005, 0.0037, 
			   0.0027, 0.0019, 0.0014, 0.0005, 0.0001, 0.0000739, 0.0000342, 0.000008, 0.0000054, 0.000000739, 0.00000000359, 0.000000000734};


	//objective function
	this->MC = MC;
	ecWeight = MC->getWeight("ec");
	s3Weight = MC->getWeight("s3");
	secWeight = MC->getWeight("sec");
	try {
		wecWeight = MC->getWeight("wec");
	} catch(...) {
		wecWeight = 0;
	}
	localWeight = MC->getSumLocalWeight();


	//restart scheme
	restart = false;


	//to evaluate EC incrementally
	needAligEdges = ecWeight > 0 or s3Weight > 0 or wecWeight > 0 or secWeight > 0;


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
	if (needLocal) {
		sims = MC->getAggregatedLocalSims();
		localWeight = 1; //the values in the sim matrix 'sims'
		//have already been scaled by the weight
	} else {
		localWeight = 0;
	}


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
	vector<double> eIncs = energyIncSample();
}

SANA::~SANA() {
}


Alignment SANA::getStartingAlignment(){
    if(G1->hasNodeTypes()){
        Alignment randomAlig = Alignment::randomAlignmentWithNodeType(G1,G2);
        randomAlig.reIndexBefore_Iterations(G1->getNodeTypes_ReIndexMap());
        return randomAlig;
    }
    else if(lockFileName != ""){
		Alignment randomAlig = Alignment::randomAlignmentWithLocking(G1,G2);
		randomAlig.reIndexBefore_Iterations(G1->getLocking_ReIndexMap());
		return randomAlig;
	}
	else{
		return Alignment::random(n1, n2);
	}
}

Alignment SANA::run() {
	if (restart) return runRestartPhases();
	else {
		long long unsigned int iter = 0;

		if(maxIterations == 0){
			return simpleRun(getStartingAlignment(), minutes*60, iter);
		}else{
			return simpleRun(getStartingAlignment(), ((long long unsigned int)(maxIterations))*100000000, iter);
		}
	}
}


// Used for method #2 of locking
inline ushort SANA::G1RandomUnlockedNode_Fast(){
	ushort index = G1RandomUnlockedNodeDist(gen);
	return unLockedNodesG1[index];
}

inline ushort SANA::G1RandomUnlockedNode(){
    return G1RandomUnlockedNodeDist(gen); // method #3
            //  return G1RandomUnlockedNode_Fast(); Method #2

            // Method #1
            //  ushort node;
            //  do{
            //      node =  G1RandomNode(gen);
            //  }while(G1->isLocked(node));
            //  return node;
}

// Gives a random unlocked nodes with the same type as source1
inline ushort SANA::G1RandomUnlockedNode(uint source1){
    if(!nodesHaveType){
        return G1RandomUnlockedNodeDist(gen);
    }
    else{
        bool isGene = source1 < (uint) G1->unlockedGeneCount;
        if(isGene)
            return G1RandomUnlockedGeneDist(gen);
        else
            return G1->unlockedGeneCount + G1RandomUnlockedmiRNADist(gen);
    }
}
inline ushort SANA::G2RandomUnlockedNode(uint target1){
    if(!nodesHaveType){
        return G2RandomUnlockedNode_Fast(); // Method #2 and #3

                //  Method #1
                //  ushort node;
                //  do{
                //      node =  G2RandomUnassignedNode(gen);
                //  }while(G2->isLocked(unassignedNodesG2[node]));
                //  return node;
    }
    else
    {
        ushort node;
        do{
            node = G2RandomUnlockedNode_Fast(); // gives back unlocked G2 node
        }while(G2->nodeTypes[target1] != G2->nodeTypes[unassignedNodesG2[node]]);
        return node;
    }
}

inline ushort SANA::G2RandomUnlockedNode_Fast(){
	ushort node =  G2RandomUnassignedNode(gen);
	return node;
}


void SANA::describeParameters(ostream& sout) {
	sout << "Temperature schedule:" << endl;
	sout << "T_initial: " << TInitial << endl;
	sout << "T_decay: " << TDecay << endl;

	sout << "Optimize: " << endl;
	MC->printWeights(sout);

	if (maxIterations == 0){
		sout << "Execution time: ";
		if (restart) {sout << minutesNewAlignments + minutesPerCandidate*numCandidates + minutesFinalist;}
		else {sout << minutes;}
		sout << "m" << endl;
	}else{
		sout << "Planned Iterations Run: " << maxIterations << " sets of 100,000,000" << endl;
		sout << "Total Iterations Run: " << iterationCount << endl;
	}


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

	// candidates = vector<Alignment> (numCandidates, Alignment::random(n1, n2));
	candidates = vector<Alignment> (numCandidates, getStartingAlignment());
	candidatesScores = vector<double> (numCandidates, 0);
	for (uint i = 0; i < numCandidates; i++) {
		candidatesScores[i] = eval(candidates[i]);
	}
}

double SANA::temperatureFunction(double iter, double TInitial, double TDecay) {
	SANAtime = TDecayScaling*iter;
	return TInitialScaling * TInitial * (constantTemp ? 1 : exp(-TDecay*SANAtime));
}

double SANA::acceptingProbability(double energyInc, double T) {
	//return energyInc >= 0 ? 1 : exp(energyInc/T);
	return energyInc >= 0 ? 1 : pBad;
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

	//  Init unlockedNodesG1
	uint unlockedG1 = n1 - G1->getLockedCount();
	unLockedNodesG1 = vector<ushort> (unlockedG1);
	uint index = 0;
	for(uint i = 0; i < n1; i++){
		if(not G1->isLocked(i)){
			unLockedNodesG1[index] = i;
			index++;
		}
	}
	assert(index == unlockedG1);
	nodesHaveType = G1->hasNodeTypes();

	if (needAligEdges or needSec) {
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
		Measure* wec = MC->getMeasure("wec");
		double wecScore = wec->eval(A);
		wecSum = wecScore*2*g1Edges;
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
		T = temperatureFunction(iter, TInitial, TDecay);
		if (interrupt) {
			return A;
		}
		if (iter%iterationsPerStep == 0) {
			trackProgress(iter);
			if (iter != 0 and timer.elapsed() > maxExecutionSeconds) {
				return A;
			}
		} //This is somewhat redundant with iter, but this is specifically for counting total iterations in the entire SANA object.  If you want this changed, post a comment on one of Dillon's commits and he'll make it less redundant but he needs here for now.
		SANAIteration();
	}
	return A; //dummy return to shut compiler warning
}

Alignment SANA::simpleRun(const Alignment& startA, long long unsigned int maxExecutionIterations,
		long long unsigned int& iter) {

	initDataStructures(startA);

	setInterruptSignal();

	for (; ; iter++) {
		T = temperatureFunction(iter, TInitial, TDecay);
		if (interrupt) {
			return A;
		}
		if (iter%iterationsPerStep == 0) {
			trackProgress(iter);
		}
		if (iter != 0 and iter > maxExecutionIterations) {
			return A;
		}
		iterationCount++; //This is somewhat redundant with iter, but this is specifically for counting total iterations in the entire SANA object.  If you want this changed, post a comment on one of Dillon's commits and he'll make it less redundant but he needs here for now.
		SANAIteration();
	}
	return A; //dummy return to shut compiler warning
}

void SANA::SANAIteration() {
	if (randomReal(gen) <= changeProbability) performChange();
	else performSwap();
}

void SANA::performChange() {
	ushort source = G1RandomUnlockedNode();
	ushort oldTarget = A[source];

	uint newTargetIndex =  G2RandomUnlockedNode(oldTarget);
	ushort newTarget = unassignedNodesG2[newTargetIndex];

//	assert(!G1->isLocked(source));
//	assert(!G2->isLocked(newTarget));

//	bool G1Gene = source < G1->unlockedGeneCount;
//	bool G2Gene =  G2->nodeTypes[newTarget] == "gene";
//	assert((G1Gene && G2Gene) || (!G1Gene && !G2Gene));


	int newAligEdges = -1; //dummy initialization to shut compiler warnings
	if (needAligEdges or needSec) {
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
	bool makeChange = scoreComparison(newAligEdges, newInducedEdges, newLocalScoreSum, newWecSum, newCurrentScore);

	if (makeChange) {
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
	ushort source1 =  G1RandomUnlockedNode();
	ushort source2 =  G1RandomUnlockedNode(source1);
	ushort target1 = A[source1], target2 = A[source2];

//	if(!(source1 >= 0 and source1 < G1->getNumNodes()) || !(source2 >= 0 and source2 < G1->getNumNodes())){
//	    cerr << source1 << "   " << source2 << endl;
//	    cerr << G1->getNumNodes() << endl;
//	}
//	assert(source1 >= 0 and source1 < G1->getNumNodes());
//	assert(source2 >= 0 and source2 < G1->getNumNodes());
//
//	bool s1Gene = source1 < G1->unlockedGeneCount;
//    bool s2Gene = source2 < G1->unlockedGeneCount;
//    if(not((s1Gene && s2Gene) || (!s1Gene && !s2Gene))){
//        cerr << source1 << " " << source2 << endl;
//        cerr << G1->unlockedGeneCount << "   " << G1->getNumNodes() << endl;
//    }
//    assert((s1Gene && s2Gene) || (!s1Gene && !s2Gene));


	int newAligEdges = -1; //dummy initialization to shut compiler warnings
	if (needAligEdges or needSec) {
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
	bool makeChange = scoreComparison(newAligEdges, inducedEdges, newLocalScoreSum, newWecSum, newCurrentScore);

	if (makeChange) {
		A[source1] = target2;
		A[source2] = target1;
		aligEdges = newAligEdges;
		localScoreSum = newLocalScoreSum;
		wecSum = newWecSum;
		currentScore = newCurrentScore;
	}
}

double SANA::scoreComparison(double newAligEdges, double newInducedEdges, double newLocalScoreSum, double newWecSum, double& newCurrentScore) {
	bool makeChange = false;
	
	if(score == "sum") {
		newCurrentScore += ecWeight * (newAligEdges/g1Edges);
		newCurrentScore += s3Weight * (newAligEdges/(g1Edges+newInducedEdges-newAligEdges));
		newCurrentScore += secWeight * (newAligEdges/g1Edges+newAligEdges/g2Edges)*0.5;
		newCurrentScore += localWeight * (newLocalScoreSum/n1);
		newCurrentScore += wecWeight * (newWecSum/(2*g1Edges));

		energyInc = newCurrentScore-currentScore;
		makeChange = (energyInc >= 0 or randomReal(gen) <= exp(energyInc/T));
	}
	else if(score == "product") {
		newCurrentScore = 1;
		newCurrentScore *= ecWeight * (newAligEdges/g1Edges);
		newCurrentScore *= s3Weight * (newAligEdges/(g1Edges+newInducedEdges-newAligEdges));
		newCurrentScore *= localWeight * (newLocalScoreSum/n1);
		newCurrentScore *= secWeight * (newAligEdges/g1Edges+newAligEdges/g2Edges)*0.5;
		newCurrentScore *= wecWeight * (newWecSum/(2*g1Edges));
		
		energyInc = newCurrentScore-currentScore;
		makeChange = (energyInc >= 0 or randomReal(gen) <= exp(energyInc/T));
	}
	else if(score == "max") {
		double deltaEnergy = max(max(ecWeight*(newAligEdges/g1Edges - aligEdges/g1Edges),max(
									 s3Weight*((newAligEdges/(g1Edges+newInducedEdges-newAligEdges) - (aligEdges/(g1Edges+inducedEdges-aligEdges)))),
									 secWeight*0.5*(newAligEdges/g1Edges - aligEdges/g1Edges + newAligEdges/g2Edges - aligEdges/g2Edges))),
								 max(localWeight*((newLocalScoreSum/n1) - (localScoreSum)),
									 wecWeight*(newWecSum/(2*g1Edges) - wecSum/(2*g1Edges))));

		newCurrentScore += ecWeight * (newAligEdges/g1Edges);
		newCurrentScore += secWeight * (newAligEdges/g1Edges+newAligEdges/g2Edges)*0.5;
		newCurrentScore += s3Weight * (newAligEdges/(g1Edges+newInducedEdges-newAligEdges));
		newCurrentScore += localWeight * (newLocalScoreSum/n1);
		newCurrentScore += wecWeight * (newWecSum/(2*g1Edges));
		
		energyInc = newCurrentScore - currentScore;
		makeChange = deltaEnergy >= 0 or randomReal(gen) <= exp(energyInc/T);
	}
	else if(score == "min") {
		double deltaEnergy = min(min(ecWeight*(newAligEdges/g1Edges - aligEdges/g1Edges),min(
									 s3Weight*((newAligEdges/(g1Edges+newInducedEdges-newAligEdges) - (aligEdges/(g1Edges+inducedEdges-aligEdges)))),
									 secWeight*0.5*(newAligEdges/g1Edges - aligEdges/g1Edges + newAligEdges/g2Edges - aligEdges/g2Edges))),
								 min(localWeight*((newLocalScoreSum/n1) - (localScoreSum)),
									 wecWeight*(newWecSum/(2*g1Edges) - wecSum/(2*g1Edges))));
		
		newCurrentScore += ecWeight * (newAligEdges/g1Edges);
		newCurrentScore += s3Weight * (newAligEdges/(g1Edges+newInducedEdges-newAligEdges));
		newCurrentScore += secWeight * (newAligEdges/g1Edges+newAligEdges/g2Edges)*0.5;
		newCurrentScore += localWeight * (newLocalScoreSum/n1);
		newCurrentScore += wecWeight * (newWecSum/(2*g1Edges));
		
		energyInc = newCurrentScore - currentScore;
		makeChange = deltaEnergy >= 0 or randomReal(gen) <= exp(newCurrentScore/T);
	}
	else if(score == "inverse") {
		newCurrentScore += ecWeight/(newAligEdges/g1Edges);
		newCurrentScore += secWeight * (newAligEdges/g1Edges+newAligEdges/g2Edges)*0.5;
		newCurrentScore += s3Weight/(newAligEdges/(g1Edges+newInducedEdges-newAligEdges));
		newCurrentScore += localWeight/(newLocalScoreSum/n1);
		newCurrentScore += wecWeight/(newWecSum/(2*g1Edges));

		energyInc = newCurrentScore-currentScore;
		makeChange = (energyInc >= 0 or randomReal(gen) <= exp(energyInc/T));
	}
	else if(score == "maxFactor") {
		double maxScore = max(max(ecWeight*(newAligEdges/g1Edges - aligEdges/g1Edges),max(
									 s3Weight*((newAligEdges/(g1Edges+newInducedEdges-newAligEdges) - (aligEdges/(g1Edges+inducedEdges-aligEdges)))),
									 secWeight*0.5*(newAligEdges/g1Edges - aligEdges/g1Edges + newAligEdges/g2Edges - aligEdges/g2Edges))),
							  max(localWeight*((newLocalScoreSum/n1) - (localScoreSum)),
									 wecWeight*(newWecSum/(2*g1Edges) - wecSum/(2*g1Edges))));
									 
		double minScore = min(min(ecWeight*(newAligEdges/g1Edges - aligEdges/g1Edges),min(
									 s3Weight*((newAligEdges/(g1Edges+newInducedEdges-newAligEdges) - (aligEdges/(g1Edges+inducedEdges-aligEdges)))),
									 secWeight*0.5*(newAligEdges/g1Edges - aligEdges/g1Edges + newAligEdges/g2Edges - aligEdges/g2Edges))),
							  min(localWeight*((newLocalScoreSum/n1) - (localScoreSum)),
									 wecWeight*(newWecSum/(2*g1Edges) - wecSum/(2*g1Edges))));

		newCurrentScore += ecWeight * (newAligEdges/g1Edges);
		newCurrentScore += secWeight * (newAligEdges/g1Edges+newAligEdges/g2Edges)*0.5;
		newCurrentScore += s3Weight * (newAligEdges/(g1Edges+newInducedEdges-newAligEdges));
		newCurrentScore += localWeight * (newLocalScoreSum/n1);
		newCurrentScore += wecWeight * (newWecSum/(2*g1Edges));
		
		energyInc = newCurrentScore - currentScore;
		makeChange = maxScore >= -1 * minScore or randomReal(gen) <= exp(energyInc/T);
	}

	return makeChange;
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
	//original one is commented out for testing sec 
	//if (printDetails) cerr << " (" << Al.numAlignedEdges(*G1, *G2) << ", " << G2->numNodeInducedSubgraphEdges(A) << ")";
	if (printDetails) cerr << "Al.numAlignedEdges = " << Al.numAlignedEdges(*G1, *G2) << ", g1Edges = " <<g1Edges<< " ,g2Edges = "<<g2Edges<< endl;
	if (printScores) {
		SymmetricSubstructureScore S3(G1, G2);
		EdgeCorrectness EC(G1, G2);
		SymmetricEdgeCoverage SEC(G1,G2);
		cerr << "S3: " << S3.eval(Al) << "  EC: " << EC.eval(Al) << "  SEC: " << SEC.eval(Al) <<endl;
	}
	if (checkScores) {
		double realScore = eval(Al);
		if (realScore-currentScore > 0.000001) {
			cerr << "internal error: incrementally computed score (" << currentScore;
			cerr << ") is not correct (" << realScore << ")" << endl;
		}
	}
	if (dynamic_tdecay) { // Code for estimating dynamic TDecay 
		//The dynamic method uses linear interpolation to obtain an 
		//an "ideal" P(bad) as a basis for SANA runs. If the current P(bad)
		//is significantly different from out "ideal" P(bad), then decay is either 
		//"sped up" or "slowed down"
		int NSteps = 100; 
		double fractional_time = (timer.elapsed()/(minutes*60)); 
		double lowIndex = fmax(0,floor(NSteps*fractional_time));
		double highIndex = fmin(NSteps,ceil(NSteps*fractional_time));
		double betweenFraction = NSteps*fractional_time - lowIndex;
		double PLow = tau[lowIndex];
		double PHigh = tau[highIndex]; 
		
		double PBetween = PLow + betweenFraction * (PHigh - PLow);
		pBad = PBetween;
		return;
		
		// if the ratio if off by more than a few percent, adjust.
		double ratio = acceptingProbability(avgEnergyInc, T) / PBetween;
		if (abs(1-ratio) >= .01 &&
		    (ratio < 1 || SANAtime > .2)) // don't speed it up too soon
		{
		    //cerr << "avgEnergyInc " << avgEnergyInc << " TInitialScaling " << TInitialScaling << " TInitial " << TInitial << " PBetween " << PBetween << " TDecayScaling " << TDecayScaling << " SANAtime " << SANAtime << endl;
		    double shouldBe;
		    shouldBe = -log(avgEnergyInc/(TInitialScaling*TInitial*log(PBetween)))/(SANAtime);
		    if(SANAtime==0 || shouldBe != shouldBe || shouldBe <= 0) 
			shouldBe = TDecay * (ratio >= 0 ? ratio*ratio : 0.5);
		    cerr << "TDecay " << TDecay << " too ";
		    cerr << (ratio < 1 ? "fast" : "slow") << " shouldBe " << shouldBe;
		    TDecay = sqrt(TDecay * shouldBe); // geometric mean
		    //TDecay = (TDecay + shouldBe)/2; // arithmetic mean
		    cerr << "; try " << TDecay << endl;
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
		// Alignment A = simpleRun(Alignment::random(n1, n2), 0.0, iter);
		Alignment A = simpleRun(getStartingAlignment(), 0.0, iter);
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
	setTInitialAutomatically();
	setTDecayAutomatically();
}

void SANA::setTInitialAutomatically() {
	//TInitial = searchTInitial(); // Nil's code using fancy statistics
	//TInitial = simpleSearchTInitial(); // Wayne's simplistic "make it bigger!!" code
}

void SANA::setTDecayAutomatically() {
	if(maxIterations == 0){
		TDecay = searchTDecay(TInitial, minutes);
	}else{
		TDecay = searchTDecay(TInitial, maxIterations);
	}
	//TDecay /= 2; // always seems too fast; dynamic TDecay will fix it.
}

double SANA::searchTInitial() {
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

	double lowerBoundTInitial = 0;
	double upperBoundTInitial = 1;
	while (not isRandomTInitial(upperBoundTInitial, highThresholdScore, lowThresholdScore)) {
		cerr << endl;
		upperBoundTInitial *= 2;
	}
	upperBoundTInitial *= 2;	// one more doubling just to be sure
	cerr << endl;
	if (upperBoundTInitial > 1) lowerBoundTInitial = upperBoundTInitial/4;

	uint n1 = G1->getNumNodes();
	uint n2 = G2->getNumNodes();
	cerr << "Iterations per run: " << 10000.+100.*n1+10.*n2+n1*n2*0.1 << endl;

	uint count = 0;
	T.start();
	while (fabs(lowerBoundTInitial - upperBoundTInitial)/lowerBoundTInitial > 0.05 and
			count <= 10) {
		//search in log space
		double lowerBoundTInitialLog = log2(lowerBoundTInitial+1);
		double upperBoundTInitialLog = log2(upperBoundTInitial+1);
		double midTInitialLog = (lowerBoundTInitialLog+upperBoundTInitialLog)/2.;
		double midTInitial = exp2(midTInitialLog)-1;

		//we prefer false negatives (random scores classified as non-random)
		//than false positives (non-random scores classified as random)
		cerr << "Test " << count << " (" << T.elapsedString() << "): ";
		count++;
		if (isRandomTInitial(midTInitial, highThresholdScore, lowThresholdScore)) {
			upperBoundTInitial = midTInitial;
			cerr << " (random behavior)";
		}
		else {
			lowerBoundTInitial = midTInitial;
			cerr << " (NOT random behavior)";
		}
		cerr << " New range: (" << lowerBoundTInitial << ", " << upperBoundTInitial << ")" << endl;
	}
	//return the top of the range
	cerr << "Final range: (" << lowerBoundTInitial << ", " << upperBoundTInitial << ")" << endl;
	cerr << "Final TInitial: " << upperBoundTInitial << endl;
	return upperBoundTInitial;
}

//takes a random alignment, lets it run for a certain number
//of iterations (ITERATIONS) with fixed temperature TInitial
//and returns its score
double SANA::scoreForTInitial(double TInitial) {
	uint ITERATIONS = 10000.+100.*n1+10.*n2+n1*n2*0.1; //heuristic value

	double oldIterationsPerStep = this->iterationsPerStep;
	double oldTInitial = this->TInitial;
	bool oldRestart = restart;

	this->iterationsPerStep = ITERATIONS;
	this->TInitial = TInitial;
	constantTemp = true;
	enableTrackProgress = false;
	restart = false;

	long long unsigned int iter = 0;
	// simpleRun(Alignment::random(n1, n2), 0.0, iter);
	simpleRun(getStartingAlignment(), 0.0, iter);
	this->iterationsPerStep = oldIterationsPerStep;
	this->TInitial = oldTInitial;
	constantTemp = false;
	enableTrackProgress = true;
	restart = oldRestart;

	return currentScore;
}

bool SANA::isRandomTInitial(double TInitial, double highThresholdScore, double lowThresholdScore) {
	const double NUM_SAMPLES = 5;

	double score = scoreForTInitial(TInitial);
	cerr << "T_initial = " << TInitial << ", score = " << score;
	//quick filter all the scores that are obviously not random
	if (score > highThresholdScore) return false;
	if (score < lowThresholdScore) return true;
	//make sure that alignments that passed the first test are truly random
	//(among NUM_SAMPLES runs, at least one of them has a p-value smaller than LOW_THRESHOLD_P)
	for (uint i = 0; i < NUM_SAMPLES; i++) {
		if (scoreForTInitial(TInitial) <= lowThresholdScore) return true;
	}
	return false;
}

double SANA::scoreRandom() {
	//    return eval(Alignment::random(n1, n2));
	return eval(Alignment::randomAlignmentWithLocking(G1,G2));
}

double SANA::searchSpaceSizeLog() {
	//the search space size is (n2 choose n1) * n1!
	//we use the stirling approximation
	if (n2 == n1) return n1*log(n1)-n1;
	return n2*log(n2)-(n2-n1)*log(n2-n1)-n1;
}

long long unsigned int SANA::hillClimbingIterations(long long unsigned int idleCountTarget) {
	// Alignment startA = Alignment::random(n1, n2);
	Alignment startA = getStartingAlignment();
	long long unsigned int iter = 0;

	//cerr << "We consider that SANA has stagnated if it goes ";
	//cerr << idleCountTarget << " without improving" << endl;

	initDataStructures(startA);
	T = 0;
	long long unsigned int idleCount = 0;
	for (; ; iter++) {
		if (iter%iterationsPerStep == 0) {
			trackProgress(iter);
		}
		double oldScore = currentScore;
		SANAIteration();
		if (abs(oldScore-currentScore) < 0.00001) idleCount++;
		else idleCount = 0;
		if (idleCount == idleCountTarget) {
			return  (iter+1) - idleCount;
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
vector<double> SANA::energyIncSample(double temp) {

	getIterPerSecond(); //this runs HC, so it updates the values
	//of A and currentScore (besides iterPerSecond)

	double iter = iterPerSecond;
	//cout << "Hill climbing score: " << currentScore << endl;
	//generate a sample of energy increments, with size equal to the number of iterations per second
	vector<double> EIncs(0);
	T = temp;
	for (uint i = 0; i < iter; i++) {
		SANAIteration();
		if (energyInc < 0) {
			EIncs.push_back(energyInc);
		}
	}
	avgEnergyInc = vectorMean(EIncs);
	return EIncs;
}

double SANA::simpleSearchTInitial() {
	T = .5e-6;
	double pBad;
	do {
	    T *= 2;
	    vector<double> EIncs = energyIncSample(T);
	    cerr << "Trying TInitial " << T;
	    //uint nBad = 0;
	    //for(uint i=0; i<EIncs.size();i++)
		//nBad += (randomReal(gen) <= exp(EIncs[i]/T));
	    pBad = exp(avgEnergyInc/T); // (double)nBad/(EIncs.size());
	    cerr << " p(Bad) = " << pBad << endl;
	} while(pBad < 0.9999999); // How close to 1? I pulled this out of my ass.
	return T;
}

double SANA::searchTDecay(double TInitial, double minutes) {

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
	double iter_t = minutes*60*getIterPerSecond();

	double lambda = log((TInitial*TInitialScaling)/epsilon)/(iter_t*TDecayScaling);
	cerr << "Final T_decay: " << lambda << endl;
	return lambda;
}

double SANA::searchTDecay(double TInitial, uint iterations) {

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
	long long unsigned int iter_t = (long long unsigned int)(iterations)*100000000;

	double lambda = log((TInitial*TInitialScaling)/epsilon)/(iter_t*TDecayScaling);
	cerr << "Final T_decay: " << lambda << endl;
	return lambda;
}

double SANA::getIterPerSecond() {
	if (not initializedIterPerSecond) {
		initIterPerSecond();
	}
	return iterPerSecond;
}

void SANA::initIterPerSecond() {
	long long unsigned int iter = hillClimbingIterations(500000+searchSpaceSizeLog());
	if (iter == 0) {
		throw runtime_error("hill climbing stagnated after 0 iterations");
	}
	double res = iter/timer.elapsed();
	cerr << "SANA does " << to_string(res) << " iterations per second" << endl;

	initializedIterPerSecond = true;
	iterPerSecond = res;
}

void SANA::setDynamicTDecay() {
	dynamic_tdecay = true; 	
}

