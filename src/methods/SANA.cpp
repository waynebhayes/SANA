#include <string>
#include <vector>
#include <utility>
#include <iostream>
#include <fstream>
#include <iomanip>
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
#include <limits>
#include <thread>
#include <mutex>
#include <cassert>
#include <signal.h>
#include <stdio.h>
#include <unistd.h>

#include "SANA.hpp"
#include "../measures/SymmetricSubstructureScore.hpp"
#include "../measures/JaccardSimilarityScore.hpp"
#include "../measures/InducedConservedStructure.hpp"
#include "../measures/EdgeCorrectness.hpp"
#include "../measures/EdgeDifference.hpp"
#include "../measures/EdgeRatio.hpp"
#include "../measures/SquaredEdgeScore.hpp"
#include "../measures/WeightedEdgeConservation.hpp"
#include "../measures/TriangleCorrectness.hpp"
#include "../measures/NodeCorrectness.hpp"
#include "../measures/SymmetricEdgeCoverage.hpp"
#include "../measures/localMeasures/Sequence.hpp"
#include "../measures/EdgeExposure.hpp"
#include "../measures/MultiS3.hpp"
#include "../utils/utils.hpp"
#include "../report.hpp"


using namespace std;

#ifdef CORES
// All comparisons with nan variables are false, including self-equality. Thus if it's not equal to itself, it's NAN.
// Note you can't just do (x!=x), because that's always false, NAN or not.
static bool myNan(double x) { return !(x==x); }
#endif

static long long _maxExecutionIterations;

void SANA::initTau(void) {
    /*
    tau = vector<double> {
    1.000, 0.985, 0.970, 0.960, 0.950, 0.942, 0.939, 0.934, 0.928, 0.920,
    0.918, 0.911, 0.906, 0.901, 0.896, 0.891, 0.885, 0.879, 0.873, 0.867,
    0.860, 0.853, 0.846, 0.838, 0.830, 0.822, 0.810, 0.804, 0.794, 0.784,
    0.774, 0.763, 0.752, 0.741, 0.728, 0.716, 0.703, 0.690, 0.676, 0.662,
    0.647, 0.632, 0.616, 0.600, 0.584, 0.567, 0.549, 0.531, 0.514, 0.495,
    0.477, 0.458, 0.438, 0.412, 0.400, 0.381, 0.361, 0.342, 0.322, 0.303,
    0.284, 0.264, 0.246, 0.228, 0.210, 0.193, 0.177, 0.161, 0.145, 0.131,
    0.116, 0.104, 0.092, 0.081, 0.070, 0.061, 0.052, 0.044, 0.0375, 0.031,
    0.026, 0.0212, 0.0172, 0.0138, 0.011, 0.008, 0.006, 0.005, 0.004, 0.003,
    0.002, 0.001, 0.0003, 0.0001, 3e-5, 1e-6, 0, 0, 0, 0,
    0};
     */
    tau = vector<double> {
        0.996738, 0.994914, 0.993865, 0.974899, 0.977274, 0.980926, 0.97399, 0.970583, 0.967492, 0.962373,
        0.953197, 0.954104, 0.951387, 0.953532, 0.948492, 0.939501, 0.939128, 0.932902, 0.912378, 0.896011,
        0.89535, 0.88642, 0.874628, 0.856721, 0.855782, 0.838483, 0.820407, 0.784303, 0.771297, 0.751457,
        0.735902, 0.676393, 0.633939, 0.604872, 0.53482, 0.456856, 0.446905, 0.377708, 0.337258, 3.04e-01,
        0.280585, 0.240093, 1.95e-01, 1.57e-01, 1.21e-01, 1.00e-01, 8.04e-02, 5.95e-02, 4.45e-02, 3.21e-02,
        1.81e-02, 1.82e-02, 1.12e-02, 7.95e-03, 4.82e-03, 3.73e-03, 2.11e-03, 1.41e-03, 9.69e-04, 6.96e-04,
        5.48e-04, 4.20e-04, 4.00e-04, 3.50e-04, 3.10e-04, 2.84e-04, 2.64e-04, 1.19e-04, 8.16e-05, 7.22e-05,
        6.16e-05, 4.46e-05, 3.36e-05, 2.66e-05, 1.01e-05, 9.11e-06, 4.09e-06, 3.96e-06, 3.43e-06, 3.12e-06,
        2.46e-06, 2.02e-06, 1.85e-06, 1.72e-06, 1.10e-06, 9.13e-07, 8.65e-07, 8.21e-07, 7.26e-07, 6.25e-07,
        5.99e-07, 5.42e-07, 8.12e-08, 4.16e-08, 6.56e-09, 9.124e-10, 6.1245e-10, 3.356e-10, 8.124e-11, 4.587e-11};
}

SANA::SANA(Graph* G1, Graph* G2,
        double TInitial, double TDecay, double t, bool usingIterations, bool addHillClimbing, MeasureCombination* MC, string& objectiveScore
#ifdef MULTI_PAIRWISE
        ,string& startAligName
#endif
        ): Method(G1, G2, "SANA_"+MC->toString()),
           G1FloatWeights(G1->getFloatWeights()),
           G2FloatWeights(G2->getFloatWeights())
{
    //data structures for the networks
    n1              = G1->getNumNodes();
    n2              = G2->getNumNodes();
    g1Edges         = G1->getNumEdges();
    pairsCount      = n1 * (n1 + 1) / 2;
#ifdef MULTI_PAIRWISE
    g1WeightedEdges = G1->getWeightedNumEdges();
    g2WeightedEdges = G2->getWeightedNumEdges();
#endif
    g2Edges         = G2->getNumEdges();
    if (objectiveScore == "sum")
        score = Score::sum;
    else if (objectiveScore == "product")
        score = Score::product;
    else if (objectiveScore == "inverse")
        score = Score::inverse;
    else if (objectiveScore == "max")
        score = Score::max;
    else if (objectiveScore == "min")
        score = Score::min;
    else if (objectiveScore == "maxFactor")
        score = Score::maxFactor;
    else if (objectiveScore == "pareto")
        score = Score::pareto;

    paretoInitial   = MC->getParetoInitial();
    paretoCapacity  = MC->getParetoCapacity();
    paretoIterations = MC->getParetoIterations();
    paretoThreads = MC->getParetoThreads();

    G1->getMatrix(G1Matrix);
    G2->getMatrix(G2Matrix);
    G1->getAdjLists(G1AdjLists);
#ifdef MULTI_PAIRWISE
    if (startAligName != "") {
        prune(startAligName);
        this->startAligName = startAligName;
    }
#endif
    G2->getAdjLists(G2AdjLists);

    //random number generation
    random_device rd;
    gen                       = mt19937(getRandomSeed());
    G1RandomNode              = uniform_int_distribution<>(0, n1-1);
    uint G1UnLockedCount      = n1 - G1->getLockedCount() -1;
    G1RandomUnlockedNodeDist  = uniform_int_distribution<>(0, G1UnLockedCount);
    G2RandomUnassignedNode    = uniform_int_distribution<>(0, n2-n1-1);
    
    
    if(G1->getNumMultipartite() == 2) {
        G1RandomUnlockedGeneDist  = uniform_int_distribution<>(0, G1->unlockedGeneCount - 1);
        G1RandomUnlockedmiRNADist = uniform_int_distribution<>(0, G1->unlockedmiRNACount - 1);

        G2RandomUnassignedGeneDist = uniform_int_distribution<>(0, G2->geneCount - G1->geneCount -1);
        G2RandomUnassignedmiRNADist = uniform_int_distribution<>(0, G2->miRNACount - G1->miRNACount -1);
        
    }
    
    
    
    randomReal                = uniform_real_distribution<>(0, 1);

    //temperature schedule
    this->TInitial        = TInitial;
    this->TDecay          = TDecay;
    this->usingIterations = usingIterations;

    if (this->usingIterations) {
        maxIterations = (uint)(t);
    } else {
        minutes = t;
    }
    initializedIterPerSecond = false;
    pBadBuffer = vector<double> (PBAD_CIRCULAR_BUFFER_SIZE, 0);

    //data structures for the solution space search
    assert(G1->isBipartite() == G2->isBipartite());
    uint ramificationChange[2], ramificationSwap[2], totalRamification[2];
    if(G1->isBipartite()) {
	assert(G1->geneCount <= G2->geneCount);
	assert(G1->miRNACount <= G2->miRNACount);
	ramificationChange[0] = G1->geneCount*(G2->geneCount - G1->geneCount);
	ramificationChange[1] = G1->miRNACount*(G2->miRNACount - G1->miRNACount);
	ramificationSwap[0]   = G1->geneCount*(G1->geneCount - 1)/2;
	ramificationSwap[1]   = G1->miRNACount*(G1->miRNACount - 1)/2;
	totalRamification[0]  = ramificationSwap[0] + ramificationChange[0];
	totalRamification[1]  = ramificationSwap[1] + ramificationChange[1];
	changeProbability[0]       = (double) ramificationChange[0]/totalRamification[0];
	changeProbability[1]       = (double) ramificationChange[1]/totalRamification[1];
    }
    else
    {
	ramificationChange[0] = n1*(n2-n1);
	ramificationSwap[0]   = n1*(n1-1)/2;
	totalRamification[0]  = ramificationSwap[0] + ramificationChange[0];
	changeProbability[0] = (double)ramificationChange[0]/totalRamification[0];
    }

    initTau();

    //objective function
    this->MC  = MC;
    ecWeight  = MC->getWeight("ec");
    edWeight  = MC->getWeight("ed");
    erWeight  = MC->getWeight("er");
    s3Weight  = MC->getWeight("s3");
    jsWeight  = MC->getWeight("js");
    icsWeight = MC->getWeight("ics");
    secWeight = MC->getWeight("sec");
    mecWeight = MC->getWeight("mec");
    sesWeight = MC->getWeight("ses");
    eeWeight  = MC->getWeight("ee");
    ms3Weight = MC->getWeight("ms3");

    try {
        wecWeight = MC->getWeight("wec");
    } catch(...) {
        wecWeight = 0;
    }

    try {
        ewecWeight = MC->getWeight("ewec");
    } catch(...) {
        ewecWeight = 0;
    }

    try {
        needNC      = false;
        ncWeight    = MC->getWeight("nc");
        Measure* nc = MC->getMeasure("nc");
        trueA       = nc->getMappingforNC();
        needNC      = true;
    } catch(...) {
        ncWeight = 0;
        trueA    = {static_cast<uint>(G2->getNumNodes()), 1};
    }

    try {
        TCWeight = MC->getWeight("tc");
    } catch (...) {
        TCWeight = 0;
    }

    localWeight = MC->getSumLocalWeight();


    restart              = false; //restart scheme
    dynamic_tdecay       = false; //temperature decay dynamically
    needAligEdges        = icsWeight > 0 || ecWeight > 0 || s3Weight > 0 || wecWeight > 0 || secWeight > 0 || mecWeight > 0; //to evaluate EC incrementally
    needEd               = edWeight > 0; // to evaluate edge difference score incrementally
    needEr               = erWeight > 0; // to evaluate edge ratio score incrementally
    needSquaredAligEdges = sesWeight > 0; // to evaluate SES incrementally
    needExposedEdges	 = eeWeight > 0 || MultiS3::denominator_type == MultiS3::ee_global; // to eval EE incrementally; if needMS3, might use ee as denom
    needMS3              = ms3Weight > 0; // to eval MS3 incrementally
    needInducedEdges     = s3Weight > 0 || icsWeight > 0 ; //to evaluate S3 & ICS incrementally
    needJs               = jsWeight > 0; //to evaluate JS incrementally
    needAlignedByNode    = jsWeight > 0; //to evaluate JS incrementally
    needWec              = wecWeight > 0; //to evaluate WEC incrementally
    needEwec             = ewecWeight>0; //to evaluate EWEC incrementally
    needSec              = secWeight > 0; //to evaluate SEC incrementally
    needTC               = TCWeight > 0; //to evaluation TC incrementally
    needLocal            = localWeight > 0;

    if (needWec) {
        Measure* wec                     = MC->getMeasure("wec");
        LocalMeasure* m                  = ((WeightedEdgeConservation*) wec)->getNodeSimMeasure();
        vector<vector<float> >* wecSimsP = m->getSimMatrix();
        wecSims                          = (*wecSimsP);
    }
#ifdef CORES
#if UNWEIGHTED_CORES
    numPegSamples = vector<unsigned long>(n1, 0);
    pegHoleFreq = Matrix<unsigned long>(n1, n2);
#endif
    weightedPegHoleFreq_pBad = Matrix<double>(n1, n2);
    totalWeightedPegWeight_pBad = vector<double>(n1, 0);
    weightedPegHoleFreq_1mpBad = Matrix<double>(n1, n2);
    totalWeightedPegWeight_1mpBad = vector<double>(n1, 0);
#endif
    //to evaluate local measures incrementally
    if (needLocal) {
        sims              = MC->getAggregatedLocalSims();
        localSimMatrixMap = MC->getLocalSimMap();
        localWeight       = 1; //the values in the sim matrix 'sims' have already been scaled by the weight
    } else {
        localWeight = 0;
    }


    //other execution options
    constantTemp        = false;
    enableTrackProgress = true;
    iterationsPerStep   = 10000000;

    //Make sana think were running in (score == sum) to avoid crashing during linear regression.
    if(score == Score::pareto)
        score = Score::sum;

    /*
    this does not need to be initialized here,
    but the space has to be reserved in the
    stack. it is initialized properly before
    running the algorithm
    */
    assignedNodesG2       = new vector<bool> (n2);
    unassignedNodesG2     = new vector<uint> (n2-n1);
    unassignedgenesG2     = new vector<uint> (G2->geneCount - G1->geneCount);
    unassignedmiRNAsG2    = new vector<uint> (G2->miRNACount - G1->miRNACount);
    
    
    unassignedMultipartiteNodes = new vector<vector<uint>>();
    for(int i = 0; i < G1->getNumMultipartite(); i++)
        unassignedMultipartiteNodes->push_back(vector<uint>{});
        
    A                     = new vector<uint> (n1);
    avgEnergyInc          = -0.00001; //to track progress
    this->addHillClimbing = addHillClimbing;

    //check if graph is fully connected
    int g1MaxEdges = n1 * (n1 - 1) / 2;
    int g2MaxEdges = n2 * (n2 - 1) / 2;
    if ((g1Edges == g1MaxEdges || g2Edges == g2MaxEdges)
         && !needLocal && !needEd && !needEr) {
        throw runtime_error("FATAL ERROR: At least one graph is fully connected."
            " Every alignment is a perfect alignment, thus SANA cannot pick the best alignment.");
    }

}

Alignment SANA::getStartingAlignment(){
    Alignment startAlig;

    if(this->startAligName != "")
        startAlig = Alignment::loadEdgeList(G1, G2, startAligName);
    else if(G1->isBipartite())
        startAlig = Alignment::startingMultipartiteAlignment(G1,G2);
    else if(G1->getNumMultipartite() > 1)
        startAlig = Alignment::startingMultipartiteAlignment(G1,G2);
    else if (lockFileName != "")
        startAlig = Alignment::randomAlignmentWithLocking(G1,G2);
    else
        startAlig = Alignment::random(n1, n2);
        

    // Doing a Rexindexing if required
#ifdef REINDEX
    if (G1->isBipartite()) {
    	startAlig.reIndexBefore_Iterations(G1->getBipartiteNodeTypes_ReIndexMap());
    }
    else if (lockFileName != "") {
	    startAlig.reIndexBefore_Iterations(G1->getLocking_ReIndexMap());
    }
#endif

    return startAlig;
}

SANA::~SANA(){
    delete assignedNodesG2;
    delete unassignedNodesG2;
    delete unassignedgenesG2;
    delete unassignedmiRNAsG2;
    delete A;
    delete storedAlignments;
}

/*
** The following is designed so that every single node from both networks
** is printed at least once. First, we find for every single node (across
** BOTH networks) what it's *highest* score is with a partner in the other
** network. Once we compute every node's highest score, we then go and find
** the smallest such score, and call it Smin. This defines the minimum score
** that we want to output, and it guarantees that every node appears in at
** least one aligned node-pair according to this score.  We output this Smin.
*/
double SANA::TrimCoreScores(Matrix<unsigned long>& Freq, vector<unsigned long>& numPegSamples)
{
    uint n1 = Freq.size(), n2 = Freq[0].size();
    vector<double> high1(n1,0.0);
    vector<double> high2(n2,0.0);

    for(uint i=0; i<n1; i++){
	double denom =  1.0 / (double)numPegSamples[i];
	for(uint j=0;j<n2; j++)
	{
	    double score = Freq[i][j] * denom;
	    if(score > high1[i]) high1[i] = score;
	    if(score > high2[j]) high2[j] = score;
	}
    }
    double Smin = high1[0];
    for(uint i=0;i<n1;i++) if(high1[i] < Smin) Smin = high1[i];
    for(uint j=0;j<n2;j++) if(high2[j] < Smin) Smin = high2[j];
    return Smin;
}

double SANA::TrimCoreScores(Matrix<double>& Freq, vector<double>& totalPegWeight)
{
    uint n1 = Freq.size(), n2 = Freq[0].size();
    vector<double> high1(n1,0.0);
    vector<double> high2(n2,0.0);

    for(uint i=0; i<n1; i++){
	double denom =  1.0 / (double)totalPegWeight[i];
	for(uint j=0;j<n2; j++)
	{
	    double score = Freq[i][j] * denom;
	    if(score > high1[i]) high1[i] = score;
	    if(score > high2[j]) high2[j] = score;
	}
    }
    double Smin = high1[0];
    for(uint i=0;i<n1;i++) if(high1[i] < Smin) Smin = high1[i];
    for(uint j=0;j<n2;j++) if(high2[j] < Smin) Smin = high2[j];
    return Smin;
}

Alignment SANA::run() {
    if (restart)
        return runRestartPhases();
    else {
        long long int iter = 0;
        Alignment align;
        if(!usingIterations) {
          cout << "usingIterations = 0" << endl;
	  double leeway = 2;
          align = simpleRun(getStartingAlignment(), minutes * 60 * leeway, (long long int) (getIterPerSecond()*minutes*60), iter);
        }
        else {
          cout << "usingIterations = 1" << endl;
          align = simpleRun(getStartingAlignment(), ((long long int)(maxIterations))*10000000, iter);
        }

        if(addHillClimbing){
            Timer hill;
            hill.start();
            cout << "Adding HillClimbing at the end.. ";
            align = hillClimbingAlignment(align, (long long int)(10000000)); //arbitrarily chosen, probably too big.
            cout << hill.elapsedString() << endl;
        }
	cout << "Performed " << iter << " total iterations\n";
#define PRINT_CORES 0
#define MIN_CORE_SCORE 1e-4
#if PRINT_CORES
#ifndef CORES
#error must have CORES macro defined to print them
#else
#if UNWEIGHTED_CORES
	double SminUnW = TrimCoreScores(pegHoleFreq,numPegSamples);
	cout << "Smin_UnW "<< SminUnW << " ";
#endif
	double Smin_pBad =TrimCoreScores(weightedPegHoleFreq_pBad, totalWeightedPegWeight_pBad);
	double Smin_1mpBad =TrimCoreScores(weightedPegHoleFreq_1mpBad, totalWeightedPegWeight_1mpBad);
	cout << "Smin_pBad "<< Smin_pBad << " Smin_(1-pBad) " << Smin_1mpBad << endl;

	unordered_map<uint,string> G1Index2Name = G1->getIndexToNodeNameMap();
	unordered_map<uint,string> G2Index2Name = G2->getIndexToNodeNameMap();
	printf("######## core frequencies#########\n");
	printf("p1 p2");
#if UNWEIGHTED_CORES
	printf(" unwgtd");
#endif
	printf("wpB w1_pB\n");
	for(uint i=0; i<n1; i++) for(uint j=0; j<n2; j++)
	{
#if UNWEIGHTED_CORES
	    double unweightdedScore = pegHoleFreq[i][j]/(double)numPegSamples[i];
#endif
	    double weightedScore_pBad = weightedPegHoleFreq_pBad[i][j]/totalWeightedPegWeight_pBad[i];
	    double weightedScore_1mpBad = weightedPegHoleFreq_1mpBad[i][j]/totalWeightedPegWeight_1mpBad[i];
	    if(
#if UNWEIGHTED_CORES
		unweightdedScore  >= max(MIN_CORE_SCORE,SminUnW) ||
#endif
		weightedScore_pBad >= max(MIN_CORE_SCORE,Smin_pBad) ||
		weightedScore_1mpBad >= max(MIN_CORE_SCORE,Smin_1mpBad))
		    printf(
#if UNWEIGHTED_CORES
		    "%s %s %.6f %.6f %.6f\n",
#else
		    "%s %s %.6f %.6f\n",
#endif
			G1Index2Name[i].c_str(), G2Index2Name[j].c_str(),
#if UNWEIGHTED_CORES
			unweightdedScore,
#endif
			weightedScore_pBad, weightedScore_1mpBad);
	}
#endif // cores
#endif // PRINT_CORES
        return align;
    }
}

double ecC(PARAMS) { return double(aligEdges) / g1Edges; }
double edC(PARAMS) { return EdgeDifference::adjustSumToTargetScore(edSum, pairsCount); }
double erC(PARAMS) { return EdgeRatio::adjustSumToTargetScore(erSum, pairsCount); }
double s3C(PARAMS) { return double(aligEdges) / (g1Edges + inducedEdges - aligEdges); }
double icsC(PARAMS) { return double(aligEdges) / inducedEdges; }
double secC(PARAMS) { return double(aligEdges) / (g1Edges + aligEdges) / g2Edges * 0.5; }
double tcC(PARAMS) { return TCSum; }
double localC(PARAMS) { return double(localScoreSum) / n1; }
double wecC(PARAMS) { return double(wecSum) / (2 * g1Edges); }
double jsC(PARAMS) { return double(jsSum);}
double ewecC(PARAMS) { return ewecSum; }
double ncC(PARAMS) { return double(ncSum) / trueA_back; }
#ifdef MULTI_PAIRWISE
double mecC(PARAMS) { return double(aligEdges) / (g1WeightedEdges + g2WeightedEdges); }
double sesC(PARAMS) { return squaredAligEdges; }
double eeC(PARAMS)  { return 1-EdgeExposure::numer/(double)EdgeExposure::getDenom(); }
double ms3C(PARAMS)  { return (MultiS3::numer / (double)MultiS3::denom) / (double) NUM_GRAPHS; }
#endif

unordered_set<vector<uint>*>* SANA::paretoRun(const string& fileName) {
    measureCalculation["ec"] = ecC;
    measureCalculation["ed"] = edC;
    measureCalculation["er"] = erC;
    measureCalculation["s3"] = s3C;
    measureCalculation["ics"] = icsC;
    measureCalculation["sec"] = secC;
    measureCalculation["tc"] = tcC;
    measureCalculation["local"] = localC;
    measureCalculation["wec"] = wecC;
    measureCalculation["js"] = jsC;
    measureCalculation["ewec"] = ewecC;
    measureCalculation["nc"] = ncC;
#ifdef MULTI_PAIRWISE
    measureCalculation["mec"] = mecC;
    measureCalculation["ses"] = sesC;
    measureCalculation["ee"] = eeC;
    measureCalculation["ms3"] = ms3C;
#endif
    cout << "pareto mode running in " << paretoThreads << " number of threads" << endl;

    if (paretoThreads > 1) {
        if (!usingIterations) {
            return parallelParetoRun(getStartingAlignment(), (long long int) (getIterPerSecond()*minutes*60), fileName);
        } else {
            return parallelParetoRun(getStartingAlignment(), ((long long int)(maxIterations))*100000000, fileName);
        }
    } else if (paretoThreads == 1) {
        long long int iter = 0;
        if (!usingIterations) {
            return simpleParetoRun(getStartingAlignment(), (long long int) (getIterPerSecond()*minutes*60), iter, fileName);
        } else {
            return simpleParetoRun(getStartingAlignment(), ((long long int)(maxIterations))*100000000, iter, fileName);
        }
    } else {
        cerr << "ERROR: INVALID VALUE OF 'paretoThreads': " << paretoThreads << ". Expecting paretoThreads >= 1." << endl;
        assert(paretoThreads >= 1);
    }

    return storedAlignments;
}

// Used for method #2 of locking
inline uint SANA::G1RandomUnlockedNode_Fast() {
    return unLockedNodesG1[G1RandomUnlockedNodeDist(gen)];
}

inline uint SANA::G1RandomUnlockedNode(){
    #ifdef REINDEX
        return G1RandomUnlockedNodeDist(gen);
    #else
        return G1RandomUnlockedNode_Fast();
    #endif
}

// Gives a random unlocked nodes with the same type as source1
// Only called from performswap
inline uint SANA::G1RandomUnlockedNode(uint source1){
    if(multipartite == 1){
        return SANA::G1RandomUnlockedNode();
    }
    else{
        // Checking node type and returning one with same type
        #ifdef REINDEX
            bool isGene = source1 < (uint) G1->unlockedGeneCount;
            if(isGene)
                return G1RandomUnlockedGeneDist(gen);
            else
                return G1->unlockedGeneCount + G1RandomUnlockedmiRNADist(gen);
        #else
            int type = G1->nodeTypes[source1];
            uniform_int_distribution<> dist = uniform_int_distribution<>(0, G1->typedNodesIndexList[type].size() - 1);
            return G1->typedNodesIndexList[type][dist(gen)];

            // bool isGene = G1->nodeTypes[source1] == Graph::NODE_TYPE_GENE;
            // if(isGene){
            //     int index = G1RandomUnlockedGeneDist(gen);
            //     return G1->geneIndexList[index];
            // }
            // else{
            //     int index =  G1RandomUnlockedmiRNADist(gen);
            //     return G1->miRNAIndexList[index];
            // }
        #endif
    }
}

// Return index of the unassigned node in the unassginedNodesG2 (or unassignedgenesG2, ...)
inline uint SANA::G2RandomUnlockedNode(uint target1){
    if(multipartite == 1){
        return G2RandomUnlockedNode_Fast();
    } else {
        int type = G2->nodeTypes[target1];
        uniform_int_distribution<> dist = uniform_int_distribution<>(0, abs(G2->nodeTypesCount[type] - G1->nodeTypesCount[type]) - 1);
        return dist(gen);
        // if(isGene){
        //     int index = G2RandomUnassignedGeneDist(gen);
        //     return index;
        // }
        // else {
        //     int index = G2RandomUnassignedmiRNADist(gen);
        //     return index;
        // }
        // uint node;
        // do {
        //     node = G2RandomUnlockedNode_Fast(); // gives back unlocked G2 node
        // } while(G2->nodeTypes[target1] != G2->nodeTypes[(*unassignedNodesG2)[node]]);
        // return node;
    }
}

inline uint SANA::G2RandomUnlockedNode_Fast(){
    return G2RandomUnassignedNode(gen);
}

void SANA::describeParameters(ostream& sout) {
    sout << "Temperature schedule:" << endl;
    sout << "T_initial: " << TInitial << endl;
    sout << "T_decay: " << TDecay << endl;
    sout << "Optimize: " << endl;
    MC->printWeights(sout);

    if (!usingIterations){
        sout << "Execution time: ";
        if (restart) {sout << minutesNewAlignments + minutesPerCandidate*numCandidates + minutesFinalist;}
        else {sout << minutes;}
        sout << "m" << endl;
    } else {
        sout << "Iterations Run: " << maxIterations << "00,000,000" << endl; //it's in hundred millions
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
    restart                    = true;
    this->minutesNewAlignments = minutesNewAlignments;
    this->iterationsPerStep    = iterationsPerStep;
    this->numCandidates        = numCandidates;
    this->minutesPerCandidate  = minutesPerCandidate;
    this->minutesFinalist      = minutesFinalist;
    candidates                 = vector<Alignment> (numCandidates, getStartingAlignment());
    candidatesScores           = vector<double> (numCandidates, 0);
    for (uint i = 0; i < numCandidates; i++) {
        candidatesScores[i] = eval(candidates[i]);
    }
}

double SANA::temperatureFunction(long long int iter, double TInitial, double TDecay) {
    if (constantTemp) {
        return TInitial;
    }
    double fraction;
    if(usingIterations) {
        fraction = iter / (100000000.0 * maxIterations);
    } else {
        fraction = iter / (minutes * 60.0 * getIterPerSecond());
    }
    return TInitial * exp(-TDecay * fraction);
}

double SANA::acceptingProbability(double energyInc, double Temperature) {
    return energyInc >= 0 ? 1 : exp(energyInc/Temperature);
}

double SANA::trueAcceptingProbability(){
    return pBadBufferSum/(double) numPBadsInBuffer;
}

//the other method can give incorrect probabilities (even negative) if the pbads in the buffer are small enough
//due to accumulated precision errors of adding and subtracting tiny values
double SANA::slowTrueAcceptingProbability() {
    double sum = 0;
    for (int i = 0; i < numPBadsInBuffer; i++) {
        sum += pBadBuffer[i];
    }
    return sum/(double) numPBadsInBuffer;
}

void SANA::initDataStructures(const Alignment& startA) {
    multipartite = G1->getNumMultipartite();

    A = new vector<uint>(startA.getMapping());
    assignedNodesG2 = new vector<bool> (n2, false);
    for (uint i = 0; i < n1; i++) {
        (*assignedNodesG2)[(*A)[i]] = true;
    }
    unassignedNodesG2        = new vector<uint> (n2-n1);
    unassignedgenesG2        = new vector<uint> ();
    unassignedmiRNAsG2       = new vector<uint> ();
    
    unassignedMultipartiteNodes = new vector<vector<uint>>();
    
    for(int i = 0; i < multipartite; i++)
        unassignedMultipartiteNodes->push_back(vector<uint>{});

    // if(multipartite > 1){
    //     cerr << "Seperating unassigned node types" << endl;
    //     unassignedgenesG2->reserve(G2->geneCount - G1->geneCount);
    //     unassignedmiRNAsG2->reserve(G2->miRNACount - G1->miRNACount);
    // }
    
    for (uint i = 0, j = 0; i < n2; i++) {
    	if (not (*assignedNodesG2)[i]) {
    	    (*unassignedNodesG2)[j++] = i;
    	    if(multipartite > 1){
                (*unassignedMultipartiteNodes)[G2->nodeTypes[i]].push_back(i);
    	    }
        }
    }
    //  Init unlockedNodesG1
    uint unlockedG1            = n1 - G1->getLockedCount();
    unLockedNodesG1            = vector<uint> (unlockedG1);
    uint index                 = 0;
    for (uint i = 0; i < n1; i++){
        if(not G1->isLocked(i)){
            unLockedNodesG1[index++] = i;
        }
    }
    assert(index == unlockedG1);

    if (needAligEdges or needSec) {
        aligEdges = startA.numAlignedEdges(*G1, *G2);
    }

    if (needEd) {
        edSum = EdgeDifference::getEdgeDifferenceSum(G1, G2, startA);
    }

    if (needEr) {
        erSum = EdgeRatio::getEdgeRatioSum(G1, G2, startA);
    }

#if MULTI_PAIRWISE
    if (needSquaredAligEdges) {
        squaredAligEdges = startA.numSquaredAlignedEdges(*G1, *G2);
    }
    if (needExposedEdges)
    {
	EdgeExposure::numer = startA.numExposedEdges(*G1, *G2);// - EdgeExposure::getMaxEdge();
    }
    if (needMS3)
    {
        //cout << "IIIIIIIIINNNNNnIIIIIITT" << endl;
        MultiS3::numer = startA.multiS3Numerator(*G1, *G2);
        //cout << "needMS3 declare numer:" << MultiS3::numer << endl;
        MultiS3::denom = startA.multiS3Denominator(*G1, *G2);
        //cout << "needMS3 declare denom:" << MultiS3::denom << endl;
    }
#endif
    if (needInducedEdges) {
        inducedEdges = G2->numNodeInducedSubgraphEdges(*A);
    }

    if (needLocal) {
        localScoreSum = 0;
        for (uint i = 0; i < n1; i++) {
            localScoreSum += sims[i][(*A)[i]];
        }
        localScoreSumMap = new map<string, double>;
    }

    if (needWec) {
        Measure* wec    = MC->getMeasure("wec");
        double wecScore = wec->eval(*A);
        wecSum          = wecScore*2*g1Edges;
    }

    if (needJs) {
        Measure* js = MC->getMeasure("js");
        // jsAlignedByNode = JaccardSimilarityScore::getAlignedByNode(G1, G2, startA);
        jsSum       = js->eval(*A);
        alignedByNode = JaccardSimilarityScore::getAlignedByNode(G1, G2, startA);
    }

    if(needEwec){
        ewec    = (ExternalWeightedEdgeConservation*)(MC->getMeasure("ewec"));
        ewecSum = ewec->eval(*A);
    }

    if (needNC) {
        Measure* nc = MC->getMeasure("nc");
        ncSum       = (nc->eval(*A))*trueA.back();
    }

    if(needTC){
        Measure* tc  = MC->getMeasure("tc");
        maxTriangles = ((TriangleCorrectness*)tc)->getMaxTriangles();
        TCSum        = tc->eval(*A);
    }

    iterationsPerformed = 0;

    numPBadsInBuffer = 0;
    pBadBufferSum = 0;
    pBadBufferIndex = 0;

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

void SANA::printReport() {
    saveAlignment = false;
    string timestamp = string(currentDateTime()); //necessary to make it not const
    std::replace(timestamp.begin(), timestamp.end(), ' ', '_');
    string out = outputFileName+"_"+timestamp;
    string local = localScoresFileName+"_"+timestamp;
    saveReport(*G1, *G2, Alignment(*A), *MC, this, out, false);
    saveLocalMeasures(*G1, *G2, Alignment(*A), *MC, this, local);
    cout << "Alignment saved. SANA will now continue." << endl;
}
void SANA::setOutputFilenames(string outputFileName, string localScoresFileName)
{
    this->outputFileName = outputFileName;
    this->localScoresFileName =  localScoresFileName;
}

Alignment SANA::simpleRun(const Alignment& startA, double maxExecutionSeconds, long long int& iter) {
    initDataStructures(startA);
    setInterruptSignal();

    for (; ; ++iter) {
        Temperature = temperatureFunction(iter, TInitial, TDecay);
        if (interrupt) {
            break;
        }
        if (saveAlignment) {
            printReport();
        }
        if (iter%iterationsPerStep == 0) {
            trackProgress(iter);
            if (iter != 0 and timer.elapsed() > maxExecutionSeconds) {
                break;
            }
        }
        SANAIteration();
    }
    trackProgress(iter);
    return *A;
}

Alignment SANA::simpleRun(const Alignment& startA, double maxExecutionSeconds, long long int maxExecutionIterations,
		long long int& iter) {
    _maxExecutionIterations = maxExecutionIterations;
    initDataStructures(startA);
    setInterruptSignal();
	for (; ; ++iter) {
		Temperature = temperatureFunction(iter, TInitial, TDecay);
		if (interrupt) {
			break; // return *A;
		}
        if (saveAlignment) {
            printReport();
        }
		if (iter%iterationsPerStep == 0) {
			trackProgress(iter);
			if( iter != 0 and timer.elapsed() > maxExecutionSeconds and currentScore - previousScore < 0.005 ){
				break;
			}
			previousScore = currentScore;
		}
		if (iter != 0 and iter > maxExecutionIterations) {
			break;
		}
		SANAIteration();
	}
	trackProgress(iter);
	return *A; //dummy return to shut compiler warning
}

Alignment SANA::simpleRun(const Alignment& startA, long long int maxExecutionIterations, long long int& iter) {

        initDataStructures(startA);
	_maxExecutionIterations = maxExecutionIterations;

        setInterruptSignal();

        for (; ; ++iter) {
                Temperature = temperatureFunction(iter, TInitial, TDecay);
                if (interrupt) {
                        break; // return *A;
                }
                if (saveAlignment) {
                    printReport();
                }
                if (iter%iterationsPerStep == 0) {
                        trackProgress(iter);

                }
                if (iter != 0 and iter > maxExecutionIterations) {
                        break; // return *A;
                }
                SANAIteration();
        }
        trackProgress(iter);
        return *A; //dummy return to shut compiler warning
}

unordered_set<vector<uint>*>* SANA::simpleParetoRun(const Alignment& startA, double maxExecutionSeconds,
        long long int& iter, const string &fileName) {

    initDataStructures(startA);
    setInterruptSignal();
    vector<double> scores;
    scoreNamesToIndexes = mapScoresToIndexes();
    paretoFront = ParetoFront(paretoCapacity, numOfMeasures, measureNames);
    assert(numOfMeasures > 1 && "Pareto mode must optimize on more than one measure");
    score = Score::pareto;
    initializeParetoFront();

    for (; ; ++iter) {
	Temperature = temperatureFunction(iter, TInitial, TDecay);
	if (interrupt) {
	    return storedAlignments;
	}
    if (saveAlignment) {
        printReport();
    }
	if (iter%iterationsPerStep == 0) {
	    trackProgress(iter);
	    if( iter != 0 and timer.elapsed() > maxExecutionSeconds){
		cout << "ending seconds " << timer.elapsed() << " " << maxExecutionSeconds << endl;
                if (paretoFront.paretoPropertyViolated()) {
                    cerr << "Warning: Pareto property violated, which means pareto front might not be correct!" << endl;
                }
                printParetoFront(fileName);
                deallocateParetoData();
		return storedAlignments;
	    }
	}
	SANAIteration();
    }
    trackProgress(iter);
    return storedAlignments;
}

unordered_set<vector<uint>*>* SANA::simpleParetoRun(const Alignment& startA, long long int maxExecutionIterations,
        long long int& iter, const string &fileName) {

    initDataStructures(startA);
    setInterruptSignal();
    vector<double> scores;
    scoreNamesToIndexes = mapScoresToIndexes();
    paretoFront = ParetoFront(paretoCapacity, numOfMeasures, measureNames);
    assert(numOfMeasures > 1 && "Pareto mode must optimize on more than one measure");
    score = Score::pareto;
    initializeParetoFront();

    for (; ; ++iter) {
    	Temperature = temperatureFunction(iter, TInitial, TDecay);
        if (interrupt) {
            return storedAlignments;
        }
        if (saveAlignment) {
            printReport();
        }
        if (iter%iterationsPerStep == 0) {
            trackProgress(iter);
        }
        if (iter != 0 and iter > maxExecutionIterations) {
            cout << "ending iterations " << iter << " " << maxExecutionIterations << endl;
            if (paretoFront.paretoPropertyViolated()) {
                cerr << "Warning: Pareto property violated, which means pareto front might not be correct!" << endl;
            }
            printParetoFront(fileName);
            deallocateParetoData();
            return storedAlignments;
        }
        SANAIteration();
    }
    trackProgress(iter);
    return storedAlignments;
}

unordered_map<string, int> SANA::mapScoresToIndexes() {
    numOfMeasures = MC->numMeasures();
    assert(numOfMeasures == 1);
    /*numOfMeasures = 8;
    #ifdef MULTI_PAIRWISE
    numOfMeasures = 10;
    #endif*/
    measureNames = vector<string>(0);
    for(uint i = 0; i < numOfMeasures; ++i)
        if(MC->getWeight(MC->getMeasure(i)->getName()) > 0)
            measureNames.push_back(MC->getMeasure(i)->getName());
    for(uint i = 0; i < measureNames.size(); ++i) {
        if(localScoreNames.find(measureNames[i]) != localScoreNames.end())
            measureNames[i] = "local";
    }
#if 0
    //SANA is using these predifined measures:-----------------------------------------------------
    measureNames[0] = "ec";
    measureNames[1] = "ics";
    measureNames[2] = "s3";
    measureNames[3] = "sec";
    measureNames[4] = "tc";
    measureNames[5] = "local"; //   <-------- Contains many sub-measures.
    measureNames[6] = "wec";
    measureNames[7] = "nc";
    #ifdef MULTI_PAIRWISE
    measureNames[8] = "mec";
    measureNames[9] = "ses";
    #endif
    //---------------------------------------------------------------------------------------------
#endif
    sort(measureNames.begin(), measureNames.end()); //Must be sorted to function correctly.
    measureNames.erase(unique(measureNames.begin(), measureNames.end()), measureNames.end());
    //measureNames.resize(distance(measureNames.begin(), unique(measureNames.begin(), measureNames.end()))); //Removes any duplicate measure names. Somehow, ICS measure appears twice... a good check to perform.
    numOfMeasures = measureNames.size();
    unordered_map<string, int> toReturn;
    for(uint i = 0; i < numOfMeasures; ++i) {
    	toReturn[measureNames[i]] = i;
        cout << measureNames[i] << ' ' << i << endl;
    }
    return toReturn;
}

vector<double> SANA::translateScoresToVector() {
    vector<double> addScores(numOfMeasures);
#ifdef MULTI_PAIRWISE
    for(uint i = 0; i < numOfMeasures; i++) {
        addScores[scoreNamesToIndexes[measureNames[i]]] = measureCalculation[measureNames[i]]
                                                                            ( aligEdges, g1Edges, inducedEdges,
                                                                              g2Edges, TCSum, localScoreSum, n1,
                                                                              wecSum, jsSum, ewecSum, ncSum, trueA.back(),
                                                                              g1WeightedEdges, g2WeightedEdges,
                                                                              squaredAligEdges, EdgeExposure::numer, MS3Numer,
                                                                              edSum, erSum, pairsCount
                                                                            );
    }
#else
    for(uint i = 0; i < numOfMeasures; i++) {
        addScores[scoreNamesToIndexes[measureNames[i]]] = measureCalculation[measureNames[i]]
                                                                            ( aligEdges, g1Edges, inducedEdges,
                                                                              g2Edges, TCSum, localScoreSum, n1,
                                                                              wecSum, jsSum, ewecSum, ncSum, trueA.back(),
                                                                              edSum, erSum, pairsCount
                                                                            );
    }
#endif
#if 0
    addScores[scoreNamesToIndexes["ec"]] = double(aligEdges) / g1Edges;
    addScores[scoreNamesToIndexes["s3"]] = double(aligEdges) / (g1Edges + inducedEdges - aligEdges);
    addScores[scoreNamesToIndexes["ics"]] = double(aligEdges) / inducedEdges;
    addScores[scoreNamesToIndexes["sec"]] = double(aligEdges) / (g1Edges + aligEdges) / g2Edges * 0.5;
    addScores[scoreNamesToIndexes["tc"]] = TCSum;
    addScores[scoreNamesToIndexes["local"]] = double(localScoreSum) / n1;
    addScores[scoreNamesToIndexes["wec"]] = double(wecSum) / (2 * g1Edges);
    //addScores[scoreNamesToIndexes["ewec"]] = ewecSum;
    addScores[scoreNamesToIndexes["nc"]] = double(ncSum) / trueA.back();
#ifdef MULTI_PAIRWISE
    addScores[scoreNamesToIndexes["mec"]] = double(aligEdges) / (g1WeightedEdges + g2WeightedEdges);
    addScores[scoreNamesToIndexes["ses"]] = double(squaredAligEdges);
#endif
#endif // 0
    return addScores;
}

vector<double> SANA::getMeasureScores(double newAligEdges, double newInducedEdges,
            double newTCSum, double newLocalScoreSum, double newWecSum, double newJsSum, double newNcSum,
            double newEwecSum, double newSquaredAligEdges, double newExposedEdgesNumer, double newMS3Numer,
            double newEdSum, double newErSum) {

    vector<double> addScores(numOfMeasures);
#ifdef MULTI_PAIRWISE
    for(uint i = 0; i < numOfMeasures; i++) {
        addScores[scoreNamesToIndexes[measureNames[i]]] = measureCalculation[measureNames[i]]
                                                                            ( newAligEdges, g1Edges, newInducedEdges,
                                                                              g2Edges, newTCSum, newLocalScoreSum, n1,
                                                                              newWecSum, newJsSum, newEwecSum, newNcSum, trueA.back(),
                                                                              g1WeightedEdges, g2WeightedEdges,
                                                                              newSquaredAligEdges, newExposedEdgesNumer,
									      newMS3Numer, newEdSum, newErSum, pairsCount
                                                                            );
    }
#else
    for(uint i = 0; i < numOfMeasures; i++) {
        addScores[scoreNamesToIndexes[measureNames[i]]] = measureCalculation[measureNames[i]]
                                                                            ( newAligEdges, g1Edges, newInducedEdges,
                                                                              g2Edges, newTCSum, newLocalScoreSum, n1,
                                                                              newWecSum, newJsSum, newEwecSum, newNcSum, trueA.back(),
                                                                              newEdSum, newErSum, pairsCount
                                                                            );
    }
#endif
#if 0
    addScores[scoreNamesToIndexes["ec"]] = double(newAligEdges) / g1Edges;
    addScores[scoreNamesToIndexes["s3"]] = double(newAligEdges) / (g1Edges + newInducedEdges - newAligEdges);
    addScores[scoreNamesToIndexes["ics"]] = double(newAligEdges) / newInducedEdges;
    addScores[scoreNamesToIndexes["sec"]] = (double(newAligEdges) / g1Edges + newAligEdges / g2Edges)*0.5;
    addScores[scoreNamesToIndexes["tc"]] = double(newTCSum);
    addScores[scoreNamesToIndexes["local"]] = double(newLocalScoreSum) / n1;
    addScores[scoreNamesToIndexes["wec"]] = double(newWecSum) / (2 * g1Edges);
    //addScores[scoreNamesToIndexes["ewec"]] = ewecWeight * (newEwecSum);
    addScores[scoreNamesToIndexes["nc"]] = double(newNcSum) / trueA.back();
#ifdef MULTI_PAIRWISE
    addScores[scoreNamesToIndexes["mec"]] = double(newAligEdges) / (g1WeightedEdges + g2WeightedEdges);
    addScores[scoreNamesToIndexes["ses"]] = double(newSquaredAligEdges) / double(SquaredEdgeScore::getDenom());
#endif
#endif // 0
    return addScores;
}

void SANA::prepareMeasureDataByAlignment() {
    assert(storedAlignments->find(A) != storedAlignments->end() and "Alignment does not exist in the Pareto front.");
    aligEdges        = (needAligEdges or needSec) ?  storedAligEdges[A] : -1;
    squaredAligEdges = (needSquaredAligEdges) ?  storedSquaredAligEdges[A] : -1;
    EdgeExposure::numer= (needExposedEdges) ?  storedExposedEdgesNumer[A] : -1;
    MultiS3::numer   = (needMS3) ?  storedMS3Numer[A] : -1;
    inducedEdges     = (needInducedEdges) ?  storedInducedEdges[A] : -1;
    TCSum            = (needTC) ?  storedTCSum[A] : -1;
    edSum            = (needEd) ?  storedEdSum[A] : -1;
    erSum            = (needEr) ?  storedErSum[A] : -1;
    localScoreSum    = (needLocal) ? storedLocalScoreSum[A] : -1;
    localScoreSumMap = (needLocal) ? new map<string, double>(*storedLocalScoreSumMap[A]) : nullptr;
    wecSum           = (needWec) ?  storedWecSum[A] : -1;
    jsSum            = (needJs) ?   storedJsSum[A] : -1;
    ewecSum          = (needEwec) ?  storedEwecSum[A] : -1;
    ncSum            = (needNC) ? storedNcSum[A] : -1;
    currentScore     = storedCurrentScore[A];

    currentScores = paretoFront.procureScoresByAlignment(A);
    currentMeasure = paretoFront.getRandomMeasure();

    assignedNodesG2 = new vector<bool>(*storedAssignedNodesG2[A]);
    if(multipartite == 1)
        unassignedNodesG2 = new vector<uint>(*storedUnassignedNodesG2[A]);
    else {
        unassignedmiRNAsG2 = new vector<uint>(*storedUnassignedmiRNAsG2[A]);
        unassignedgenesG2 = new vector<uint>(*storedUnassignedgenesG2[A]);
    }

    A = new vector<uint>(*A);
}

void SANA::releaseAlignment() {
    delete A;
    delete assignedNodesG2;
    delete unassignedNodesG2;
    delete unassignedmiRNAsG2;
    delete unassignedgenesG2;
    if (needLocal)
        delete localScoreSumMap;
}

void SANA::insertCurrentAndPrepareNewMeasureDataByAlignment(vector<double> &addScores) {
    bool inserted = false;
    vector<vector<uint>*> toRemove = paretoFront.addAlignmentScores(A, addScores, inserted);
    if (inserted) {
        for (unsigned int i = 0; i < toRemove.size(); i++)
            removeAlignmentData(toRemove[i]);

        insertCurrentAlignmentAndData();
    }
    else {
        delete A;
    }

    assert(paretoFront.size() == storedAlignments->size() and "Number of elements in paretoFront and storedAlignments don't match.");
    A = paretoFront.procureRandomAlignment();
    assert(storedAlignments->find(A) != storedAlignments->end() && "There exists an alignment in the Pareto front which does not exist inside storedAlignments");
    prepareMeasureDataByAlignment();
}

void SANA::insertCurrentAlignmentAndData() {
    storedAlignments->insert(A);

    storedAssignedNodesG2[A]        = assignedNodesG2;
    if(multipartite == 1)
        storedUnassignedNodesG2[A]  = unassignedNodesG2;
    else {
        storedUnassignedmiRNAsG2[A] = unassignedmiRNAsG2;
        storedUnassignedgenesG2[A]  = unassignedgenesG2;
    }

    if(needAligEdges or needSec) storedAligEdges[A]        = aligEdges;
    if(needSquaredAligEdges)     storedSquaredAligEdges[A] = squaredAligEdges;
    if(needExposedEdges)	 storedExposedEdgesNumer[A]= EdgeExposure::numer;
    if(needMS3)         	 storedMS3Numer[A]         = MultiS3::numer;
    if(needInducedEdges)         storedInducedEdges[A]     = inducedEdges;
    if(needEd)                   storedEdSum[A]            = edSum;
    if(needEr)                   storedErSum[A]            = erSum;
    if(needTC)                   storedTCSum[A]            = TCSum;
    if(needLocal)                storedLocalScoreSum[A]    = localScoreSum;
    if(needLocal)                storedLocalScoreSumMap[A] = localScoreSumMap;
    if(needWec)                  storedWecSum[A]           = wecSum;
    if(needJs)                    storedJsSum[A]            = jsSum;
    if(needEwec)                 storedEwecSum[A]          = ewecSum;
    if(needNC)                   storedNcSum[A]            = ncSum;
    /*------------------------>*/storedCurrentScore[A]     = currentScore;
}

void SANA::removeAlignmentData(vector<uint>* toRemove) {
    storedAlignments->erase(toRemove);

    vector<bool>* removeAssignedNodesG2 = storedAssignedNodesG2[toRemove];
    storedAssignedNodesG2.erase(toRemove);
    delete removeAssignedNodesG2;
    if(multipartite == 1) {
        vector<uint>* removeUnassignedNodesG2 = storedUnassignedNodesG2[toRemove];
        storedUnassignedNodesG2.erase(toRemove);
        delete removeUnassignedNodesG2;
    }
    else {
        vector<uint>* removeUnassignedmiRNAsG2 = storedUnassignedmiRNAsG2[toRemove];
        vector<uint>* removeUnassignedgenesG2 = storedUnassignedgenesG2[toRemove];
        storedUnassignedmiRNAsG2.erase(toRemove);
        storedUnassignedgenesG2.erase(toRemove);
        delete removeUnassignedmiRNAsG2;
        delete removeUnassignedgenesG2;
    }

    if(needAligEdges or needSec) storedAligEdges.erase(toRemove);
    if(needSquaredAligEdges)     storedSquaredAligEdges.erase(toRemove);
    if(needExposedEdges)	 storedExposedEdgesNumer.erase(toRemove);
    if(needMS3)		         storedMS3Numer.erase(toRemove);
    if(needInducedEdges)         storedInducedEdges.erase(toRemove);
    if(needTC)                   storedTCSum.erase(toRemove);
    if(needEd)                   storedEdSum.erase(toRemove);
    if(needJs)                   storedJsSum.erase(toRemove);
    if(needLocal)                storedLocalScoreSum.erase(toRemove);
    if(needLocal)                storedLocalScoreSumMap.erase(toRemove);
    if(needWec)                  storedWecSum.erase(toRemove);
    if(needEwec)                 storedEwecSum.erase(toRemove);
    if(needNC)                   storedNcSum.erase(toRemove);
    //TAKE CARE OF THE MEMORY LEAK!
    delete toRemove;
}

void SANA::initializeParetoFront() {
    vector<double> addScores = translateScoresToVector();
    insertCurrentAndPrepareNewMeasureDataByAlignment(addScores);
    cout << "Initializing " << paretoInitial << " random alignments." << endl;
    cout << "Current size: ";
    while(paretoFront.size() < paretoInitial) {
        cout << paretoFront.size() << ", ";
        Alignment newAlig = getStartingAlignment();
        initDataStructures(newAlig);
        addScores = translateScoresToVector();
        insertCurrentAndPrepareNewMeasureDataByAlignment(addScores);
    }
    cout << paretoFront.size() << endl;
}

bool SANA::dominates(vector<double> &left, vector<double> &right) {
    for(unsigned int i = 0; i < left.size(); i++)
        if(left[i] < right[i])
            return false;
    return true;
}

void SANA::printParetoFront(const string &fileName) {
    string outputFileName = fileName;
    if(outputFileName.find(".out") != string::npos && outputFileName.rfind(".out") + 4 == outputFileName.size())
        outputFileName = outputFileName.substr(0, outputFileName.size() - 4);
    ofstream output(fileName + "_ParetoFrontScores.out");
    paretoFront.printAlignmentScores(output);
    output.close();
    paretoFront.printAlignmentScores(cout); cout << endl;
}

void SANA::deallocateParetoData() {
    for(auto i = storedAssignedNodesG2.begin(); i != storedAssignedNodesG2.end(); i++)
        delete i->second;
    if(multipartite == 1) {
        for(auto i = storedUnassignedNodesG2.begin(); i != storedUnassignedNodesG2.end(); i++)
            delete i->second;
    }
    else {
        for(auto i = storedUnassignedmiRNAsG2.begin(); i != storedUnassignedmiRNAsG2.end(); i++)
            delete i->second;
        for(auto i = storedUnassignedgenesG2.begin(); i != storedUnassignedgenesG2.end(); i++)
            delete i->second;
    }
    for(auto i = storedLocalScoreSumMap.begin(); i != storedLocalScoreSumMap.end(); i++)
        delete i->second;
}

void SANA::SANAIteration() {
    ++iterationsPerformed;
    if(G1->multipartite >  1) {
        // Choose the type here based on counts (and locking...)
        // For example if no locking, then prob (gene) >> prob (miRNA)
        // And if locking, then arrange prob(gene) and prob(miRNA) appropriately
        int type = /* something clever */ 0;
        (randomReal(gen) < changeProbability[type]) ? performChange(type) : performSwap(type);
//performChange(type);
    } else {
        (randomReal(gen) < changeProbability[0]) ? performChange(0) : performSwap(0);
    }
}

void SANA::performChange(int type) {
    uint source       = G1RandomUnlockedNode();
    uint oldTarget    = (*A)[source];

    uint newTargetIndex;

    uint newTarget = -1;
    if(multipartite == 1) {
        newTargetIndex = G2RandomUnlockedNode(oldTarget);
        newTarget    = (*unassignedNodesG2)[newTargetIndex];
    }
    else{
        int nodeType = G2->nodeTypes[oldTarget];
        if((*unassignedMultipartiteNodes)[nodeType].size() == 0)
            return; // cannot perform change, all genes are assigned
        newTargetIndex = G2RandomUnlockedNode(oldTarget);
        
    
        newTarget = (*unassignedMultipartiteNodes)[nodeType][newTargetIndex];
        
        
        // bool isGene = false;
        // isGene = G2->nodeTypes[oldTarget] == Graph::NODE_TYPE_GENE;
        // if(isGene){
        //     if(unassignedgenesG2->size() == 0)
        //         return; // cannot perform change, all genes are assigned
        //     newTarget = (*unassignedgenesG2)[newTargetIndex];
        // }
        // else{
        //     if(unassignedmiRNAsG2->size() == 0)
        //         return; // cannot perform change, all miRNA are assigned
        //     newTarget = (*unassignedmiRNAsG2)[newTargetIndex];
        // }
    }
    //added dummy initialization to shut compiler warning -Nil
    unsigned oldOldTargetDeg = 0, oldNewTargetDeg = 0, oldMs3Denom = 0, oldMs3Numer =0;
   if (needMS3)
    {
        oldOldTargetDeg = MultiS3::shadowDegree[oldTarget];
        oldNewTargetDeg = MultiS3::shadowDegree[newTarget];
        oldMs3Denom = MultiS3::denom;
        oldMs3Numer = MultiS3::numer;
    }
    int newAligEdges           = (needAligEdges or needSec) ?  aligEdges + aligEdgesIncChangeOp(source, oldTarget, newTarget) : -1;
    double newEdSum            = (needEd) ?  edSum + edgeDifferenceIncChangeOp(source, oldTarget, newTarget) : -1;
    double newErSum            = (needEr) ?  erSum + edgeRatioIncChangeOp(source, oldTarget, newTarget) : -1;
    double newSquaredAligEdges = (needSquaredAligEdges) ?  squaredAligEdges + squaredAligEdgesIncChangeOp(source, oldTarget, newTarget) : -1;
    double newExposedEdgesNumer= (needExposedEdges) ? EdgeExposure::numer + exposedEdgesIncChangeOp(source, oldTarget, newTarget) : -1;
    double newMS3Numer         = (needMS3) ? MultiS3::numer + MS3IncChangeOp(source, oldTarget, newTarget) : -1;
    int newInducedEdges        = (needInducedEdges) ?  inducedEdges + inducedEdgesIncChangeOp(source, oldTarget, newTarget) : -1;
    double newTCSum            = (needTC) ?  TCSum + TCIncChangeOp(source, oldTarget, newTarget) : -1;
    double newLocalScoreSum    = (needLocal) ? localScoreSum + localScoreSumIncChangeOp(sims, source, oldTarget, newTarget) : -1;
    double newWecSum           = (needWec) ?  wecSum + WECIncChangeOp(source, oldTarget, newTarget) : -1;
    double newJsSum            = (needJs) ?  jsSum + JSIncChangeOp(source, oldTarget, newTarget) : -1;
    double newEwecSum          = (needEwec) ?  ewecSum + EWECIncChangeOp(source, oldTarget, newTarget) : -1;
    double newNcSum            = (needNC) ? ncSum + ncIncChangeOp(source, oldTarget, newTarget) : -1;

    map<string, double> newLocalScoreSumMap;
    if (needLocal) {
        newLocalScoreSumMap = map<string, double>(*localScoreSumMap);
        for(auto &item : newLocalScoreSumMap)
            item.second += localScoreSumIncChangeOp(localSimMatrixMap[item.first], source, oldTarget, newTarget);
    }

    double newCurrentScore = 0;
    bool makeChange = scoreComparison(newAligEdges, newInducedEdges, newTCSum,
            newLocalScoreSum, newWecSum, newJsSum, newNcSum, newCurrentScore, newEwecSum,
            newSquaredAligEdges, newExposedEdgesNumer, newMS3Numer, newEdSum, newErSum);

#define DISALLOW_SELF 1 // testing to see how good score can get if G1==G2 but we disallow all nodes to self-align
#if DISALLOW_SELF
    if(source==newTarget) makeChange=false;
#endif

#ifdef CORES
	// Statistics on the emerging core alignment.
	// only update pBad if it's nonzero; re-use previous nonzero pBad if the current one is zero.
	uint betterHole = wasBadMove ? oldTarget : newTarget;

        double pBad = trueAcceptingProbability();
        if(pBad <= 0 || myNan(pBad)) pBad = LOW_PBAD_LIMIT;
#if UNWEIGHTED_CORES
	numPegSamples[source]++;
	pegHoleFreq[source][betterHole]++;
#endif
	totalWeightedPegWeight_pBad[source] += pBad;
	weightedPegHoleFreq_pBad[source][betterHole] += pBad;
	totalWeightedPegWeight_1mpBad[source] += 1-pBad;
	weightedPegHoleFreq_1mpBad[source][betterHole] += 1-pBad;
#endif

    if (makeChange)
    {
        (*A)[source]                         = newTarget;
        
        
        if(multipartite == 1)
            (*unassignedNodesG2)[newTargetIndex] = oldTarget;
        else {
            int nodeType = G2->nodeTypes[oldTarget];
            (*unassignedMultipartiteNodes)[nodeType][newTargetIndex] = oldTarget;
            // if(isGene){
            //     (*unassignedgenesG2)[newTargetIndex] = oldTarget;
            // }
            // else {
            //     (*unassignedmiRNAsG2)[newTargetIndex] = oldTarget;
            // }
        }

        (*assignedNodesG2)[oldTarget]        = false;
        (*assignedNodesG2)[newTarget]        = true;

        aligEdges                            = newAligEdges;
        edSum                                = newEdSum;
        erSum                                = newErSum;
        inducedEdges                         = newInducedEdges;
        TCSum                                = newTCSum;
        localScoreSum                        = newLocalScoreSum;
        wecSum                               = newWecSum;
        ewecSum                              = newEwecSum;
        ncSum                                = newNcSum;
        if (needLocal)
            (*localScoreSumMap) = newLocalScoreSumMap;
#if 0
        if(randomReal(gen)<=1) {
        double foo = eval(*A);
        if(fabs(foo - newCurrentScore)>20){
            cout << "\nChange: nCS " << newCurrentScore << " (nSAE) " 
                << newSquaredAligEdges << " eval " << foo   
                << " nCS - eval " << newCurrentScore-foo;
            //cout << "source " << source << " oldTarget " 
            << oldTarget << " newTarget " << newTarget << " adj? "  
            << G2Matrix[oldTarget][newTarget] << endl;
            newCurrentScore = newSquaredAligEdges = foo;
        } else cout << "c";
        }
#endif
        currentScore     = newCurrentScore;
	EdgeExposure::numer= newExposedEdgesNumer;
        squaredAligEdges = newSquaredAligEdges;
        MultiS3::numer         = newMS3Numer;
    }
    else
    {
       if (needMS3)
        {
            MultiS3::shadowDegree[oldTarget] = oldOldTargetDeg;
            MultiS3::shadowDegree[newTarget] = oldNewTargetDeg;
            MultiS3::denom = oldMs3Denom;
            MultiS3::numer = oldMs3Numer;
        }
    }

    if(score == Score::pareto and ((iterationsPerformed % paretoIterations) == 0)) {
        /*vector<double> addScores = getMeasureScores(aligEdges, inducedEdges, TCSum, localScoreSum,
                                                    wecSum, ncSum, ewecSum, squaredAligEdges);*/
        vector<double> addScores = translateScoresToVector();
        insertCurrentAndPrepareNewMeasureDataByAlignment(addScores);
    }
}

void SANA::performSwap(int type) {
    uint source1 = G1RandomUnlockedNode();
    
    uint max_try = 1000;
    uint source2,i;
    for (i=0;i<max_try;i++){
        source2 = G1RandomUnlockedNode(source1);
        if (source1!=source2){break;}
    }
    assert(i!=max_try);
    
    uint target1 = (*A)[source1], target2 = (*A)[source2];
    
    //added initialization to shut compiler warning -Nil
    unsigned oldTarget1Deg = 0, oldTarget2Deg = 0, oldMs3Denom = 0;
    if (needMS3)
    {
        oldTarget1Deg = MultiS3::shadowDegree[target1];
        oldTarget2Deg = MultiS3::shadowDegree[target2];
        oldMs3Denom = MultiS3::denom;
    }

    int newAligEdges           = (needAligEdges or needSec) ?  aligEdges + aligEdgesIncSwapOp(source1, source2, target1, target2) : -1;
    int newTCSum               = (needTC) ?  TCSum + TCIncSwapOp(source1, source2, target1, target2) : -1;
    double newSquaredAligEdges = (needSquaredAligEdges) ? squaredAligEdges + squaredAligEdgesIncSwapOp(source1, source2, target1, target2) : -1;
    double newExposedEdgesNumer= (needExposedEdges) ?  EdgeExposure::numer + exposedEdgesIncSwapOp(source1, source2, target1, target2) : -1;
    double newMS3Numer         = (needMS3) ?  MultiS3::numer + MS3IncSwapOp(source1, source2, target1, target2) : -1;
    double newWecSum           = (needWec) ?  wecSum + WECIncSwapOp(source1, source2, target1, target2) : -1;
    double newJsSum            = (needJs) ?  jsSum + JSIncSwapOp(source1, source2, target1, target2) : -1;
    double newEwecSum          = (needEwec) ?  ewecSum + EWECIncSwapOp(source1, source2, target1, target2) : -1;
    double newNcSum            = (needNC) ? ncSum + ncIncSwapOp(source1, source2, target1, target2) : -1;
    double newLocalScoreSum    = (needLocal) ? localScoreSum + localScoreSumIncSwapOp(sims, source1, source2, target1, target2) : -1;
    double newEdSum            = (needEd) ?  edSum + edgeDifferenceIncSwapOp(source1, source2, target1, target2) : -1;
    double newErSum            = (needEr) ?  erSum + edgeRatioIncSwapOp(source1, source2, target1, target2) : -1;

    map<string, double> newLocalScoreSumMap;
    if (needLocal) {
        newLocalScoreSumMap = map<string, double>(*localScoreSumMap);
        for (auto &item : newLocalScoreSumMap)
            item.second += localScoreSumIncSwapOp(localSimMatrixMap[item.first], source1, source2, target1, target2);
    }

    double newCurrentScore = 0;
    bool makeChange = scoreComparison(newAligEdges, inducedEdges, newTCSum, newLocalScoreSum,
                        newWecSum, newJsSum, newNcSum, newCurrentScore, newEwecSum, newSquaredAligEdges,
                        newExposedEdgesNumer, newMS3Numer, newEdSum, newErSum);

#define DISALLOW_SELF 1 // testing to see how good score can get if G1==G2 but we disallow all nodes to self-align
#if DISALLOW_SELF
    if(source1==target2 || source2==target1) makeChange=false;
#endif

#ifdef CORES
        // Statistics on the emerging core alignment.
        // only update pBad if it's nonzero; re-use previous nonzero pBad if the current one is zero.
        double pBad = trueAcceptingProbability();
        if(pBad <= 0 || myNan(pBad)) pBad = LOW_PBAD_LIMIT;

        uint betterDest1 = wasBadMove ? target1 : target2;
        uint betterDest2 = wasBadMove ? target2 : target1;
#if UNWEIGHTED_CORES
        numPegSamples[source1]++; numPegSamples[source2]++;
        pegHoleFreq[source1][betterDest1]++; pegHoleFreq[source2][betterDest2]++;
#endif
        totalWeightedPegWeight_pBad[source1] += pBad;
        weightedPegHoleFreq_pBad[source1][betterDest1] += pBad;
        totalWeightedPegWeight_pBad[source2] += pBad;
        weightedPegHoleFreq_pBad[source2][betterDest2] += pBad;

        totalWeightedPegWeight_1mpBad[source1] += 1-pBad;
        weightedPegHoleFreq_1mpBad[source1][betterDest1] += 1-pBad;
        totalWeightedPegWeight_1mpBad[source2] += 1-pBad;
        weightedPegHoleFreq_1mpBad[source2][betterDest2] += 1-pBad;
#endif

    if (makeChange)
    {
        (*A)[source1]       = target2;
        (*A)[source2]       = target1;
        aligEdges           = newAligEdges;
        edSum               = newEdSum;
        erSum               = newErSum;
        localScoreSum       = newLocalScoreSum;
        TCSum               = newTCSum;
        wecSum              = newWecSum;
        ewecSum             = newEwecSum;
        ncSum               = newNcSum;
        currentScore        = newCurrentScore;
        squaredAligEdges    = newSquaredAligEdges;
	EdgeExposure::numer   = newExposedEdgesNumer;
        MultiS3::numer      = newMS3Numer;
        if (needLocal)
            (*localScoreSumMap) = newLocalScoreSumMap;
#if 0
        if (randomReal(gen) <= 1) {
            double foo = eval(*A);
            if (fabs(foo - newCurrentScore) > 20) {
                cout << "\nSwap: nCS " << newCurrentScore << " eval " << foo 
                << " nCS - eval " << newCurrentScore - foo << " adj? "
                << (G1Matrix[source1][source2] & G2Matrix[target1][target2]);
                newCurrentScore = newSquaredAligEdges = foo;
            }
            else cout << "s";
        }
#endif
    }
    else
    {
        if (needMS3)
        {
            MultiS3::shadowDegree[target1] = oldTarget1Deg;
            MultiS3::shadowDegree[target2] = oldTarget2Deg;
            MultiS3::denom = oldMs3Denom;
        }
    }
    
    if(score == Score::pareto and ((iterationsPerformed % paretoIterations) == 0)) {
        /*vector<double> addScores = getMeasureScores(aligEdges, inducedEdges, TCSum, localScoreSum,
                                                    wecSum, ncSum, ewecSum, squaredAligEdges);*/
        vector<double> addScores = translateScoresToVector();
        insertCurrentAndPrepareNewMeasureDataByAlignment(addScores);
    }
}

bool SANA::scoreComparison(double newAligEdges, double newInducedEdges, double newTCSum,
        double newLocalScoreSum, double newWecSum, double newJsSum, double newNcSum, double& newCurrentScore,
        double newEwecSum, double newSquaredAligEdges, double newExposedEdgesNumer, double newMS3Numer,
        double newEdgeDifferenceSum, double newEdgeRatioSum) {

    bool makeChange = false;
    wasBadMove = false;
    double badProbability = 0;

    switch (score)
    {
    case Score::sum:
    {
        newCurrentScore += ecWeight * (newAligEdges / g1Edges);
        newCurrentScore += edWeight * EdgeDifference::adjustSumToTargetScore(newEdgeDifferenceSum, pairsCount);
        newCurrentScore += erWeight * EdgeRatio::adjustSumToTargetScore(newEdgeRatioSum, pairsCount);
        newCurrentScore += s3Weight * (newAligEdges / (g1Edges + newInducedEdges - newAligEdges));
        newCurrentScore += icsWeight * (newAligEdges / newInducedEdges);
        newCurrentScore += secWeight * (newAligEdges / g1Edges + newAligEdges / g2Edges)*0.5;
        newCurrentScore += TCWeight * (newTCSum);
        newCurrentScore += localWeight * (newLocalScoreSum / n1);
        newCurrentScore += wecWeight * (newWecSum / (2 * g1Edges));
        newCurrentScore += jsWeight * (newJsSum);
        newCurrentScore += ewecWeight * (newEwecSum);
        newCurrentScore += ncWeight * (newNcSum / trueA.back());
#ifdef MULTI_PAIRWISE
        newCurrentScore += mecWeight * (newAligEdges / (g1WeightedEdges + g2WeightedEdges));
        newCurrentScore += sesWeight * newSquaredAligEdges / (double)SquaredEdgeScore::getDenom();
	newCurrentScore += eeWeight * (1 - (newExposedEdgesNumer / (double)EdgeExposure::getDenom()));
        if (MultiS3::denominator_type==MultiS3::ee_global){
	    MultiS3::denom = newExposedEdgesNumer;
	}
        //if (MultiS3::numerator_type==MultiS3::ra_i or MultiS3::numerator_type==MultiS3::ra_global and)
	//newCurrentScore += ms3Weight * (double)newMS3Numer / (double)MultiS3::denom / (double)NUM_GRAPHS;
        //if (   (   (MultiS3::numerator_type==MultiS3::ra_i   or MultiS3::numerator_type==MultiS3::ra_global)
        //       and (MultiS3::denominator_type==MultiS3::ee_i or MultiS3::denominator_type==MultiS3::ee_global))
        //    or (MultiS3::numerator_type==MultiS3::_default and MultiS3::denominator_type==MultiS3::_default)  )
       // {
        newCurrentScore += ms3Weight * (double)newMS3Numer / (double)MultiS3::denom / (double)NUM_GRAPHS;
       // }
       // else {
       // newCurrentScore += ms3Weight * (double)newMS3Numer / (double)MultiS3::denom;
       // }
#endif
        energyInc = newCurrentScore - currentScore;
        wasBadMove = energyInc < 0;
        //using max and min here because with extremely low temps I was seeing invalid probabilities
        //note: I did not make this change for the other types of Score::  -Nil
        badProbability = max(0.0, min(1.0, exp(energyInc / Temperature)));
        makeChange = (energyInc >= 0 or randomReal(gen) <= badProbability);
        break;
    }
    case Score::product:
    {
        newCurrentScore = 1;
        newCurrentScore *= ecWeight * (newAligEdges / g1Edges);
        newCurrentScore *= s3Weight * (newAligEdges / (g1Edges + newInducedEdges - newAligEdges));
        newCurrentScore *= icsWeight * (newAligEdges / newInducedEdges);
        newCurrentScore *= TCWeight * (newTCSum);
        newCurrentScore *= localWeight * (newLocalScoreSum / n1);
        newCurrentScore *= secWeight * (newAligEdges / g1Edges + newAligEdges / g2Edges)*0.5;
        newCurrentScore *= wecWeight * (newWecSum / (2 * g1Edges));
        newCurrentScore += jsWeight * (newJsSum);
        newCurrentScore *= ncWeight * (newNcSum / trueA.back());
        energyInc = newCurrentScore - currentScore;
        wasBadMove = energyInc < 0;
        badProbability = exp(energyInc / Temperature);
        makeChange = (energyInc >= 0 or randomReal(gen) <= exp(energyInc / Temperature));
        break;
    }
    case Score::max:
    {
	// this is a terrible way to compute the max; we should loop through all of them and figure out which is the biggest
	// and in fact we haven't yet integrated icsWeight here yet, so assert so
	assert(icsWeight == 0.0);
        double deltaEnergy = max(ncWeight* (newNcSum / trueA.back() - ncSum / trueA.back()), max(max(ecWeight*(newAligEdges / g1Edges - aligEdges / g1Edges), max(
            s3Weight*((newAligEdges / (g1Edges + newInducedEdges - newAligEdges) - (aligEdges / (g1Edges + inducedEdges - aligEdges)))),
            secWeight*0.5*(newAligEdges / g1Edges - aligEdges / g1Edges + newAligEdges / g2Edges - aligEdges / g2Edges))),
            max(localWeight*((newLocalScoreSum / n1) - (localScoreSum)),
                max(wecWeight*(newWecSum / (2 * g1Edges) - wecSum / (2 * g1Edges)), jsWeight*(newJsSum - jsSum)))));

        newCurrentScore += ecWeight * (newAligEdges / g1Edges);
        newCurrentScore += secWeight * (newAligEdges / g1Edges + newAligEdges / g2Edges)*0.5;
        newCurrentScore += s3Weight * (newAligEdges / (g1Edges + newInducedEdges - newAligEdges));
        newCurrentScore += icsWeight * (newAligEdges / newInducedEdges);
        newCurrentScore += localWeight * (newLocalScoreSum / n1);
        newCurrentScore += wecWeight * (newWecSum / (2 * g1Edges));
        newCurrentScore += jsWeight * (newJsSum);
        newCurrentScore += ncWeight * (newNcSum / trueA.back());

        energyInc = newCurrentScore - currentScore;
        wasBadMove = energyInc < 0;
        badProbability = exp(energyInc / Temperature);
        makeChange = deltaEnergy >= 0 or randomReal(gen) <= exp(energyInc / Temperature);
        break;
    }
    case Score::min:
    {
	// see comment above in max
	assert(icsWeight == 0.0);
        double deltaEnergy = min(ncWeight* (newNcSum / trueA.back() - ncSum / trueA.back()), min(min(ecWeight*(newAligEdges / g1Edges - aligEdges / g1Edges), min(
            s3Weight*((newAligEdges / (g1Edges + newInducedEdges - newAligEdges) - (aligEdges / (g1Edges + inducedEdges - aligEdges)))),
            secWeight*0.5*(newAligEdges / g1Edges - aligEdges / g1Edges + newAligEdges / g2Edges - aligEdges / g2Edges))),
            min(localWeight*((newLocalScoreSum / n1) - (localScoreSum)),
                min(wecWeight*(newWecSum / (2 * g1Edges) - wecSum / (2 * g1Edges)), jsWeight*(newJsSum - jsSum)))));

        newCurrentScore += ecWeight * (newAligEdges / g1Edges);
        newCurrentScore += s3Weight * (newAligEdges / (g1Edges + newInducedEdges - newAligEdges));
        newCurrentScore += icsWeight * (newAligEdges / newInducedEdges);
        newCurrentScore += secWeight * (newAligEdges / g1Edges + newAligEdges / g2Edges)*0.5;
        newCurrentScore += localWeight * (newLocalScoreSum / n1);
        newCurrentScore += wecWeight * (newWecSum / (2 * g1Edges));
        newCurrentScore += jsWeight * (newJsSum);
        newCurrentScore += ncWeight * (newNcSum / trueA.back());

        energyInc = newCurrentScore - currentScore; //is this even used?
        wasBadMove = deltaEnergy < 0;
        badProbability = exp(energyInc / Temperature);
        makeChange = deltaEnergy >= 0 or randomReal(gen) <= exp(newCurrentScore / Temperature);
        break;
    }
    case Score::inverse:
    {
        newCurrentScore += ecWeight / (newAligEdges / g1Edges);
        newCurrentScore += secWeight * (newAligEdges / g1Edges + newAligEdges / g2Edges)*0.5;
        newCurrentScore += s3Weight / (newAligEdges / (g1Edges + newInducedEdges - newAligEdges));
        newCurrentScore += icsWeight / (newAligEdges / newInducedEdges);
        newCurrentScore += localWeight / (newLocalScoreSum / n1);
        newCurrentScore += wecWeight / (newWecSum / (2 * g1Edges));
        newCurrentScore += jsWeight * (newJsSum);
        newCurrentScore += ncWeight / (newNcSum / trueA.back());

        energyInc = newCurrentScore - currentScore;
        wasBadMove = energyInc < 0;
        badProbability = exp(energyInc / Temperature);
        makeChange = (energyInc >= 0 or randomReal(gen) <= exp(energyInc / Temperature));
        break;
    }
    case Score::maxFactor:
    {
	assert(icsWeight == 0.0);
        double maxScore = max(ncWeight*(newNcSum / trueA.back() - ncSum / trueA.back()), max(max(ecWeight*(newAligEdges / g1Edges - aligEdges / g1Edges), max(
            s3Weight*((newAligEdges / (g1Edges + newInducedEdges - newAligEdges) - (aligEdges / (g1Edges + inducedEdges - aligEdges)))),
            secWeight*0.5*(newAligEdges / g1Edges - aligEdges / g1Edges + newAligEdges / g2Edges - aligEdges / g2Edges))),
            max(localWeight*((newLocalScoreSum / n1) - (localScoreSum)),
                max(wecWeight*(newWecSum / (2 * g1Edges) - wecSum / (2 * g1Edges)), jsWeight*(newJsSum - jsSum)))));

        double minScore = min(ncWeight*(newNcSum / trueA.back() - ncSum / trueA.back()), min(min(ecWeight*(newAligEdges / g1Edges - aligEdges / g1Edges), min(
            s3Weight*((newAligEdges / (g1Edges + newInducedEdges - newAligEdges) - (aligEdges / (g1Edges + inducedEdges - aligEdges)))),
            secWeight*0.5*(newAligEdges / g1Edges - aligEdges / g1Edges + newAligEdges / g2Edges - aligEdges / g2Edges))),
            min(localWeight*((newLocalScoreSum / n1) - (localScoreSum)),
                min(wecWeight*(newWecSum / (2 * g1Edges) - wecSum / (2 * g1Edges)), jsWeight*(newJsSum - jsSum)))));

        newCurrentScore += ecWeight * (newAligEdges / g1Edges);
        newCurrentScore += secWeight * (newAligEdges / g1Edges + newAligEdges / g2Edges)*0.5;
        newCurrentScore += s3Weight * (newAligEdges / (g1Edges + newInducedEdges - newAligEdges));
        newCurrentScore += icsWeight * (newAligEdges / newInducedEdges);
        newCurrentScore += localWeight * (newLocalScoreSum / n1);
        newCurrentScore += wecWeight * (newWecSum / (2 * g1Edges));
        newCurrentScore += jsWeight * (newJsSum);
        newCurrentScore += ncWeight * (newNcSum / trueA.back());

        energyInc = newCurrentScore - currentScore;
        wasBadMove = maxScore < -1 * minScore;
        badProbability = exp(energyInc / Temperature);
        makeChange = maxScore >= -1 * minScore or randomReal(gen) <= exp(energyInc / Temperature);
        break;
    }
    case Score::pareto:
    { //This determines whether we should update the current alignment.
        vector<double> addScores = getMeasureScores(newAligEdges, newInducedEdges, newTCSum,
                    newLocalScoreSum, newWecSum, newJsSum, newNcSum, newEwecSum, newSquaredAligEdges,
                    newExposedEdgesNumer, newMS3Numer, newEdgeDifferenceSum, newEdgeRatioSum);
        if(dominates(addScores, currentScores))
        {
            currentScores = addScores;
            makeChange = true;
            newCurrentScore = 0;
            newCurrentScore += ecWeight * (newAligEdges / g1Edges);
            newCurrentScore += edWeight * EdgeDifference::adjustSumToTargetScore(newEdgeDifferenceSum, pairsCount);
            newCurrentScore += erWeight * EdgeRatio::adjustSumToTargetScore(newEdgeRatioSum, pairsCount);
            newCurrentScore += s3Weight * (newAligEdges / (g1Edges + newInducedEdges - newAligEdges));
            newCurrentScore += icsWeight * (newAligEdges / newInducedEdges);
            newCurrentScore += TCWeight * (newTCSum);
            newCurrentScore += localWeight * (newLocalScoreSum / n1);
            newCurrentScore += secWeight * (newAligEdges / g1Edges + newAligEdges / g2Edges)*0.5;
            newCurrentScore += wecWeight * (newWecSum / (2 * g1Edges));
            newCurrentScore += jsWeight * (newJsSum);
            newCurrentScore += ncWeight * (newNcSum / trueA.back());
        }
        else
        {
            newCurrentScore = 0;
            newCurrentScore += ecWeight * (newAligEdges / g1Edges);
            newCurrentScore += edWeight * EdgeDifference::adjustSumToTargetScore(newEdgeDifferenceSum, pairsCount);
            newCurrentScore += erWeight * EdgeRatio::adjustSumToTargetScore(newEdgeRatioSum, pairsCount);
            newCurrentScore += s3Weight * (newAligEdges / (g1Edges + newInducedEdges - newAligEdges));
            newCurrentScore += icsWeight * (newAligEdges / newInducedEdges);
            newCurrentScore += TCWeight * (newTCSum);
            newCurrentScore += localWeight * (newLocalScoreSum / n1);
            newCurrentScore += secWeight * (newAligEdges / g1Edges + newAligEdges / g2Edges)*0.5;
            newCurrentScore += wecWeight * (newWecSum / (2 * g1Edges));
            newCurrentScore += jsWeight * (newJsSum);
            newCurrentScore += ncWeight * (newNcSum / trueA.back());
            energyInc = newCurrentScore - currentScore;
            wasBadMove = energyInc < 0;
            badProbability = exp(energyInc / Temperature);
            makeChange = (addScores[currentMeasure] > currentScores[currentMeasure] or energyInc >= 0 or randomReal(gen) <= exp(energyInc / Temperature));
            if(makeChange) currentScores = addScores;
        }
        break;
    }
    }

    //if (wasBadMove && (iterationsPerformed % 512 == 0 || (TCWeight > 0 && iterationsPerformed % 32 == 0))) {
	//the above will never be true in the case of iterationsPerformed never being changed so that it doesn't greatly
	// slow down the program if for some reason iterationsPerformed doesn't need to be changed.
    if (wasBadMove) { // I think Dillon was wrong above, just do it always - WH
        if (numPBadsInBuffer == PBAD_CIRCULAR_BUFFER_SIZE) {
            pBadBufferIndex = (pBadBufferIndex == PBAD_CIRCULAR_BUFFER_SIZE ? 0 : pBadBufferIndex);
            pBadBufferSum -= pBadBuffer[pBadBufferIndex];
            pBadBuffer[pBadBufferIndex] = badProbability;
        }
        else
        {
            pBadBuffer[pBadBufferIndex] = badProbability;
            numPBadsInBuffer++;
        }
        pBadBufferSum += badProbability;
        pBadBufferIndex++;
    }
    
    return makeChange;
}

int SANA::aligEdgesIncChangeOp(uint source, uint oldTarget, uint newTarget) {
    int res = 0;

    bool selfLoopAtSource, selfLoopAtOldTarget, selfLoopAtNewTarget;
#ifdef SPARSE
    selfLoopAtSource = G1->hasSelfLoop(source);
    selfLoopAtOldTarget = G2->hasSelfLoop(oldTarget);
    selfLoopAtNewTarget = G2->hasSelfLoop(newTarget);
#else
    selfLoopAtSource = G1Matrix[source][source];
    selfLoopAtOldTarget = G2Matrix[oldTarget][oldTarget];
    selfLoopAtNewTarget = G2Matrix[newTarget][newTarget];
#endif

    const vector<uint>& v = G1AdjLists[source];
    if(selfLoopAtSource) {
        if (selfLoopAtOldTarget) res--;
        if (selfLoopAtNewTarget) res++;
    }
#ifdef SPARSE
cout<<"sprase correct********";
    for (uint neighbor : v) if(neighbor != source) {
        if(G2Matrix[oldTarget].count(neighbor)){
            res -= G2Matrix[oldTarget][(*A)[neighbor]];
        }
        if(G2Matrix[newTarget].count(neighbor)){
            res += G2Matrix[newTarget][(*A)[neighbor]];
        }
    }
#else
    for (uint neighbor : v) if(neighbor != source) {
        res -= G2Matrix[oldTarget][(*A)[neighbor]];
        res += G2Matrix[newTarget][(*A)[neighbor]];
    }
#endif
    return res;
}

int SANA::aligEdgesIncSwapOp(uint source1, uint source2, uint target1, uint target2) {
    int res = 0;

    bool selfLoopAtSource1, selfLoopAtSource2, selfLoopAtTarget1, selfLoopAtTarget2;
#ifdef SPARSE
    selfLoopAtSource1 = G1->hasSelfLoop(source1);
    selfLoopAtSource2 = G1->hasSelfLoop(source2);
    selfLoopAtTarget1 = G2->hasSelfLoop(target1);
    selfLoopAtTarget2 = G2->hasSelfLoop(target2);
#else
    selfLoopAtSource1 = G1Matrix[source1][source1];
    selfLoopAtSource2 = G1Matrix[source2][source2];
    selfLoopAtTarget1 = G2Matrix[target1][target1];
    selfLoopAtTarget2 = G2Matrix[target2][target2];
#endif

    const vector<uint>& v1 = G1AdjLists[source1];
    if(selfLoopAtSource1) {
        if (selfLoopAtTarget1) res--;
        if (selfLoopAtTarget2) res++;
    }
#ifdef SPARSE
    for(uint neighbor : v1) if(neighbor != source1) {
        if(G2Matrix[target1].count(neighbor)){
            res -= G2Matrix[target1][(*A)[neighbor]];
        }
        if(G2Matrix[target2].count(neighbor)){
            res += G2Matrix[target2][(*A)[neighbor]];
        }
    }
#else
    for(uint neighbor : v1) if(neighbor != source1) {
        res -= G2Matrix[target1][(*A)[neighbor]];
        res += G2Matrix[target2][(*A)[neighbor]];
    }
#endif

    const vector<uint>& v2 = G1AdjLists[source2];
    if(selfLoopAtSource2) {
        if (selfLoopAtTarget2) res--;
        if (selfLoopAtTarget1) res++;
    }
#ifdef SPARSE
    for(uint neighbor : v2) if(neighbor != source2) {
        if(G2Matrix[target2].count(neighbor)){
            res -= G2Matrix[target2][(*A)[neighbor]];
        }
        if(G2Matrix[target1].count(neighbor)){
            res += G2Matrix[target1][(*A)[neighbor]];
        }
    }
#else
    for(uint neighbor : v2) if(neighbor != source2) {
        res -= G2Matrix[target2][(*A)[neighbor]];
        res += G2Matrix[target1][(*A)[neighbor]];
    }
#endif
#ifdef MULTI_PAIRWISE
    res += (-1 << 1) & (G1Matrix[source1][source2] + G2Matrix[target1][target2]);
#else
    #ifdef SPARSE
        if(G1Matrix[source1].count(source2) & G2Matrix[target1].count(target2)){
            res += 2 * (G1Matrix[source1][source2] & G2Matrix[target1][target2]);
        }
    #else
        res += 2 * (G1Matrix[source1][source2] & G2Matrix[target1][target2]);
    #endif
#endif
    return res;
}

static double getRatio(double w1, double w2) {
    double r;
    if(w1==0 && w2==0)r=1;
    else if(abs(w1)<abs(w2))r=w1/w2;
    else r=w2/w1;
    // At this point, r can be in [-1,1], but we want it [0,1], so add 1 and divide by 2
    r = (r+1)/2;
    assert(r>=0 && r<=1);
    return r;
}

/*
 * We swap the mapping of two nodes source1 and source2
 * We can first handle source1, then do the same with source2
 * Subtract old edge difference with edge (source1, target1)
 * Add new edge difference with edge (source1, target2)
 */
double SANA::edgeDifferenceIncSwapOp(uint source1, uint source2, uint target1, uint target2) {
    if (source1 == source2)
        return 0;

    /*
     * Handle source1
     */
    double edgeDifferenceIncDiff = 0;
    double c = 0;
    vector<uint> &adjList = G1AdjLists[source1];
    for (uint i = 0; i < adjList.size(); ++i) {
        uint node2 = adjList[i];
        double y = -abs(G1FloatWeights[source1][node2] - G2FloatWeights[target1][(*A)[node2]])
                  - c;
        double t = edgeDifferenceIncDiff + y;
        c = (t - edgeDifferenceIncDiff) - y;
        edgeDifferenceIncDiff = t;

        /*
         * Determine the new target node for node2
         */
        uint node2Target = 0;
        if (node2 == source1) {
            node2Target = target2;
        } else if (node2 == source2) {
            node2Target = target1;
        } else {
            node2Target = (*A)[node2];
        }

        y = +abs(G1FloatWeights[source1][node2] - G2FloatWeights[target2][node2Target])
           - c;
        t = edgeDifferenceIncDiff + y;
        c = (t - edgeDifferenceIncDiff) - y;
        edgeDifferenceIncDiff = t;
   }

   /*
    * Handle source2
    */
   vector<uint> &adjList2 = G1AdjLists[source2];
   for (uint i = 0; i < adjList2.size(); ++i) {
       uint node2 = adjList2[i];
       if (node2 == source1)
         continue;

       double y = -abs(G1FloatWeights[source2][node2] - G2FloatWeights[target2][(*A)[node2]])
                  - c;
       double t = edgeDifferenceIncDiff + y;
       c = (t - edgeDifferenceIncDiff) - y;
       edgeDifferenceIncDiff = t;

       uint node2Target = 0;
       if (node2 == source2) {
           node2Target = target1;
       } else {
           node2Target = (*A)[node2];
       }
       y = +abs(G1FloatWeights[source2][node2] - G2FloatWeights[target1][node2Target])
          - c;
       t = edgeDifferenceIncDiff + y;
       c = (t - edgeDifferenceIncDiff) - y;
       edgeDifferenceIncDiff = t;
   }

   return edgeDifferenceIncDiff;
}


double SANA::edgeRatioIncSwapOp(uint source1, uint source2, uint target1, uint target2) {
    if (source1 == source2)
      return 0;

    // Subtract source1-target1
    // Add source1-target2
    double edgeRatioIncDiff = 0;
    double c = 0;
  
    vector<uint> &adjList = G1AdjLists[source1];
    for (uint i = 0; i < adjList.size(); ++i) {
        uint node2 = adjList[i];

        double r = getRatio(G1FloatWeights[source1][node2], G2FloatWeights[target1][(*A)[node2]]);
        double y = -r - c;
        double t = edgeRatioIncDiff + y;
        c = (t - edgeRatioIncDiff) - y;
        edgeRatioIncDiff = t;

        uint node2Target = 0;
        if (node2 == source1) {
            node2Target = target2;
        } else if (node2 == source2) {
            node2Target = target1;
        } else {
            node2Target = (*A)[node2];
        }
        r = +getRatio(G1FloatWeights[source1][node2], G2FloatWeights[target2][node2Target]);
        y = +r - c;
        t = edgeRatioIncDiff + y;
        c = (t - edgeRatioIncDiff) - y;
        edgeRatioIncDiff = t;
   }

   // Subtract source2-target2
   // Add source2-target1
   vector<uint> &adjList2 = G1AdjLists[source2];
   for (uint i = 0; i < adjList2.size(); ++i) {
        uint node2 = adjList2[i];
        if (node2 == source1)
          continue;

        double r = getRatio(G1FloatWeights[source2][node2], G2FloatWeights[target2][(*A)[node2]]);
        double y = -r - c;
        double t = edgeRatioIncDiff + y;
        c = (t - edgeRatioIncDiff) - y;
        edgeRatioIncDiff = t;

        uint node2Target = 0;
        if (node2 == source2) {
            node2Target = target1;
        } else {
            node2Target = (*A)[node2];
        }
        r = getRatio(G1FloatWeights[source2][node2], G2FloatWeights[target1][node2Target]);
        y = +r - c;
        t = edgeRatioIncDiff + y;
        c = (t - edgeRatioIncDiff) - y;
        edgeRatioIncDiff = t;
   }

    return edgeRatioIncDiff;
}


double SANA::edgeDifferenceIncChangeOp(uint source, uint oldTarget, uint newTarget) {
   double edgeDifferenceIncDiff = 0;
   double c = 0;

   vector<uint> &adjList = G1AdjLists[source];
   for (uint i = 0; i < adjList.size(); ++i) {
       uint node2 = adjList[i];

       double y = -abs(G1FloatWeights[source][node2] - G2FloatWeights[oldTarget][(*A)[node2]])
                  - c;
       double t = edgeDifferenceIncDiff + y;
       c = (t - edgeDifferenceIncDiff) - y;
       edgeDifferenceIncDiff = t;

       uint node2Target = node2 == source ? newTarget : (*A)[node2];
       y = +abs(G1FloatWeights[source][node2] - G2FloatWeights[newTarget][node2Target])
                  - c;
       t = edgeDifferenceIncDiff + y;
       c = (t - edgeDifferenceIncDiff) - y;
       edgeDifferenceIncDiff = t;
   }


   return edgeDifferenceIncDiff;
}


double SANA::edgeRatioIncChangeOp(uint source, uint oldTarget, uint newTarget) {
   double edgeRatioIncDiff = 0;
   double c = 0;

   vector<uint> &adjList = G1AdjLists[source];
   for (uint i = 0; i < adjList.size(); ++i) {
       uint node2 = adjList[i];

       double r = getRatio(G1FloatWeights[source][node2], G2FloatWeights[oldTarget][(*A)[node2]]);
       double y = -r - c;
       double t = edgeRatioIncDiff + y;
       c = (t - edgeRatioIncDiff) - y;
       edgeRatioIncDiff = t;

       uint node2Target = node2 == source ? newTarget : (*A)[node2];
       r = getRatio(G1FloatWeights[source][node2], G2FloatWeights[newTarget][node2Target]);
       y = +r - c;
       t = edgeRatioIncDiff + y;
       c = (t - edgeRatioIncDiff) - y;
       edgeRatioIncDiff = t;
   }

   return edgeRatioIncDiff;
}



static int _edgeVal;
// UGLY GORY HACK BELOW!! Sometimes the edgeVal is crazily wrong, like way above 1,000, when it
// cannot possibly be greater than the number of networks we're aligning when MULTI_PAIRWISE is on.
// It happens only rarely, so here I ask if the edgeVal is less than 1,000; if it's less than 1,000
// then we assume it's OK, otherwise we just ignore this edge entirely and say the diff is 0.
// Second problem: even if the edgeVal is correct, I couldn't seem to figure out the difference
// between the value of this ladder and the ladder with one edge added or removed.  Mathematically
// it should be edgeVal^2 - (edgeVal+1)^2 which is (2e + 1), but for some reason I had to make
// it 2*(e+1).  That seemed to work better.  So yeah... big ugly hack.
#define SQRDIFF(i,j) ((_edgeVal=G2Matrix[i][(*A)[j]]), 2*_edgeVal + 1)
int SANA::squaredAligEdgesIncChangeOp(uint source, uint oldTarget, uint newTarget) {
    int res = 0, diff;
    uint neighbor;
    const uint n = G1AdjLists[source].size();
    for (uint i = 0; i < n; ++i) {
        neighbor = G1AdjLists[source][i];
        // Account for uint edges? Or assume smaller graph is edge value 1?
        diff = SQRDIFF(oldTarget, neighbor);
        // assert(fabs(diff)<1100);
        res -= diff;// >0?diff:0;
        diff = SQRDIFF(newTarget, neighbor);
        // assert(fabs(diff)<1100);
        res += diff;// >0?diff:0;
    }
    return res;
}

int SANA::squaredAligEdgesIncSwapOp(uint source1, uint source2, uint target1, uint target2) {
    int res = 0, diff;
    uint neighbor;
    const uint n = G1AdjLists[source1].size();
    uint i = 0;
    for (; i < n; ++i) {
        neighbor = G1AdjLists[source1][i];
        diff = SQRDIFF(target1, neighbor);
        // assert(fabs(diff)<1100);
        res -= diff;// >0?diff:0;
        diff = SQRDIFF(target2, neighbor);
        if (target2==(*A)[neighbor]){
                diff=0;
        }// assert(fabs(diff)<1100);
        res += diff;// >0?diff:0;
    }
    const uint m = G1AdjLists[source2].size();
    for (i = 0; i < m; ++i) {
        neighbor = G1AdjLists[source2][i];
        diff = SQRDIFF(target2, neighbor);
        // assert(fabs(diff)<1100);
        res -= diff;// >0?diff:0;
        diff = SQRDIFF(target1, neighbor);
        if (target1==(*A)[neighbor]){
                diff=0;
        }// assert(fabs(diff)<1100);
        res += diff;// >0?diff:0;
    }
    // How to do for squared?
    // address case swapping between adjacent nodes with adjacent images:
    if(G1Matrix[source1][source2] and G2Matrix[target1][target2])
    {
        res += 2 * SQRDIFF(target1,source2);
    }
    return res;
}

int SANA::exposedEdgesIncChangeOp(uint source, uint oldTarget, uint newTarget) {
    
    int ret = 0;
    uint neighbor;
    const uint n = G1AdjLists[source].size();
    for (uint i = 0; i < n; ++i) {
        neighbor = G1AdjLists[source][i];
		if (G2Matrix[oldTarget][(*A)[neighbor]] == 0)
		{
			--ret;
		}
		if (!G2Matrix[newTarget][(*A)[neighbor]])
		{
			++ret;
		}
    }
    return ret;

}

int SANA::exposedEdgesIncChangeOp(Job &job, uint source, uint oldTarget, uint newTarget) {
    int ret = 0;
    uint neighbor;
    vector<uint> *A = job.info.A;
    const uint n = G1AdjLists[source].size();
    for (uint i = 0; i < n; ++i) {
        neighbor = G1AdjLists[source][i];
		if (G2Matrix[oldTarget][(*A)[neighbor]] == 0)
		{
			--ret;
		}
		if (!G2Matrix[newTarget][(*A)[neighbor]])
		{
			++ret;
		}
    }
    return ret;
}

int SANA::exposedEdgesIncSwapOp(uint source1, uint source2, uint target1, uint target2) {

    
    int ret = 0;
    uint neighbor;
    const uint n = G1AdjLists[source1].size();
    uint i = 0;
    for (; i < n; ++i) {
        neighbor = G1AdjLists[source1][i];
        if (G2Matrix[target1][(*A)[neighbor]] == 0)
		{
			--ret;
		}
		if (!G2Matrix[target2][(*A)[neighbor]])
		{
			++ret;
		}
    }
    const uint m = G1AdjLists[source2].size();
    for (i = 0; i < m; ++i) {
        neighbor = G1AdjLists[source2][i];
        if (G2Matrix[target2][(*A)[neighbor]] == 0)
		{
			--ret;
		}
		if (!G2Matrix[target1][(*A)[neighbor]])
		{
			++ret;
		}
    }
    return ret;
}

int SANA::exposedEdgesIncSwapOp(Job &job, uint source1, uint source2, uint target1, uint target2) {
    vector<uint> *A = job.info.A;
    int ret = 0;
    uint neighbor;
    const uint n = G1AdjLists[source1].size();
    uint i = 0;
    for (; i < n; ++i) {
		neighbor = G1AdjLists[source1][i];
        if (G2Matrix[target1][(*A)[neighbor]] == 1)
		{
			--ret;
		}
		if (!G2Matrix[target2][(*A)[neighbor]])
		{
			++ret;
		}
    }
    const uint m = G1AdjLists[source2].size();
    for (i = 0; i < m; ++i) {
		neighbor = G1AdjLists[source2][i];
        if (G2Matrix[target2][(*A)[neighbor]] == 1)
		{
			--ret;
		}
		if (!G2Matrix[target1][(*A)[neighbor]])
		{
			++ret;
		}
    }
    return ret;
}

int SANA::MS3IncChangeOp(Job &job, uint source, uint oldTarget, uint newTarget) {
    cerr << "MS3 not implemented yet for Pareto mode\n";
    assert(false);
    return 0;
}

// Return the change in NUMERATOR of MS3
int SANA::MS3IncChangeOp(uint source, uint oldTarget, uint newTarget) {
    switch (MultiS3::denominator_type) {
        case 1: //MultiS3::rt_i
        {
                 uint neighbor;
                 const uint n1 = G1->getNumNodes();
                 const uint n2 = G2->getNumNodes();
                 vector<uint> whichPeg(n2, n1); // value of n1 represents not used
                 for (uint i = 0; i < n1; ++i){
                     whichPeg[(*A)[i]] = i; // inverse of the alignment
                  }
                 uint n_num = G2AdjLists[oldTarget].size();
                 for (uint i =0; i < n_num; i++){
                     neighbor = G2AdjLists[oldTarget][i];
                     if (whichPeg[neighbor]<n1){
                         MultiS3::denom-=G2Matrix[oldTarget][neighbor];
                 }
             }
            
                 n_num = G2AdjLists[newTarget].size();
                 for (uint i =0; i < n_num; i++){
                     neighbor = G2AdjLists[newTarget][i];
                   if (whichPeg[neighbor]<n1 and neighbor != oldTarget){

                     MultiS3::denom+=G2Matrix[newTarget][neighbor];

                 }
             }
        }
            break;
            
        case 2: //MultiS3::ee_i
        {

                uint neighbor;
                const uint n1 = G1->getNumNodes();
                const uint n2 = G2->getNumNodes();
                vector<uint> whichPeg(n2, n1); // value of n1 represents not used
                for (uint i = 0; i < n1; ++i){
                    whichPeg[(*A)[i]] = i; // inverse of the alignment
                 }
                uint n_num = G2AdjLists[oldTarget].size();
                for (uint i =0; i < n_num; i++){
                    neighbor = G2AdjLists[oldTarget][i];
                    if (whichPeg[neighbor]<n1){
                        MultiS3::denom--;
                }
            }
                n_num = G1AdjLists[source].size();
                for (uint i =0; i < n_num; i++){
                    neighbor = G1AdjLists[source][i];
                    if (!G2Matrix[(*A)[neighbor]][oldTarget]){
                        MultiS3::denom--;
                }
                    if ((*A)[neighbor]!=newTarget and !G2Matrix[(*A)[neighbor]][newTarget]){
                        MultiS3::denom++;
                }
            }
                n_num = G2AdjLists[newTarget].size();
                for (uint i =0; i < n_num; i++){
                    neighbor = G2AdjLists[newTarget][i];
                  if (whichPeg[neighbor]<n1 and source!=whichPeg[neighbor] ){
                    MultiS3::denom++;
                }
            }
        }
            break;
        default:
            break;
    }
////////
    switch (MultiS3::numerator_type){
        case 1: //MultiS3::ra_i
        {
            int res = 0, diff;
            uint neighbor;
            const uint n = G1AdjLists[source].size();
            for (uint i = 0; i < n; ++i) {
                neighbor = G1AdjLists[source][i];
                diff = G2Matrix[oldTarget][(*A)[neighbor]] + 1;

                res -= (diff==1?0:diff);
                diff = G2Matrix[newTarget][(*A)[neighbor]] + 1;

                res += (diff==1?0:diff);
            }
            return res;
        }
            break;
        
        case 4: //MultiS3::ra_global
        {
            int res = 0;
            int diff= 0;
                uint neighbor;
                const uint n = G1AdjLists[source].size();
                for (uint i = 0; i < n; ++i) {
                neighbor = G1AdjLists[source][i];
                if (G1Matrix[source][neighbor]>0){
                diff = G2Matrix[oldTarget][(*A)[neighbor]] + 1;
                        if (diff <= 1){res -= 0;}
                        else if (G2Matrix[oldTarget][(*A)[neighbor]]>1){res --;}
                        else{res-=2;}

                        diff = G2Matrix[newTarget][(*A)[neighbor]] + 1;
                        if (diff <= 1){res += 0;}
                        else if (G2Matrix[newTarget][(*A)[neighbor]]>1){res ++;}
                        else{res+=2;}
                }
            }
            return res;
        }
            break;
        
        case 2: //MultiS3::la_i
        {
            int res = 0, diff;
            uint neighbor;
            const uint n = G1AdjLists[source].size();
            bool ladder = false;
            for (uint i = 0; i < n; ++i) {
                neighbor = G1AdjLists[source][i];
                diff = G2Matrix[oldTarget][(*A)[neighbor]] + 1;

                ladder = (diff>1?true:false);
                if (ladder){res -= 1;}
                diff = G2Matrix[newTarget][(*A)[neighbor]] + 1;

                ladder = (diff>1?true:false);
                if (ladder){res += 1;}
            }
            return res;
        }
            break;
            
        case 3: //MultiS3::la_global
        {
            int res = 0;
            int diff = 0;
            uint neighbor = 0;
                bool ladder = false;
            const uint n = G1AdjLists[source].size();
            for (uint i=0;i<n;i++){
                neighbor = G1AdjLists[source][i];
                if (G1Matrix[neighbor][source]>0){
                    diff = G2Matrix[oldTarget][(*A)[neighbor]] + 1;
                            ladder = (diff > 1);
                    if (ladder and G2Matrix[oldTarget][(*A)[neighbor]]==1) {res --;}
                    diff = G2Matrix[newTarget][(*A)[neighbor]] + 1;
                    ladder = (diff > 1);
                    if (ladder and G2Matrix[newTarget][(*A)[neighbor]]==1) {res ++;}
                }
            }
            return res;
        }
            break;
        default:
        {
            int ret = 0;
                unsigned oldOldTargetDeg = MultiS3::shadowDegree[oldTarget];
                unsigned oldNewTargetDeg = MultiS3::shadowDegree[newTarget];
                bool selfLoopAtSource, selfLoopAtOldTarget, selfLoopAtNewTarget;
            #ifdef SPARSE
                selfLoopAtSource = G1->hasSelfLoop(source);
                selfLoopAtOldTarget = G2->hasSelfLoop(oldTarget);
                selfLoopAtNewTarget = G2->hasSelfLoop(newTarget);
            #else
                selfLoopAtSource = G1Matrix[source][source];
                selfLoopAtOldTarget = G2Matrix[oldTarget][oldTarget];
                selfLoopAtNewTarget = G2Matrix[newTarget][newTarget];
            #endif

                const vector<uint>& neighbors = G1AdjLists[source];
                
                if (selfLoopAtSource)
                {
                    if (selfLoopAtOldTarget)
                    {
                        --ret;
                    }
                    if (selfLoopAtNewTarget)
                    {
                        ++ret;
                    }
                }
                
                for (auto neighbor : neighbors)
                {
                    if (neighbor != source)
                    {
                        --MultiS3::shadowDegree[oldTarget];
                        ++MultiS3::shadowDegree[newTarget];
                        ret -= G2Matrix[oldTarget][(*A)[neighbor]];
                        ret += G2Matrix[newTarget][(*A)[neighbor]];
                    }
                }
                
                if (oldOldTargetDeg > 0 && !MultiS3::shadowDegree[oldTarget])
                {
                    MultiS3::denom -= 1;
                }
                
                if (oldNewTargetDeg > 0 && !MultiS3::shadowDegree[newTarget])
                {
                    MultiS3::denom += 1;
                }
                
                return ret;
        }
            break;
    }
}
int SANA::MS3IncSwapOp(Job &job, uint source1, uint source2, uint target1, uint target2) {
    cerr << "MS3 not implemented yet for Pareto mode\n";
    assert(false);
    return 0;
}
// Return change in NUMERATOR only
int SANA::MS3IncSwapOp(uint source1, uint source2, uint target1, uint target2) {
    switch (MultiS3::denominator_type){
        case 1: //MultiS3::rt_i
        {
            
            uint neighbor;
            const uint n1 = G1->getNumNodes();
            const uint n2 = G2->getNumNodes();
            vector<uint> whichPeg(n2, n1);
            for (uint i = 0; i < n1; ++i){
                whichPeg[(*A)[i]] = i;
             }

            uint n_num = G2AdjLists[target1].size();
            for (uint i =0; i < n_num; i++){
                neighbor = G2AdjLists[target1][i];
                if (whichPeg[neighbor]<n1){
                    MultiS3::denom-=G2Matrix[target1][neighbor];
                }
            }
            n_num = G1AdjLists[source1].size();
            for (uint i =0; i < n_num; i++){
                neighbor = G1AdjLists[source1][i];
                if (G1Matrix[source1][neighbor]){
                    MultiS3::denom--;
                }
                if (G1Matrix[source1][neighbor]){
                    MultiS3::denom++;

                }
            }
            n_num = G2AdjLists[target2].size();
            for (uint i =0; i < n_num; i++){
                neighbor = G2AdjLists[target2][i];
                if (whichPeg[neighbor]<n1){
                    MultiS3::denom+=G2Matrix[target2][neighbor];
                 }
            }

             n_num = G2AdjLists[target2].size();
            for (uint i =0; i < n_num; i++){
                neighbor = G2AdjLists[target2][i];
                if (whichPeg[neighbor]<n1 and neighbor != target1){
                    MultiS3::denom-=G2Matrix[target2][neighbor];
                 }
            }
            n_num = G1AdjLists[source2].size();
            for (uint i =0; i < n_num; i++){
                neighbor = G1AdjLists[source2][i];
                if (G1Matrix[source2][neighbor]  and source1 != neighbor){
                    MultiS3::denom--;
                }
                if (neighbor!=source1 and G1Matrix[source2][neighbor]){
                    MultiS3::denom++;
                }
            }
            n_num = G2AdjLists[target1].size();
            for (uint i =0; i < n_num; i++){
                neighbor = G2AdjLists[target1][i];
                if (neighbor != target2 and whichPeg[neighbor]<n1){
                    MultiS3::denom+=G2Matrix[target1][neighbor];
                }
             }
        }
            break;
        case 2: //MultiS3::ee_i
        {
               uint neighbor;
               const uint n1 = G1->getNumNodes();
               const uint n2 = G2->getNumNodes();
               vector<uint> whichPeg(n2, n1); // value of n1 represents not used
               for (uint i = 0; i < n1; ++i){
                   whichPeg[(*A)[i]] = i; // inverse of the alignment
               }

               uint n_num = G2AdjLists[target1].size();
               for (uint i =0; i < n_num; i++){
                       neighbor = G2AdjLists[target1][i];
                       if (whichPeg[neighbor]<n1 and neighbor!=target2 and G2Matrix[neighbor][target1]){
                       MultiS3::denom--;
                   }
               }
               n_num = G1AdjLists[source1].size();
               for (uint i =0; i < n_num; i++){
                       neighbor = G1AdjLists[source1][i];
                   
                   if (neighbor!=source2){
                           if (!G2Matrix[(*A)[neighbor]][target1]){
                           MultiS3::denom--;
                       }
                           if (!G2Matrix[(*A)[neighbor]][target2]){
                           MultiS3::denom++;
                       }
                   }
               }

               n_num = G2AdjLists[target2].size();
               for (uint i =0; i < n_num; i++){
                       neighbor = G2AdjLists[target2][i];
                   if (neighbor!=target1 and whichPeg[neighbor]<n1 and source1!=whichPeg[neighbor] and !G2Matrix[neighbor][target2]){
                       MultiS3::denom++;
                   }
               }


               n_num = G2AdjLists[target2].size();
               for (uint i =0; i < n_num; i++){
                       neighbor = G2AdjLists[target2][i];
                       if (neighbor!=target1 and whichPeg[neighbor]<n1 and !G2Matrix[target2][neighbor]){
                       MultiS3::denom--;
                       }
               }
            
               n_num = G1AdjLists[source2].size();
               for (uint i =0; i < n_num; i++){
                       neighbor = G1AdjLists[source2][i];
                   if (neighbor!=source1){
                           if (!G2Matrix[(*A)[neighbor]][target2]){
                           MultiS3::denom--;
                           }
                           if (!G2Matrix[(*A)[neighbor]][target1]){
                       MultiS3::denom++;
                           }
                   }
               }
            
               n_num = G2AdjLists[target1].size();
               for (uint i =0; i < n_num; i++){
                       neighbor = G2AdjLists[target1][i];
                       if ( G2Matrix[target1][neighbor]>0 and whichPeg[neighbor]<n1 and neighbor!=target2 and source2!=whichPeg[neighbor]){
                       MultiS3::denom++;
                       }
               }
        }
            break;
            
    }
    switch (MultiS3::numerator_type){
        case 1: //MultiS3::ra_i
        {
            
            int res = 0, diff;
            uint neighbor;
            const uint n = G1AdjLists[source1].size();
            uint i = 0;
            for (; i < n; ++i) {
                neighbor = G1AdjLists[source1][i];
                diff = G2Matrix[target1][(*A)[neighbor]] + 1;
                res -= (diff==1?0:diff);
                diff = G2Matrix[target2][(*A)[neighbor]] + 1;
                if (target2==(*A)[neighbor]){
                    diff=0;
                }

                res += (diff==1?0:diff);
            }
            const uint m = G1AdjLists[source2].size();
            for (i = 0; i < m; ++i) {
                neighbor = G1AdjLists[source2][i];
                diff = G2Matrix[target2][(*A)[neighbor]] + 1;
                res -= (diff==1?0:diff);
                diff = G2Matrix[target1][(*A)[neighbor]] + 1;
                if (target1==(*A)[neighbor]){
                    diff=0;
                }

                res += (diff==1?0:diff);
            }
            if(G2Matrix[target1][target2] and  G1Matrix[source1][source2])
            {
                diff = ( G2Matrix[target1][(*A)[source2]] + 1);

                res += 2*(diff==1?0:diff);
            }
            return res;
        }
            break;
        case 4: //MultiS3::ra_global
        {
            int res = 0, diff;
            uint neighbor;
            const uint n = G1AdjLists[source1].size();
            uint i = 0;
            bool ladder = false;
            
            for (; i < n; ++i) {
                neighbor = G1AdjLists[source1][i];
                if (G1Matrix[neighbor][source1]>0){
                    diff = G2Matrix[target1][(*A)[neighbor]] + 1;
                    ladder = (diff>1?true:false);
                    if (ladder){
                        res--;
                        if (G2Matrix[target1][(*A)[neighbor]] == 1){res--;}
                    }

                    diff = G2Matrix[target2][(*A)[neighbor]] + 1;
                    if (target2!=(*A)[neighbor]){
                        if (diff==2){res+=2;}
                        else if (diff>1){res++;}
                    }
                }
            }
            
            if (G1Matrix[source1][source2]==1){
                res++;
                if (G2Matrix[target2][target1]==1){res++;}
                else if (G2Matrix[target2][target1]==0){res--;}
            }
            
            const uint m = G1AdjLists[source2].size();
            for (i = 0; i < m; ++i) {
                neighbor = G1AdjLists[source2][i];
                if (G1Matrix[neighbor][source2]>0){
                    diff = G2Matrix[target2][(*A)[neighbor]] + 1;

                    ladder = (diff>1?true:false);
                    if (ladder and neighbor!=source1 and G2Matrix[target2][(*A)[neighbor]]){
                        res--;
                        if (G2Matrix[target2][(*A)[neighbor]] == 1){res--;}
                        }
                    
                    diff = G2Matrix[target1][(*A)[neighbor]] + 1;
                    if (target1!=(*A)[neighbor]){
                        if (diff==2){res+=2;}
                        else if (diff>1){res++;}
                    }
                }
            }
            return res;
        }
            break;
        case 2: //MultiS3::la_i
        {
             int res = 0, diff;
             uint neighbor;
             const uint n = G1AdjLists[source1].size();
             uint i = 0;
             bool ladder = false;
             for (; i < n; ++i) {
                 neighbor = G1AdjLists[source1][i];
                 diff = G2Matrix[target1][(*A)[neighbor]] + 1;

                 ladder = (diff>1?true:false);
                 diff = G2Matrix[target2][(*A)[neighbor]] + 1;
                 if (target2==(*A)[neighbor]){
                     diff=0;
                 }

                 if (ladder and diff<=1){
                     res--;
                 }else if (not ladder and diff>1){
                     res++;
                 }
             }
             const uint m = G1AdjLists[source2].size();
             for (i = 0; i < m; ++i) {
                 neighbor = G1AdjLists[source2][i];
                 diff = G2Matrix[target2][(*A)[neighbor]] + 1;

                 ladder = (diff>1?true:false);
                 diff = G2Matrix[target1][(*A)[neighbor]] + 1;
                 if (target1==(*A)[neighbor]){
                     diff=0;
                 }

                 if (ladder and diff<=1){
                     res--;
                 }else if (not ladder and diff>1){
                     res++;
                 }
             }
             if(G2Matrix[target1][target2] and  G1Matrix[source1][source2])
             {
                 diff = ( G2Matrix[target1][(*A)[source2]] + 1);

                 res += 2*(diff>1?1:0);
             }
             return res;
        }
            break;
        case 3: //MultiS3::la_global
        {
            int res = 0, diff;
            uint neighbor;
            const uint n = G1AdjLists[source1].size();
            uint i = 0;
            bool ladder = false;
            
            for (; i < n; ++i) {
                neighbor = G1AdjLists[source1][i];
                if (G1Matrix[neighbor][source1]>0){
                    diff = G2Matrix[target1][(*A)[neighbor]] + 1;

                    ladder = (diff>1?true:false);
                    if (ladder and G2Matrix[target1][(*A)[neighbor]] == 1){res--;}
                    if (target2!=(*A)[neighbor] and G2Matrix[target2][(*A)[neighbor]]==1){res++;}
                }
            }
            if (G2Matrix[target2][(*A)[source1]]==1 and G1Matrix[source1][source2]==1){res++;}
            
            const uint m = G1AdjLists[source2].size();
            for (i = 0; i < m; ++i) {
                neighbor = G1AdjLists[source2][i];
                if (G1Matrix[neighbor][source2]>0){
                    diff = G2Matrix[target2][(*A)[neighbor]] + 1;

                    ladder = (diff>1?true:false);
                    if (ladder and neighbor!=source1 and G2Matrix[target2][(*A)[neighbor]] == 1){res--;}
                    if (target1!=(*A)[neighbor] and G2Matrix[target1][(*A)[neighbor]]==1 and neighbor!=source1){res++;}
                }
            }
            return res;
        }
            break;
        default:
        {
            int ret = 0;
                    unsigned oldTarget1Deg = MultiS3::shadowDegree[target1];
                unsigned oldTarget2Deg = MultiS3::shadowDegree[target2];
                
                    bool selfLoopAtSource1, selfLoopAtSource2, selfLoopAtTarget1, selfLoopAtTarget2;
            #ifdef SPARSE
                    selfLoopAtSource1 = G1->hasSelfLoop(source1);
                    selfLoopAtSource2 = G1->hasSelfLoop(source2);
                    selfLoopAtTarget1 = G2->hasSelfLoop(target1);
                    selfLoopAtTarget2 = G2->hasSelfLoop(target2);
            #else
                    selfLoopAtSource1 = G1Matrix[source1][source1];
                selfLoopAtSource2 = G1Matrix[source2][source2];
                    selfLoopAtTarget1 = G2Matrix[target1][target1];
                    selfLoopAtTarget2 = G2Matrix[target2][target2];
            #endif

                const vector<uint>& neighbors1 = G1AdjLists[source1];
                const vector<uint>& neighbors2 = G1AdjLists[source2];
                
                if (selfLoopAtSource1)
                {
                    if (selfLoopAtTarget1)
                    {
                        --ret;
                    }
                    if (selfLoopAtTarget2)
                    {
                        ++ret;
                    }
                }
                
                if (selfLoopAtSource2)
                {
                    if (selfLoopAtTarget1)
                    {
                        --ret;
                    }
                    if (selfLoopAtTarget2)
                    {
                        ++ret;
                    }
                }
                
                for (auto neighbor : neighbors1)
                {
                    if (neighbor != source1)
                    {
                        --MultiS3::shadowDegree[target1];
                        ++MultiS3::shadowDegree[target2];
                        ret -= G2Matrix[target1][(*A)[neighbor]];
                        ret += G2Matrix[target2][(*A)[neighbor]];
                    }
                }
                
                for (auto neighbor : neighbors2)
                {
                    if (neighbor != source1)
                    {
                        --MultiS3::shadowDegree[target2];
                        ++MultiS3::shadowDegree[target1];
                        ret -= G2Matrix[target2][(*A)[neighbor]];
                        ret += G2Matrix[target1][(*A)[neighbor]];
                    }
                }
                
                if (oldTarget1Deg > 0 && !MultiS3::shadowDegree[target1])
                {
                    MultiS3::denom -= 1;
                }
                
                if (oldTarget2Deg > 0 && !MultiS3::shadowDegree[target2])
                {
                    MultiS3::denom += 1;
                }

                return ret;
        }
            break;
    }
  
}

int SANA::inducedEdgesIncChangeOp(uint source, uint oldTarget, uint newTarget) {
    int res = 0;
    const uint n = G2AdjLists[oldTarget].size();
    uint neighbor;
    uint i = 0;
    for (; i < n; ++i) {
        neighbor = G2AdjLists[oldTarget][i];
        res -= (*assignedNodesG2)[neighbor];
    }
    const uint m = G2AdjLists[newTarget].size();
    for (i = 0; i < m; ++i) {
        neighbor = G2AdjLists[newTarget][i];
        res += (*assignedNodesG2)[neighbor];
    }
    //address case changing between adjacent nodes:
    res -= G2Matrix[oldTarget][newTarget];
    return res;
}

double SANA::localScoreSumIncChangeOp(vector<vector<float> > const & sim, uint const & source, uint const & oldTarget, uint const & newTarget) {
    return sim[source][newTarget] - sim[source][oldTarget];
}

double SANA::localScoreSumIncSwapOp(vector<vector<float> > const & sim, uint const & source1, uint const & source2, uint const & target1, uint const & target2) {
    return sim[source1][target2] - sim[source1][target1] + sim[source2][target1] - sim[source2][target2];
}

double SANA::JSIncChangeOp(uint source, uint oldTarget, uint newTarget) {
    double change = 0;
    if (jsWeight>0){
        //eval newJsSum here
        //update alignedByNode with source and source neighbours using oldTarget and newTarget

        // eval for source from sratch
        uint sourceOldAlingedEdges = alignedByNode[source];
        uint sourceAlignedEdges = 0;
        vector<uint> sourceNeighbours = G1AdjLists[source];
        uint sourceTotalEdges = sourceNeighbours.size();
        for (uint j = 0; j < sourceTotalEdges; j++){
            uint neighbour = sourceNeighbours[j];
            uint neighbourAlignedTo = (*A)[neighbour]; //find the node neighbour is mapped to
            sourceAlignedEdges += G2Matrix[newTarget][neighbourAlignedTo];
        }
        alignedByNode[source] = sourceAlignedEdges;
        //update newJsSum here
        change += ((sourceAlignedEdges - sourceOldAlingedEdges)/(double)sourceTotalEdges);


        // for each source neighbour update do iterative changes to the jsAlingedByNode vector
        // in each update get the G2mapping of neighbour and then check if edge was aligned by oldTarget to G2mapping and reduce score if newTarget to G2mapping doesnt exist
        // increase score if oldTarget to G2 mapping edge didnt exist but newTarget to G2 mapping does
        //no changes other wise
        for (uint j = 0; j < sourceTotalEdges; j++){
            uint neighbour = sourceNeighbours[j];
            uint neighbourAlignedTo = (*A)[neighbour]; //find the node neighbour is mapped to
            uint neighbourOldAlignedEdges = alignedByNode[neighbour];
            uint neighbourTotalEdges = G1AdjLists[neighbour].size();
            alignedByNode[neighbour] -= G2Matrix[oldTarget][neighbourAlignedTo];
            alignedByNode[neighbour] += G2Matrix[newTarget][neighbourAlignedTo];
            //update newJsSum here
            change += ((alignedByNode[neighbour] - neighbourOldAlignedEdges)/(double)neighbourTotalEdges);
        }
    }

    return change;
}

double SANA::JSIncSwapOp(uint source1, uint source2, uint target1, uint target2) {
    // NOTE: eval swap as two sources and then loop neighbours

    double change = 0;

    uint source1OldAlingedEdges = alignedByNode[source1];
    uint source1AlignedEdges = 0;
    vector<uint> source1Neighbours = G1AdjLists[source1];
    uint source1TotalEdges = source1Neighbours.size();


    uint source2OldAlingedEdges = alignedByNode[source2];
    uint source2AlignedEdges = 0;
    vector<uint> source2Neighbours = G1AdjLists[source2];
    uint source2TotalEdges = source2Neighbours.size();

    // source 1 eval here

    // eval for source1 from sratch
    for (uint j = 0; j < source1TotalEdges; j++){
        uint neighbour = source1Neighbours[j];
        uint neighbourAlignedTo = (*A)[neighbour]; //find the node neighbour is mapped to
        // if(G2Matrix[target2][neighbourAlignedTo] == true){
        //     source1AlignedEdges += 1;
        // }
        source1AlignedEdges = G2Matrix[target2][neighbourAlignedTo];
    }
    alignedByNode[source1] = source1AlignedEdges;
    //update change here
    change += ((source1AlignedEdges - source1OldAlingedEdges)/(double)source1TotalEdges);


    // for each source neighbour update do iterative changes to the jsAlingedByNode vector
    // in each update get the G2mapping of neighbour and then check if edge was aligned by oldTarget to G2mapping and reduce score if newTarget to G2mapping doesnt exist
    // increase score if oldTarget to G2 mapping edge didnt exist but newTarget to G2 mapping does
    //no changes other wise
    for (uint j = 0; j < source1TotalEdges; j++){
        uint neighbour = source1Neighbours[j];
        uint neighbourAlignedTo = (*A)[neighbour]; //find the node neighbour is mapped to
        uint neighbourOldAlignedEdges = alignedByNode[neighbour];
        uint neighbourTotalEdges = G1AdjLists[neighbour].size();
        if (std::find (source1Neighbours.begin(), source1Neighbours.end(), neighbour) == source1Neighbours.end()){
            alignedByNode[neighbour] -= G2Matrix[target1][neighbourAlignedTo];
            alignedByNode[neighbour] += G2Matrix[target2][neighbourAlignedTo];
            //update newJsSum here
            change += ((alignedByNode[neighbour] - neighbourOldAlignedEdges)/(double)neighbourTotalEdges);
        }
    }

    // source 2 eval here

    // eval for source2 from sratch
    for (uint j = 0; j < source2TotalEdges; j++){
        uint neighbour = source2Neighbours[j];
        uint neighbourAlignedTo = (*A)[neighbour]; //find the node neighbour is mapped to
        source2AlignedEdges += G2Matrix[target1][neighbourAlignedTo];
    }
    alignedByNode[source2] = source2AlignedEdges;
    //update change here
    change += ((source2AlignedEdges - source2OldAlingedEdges)/(double)source2TotalEdges);


    // for each source neighbour update do iterative changes to the jsAlingedByNode vector
    // in each update get the G2mapping of neighbour and then check if edge was aligned by oldTarget to G2mapping and reduce score if newTarget to G2mapping doesnt exist
    // increase score if oldTarget to G2 mapping edge didnt exist but newTarget to G2 mapping does
    //no changes other wise
    for (uint j = 0; j < source2TotalEdges; j++){
        uint neighbour = source2Neighbours[j];
        uint neighbourAlignedTo = (*A)[neighbour]; //find the node neighbour is mapped to
        uint neighbourOldAlignedEdges = alignedByNode[neighbour];
        uint neighbourTotalEdges = G1AdjLists[neighbour].size();
        if (std::find (source1Neighbours.begin(), source1Neighbours.end(), neighbour) == source1Neighbours.end()){
            alignedByNode[neighbour] -= G2Matrix[target2][neighbourAlignedTo];
            alignedByNode[neighbour] += G2Matrix[target1][neighbourAlignedTo];
            //update newJsSum here
            change += ((alignedByNode[neighbour] - neighbourOldAlignedEdges)/(double)neighbourTotalEdges);
        }
    }

    //eval for common neighbours here
    vector<uint> source1source2commonneighbours(source1Neighbours.size() + source2Neighbours.size());

    vector<uint>::iterator it, end;

    end = set_intersection(
        source1Neighbours.begin(), source1Neighbours.end(),
        source2Neighbours.begin(), source2Neighbours.end(),
        source1source2commonneighbours.begin());

    return change;

}

double SANA::WECIncChangeOp(uint source, uint oldTarget, uint newTarget) {
    double res = 0;
    const uint n = G1AdjLists[source].size();
    uint neighbor;
    for (uint j = 0; j < n; ++j) {
        neighbor = G1AdjLists[source][j];
        if (G2Matrix[oldTarget][(*A)[neighbor]]) {
            res -= wecSims[source][oldTarget];
            res -= wecSims[neighbor][(*A)[neighbor]];
        }
        if (G2Matrix[newTarget][(*A)[neighbor]]) {
            res += wecSims[source][newTarget];
            res += wecSims[neighbor][(*A)[neighbor]];
        }
    }
    return res;
}

double SANA::WECIncSwapOp(uint source1, uint source2, uint target1, uint target2) {
    double res = 0;
    const uint n = G1AdjLists[source1].size();
    uint neighbor;
    for (uint j = 0; j < n; ++j) {
        neighbor = G1AdjLists[source1][j];
        if (G2Matrix[target1][(*A)[neighbor]]) {
            res -= wecSims[source1][target1];
            res -= wecSims[neighbor][(*A)[neighbor]];
        }
        if (G2Matrix[target2][(*A)[neighbor]]) {
            res += wecSims[source1][target2];
            res += wecSims[neighbor][(*A)[neighbor]];
        }
    }
    const uint m = G1AdjLists[source2].size();
    for (uint j = 0; j < m; ++j) {
        neighbor = G1AdjLists[source2][j];
        if (G2Matrix[target2][(*A)[neighbor]]) {
            res -= wecSims[source2][target2];
            res -= wecSims[neighbor][(*A)[neighbor]];
        }
        if (G2Matrix[target1][(*A)[neighbor]]) {
            res += wecSims[source2][target1];
            res += wecSims[neighbor][(*A)[neighbor]];
        }
    }
    //address case swapping between adjacent nodes with adjacent images:
#ifdef MULTI_PAIRWISE
    if (G1Matrix[source1][source2] > 0 and G2Matrix[target1][target2] > 0) {
#else
    if (G1Matrix[source1][source2] and G2Matrix[target1][target2]) {
#endif
        res += 2*wecSims[source1][target1];
        res += 2*wecSims[source2][target2];
    }
    return res;
}

double SANA::EWECIncChangeOp(uint source, uint oldTarget, uint newTarget){
    double score = 0;
    score = (EWECSimCombo(source, newTarget)) - (EWECSimCombo(source, oldTarget));
    return score;
}

double SANA::EWECIncSwapOp(uint source1, uint source2, uint target1, uint target2){
    double score = 0;
    score = (EWECSimCombo(source1, target2)) + (EWECSimCombo(source2, target1)) - (EWECSimCombo(source1, target1)) - (EWECSimCombo(source2, target2));
    if(G1Matrix[source1][source2] and G2Matrix[target1][target2]){
        score += ewec->getScore(ewec->getColIndex(target1, target2), ewec->getRowIndex(source1, source2))/(g1Edges); //correcting for missed edges when swapping 2 adjacent pairs
    }
    return score;
}

double SANA::EWECSimCombo(uint source, uint target){
    double score = 0;
    const uint n = G1AdjLists[source].size();
    uint neighbor;
    for (uint i = 0; i < n; ++i) {
        neighbor = G1AdjLists[source][i];
        if (G2Matrix[target][(*A)[neighbor]]) {
            int e1 = ewec->getRowIndex(source, neighbor);
            int e2 = ewec->getColIndex(target, (*A)[neighbor]);
            score+=ewec->getScore(e2,e1);
        }
    }
    return score/(2*g1Edges);
}

double SANA::TCIncChangeOp(uint source, uint oldTarget, uint newTarget){
    double deltaTriangles = 0;
    const uint n = G1AdjLists[source].size();
    uint neighbor1, neighbor2;
    for(uint i = 0; i < n; ++i){
        for(uint j = i+1; j < n; ++j){
            neighbor1 = G1AdjLists[source][i];
            neighbor2 = G1AdjLists[source][j];
            if(G1Matrix[neighbor1][neighbor2]){
                //G1 has a triangle
                if(G2Matrix[oldTarget][(*A)[neighbor1]] and G2Matrix[oldTarget][(*A)[neighbor2]] and G2Matrix[(*A)[neighbor1]][(*A)[neighbor2]]){
                    //G2 HAD a triangle
                    deltaTriangles -= 1;
                }

                if(G2Matrix[newTarget][(*A)[neighbor1]] and G2Matrix[newTarget][(*A)[neighbor2]] and G2Matrix[(*A)[neighbor1]][(*A)[neighbor2]]){
                    //G2 GAINS a triangle
                    deltaTriangles += 1;
                }
            }
        }
    }
    return ((double)deltaTriangles/maxTriangles);
}

double SANA::TCIncSwapOp(uint source1, uint source2, uint target1, uint target2){
    double deltaTriangles = 0;
    const uint n = G1AdjLists[source1].size();
    uint neighbor1, neighbor2;
    for(uint i = 0; i < n; ++i){
        for(uint j = i+1; j < n; ++j){
            neighbor1 = G1AdjLists[source1][i];
            neighbor2 = G1AdjLists[source1][j];
            if(G1Matrix[neighbor1][neighbor2]){
                //G1 has a triangle
                if(G2Matrix[target1][(*A)[neighbor1]] and G2Matrix[target1][(*A)[neighbor2]] and G2Matrix[(*A)[neighbor1]][(*A)[neighbor2]]){
                    //G2 HAD a triangle
                    deltaTriangles -= 1;
                }

                if((G2Matrix[target2][(*A)[neighbor1]] and G2Matrix[target2][(*A)[neighbor2]] and G2Matrix[(*A)[neighbor1]][(*A)[neighbor2]])
                || (neighbor1 == source2 and G2Matrix[target2][target1] and G2Matrix[target2][(*A)[neighbor2]] and G2Matrix[target1][(*A)[neighbor2]])
                || (neighbor2 == source2 and G2Matrix[target2][(*A)[neighbor1]] and G2Matrix[target2][target1] and G2Matrix[(*A)[neighbor1]][target1])) {
                    //G2 GAINS a triangle
                    deltaTriangles += 1;
                }
            }
        }
    }
    const uint m = G1AdjLists[source2].size();
    for(uint i = 0; i < m; ++i){
        for(uint j = i+1; j < m; ++j){
            neighbor1 = G1AdjLists[source2][i];
            neighbor2 = G1AdjLists[source2][j];
            if(G1Matrix[neighbor1][neighbor2]){
                //G1 has a triangle
                if(G2Matrix[target2][(*A)[neighbor1]] and G2Matrix[target2][(*A)[neighbor2]] and G2Matrix[(*A)[neighbor1]][(*A)[neighbor2]]){
                    //G2 HAD a triangle
                    deltaTriangles -= 1;
                }

                if((G2Matrix[target1][(*A)[neighbor1]] and G2Matrix[target1][(*A)[neighbor2]] and G2Matrix[(*A)[neighbor1]][(*A)[neighbor2]])
                   || (neighbor1 == source1 and G2Matrix[target1][target2] and G2Matrix[target1][(*A)[neighbor2]] and G2Matrix[target2][(*A)[neighbor2]])
                   || (neighbor2 == source1 and G2Matrix[target1][(*A)[neighbor1]] and G2Matrix[target1][target2] and G2Matrix[(*A)[neighbor1]][target2])){
                    //G2 GAINS a triangle
                    deltaTriangles += 1;
                }
            }
        }
    }

    return ((double)deltaTriangles/maxTriangles);
}

int SANA::ncIncChangeOp(uint source, uint oldTarget, uint newTarget) {
    int change = 0;
    if (trueA[source] == oldTarget) change -= 1;
    if (trueA[source] == newTarget) change += 1;
    return change;
}

int SANA::ncIncSwapOp(uint source1, uint source2, uint target1, uint target2) {
    int change = 0;
    if(trueA[source1] == target1) change -= 1;
    if(trueA[source2] == target2) change -= 1;
    if(trueA[source1] == target2) change += 1;
    if(trueA[source2] == target1) change += 1;
    return change;
}

void SANA::trackProgress(long long int i, bool end) {
    if (!enableTrackProgress) return;
    bool printDetails = false;
    bool printScores = false;
    bool checkScores = true;
    double fractional_time = i/(double)_maxExecutionIterations;
    double elapsedTime = timer.elapsed();
    uint iterationsElapsed = iterationsPerformed-oldIterationsPerformed;
    if(elapsedTime == 0) oldTimeElapsed = 0;
    double ips = (iterationsElapsed/(elapsedTime-oldTimeElapsed));
    oldTimeElapsed = elapsedTime;
    oldIterationsPerformed = iterationsPerformed;
    cout << i/iterationsPerStep << " (" << 100*fractional_time << "%," << elapsedTime << "s): score = " << currentScore;
    cout <<  " ips = " << ips << ", P(" << Temperature << ") = " << acceptingProbability(avgEnergyInc, Temperature);
    cout << ", pBad = " << trueAcceptingProbability() << endl;
    
    if (not (printDetails or printScores or checkScores)) return;
    Alignment Al(*A);
    //original one is commented out for testing sec
    //if (printDetails) cout << " (" << Al.numAlignedEdges(*G1, *G2) << ", " << G2->numNodeInducedSubgraphEdges(*A) << ")";
    if (printDetails)
        cout << "Al.numAlignedEdges = " << Al.numAlignedEdges(*G1, *G2) << ", g1Edges = " <<g1Edges<< " ,g2Edges = "<<g2Edges<< endl;
    if (printScores) {
        SymmetricSubstructureScore S3(G1, G2);
        EdgeCorrectness EC(G1, G2);
        InducedConservedStructure ICS(G1, G2);
        SymmetricEdgeCoverage SEC(G1,G2);
        cout << "S3: " << S3.eval(Al) << "  EC: " << EC.eval(Al) << "  ICS: " << ICS.eval(Al) << "  SEC: " << SEC.eval(Al) <<endl;
    }
    if (checkScores) {
	//cout <<"alalalalalalallaala:"<< MultiS3::denom << endl;
        double realScore = eval(Al);
        if (fabs(realScore-currentScore) > 0.00001) {
            cerr << "internal error: incrementally computed score (" << currentScore;
            cerr << ") is not correct (" << realScore << ")" << endl;
            currentScore = realScore;
        }
    }
    if (dynamic_tdecay) { // Code for estimating dynamic TDecay
        //The dynamic method uses linear interpolation to obtain an
        //an "ideal" P(bad) as a basis for SANA runs. If the current P(bad)
        //is significantly different from out "ideal" P(bad), then decay is either
        //"sped up" or "slowed down"
        int NSteps = 100;
        double fractional_time = (timer.elapsed()/(minutes*60));
        double lowIndex = floor(NSteps*fractional_time);
        double highIndex = ceil(NSteps*fractional_time);
        double betweenFraction = NSteps*fractional_time - lowIndex;
        double PLow = tau[lowIndex];
        double PHigh = tau[highIndex];

        double PBetween = PLow + betweenFraction * (PHigh - PLow);

        // if the ratio if off by more than a few percent, adjust.
        double ratio = acceptingProbability(avgEnergyInc, Temperature) / PBetween;
        if (abs(1-ratio) >= .01 &&
            (ratio < 1 || SANAtime > .2)) // don't speed it up too soon
        {
            double shouldBe;
            shouldBe = -log(avgEnergyInc/(TInitial*log(PBetween)))/(SANAtime);
            if(SANAtime==0 || shouldBe != shouldBe || shouldBe <= 0)
            shouldBe = TDecay * (ratio >= 0 ? ratio*ratio : 0.5);
            cout << "TDecay " << TDecay << " too ";
            cout << (ratio < 1 ? "fast" : "slow") << " shouldBe " << shouldBe;
            TDecay = sqrt(TDecay * shouldBe); // geometric mean
            cout << "; try " << TDecay << endl;
        }
    }
}

Alignment SANA::runRestartPhases() {
    cout << "new alignments phase" << endl;
    Timer TImer;
    TImer.start();
    newAlignmentsCount = 0;
    while (TImer.elapsed() < minutesNewAlignments*60) {
        long long int iter = 0;
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
    cout << "candidates phase" << endl;
    vector<long long int> iters(numCandidates, iterationsPerStep);
    for (uint i = 0; i < numCandidates; ++i) {
        candidates[i] = simpleRun(candidates[i], minutesPerCandidate*60, iters[i]);
        candidatesScores[i] = currentScore;
    }
    cout << "finalist phase" << endl;
    uint i = getHighestIndex();
    return simpleRun(candidates[i], minutesFinalist*60, iters[i]);
}

uint SANA::getLowestIndex() const {
    double lowestScore = candidatesScores[0];
    uint lowestIndex = 0;
    for (uint i = 1; i < numCandidates; ++i) {
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
    for (uint i = 1; i < numCandidates; ++i) {
        if (candidatesScores[i] > highestScore) {
            highestScore = candidatesScores[i];
            highestIndex = i;
        }
    }
    return highestIndex;
}



/*
*******************************************
*******************************************
***** Temperature schedule functions ****** 
*******************************************
*******************************************
*/

/* when we run sana at a fixed temp, scores generally go up
(especially if the temp is low) until a point of "thermal equilibrium".
This function should return the avg pBad at equilibrium.
we keep track of the score every certain number of iterations
if the score went down at least half the time,
this suggests that the upward trend is over and we are at equilirbium
once we know we are at equilibrium, we use the buffer of pbads to get an average pBad
'logLevel' can be 0 (no output) 1 (logs result in cerr) or 2 (verbose/debug mode)*/
double SANA::getPBad(double temp, double maxTime, int logLevel) {
    
    //new state for the run at fixed temperature
    constantTemp = true;
    Temperature = temp;
    enableTrackProgress = false;
    
    //note: this is a circular buffer that maintains scores sampled at intervals
    vector<double> scoreBuffer;
    //the larger 'numScores' is, the stronger evidence of reachign equilibrium. keep this value odd
    const uint numScores = 11;
    uint iter = 0;
    uint sampleInterval = 10000;

    bool reachedEquilibrium = false;

    initDataStructures(getStartingAlignment()); //this initializes the timer and resets the pBad buffer

    bool verbose = (logLevel == 2); //print everything going on, for debugging purposes
    uint verbose_i = 0;
    if (verbose) {
        cerr << endl << "****************************************" << endl;
        cerr << "starting search for pBad for temp = " << temp << endl;
    }

    while (not reachedEquilibrium) {
        SANAIteration();
        iter++;

        if (iter%sampleInterval == 0) {
            if (verbose) {
                cerr << verbose_i << " score: " << currentScore << " (avg pBad: " << slowTrueAcceptingProbability() << ")" << endl;
                verbose_i++;
            }
            //circular buffer behavior
            //(since the buffer is tiny, the cost of shifting everything is negligible)
            scoreBuffer.push_back(currentScore);            
            if (scoreBuffer.size() > numScores) {
                scoreBuffer.erase(scoreBuffer.begin());
            }

            if (scoreBuffer.size() == numScores) {
                //check if we are at eq:
                //if the score went down more than up, it suggests we are at eq
                uint scoreTrend = 0;
                for (uint i = 0; i < numScores-1; i++) {
                    if (scoreBuffer[i+1] < scoreBuffer[i]) scoreTrend--;
                    if (scoreBuffer[i+1] > scoreBuffer[i]) scoreTrend++;
                }
                reachedEquilibrium = (scoreTrend <= 0);

                if (verbose) {
                    cerr << "scoreTrend = " << scoreTrend << endl;
                    if (reachedEquilibrium) {
                        cerr << endl << "Reached equilibrium" << endl;
                        cerr << "scoreBuffer:" << endl;
                        for (uint i = 0; i < scoreBuffer.size(); i++) {
                            cerr << scoreBuffer[i] << " ";
                        }
                        cerr << endl;
                    }
                }
            }

            if (timer.elapsed() > maxTime) {
                if (verbose) {
                    cerr << "ran out of time. scoreBuffer:" << endl;
                    for (uint i = 0; i < scoreBuffer.size(); i++) {
                        cerr<<scoreBuffer[i]<<endl;
                    }
                    cerr<<endl;
                }
                break;
            }
        }
    }

    double pBadAvgAtEq = slowTrueAcceptingProbability();
    double nextIps = (double)iter / (double)timer.elapsed();
    pair<double, double> nextPair (temp, nextIps);
    ipsList.push_back(nextPair);
    if (logLevel >= 1) {
        cout << "> getPBad(" << temp << ") = " << pBadAvgAtEq << " (score: " << currentScore << ")";
        if (reachedEquilibrium) cout << " (time: " << timer.elapsed() << "s)";
        else cout << " (didn't detect eq. after " << maxTime << "s)";
	cout << " iterations = " << iter << ", ips = " << nextIps;
        cout << endl;
        
        if (verbose) {
            cerr << "final result: " << pBadAvgAtEq << endl;
            cerr << "****************************************" << endl << endl;
        }
    }
    //restore normal execution state
    constantTemp = false;
    enableTrackProgress = true;
    Temperature = TInitial;

    return pBadAvgAtEq;
}

void SANA::setTInitial(double t) {
    TInitial = t;
}

void SANA::setTFinal(double t) {
    TFinal = t;
}



string SANA::getFolder(){
    //create (if neccessary) and return the path of the measure combinations respcetive cache folder
#if USE_CACHED_FILES
// By default, USE_CACHED_FILES is 0 and SANA does not cache files. Change USE_CACHED_FILES at your own risk.
    stringstream ss;
    ss << "mkdir -p " << "cache-pbad" << "/" << MC->toString() << "/";
    (void)system(ss.str().c_str());
#endif
    stringstream sf;
    sf << "cache-pbad" << "/" << MC->toString() << "/";
    return sf.str();
}

string SANA::mkdir(const std::string& file){
    //create (if neccessary) and return the path of a path folder in the cache
#if USE_CACHED_FILES
// By default, USE_CACHED_FILES is 0 and SANA does not cache files. Change USE_CACHED_FILES at your own risk.
    stringstream ss;
    ss << "mkdir -p " << getFolder() << file << "/";
    (void)system(ss.str().c_str());
#endif
    stringstream sf;
    sf << getFolder() << file << "/";
    return sf.str();
}

double SANA::logOfSearchSpaceSize() {
    //the search space size is (n2 choose n1) * n1!
    //we use the stirling approximation
    if (n2 == n1) return n1*log(n1)-n1;
    return n2*log(n2)-(n2-n1)*log(n2-n1)-n1;
}

Alignment SANA::hillClimbingAlignment(Alignment startAlignment, long long int idleCountTarget){
    long long int iter = 0;
    uint idleCount = 0;
    Temperature = 0;
    initDataStructures(startAlignment); //this is redundant, but it's not that big of a deal.  Resets true probability.
    cout << "Beginning Final Pure Hill Climbing Stage" << endl;
    while(idleCount < idleCountTarget){
        if (iter%iterationsPerStep == 0) {
            trackProgress(iter);
        }
        double oldScore = currentScore;
        SANAIteration();
        if(abs(oldScore-currentScore) < 0.00001){
            ++idleCount;
        }else{
            idleCount = 0;
        }
        ++iter;
    }
    trackProgress(iter);
    return *A;
}

Alignment SANA::hillClimbingAlignment(long long int idleCountTarget){
    return hillClimbingAlignment(getStartingAlignment(), idleCountTarget);
}

void SANA::constantTempIterations(long long int iterTarget) {
    Alignment startA = getStartingAlignment();
    long long int iter = 1;
    initDataStructures(startA);
    for (; iter < iterTarget ; ++iter) {
        if (iter%iterationsPerStep == 0) {
            trackProgress(iter);
        }
        SANAIteration();
    }
    trackProgress(iter);
}

void SANA::setTDecayFromTempRange() {
    TDecay = -log(TFinal/TInitial);
}


double SANA::getIterPerSecond() {
    if (not initializedIterPerSecond) {
        initIterPerSecond();
    }
    return iterPerSecond;
}

void SANA::initIterPerSecond() {
    initializedIterPerSecond = true;

    cout << "Determining iteration speed...." << endl;
    double totalIps = 0.0;
    int ipsListSize = 0;
    if(ipsList.size()!=0){
        for(pair<double,double> ipsPair : ipsList){
	    if(TFinal <= ipsPair.first && ipsPair.first <= TInitial){
		totalIps+=ipsPair.second;
		ipsListSize+=1;
	    }
        }
	totalIps = totalIps / (double) ipsListSize;
    }else{
	cout << "Since temperature schedule is provided, ips will be calculated using constantTempIterations" << endl;
        long long int iter = 1E6;
        constantTempIterations(iter - 1);
        double res = iter/timer.elapsed();
        totalIps = res;
    }
    cout << "SANA does " << long(totalIps) << " iterations per second on average" << endl;

    iterPerSecond = totalIps;
    std::ostringstream ss;
    ss << "progress_" << std::fixed << std::setprecision(0) << minutes;
    ofstream header(mkdir(ss.str()) + G1->getName() + "_" + G2->getName() + "_" + std::to_string(0) + ".csv");
    header << "time,score,avgEnergyInc,Temperature,realTemp,pBad,lower,higher,timer" << endl;
    header.close();
}

void SANA::setDynamicTDecay() {
    dynamic_tdecay = true;
}

#ifdef MULTI_PAIRWISE
void SANA::prune(string& startAligName) {
    std::cerr << "Starting to prune using " <<  startAligName << std::endl;
    uint n = G1->getNumNodes();
    vector<uint> alignment;

    stringstream errorMsg;
    string format = startAligName.substr(startAligName.size()-6);
    assert(format == ".align"); // currently only 2-column format is supported


    Alignment align =  Alignment::loadEdgeList(G1, G2, startAligName);
#ifdef REINDEX
    if(G1->isBipartite())
        align.reIndexBefore_Iterations(G1->getBipartiteNodeTypes_ReIndexMap());
    else if (lockFileName != "")
        align.reIndexBefore_Iterations(G1->getLocking_ReIndexMap());

#endif


    unordered_map<uint, uint> reIndexedMap;
    // This is when no reIndexing is done, just to simplify the code
    for(uint i=0;i<n;i++)
        reIndexedMap[i] = i;

#ifdef REINDEX
    if(G1->isBipartite())
        reIndexedMap = G1->getBipartiteNodeTypes_ReIndexMap();
    else if (lockFileName != "")
        reIndexedMap = G1->getLocking_ReIndexMap();
#endif

    alignment = align.getMapping();

    std::cerr << alignment.size() << " " << n << std::endl;
    if ((uint)alignment.size() != n) {
        errorMsg << "Alignment size (" << alignment.size() << ") less than number of nodes (" << n <<")";
        throw runtime_error(errorMsg.str().c_str());
    }
    set<pair<uint,uint>> removedEdges;
    for (uint i = 0; i < n; ++i) {
        uint g1_node1 = reIndexedMap[i];
        uint shadow_node = alignment[g1_node1];
        uint m = G1AdjLists[i].size();
        for (uint j = 0; j < m; ++j) {
            if (G1AdjLists[i][j] < i)
                continue;
            uint g1_node2 = reIndexedMap[G1AdjLists[i][j]];
            uint shadow_end = alignment[g1_node2];

            assert(G1Matrix[g1_node1][g1_node2] == 0 || G2Matrix[shadow_node][shadow_end] > 0);
            assert(G1Matrix[g1_node2][g1_node1] == 0 || G2Matrix[shadow_end][shadow_node] > 0);

            G2Matrix[shadow_node][shadow_end] -= G1Matrix[g1_node1][g1_node2];
            G2Matrix[shadow_end][shadow_node] -= G1Matrix[g1_node1][g1_node2];
            if (G2Matrix[shadow_node][shadow_end] == 0) {
                removedEdges.insert(pair<uint,uint>(shadow_node,shadow_end));
            }
        }
    }
    vector<vector<uint> > t_edgeList;
    vector<vector<uint> > G2EdgeList;
    G2->getEdgeList(G2EdgeList);
    for (auto c : G2EdgeList) {
        if (removedEdges.find(pair<uint,uint>(c[0],c[1])) != removedEdges.end() or
                removedEdges.find(pair<uint,uint>(c[1],c[0])) != removedEdges.end()) {
            continue;
        }
        t_edgeList.push_back(c);
    }
    G2->setMatrix(G2Matrix);
    G2->getAdjLists(G2AdjLists);
    G2->setEdgeList(t_edgeList);
}
#endif


// Code related with parallel pareto run, these code will be later refactored along with the
// rest of SANA's code.

void SANA::performChange(Job &job, int type) {
    AlignmentInfo &info = job.info;
    vector<uint> *A = info.A;

    uint source = G1RandomUnlockedNode(job);
    uint oldTarget    = (*A)[source];
    uint newTargetIndex = G2RandomUnlockedNode(job, oldTarget);

    uint newTarget = -1;
    bool isGene = false;
    if(multipartite == 1)
        newTarget    = (*info.unassignedNodesG2)[newTargetIndex];
    else{
        isGene = G2->nodeTypes[oldTarget] == Graph::NODE_TYPE_GENE;
        if(isGene){
            if(info.unassignedgenesG2->size() == 0)
                return; // cannot perform change, all genes are assigned
            newTarget = (*info.unassignedgenesG2)[newTargetIndex];
        }
        else{
            if(info.unassignedmiRNAsG2->size() == 0)
                return; // cannot perform change, all miRNA are assigned
            newTarget = (*info.unassignedmiRNAsG2)[newTargetIndex];
        }
    }

    int newAligEdges           = (needAligEdges or needSec) ?  info.aligEdges + aligEdgesIncChangeOp(job, source, oldTarget, newTarget) : -1;
    double newSquaredAligEdges = (needSquaredAligEdges) ?  info.squaredAligEdges + squaredAligEdgesIncChangeOp(job, source, oldTarget, newTarget) : -1;
    double newExposedEdgesNumer= (needExposedEdges) ?  info.exposedEdgesNumer + exposedEdgesIncChangeOp(job, source, oldTarget, newTarget) : -1;
    double newMS3Numer		= (needMS3) ?  info.MS3Numer + MS3IncChangeOp(job, source, oldTarget, newTarget) : -1;
    int newInducedEdges        = (needInducedEdges) ?  info.inducedEdges + inducedEdgesIncChangeOp(job, source, oldTarget, newTarget) : -1;
    double newTCSum            = (needTC) ?  info.TCSum + TCIncChangeOp(job, source, oldTarget, newTarget) : -1;
    double newLocalScoreSum    = (needLocal) ? info.localScoreSum + localScoreSumIncChangeOp(job, sims, source, oldTarget, newTarget) : -1;
    double newWecSum           = (needWec) ?  info.wecSum + WECIncChangeOp(job, source, oldTarget, newTarget) : -1;
    double newJsSum            = (needJs)  ? info.jsSum + JSIncChangeOp(job, source, oldTarget, newTarget) : -1;
    double newEwecSum          = (needEwec) ?  info.ewecSum + EWECIncChangeOp(job, source, oldTarget, newTarget) : -1;
    double newNcSum            = (needNC) ? info.ncSum + ncIncChangeOp(job, source, oldTarget, newTarget) : -1;
    double newEdSum            = (needEd) ? info.edSum + edgeDifferenceIncChangeOp(job, source, oldTarget, newTarget) : -1;
    double newErSum            = (needEr) ? info.erSum + edgeRatioIncChangeOp(job, source, oldTarget, newTarget) : -1;

    map<string, double> newLocalScoreSumMap;
    if (needLocal) {
        newLocalScoreSumMap = map<string, double>(*info.localScoreSumMap);
        for(auto &item : newLocalScoreSumMap)
            item.second += localScoreSumIncChangeOp(localSimMatrixMap[item.first], source, oldTarget, newTarget);
    }

    double newCurrentScore = 0;
    bool makeChange = scoreComparison(job, newAligEdges, newInducedEdges, newTCSum, newLocalScoreSum, newWecSum, newJsSum, newNcSum, newCurrentScore, newEwecSum, newSquaredAligEdges, newExposedEdgesNumer, newMS3Numer, newEdSum, newErSum);

#ifdef CORES
	// Statistics on the emerging core alignment.
	// only update pBad if it's nonzero; re-use previous nonzero pBad if the current one is zero.
	uint betterHole = wasBadMove ? oldTarget : newTarget;
	double pBad = trueAcceptingProbability();
        if(pBad <= 0 || myNan(pBad)) pBad = LOW_PBAD_LIMIT;
#if UNWEIGHTED_CORES
	numPegSamples[source]++;
	pegHoleFreq[source][betterHole]++;
#endif
	totalWeightedPegWeight_pBad[source] += pBad;
	weightedPegHoleFreq_pBad[source][betterHole] += pBad;
	totalWeightedPegWeight_1mpBad[source] += 1-pBad;
	weightedPegHoleFreq_1mpBad[source][betterHole] += 1-pBad;
#endif

    if (makeChange) {

       (*A)[source]                         = newTarget;

       if(multipartite == 1)
           (*info.unassignedNodesG2)[newTargetIndex] = oldTarget;
       else {
           if(isGene){
               (*info.unassignedgenesG2)[newTargetIndex] = oldTarget;
           }
           else {
               (*info.unassignedmiRNAsG2)[newTargetIndex] = oldTarget;
           }
       }

       (*info.assignedNodesG2)[oldTarget]        = false;
       (*info.assignedNodesG2)[newTarget]        = true;
       info.aligEdges                            = newAligEdges;
       info.inducedEdges                         = newInducedEdges;
       info.TCSum                                = newTCSum;
       info.localScoreSum                        = newLocalScoreSum;
       info.wecSum                               = newWecSum;
       info.jsSum                                = newJsSum;
       info.ewecSum                              = newEwecSum;
       info.ncSum                                = newNcSum;
       info.edSum                                = newEdSum;
       info.erSum                                = newErSum;
       info.currentScore = newCurrentScore;
       info.squaredAligEdges = newSquaredAligEdges;
       info.exposedEdgesNumer = newExposedEdgesNumer;

       if (needLocal) {
           (*info.localScoreSumMap) = newLocalScoreSumMap;
       }
#if 0
       if (randomReal(job.gen) <= 1) {
           double foo = eval(*A);
           if (fabs(foo - newCurrentScore) > 20) {
               cout << "\nSwap: nCS " << newCurrentScore << " eval " << foo << " nCS - eval " << newCurrentScore - foo << " adj? " << (G1Matrix[source1][source2] & G2Matrix[target1][target2]);
               newCurrentScore = newSquaredAligEdges = foo;
           }
           else cout << "s";
       }
#endif
    }
}

void SANA::performSwap(Job &job, int type) {
    AlignmentInfo &info = job.info;
    vector<uint> *A = info.A;

    uint source1 = G1RandomUnlockedNode(job);
    uint source2 = G1RandomUnlockedNode(job, source1);
    uint target1 = (*A)[source1], target2 = (*A)[source2];

    int newAligEdges           = (needAligEdges or needSec) ?  info.aligEdges + aligEdgesIncSwapOp(job, source1, source2, target1, target2) : -1;
    int newTCSum               = (needTC) ?  info.TCSum + TCIncSwapOp(job, source1, source2, target1, target2) : -1;
    double newSquaredAligEdges = (needSquaredAligEdges) ? info.squaredAligEdges + squaredAligEdgesIncSwapOp(job, source1, source2, target1, target2) : -1;
    double newExposedEdgesNumer= (needExposedEdges) ? info.exposedEdgesNumer + exposedEdgesIncSwapOp(job, source1, source2, target1, target2) : -1;
    double newMS3Numer		= (needMS3) ? info.MS3Numer + MS3IncSwapOp(job, source1, source2, target1, target2) : -1;
    double newWecSum           = (needWec) ?  info.wecSum + WECIncSwapOp(job, source1, source2, target1, target2) : -1;
    double newJsSum            = (needJs)  ? info.jsSum + JSIncSwapOp(job, source1, source2, target1, target2) : -1;
    double newEwecSum          = (needEwec) ?  info.ewecSum + EWECIncSwapOp(job, source1, source2, target1, target2) : -1;
    double newNcSum            = (needNC) ? info.ncSum + ncIncSwapOp(job, source1, source2, target1, target2) : -1;
    double newLocalScoreSum    = (needLocal) ? info.localScoreSum + localScoreSumIncSwapOp(job, sims, source1, source2, target1, target2) : -1;
    double newEdSum            = (needEd) ? info.edSum + edgeDifferenceIncSwapOp(job, source1, source2, target1, target2) : -1;
    double newErSum            = (needEr) ? info.erSum + edgeRatioIncSwapOp(job, source1, source2, target1, target2) : -1;

    map<string, double> newLocalScoreSumMap;
    if (needLocal) {
        newLocalScoreSumMap = map<string, double>(*info.localScoreSumMap);
        for (auto &item : newLocalScoreSumMap)
            item.second += localScoreSumIncSwapOp(localSimMatrixMap[item.first], source1, source2, target1, target2);
    }

    double newCurrentScore = 0;
    bool makeChange = scoreComparison(job, newAligEdges, info.inducedEdges, newTCSum, newLocalScoreSum, newWecSum, newJsSum, newNcSum, newCurrentScore, newEwecSum, newSquaredAligEdges, newExposedEdgesNumer, newMS3Numer, newEdSum, newErSum);
#ifdef CORES
        // Statistics on the emerging core alignment.
        // only update pBad if it's nonzero; re-use previous nonzero pBad if the current one is zero.
        double pBad = trueAcceptingProbability();
        if(pBad <= 0 || myNan(pBad)) pBad = LOW_PBAD_LIMIT;

        uint betterDest1 = wasBadMove ? target1 : target2;
        uint betterDest2 = wasBadMove ? target2 : target1;
#if UNWEIGHTED_CORES
        numPegSamples[source1]++; numPegSamples[source2]++;
        pegHoleFreq[source1][betterDest1]++; pegHoleFreq[source2][betterDest2]++;
#endif
        totalWeightedPegWeight_pBad[source1] += pBad;
        weightedPegHoleFreq_pBad[source1][betterDest1] += pBad;
        totalWeightedPegWeight_pBad[source2] += pBad;
        weightedPegHoleFreq_pBad[source2][betterDest2] += pBad;

        totalWeightedPegWeight_1mpBad[source1] += 1-pBad;
        weightedPegHoleFreq_1mpBad[source1][betterDest1] += 1-pBad;
        totalWeightedPegWeight_1mpBad[source2] += 1-pBad;
        weightedPegHoleFreq_1mpBad[source2][betterDest2] += 1-pBad;
#endif
    if (makeChange) {
        (*A)[source1]       = target2;
        (*A)[source2]       = target1;
        info.aligEdges           = newAligEdges;
        info.localScoreSum       = newLocalScoreSum;
        info.TCSum               = newTCSum;
        info.wecSum              = newWecSum;
        info.jsSum               = newJsSum;
        info.ewecSum             = newEwecSum;
        info.ncSum               = newNcSum;
        info.currentScore        = newCurrentScore;
        info.squaredAligEdges    = newSquaredAligEdges;
        info.edSum               = newEdSum;
        info.erSum               = newErSum;
	info.exposedEdgesNumer	 = newExposedEdgesNumer;
        if (needLocal)
            (*info.localScoreSumMap) = newLocalScoreSumMap;
#if 0
        if (randomReal(job.gen) <= 1) {
            double foo = eval(*A);
            if (fabs(foo - newCurrentScore) > 20) {
                cout << "\nSwap: nCS " << newCurrentScore << " eval " << foo << " nCS - eval " << newCurrentScore - foo << " adj? " << (G1Matrix[source1][source2] & G2Matrix[target1][target2]);
                newCurrentScore = newSquaredAligEdges = foo;
            }
            else cout << "s";
        }
#endif
   }
}

void SANA::parallelParetoSANAIteration(Job &job) {
    ++job.iterationsPerformed;
    if(G1->isBipartite())
    {
	// Choose the type here based on counts (and locking...)
	// For example if no locking, then prob (gene) >> prob (miRNA)
	// And if locking, then arrange prob(gene) and prob(miRNA) appropriately
        int type = /* something clever */ 0;
        (randomReal(job.gen) < changeProbability[type]) ? performChange(job, type) : performSwap(job, type);
    }
        (randomReal(job.gen) < changeProbability[0]) ? performChange(job, 0) : performSwap(job, 0);
}


void SANA::startParallelParetoRunByIteration(Job &job, long long int maxExecutionIterations) {
    const uint iterationsPerStepForEachThread = iterationsPerStep / paretoThreads;
    while (true) {
        getJobMutex.lock();

        if (sharedIter > maxExecutionIterations) {
            getJobMutex.unlock();
            return ;
        }

        if (job.iterationsPerformed != 0) {
            attemptInsertAlignment(job);
        }

        assignRandomAlignment(job);

        if (sharedIter - iterOfLastTrackProgress > iterationsPerStep) {
            trackProgress(job);
            iterOfLastTrackProgress = sharedIter;
        }

        //uint startIter = sharedIter;

        sharedIter += iterationsPerStepForEachThread;

        getJobMutex.unlock();

        for (uint i = 0; i < iterationsPerStepForEachThread; ++i) {
            // Temperature is based on how much total iterations all threads have cumulatively performed.
            //job.Temperature = temperatureFunction(startIter + i, TInitial, TDecay);

            // Temperature is based on how much iterations each thread has yet performed.
            job.Temperature = temperatureFunction(job.iterationsPerformed, TInitial, TDecay);

            parallelParetoSANAIteration(job);
        }
    }
}

unordered_set<vector<uint>*>* SANA::parallelParetoRun(const Alignment& startA, long long int maxExecutionIterations,
                                                      const string &fileName) {
    initDataStructures(startA);
    setInterruptSignal();
    vector<double> scores;
    scoreNamesToIndexes = mapScoresToIndexes();
    paretoFront = ParetoFront(paretoCapacity, numOfMeasures, measureNames);
    assert(numOfMeasures > 1 && "Pareto mode must optimize on more than one measure");
    score = Score::pareto;
    initializeParetoFront();
    initializeJobs();


    startTime = chrono::steady_clock::now();
    vector<thread> threads;
    for (uint i = 0; i < paretoThreads; ++i) {
        threads.push_back(thread(&SANA::startParallelParetoRunByIteration, this, ref(jobs[i]), maxExecutionIterations));
    }

    for (thread &t: threads) {
        t.join();
    }

    cout << "ending iterations " << sharedIter << " " << maxExecutionIterations << endl;
    if (paretoFront.paretoPropertyViolated()) {
        cerr << "Warning: Pareto property violated, which means pareto front might not be correct!" << endl;
    }
    printParetoFront(fileName);
    deallocateParetoData();
    return storedAlignments;
}

void SANA::startParallelParetoRunByTime(Job &job, double maxExecutionSeconds) {
    const uint iterationsPerStepForEachThread = iterationsPerStep / paretoThreads;

    while (true) {
        getJobMutex.lock();

        if (getElapsedTime() / paretoThreads > maxExecutionSeconds) {
            getJobMutex.unlock();
            return ;
        }

        if (job.iterationsPerformed != 0) {
            attemptInsertAlignment(job);
        }

        assignRandomAlignment(job);

        if (sharedIter - iterOfLastTrackProgress > iterationsPerStep) {
            trackProgress(job);
            iterOfLastTrackProgress = sharedIter;
        }

        //uint startIter = sharedIter;

        sharedIter += iterationsPerStepForEachThread;

        getJobMutex.unlock();

        for (uint i = 0; i < iterationsPerStepForEachThread; ++i) {
            // Temperature is based on how much total iterations all threads have cumulatively performed.
            //job.Temperature = temperatureFunction(startIter + i, TInitial, TDecay);

            // Temperature is based on how much iterations each thread has yet performed.
            job.Temperature = temperatureFunction(job.iterationsPerformed, TInitial, TDecay);

            parallelParetoSANAIteration(job);
        }
    }
}

unordered_set<vector<uint>*>* SANA::parallelParetoRun(const Alignment& startA, double maxExecutionSeconds,
                                                    const string &fileName) {
    initDataStructures(startA);
    setInterruptSignal();
    vector<double> scores;
    scoreNamesToIndexes = mapScoresToIndexes();
    paretoFront = ParetoFront(paretoCapacity, numOfMeasures, measureNames);
    assert(numOfMeasures > 1 && "Pareto mode must optimize on more than one measure");
    score = Score::pareto;
    initializeParetoFront();
    initializeJobs();

    startTime = chrono::steady_clock::now();
    vector<thread> threads;
    for (uint i = 0; i < paretoThreads; ++i) {
        threads.push_back(thread(&SANA::startParallelParetoRunByTime, this, ref(jobs[i]), maxExecutionSeconds));
    }

    for (thread &t: threads) {
        t.join();
    }

    cout << "ending seconds " << getElapsedTime() << " " << maxExecutionSeconds << endl;
    if (paretoFront.paretoPropertyViolated()) {
        cerr << "Warning: Pareto property violated, which means pareto front might not be correct!" << endl;
    }
    printParetoFront(fileName);
    deallocateParetoData();
    return storedAlignments;
}

void SANA::initializeJobs() {
    jobs = vector<Job>(paretoThreads);
    for (uint i = 0; i < paretoThreads; ++i) {
        jobs[i].id = i;
        jobs[i].gen = mt19937(getRandomSeed());
        jobs[i].iterationsPerformed = 0;
    }
}

void SANA::releaseAlignment(Job &job) {
    AlignmentInfo &info = job.info;
    delete info.A;
    delete info.assignedNodesG2;
    delete info.unassignedNodesG2;
    delete info.unassignedmiRNAsG2;
    delete info.unassignedgenesG2;
    if (needLocal)
        delete info.localScoreSumMap;
}

double SANA::trueAcceptingProbability(Job &job) {
    throw runtime_error(
        "internal error: vectorMean too expensive; implement circular buffer for Pareto mode trueAcceptingProbability");
    return vectorMean(job.sampledProbability);
}

void SANA::attemptInsertAlignment(Job &job) {
    vector<uint> *A = job.info.A;
    vector<double> addScores = translateScoresToVector(job);
    bool inserted = false;
    vector<vector<uint>*> toRemove = paretoFront.addAlignmentScores(A, addScores, inserted);
    if (inserted) {
        for (unsigned int i = 0; i < toRemove.size(); i++)
            removeAlignmentData(toRemove[i]);
        storeAlignment(job);
    } else {
        releaseAlignment(job);
    }

    assert(paretoFront.size() == storedAlignments->size() and "Number of elements in paretoFront and storedAlignments don't match.");
}

void SANA::storeAlignment(Job &job) {
    AlignmentInfo &info = job.info;
    vector<uint> *A = info.A;

    storedAlignments->insert(A);

    storedAssignedNodesG2[A]        = info.assignedNodesG2;
    if(multipartite == 1)
        storedUnassignedNodesG2[A]  = info.unassignedNodesG2;
    else {
        storedUnassignedmiRNAsG2[A] = info.unassignedmiRNAsG2;
        storedUnassignedgenesG2[A]  = info.unassignedgenesG2;
    }

    if(needAligEdges or needSec) storedAligEdges[A]        = info.aligEdges;
    if(needSquaredAligEdges)     storedSquaredAligEdges[A] = info.squaredAligEdges;
    if(needExposedEdges)	 storedExposedEdgesNumer[A]= info.exposedEdgesNumer;
    if(needMS3)		         storedMS3Numer[A]         = info.MS3Numer;
    if(needInducedEdges)         storedInducedEdges[A]     = info.inducedEdges;
    if(needTC)                   storedTCSum[A]            = info.TCSum;
    if(needEd)                   storedEdSum[A]            = info.edSum;
    if(needEr)                   storedErSum[A]            = info.erSum;
    if(needLocal)                storedLocalScoreSum[A]    = info.localScoreSum;
    if(needLocal)                storedLocalScoreSumMap[A] = info.localScoreSumMap;
    if(needWec)                  storedWecSum[A]           = info.wecSum;
    if(needJs)                   storedJsSum[A]            = info.jsSum;
    // if(needAlignedByNode)        storedAlignedByNode[A]    = info.alignedByNode;
    if(needEwec)                 storedEwecSum[A]          = info.ewecSum;
    if(needNC)                   storedNcSum[A]            = info.ncSum;
    /*------------------------>*/storedCurrentScore[A]     = info.currentScore;
}

void SANA::copyAlignmentFromStorage(Job &job, vector<uint> *A) {
    AlignmentInfo &info = job.info;
    assert(storedAlignments->find(A) != storedAlignments->end() and "Alignment does not exist in the Pareto front.");
    info.aligEdges        = (needAligEdges or needSec) ?  storedAligEdges[A] : -1;
    info.squaredAligEdges = (needSquaredAligEdges) ?  storedSquaredAligEdges[A] : -1;
    info.exposedEdgesNumer= (needExposedEdges) ? storedExposedEdgesNumer[A] : -1;
    info.MS3Numer         = (needMS3) ? storedMS3Numer[A] : -1;
    info.inducedEdges     = (needInducedEdges) ?  storedInducedEdges[A] : -1;
    info.TCSum            = (needTC) ?  storedTCSum[A] : -1;
    info.edSum            = (needEd) ?  storedEdSum[A] : -1;
    info.erSum            = (needEr) ?  storedErSum[A] : -1;
    info.localScoreSum    = (needLocal) ? storedLocalScoreSum[A] : -1;
    info.localScoreSumMap = (needLocal) ? new map<string, double>(*storedLocalScoreSumMap[A]) : nullptr;
    info.wecSum           = (needWec) ?  storedWecSum[A] : -1;
    info.jsSum            = (needJs) ?   storedJsSum[A] : -1;
    info.ewecSum          = (needEwec) ?  storedEwecSum[A] : -1;
    info.ncSum            = (needNC) ? storedNcSum[A] : -1;
    info.currentScore     = storedCurrentScore[A];
    info.currentScores    = paretoFront.procureScoresByAlignment(A);
    info.currentMeasure   = paretoFront.getRandomMeasure();

    info.assignedNodesG2 = new vector<bool>(*storedAssignedNodesG2[A]);
    if(multipartite == 1)
        info.unassignedNodesG2 = new vector<uint>(*storedUnassignedNodesG2[A]);
    else {
        info.unassignedmiRNAsG2 = new vector<uint>(*storedUnassignedmiRNAsG2[A]);
        info.unassignedgenesG2 = new vector<uint>(*storedUnassignedgenesG2[A]);
    }

    info.A = new vector<uint>(*A);
}

void SANA::assignRandomAlignment(Job &job) {
    assert(paretoFront.size() == storedAlignments->size() and "Number of elements in paretoFront and storedAlignments don't match.");
    vector<uint> *A = paretoFront.procureRandomAlignment();
    copyAlignmentFromStorage(job, A);
}

vector<double> SANA::translateScoresToVector(Job &job) {
     vector<double> addScores(numOfMeasures);
     const AlignmentInfo &info = job.info;
#ifdef MULTI_PAIRWISE
    for(uint i = 0; i < numOfMeasures; i++) {
        addScores[scoreNamesToIndexes[measureNames[i]]] = measureCalculation[measureNames[i]]
                                                                   ( info.aligEdges, g1Edges, info.inducedEdges,
                                                                    g2Edges, info.TCSum, info.localScoreSum, n1,
                                                                    info.wecSum, info.jsSum, info.ewecSum, info.ncSum, trueA.back(),
                                                                    g1WeightedEdges, g2WeightedEdges,
                                                                    info.squaredAligEdges, info.exposedEdgesNumer,
								    info.MS3Numer, info.edSum, info.erSum, pairsCount
                                                                   );
    }
#else
    for(uint i = 0; i < numOfMeasures; i++) {
        addScores[scoreNamesToIndexes[measureNames[i]]] = measureCalculation[measureNames[i]]
                                                                  ( info.aligEdges, g1Edges, info.inducedEdges,
                                                                    g2Edges, info.TCSum, info.localScoreSum, n1,
                                                                    info.wecSum, info.jsSum, info.ewecSum, info.ncSum, trueA.back(),
                                                                    info.edSum, info.erSum, pairsCount
                                                                  );
    }
#endif
    return addScores;
}

double SANA::getElapsedTime() {
    chrono::steady_clock::time_point endTime = std::chrono::steady_clock::now();
    double seconds = chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime).count()
                     * 1.0 / 1000;
    return seconds;
}

void SANA::trackProgress(Job &job) {
    if (!enableTrackProgress) return;
    bool printDetails = false;
    bool printScores = false;
    bool checkScores = true;
    AlignmentInfo &info = job.info;
    cout << sharedIter/iterationsPerStep << " (" << getElapsedTime() << "s): score = " << info.currentScore;
    cout <<  " P(" << avgEnergyInc << ", " << job.Temperature << ") = " << acceptingProbability(avgEnergyInc, job.Temperature);
    cout << ", pBad = " << trueAcceptingProbability(job) << endl;

    if (not (printDetails or printScores or checkScores)) return;
    Alignment Al(*info.A);
    //original one is commented out for testing sec
    //if (printDetails) cout << " (" << Al.numAlignedEdges(*G1, *G2) << ", " << G2->numNodeInducedSubgraphEdges(*A) << ")";
    if (printDetails)
        cout << "Al.numAlignedEdges = " << Al.numAlignedEdges(*G1, *G2) << ", g1Edges = " <<g1Edges<< " ,g2Edges = "<<g2Edges<< endl;
    if (printScores) {
        SymmetricSubstructureScore S3(G1, G2);
        EdgeCorrectness EC(G1, G2);
        InducedConservedStructure ICS(G1, G2);
        SymmetricEdgeCoverage SEC(G1,G2);
        cout << "S3: " << S3.eval(Al) << "  EC: " << EC.eval(Al) << "  ICS: " << ICS.eval(Al) << "  SEC: " << SEC.eval(Al) <<endl;
    }
    if (checkScores) {
        double realScore = eval(Al);
        if (fabs(realScore-info.currentScore) > 0.00001) {
            cerr << "internal error: incrementally computed score (" << info.currentScore;
            cerr << ") is not correct (" << realScore << ")" << endl;
            info.currentScore = realScore;
        }
    }
    if (dynamic_tdecay) { // Code for estimating dynamic TDecay
        //The dynamic method uses linear interpolation to obtain an
        //an "ideal" P(bad) as a basis for SANA runs. If the current P(bad)
        //is significantly different from out "ideal" P(bad), then decay is either
        //"sped up" or "slowed down"
        int NSteps = 100;
        double fractional_time = (getElapsedTime()/(minutes*60));
        double lowIndex = floor(NSteps*fractional_time);
        double highIndex = ceil(NSteps*fractional_time);
        double betweenFraction = NSteps*fractional_time - lowIndex;
        double PLow = tau[lowIndex];
        double PHigh = tau[highIndex];

        double PBetween = PLow + betweenFraction * (PHigh - PLow);

        // if the ratio if off by more than a few percent, adjust.
        double ratio = acceptingProbability(avgEnergyInc, job.Temperature) / PBetween;
        if (abs(1-ratio) >= .01 &&
            (ratio < 1 || SANAtime > .2)) // don't speed it up too soon
        {
            double shouldBe;
            shouldBe = -log(avgEnergyInc/(TInitial*log(PBetween)))/(SANAtime);
            if(SANAtime==0 || shouldBe != shouldBe || shouldBe <= 0)
            shouldBe = TDecay * (ratio >= 0 ? ratio*ratio : 0.5);
            cout << "TDecay " << TDecay << " too ";
            cout << (ratio < 1 ? "fast" : "slow") << " shouldBe " << shouldBe;
            TDecay = sqrt(TDecay * shouldBe); // geometric mean
            cout << "; try " << TDecay << endl;
        }
    }
}

int SANA::aligEdgesIncChangeOp(Job &job, uint source, uint oldTarget, uint newTarget) {
    vector<uint> *A = job.info.A;
    int res = 0;
    const uint n = G1AdjLists[source].size();
    uint neighbor;
    // for (uint i = 0; i < n; ++i) {
    //     neighbor = G1AdjLists[source][i];
    //     res -= G2Matrix[oldTarget][(*A)[neighbor]];
    //     res += G2Matrix[newTarget][(*A)[neighbor]];
    // }
#ifdef SPARSE
    cout<<"sprase correct********";
    for (uint i = 0; i < n; ++i) {
        neighbor = G1AdjLists[source][i];
        if(G2Matrix[oldTarget].count(neighbor)){
            res -= G2Matrix[oldTarget][(*A)[neighbor]];
        }
        if(G2Matrix[newTarget].count(neighbor)){
            res += G2Matrix[newTarget][(*A)[neighbor]];
        }
    }
#else
    for (uint i = 0; i < n; ++i) {
        neighbor = G1AdjLists[source][i];
        res -= G2Matrix[oldTarget][(*A)[neighbor]];
        res += G2Matrix[newTarget][(*A)[neighbor]];
    }
#endif
    return res;
}

#define SQRDIFF2(A,i,j) ((_edgeVal=G2Matrix[i][(*A)[j]]), 2*_edgeVal + 1)
int SANA::squaredAligEdgesIncChangeOp(Job &job, uint source, uint oldTarget, uint newTarget) {
    int res = 0, diff;
    uint neighbor;
    vector<uint> *A = job.info.A;
    const uint n = G1AdjLists[source].size();
    for (uint i = 0; i < n; ++i) {
        neighbor = G1AdjLists[source][i];
        // Account for uint edges? Or assume smaller graph is edge value 1?
        diff = SQRDIFF2(A, oldTarget, neighbor);
        // assert(fabs(diff)<1100);
        res -= diff;// >0?diff:0;
        diff = SQRDIFF2(A, newTarget, neighbor);
        // assert(fabs(diff)<1100);
        res += diff;// >0?diff:0;
    }
    return res;
}

int SANA::inducedEdgesIncChangeOp(Job &job, uint source, uint oldTarget, uint newTarget) {
    vector<bool> *assignedNodesG2 = job.info.assignedNodesG2;
    int res = 0;
    const uint n = G2AdjLists[oldTarget].size();
    uint neighbor;
    uint i = 0;
    for (; i < n; ++i) {
        neighbor = G2AdjLists[oldTarget][i];
        res -= (*assignedNodesG2)[neighbor];
    }
    const uint m = G2AdjLists[newTarget].size();
    for (i = 0; i < m; ++i) {
        neighbor = G2AdjLists[newTarget][i];
        res += (*assignedNodesG2)[neighbor];
    }
    // address case changing between adjacent nodes:
    res -= G2Matrix[oldTarget][newTarget];
    return res;
}

double SANA::TCIncChangeOp(Job &job, uint source, uint oldTarget, uint newTarget) {
    vector<uint> *A = job.info.A;
    double deltaTriangles = 0;
    const uint n = G1AdjLists[source].size();
    uint neighbor1, neighbor2;
    for(uint i = 0; i < n; ++i){
        for(uint j = i+1; j < n; ++j){
            neighbor1 = G1AdjLists[source][i];
            neighbor2 = G1AdjLists[source][j];
            if(G1Matrix[neighbor1][neighbor2]){
                //G1 has a triangle
                if(G2Matrix[oldTarget][(*A)[neighbor1]] and G2Matrix[oldTarget][(*A)[neighbor2]] and G2Matrix[(*A)[neighbor1]][(*A)[neighbor2]]){
                    //G2 HAD a triangle
                    deltaTriangles -= 1;
                }
                if(G2Matrix[newTarget][(*A)[neighbor1]] and G2Matrix[newTarget][(*A)[neighbor2]] and G2Matrix[(*A)[neighbor1]][(*A)[neighbor2]]){
                    //G2 GAINS a triangle
                    deltaTriangles += 1;
                }
            }
        }
    }
    return ((double)deltaTriangles/maxTriangles);
}

double SANA::localScoreSumIncChangeOp(Job &job, vector<vector<float> > const & sim, uint const & source, uint const & oldTarget, uint const & newTarget) {
    return sim[source][newTarget] - sim[source][oldTarget];
}

double SANA::WECIncChangeOp(Job &job, uint source, uint oldTarget, uint newTarget) {
    vector<uint> *A = job.info.A;
    double res = 0;
    const uint n = G1AdjLists[source].size();
    uint neighbor;
    for (uint j = 0; j < n; ++j) {
        neighbor = G1AdjLists[source][j];
        if (G2Matrix[oldTarget][(*A)[neighbor]]) {
            res -= wecSims[source][oldTarget];
            res -= wecSims[neighbor][(*A)[neighbor]];
        }
        if (G2Matrix[newTarget][(*A)[neighbor]]) {
            res += wecSims[source][newTarget];
            res += wecSims[neighbor][(*A)[neighbor]];
        }
    }
    return res;
}

double SANA::JSIncChangeOp(Job &job, uint source, uint oldTarget, uint newTarget) {
    vector<uint> *A = job.info.A;
    double change = 0;
    if (jsWeight>0){
        //eval newJsSum here
        //update jsAlignedByNode with source and source neighbours using oldTarget and newTarget

        // eval for source from sratch
        uint sourceOldAlingedEdges = alignedByNode[source];
        uint sourceAlignedEdges = 0;
        vector<uint> sourceNeighbours = G1AdjLists[source];
        uint sourceTotalEdges = sourceNeighbours.size();
        for (uint j = 0; j < sourceTotalEdges; j++){
            uint neighbour = sourceNeighbours[j];
            uint neighbourAlignedTo = (*A)[neighbour]; //find the node neighbour is mapped to
            // if(G2Matrix[newTarget][neighbourAlignedTo] == true){
            //     sourceAlignedEdges += 1;
            // }
            sourceAlignedEdges += G2Matrix[newTarget][neighbourAlignedTo];
        }
        alignedByNode[source] = sourceAlignedEdges;
        //update newJsSum here
        change += ((sourceAlignedEdges - sourceOldAlingedEdges)/(double)sourceTotalEdges);


        // for each source neighbour update do iterative changes to the jsAlingedByNode vector
        // in each update get the G2mapping of neighbour and then check if edge was aligned by oldTarget to G2mapping and reduce score if newTarget to G2mapping doesnt exist
        // increase score if oldTarget to G2 mapping edge didnt exist but newTarget to G2 mapping does
        //no changes other wise
        for (uint j = 0; j < sourceTotalEdges; j++){
            uint neighbour = sourceNeighbours[j];
            uint neighbourAlignedTo = (*A)[neighbour]; //find the node neighbour is mapped to
            uint neighbourOldAlignedEdges = alignedByNode[neighbour];
            uint neighbourTotalEdges = G1AdjLists[neighbour].size();
            // if (G2Matrix[oldTarget][neighbourAlignedTo] == true && G2Matrix[newTarget][neighbourAlignedTo] == false){
            //     jsAlignedByNode[neighbour] -= 1;
            // }
            // if (G2Matrix[oldTarget][neighbourAlignedTo] == false && G2Matrix[newTarget][neighbourAlignedTo] == true){
            //     jsAlignedByNode[neighbour] += 1;
            // }
            alignedByNode[neighbour] -= G2Matrix[oldTarget][neighbourAlignedTo];
            alignedByNode[neighbour] += G2Matrix[newTarget][neighbourAlignedTo];
            //update newJsSum here
            change += ((alignedByNode[neighbour] - neighbourOldAlignedEdges)/(double)neighbourTotalEdges);
        }
    }

    return change;
}

double SANA::EWECIncChangeOp(Job &job, uint source, uint oldTarget, uint newTarget) {
    double score = 0;
    score = (EWECSimCombo(job, source, newTarget)) - (EWECSimCombo(job, source, oldTarget));
    return score;
}

double SANA::EWECSimCombo(Job &job, uint source, uint target) {
    vector<uint> *A = job.info.A;
    double score = 0;
    const uint n = G1AdjLists[source].size();
    uint neighbor;
    for (uint i = 0; i < n; ++i) {
        neighbor = G1AdjLists[source][i];
        if (G2Matrix[target][(*A)[neighbor]]) {
            int e1 = ewec->getRowIndex(source, neighbor);
            int e2 = ewec->getColIndex(target, (*A)[neighbor]);
            score+=ewec->getScore(e2,e1);
        }
    }
    return score/(2*g1Edges);
}

int SANA::ncIncChangeOp(Job &job, uint source, uint oldTarget, uint newTarget) {
    int change = 0;
    if (trueA[source] == oldTarget) change -= 1;
    if (trueA[source] == newTarget) change += 1;
    return change;
}

bool SANA::scoreComparison(Job &job, double newAligEdges, double newInducedEdges, double newTCSum,
                         double newLocalScoreSum, double newWecSum, double newJsSum, double newNcSum, double& newCurrentScore,
                         double newEwecSum, double newSquaredAligEdges, double newExposedEdgesNumer, double newMS3Numer,
			 double newEdgeDifferenceSum, double newEdgeRatioSum) {
    bool makeChange = false;
    wasBadMove = false;
    double badProbability = 0;

    AlignmentInfo &info = job.info;

    vector<double> addScores = getMeasureScores(newAligEdges, newInducedEdges, newTCSum, newLocalScoreSum, newWecSum, newJsSum, newNcSum, newEwecSum, newSquaredAligEdges, newExposedEdgesNumer, newMS3Numer, newEdgeDifferenceSum, newEdgeRatioSum);
    if(dominates(addScores, info.currentScores)) {
        info.currentScores = addScores;
        makeChange = true;
        newCurrentScore = 0;
        newCurrentScore += ecWeight * (newAligEdges / g1Edges);
        newCurrentScore += edWeight * EdgeDifference::adjustSumToTargetScore(newEdgeDifferenceSum, pairsCount);
        newCurrentScore += erWeight * EdgeRatio::adjustSumToTargetScore(newEdgeRatioSum, pairsCount);
        newCurrentScore += s3Weight * (newAligEdges / (g1Edges + newInducedEdges - newAligEdges));
        newCurrentScore += icsWeight * (newAligEdges / newInducedEdges);
        newCurrentScore += TCWeight * (newTCSum);
        newCurrentScore += localWeight * (newLocalScoreSum / n1);
        newCurrentScore += secWeight * (newAligEdges / g1Edges + newAligEdges / g2Edges)*0.5;
        newCurrentScore += wecWeight * (newWecSum / (2 * g1Edges));
        newCurrentScore += jsWeight * (newJsSum);
        newCurrentScore += ncWeight * (newNcSum / trueA.back());
    } else {
        newCurrentScore = 0;
        newCurrentScore += ecWeight * (newAligEdges / g1Edges);
        newCurrentScore += edWeight * EdgeDifference::adjustSumToTargetScore(newEdgeDifferenceSum, pairsCount);
        newCurrentScore += erWeight * EdgeRatio::adjustSumToTargetScore(newEdgeRatioSum, pairsCount);
        newCurrentScore += s3Weight * (newAligEdges / (g1Edges + newInducedEdges - newAligEdges));
        newCurrentScore += icsWeight * (newAligEdges / newInducedEdges);
        newCurrentScore += TCWeight * (newTCSum);
        newCurrentScore += localWeight * (newLocalScoreSum / n1);
        newCurrentScore += secWeight * (newAligEdges / g1Edges + newAligEdges / g2Edges)*0.5;
        newCurrentScore += wecWeight * (newWecSum / (2 * g1Edges));
        newCurrentScore += jsWeight * (newJsSum);
        newCurrentScore += ncWeight * (newNcSum / trueA.back());
        job.energyInc = newCurrentScore - info.currentScore;
        wasBadMove = job.energyInc < 0;
        badProbability = exp(job.energyInc / job.Temperature);
        makeChange = (addScores[info.currentMeasure] > info.currentScores[info.currentMeasure] or job.energyInc >= 0 or randomReal(job.gen) <= exp(job.energyInc / job.Temperature));
        if(makeChange) info.currentScores = addScores;
    }
    if (((TCWeight > 0 && job.iterationsPerformed % 32 == 0) || job.iterationsPerformed % 512 == 0) && wasBadMove) { //this will never run in the case of iterationsPerformed never being changed so that it doesn't greatly slow down the program if for some reason iterationsPerformed doesn't need to be changed.
        if (job.sampledProbability.size() == 1000) {
	    assert(false); // something is fucked up, make this consistent with the circular buffer sampledProbability
            job.sampledProbability.erase(job.sampledProbability.begin());
        }
        job.sampledProbability.push_back(badProbability);
    }

    return makeChange;
}

inline uint SANA::G1RandomUnlockedNode_Fast(Job &job) {
    return unLockedNodesG1[G1RandomUnlockedNodeDist(job.gen)];
}

inline uint SANA::G1RandomUnlockedNode(Job &job) {
#ifdef REINDEX
    return G1RandomUnlockedNodeDist(job.gen);
#else
    return G1RandomUnlockedNode_Fast(job);
#endif
}

inline uint SANA::G2RandomUnlockedNode_Fast(Job &job) {
    return G2RandomUnassignedNode(job.gen);
}

inline uint SANA::G2RandomUnlockedNode(Job &job, uint target1) {
    if(multipartite == 1){
        return G2RandomUnlockedNode_Fast(job);
    } else {
        bool isGene = G2->nodeTypes[target1] == Graph::NODE_TYPE_GENE;
        if(isGene){
            uint index = G2RandomUnassignedGeneDist(job.gen);
            return index;
        }
        else {
            int index = G2RandomUnassignedmiRNADist(job.gen);
            return index;
        }
    }
}

int SANA::aligEdgesIncSwapOp(Job &job, uint source1, uint source2, uint target1, uint target2) {
    vector<uint> *A = job.info.A;

    int res = 0;
    const uint n = G1AdjLists[source1].size();
    uint neighbor;
    uint i = 0;
#ifdef SPARSE
    for (; i < n; ++i) {
        neighbor = G1AdjLists[source1][i];
        if( G2Matrix[target1].count(neighbor)){
            res -= G2Matrix[target1][(*A)[neighbor]];
        }
        if(G2Matrix[target2].count(neighbor)){
            res += G2Matrix[target2][(*A)[neighbor]];
        }
    }
#else
    for (; i < n; ++i) {
        neighbor = G1AdjLists[source1][i];
        res -= G2Matrix[target1][(*A)[neighbor]];
        res += G2Matrix[target2][(*A)[neighbor]];
    }
#endif
    const uint m = G1AdjLists[source2].size();
#ifdef SPARSE
    for (i = 0; i < m; ++i) {
        neighbor = G1AdjLists[source2][i];
        if(G2Matrix[target2].count(neighbor)){
            res -= G2Matrix[target2][(*A)[neighbor]];
        }
        if(G2Matrix[target1].count(neighbor)){
            res += G2Matrix[target1][(*A)[neighbor]];
        }
    }
#else
    for (i = 0; i < m; ++i) {
        neighbor = G1AdjLists[source2][i];
        res -= G2Matrix[target2][(*A)[neighbor]];
        res += G2Matrix[target1][(*A)[neighbor]];
    }
#endif
    //address case swapping between adjacent nodes with adjacent images:
#ifdef MULTI_PAIRWISE
    res += (-1 << 1) & (G1Matrix[source1][source2] + G2Matrix[target1][target2]);
#else
    #ifdef SPARSE
        if(G1Matrix[source1].count(source2) & G2Matrix[target1].count(target2)){
            res += 2*(G1Matrix[source1][source2] & G2Matrix[target1][target2]);
        }
    #else
        res += 2*(G1Matrix[source1][source2] & G2Matrix[target1][target2]);
    #endif
#endif
    return res;
}

double SANA::TCIncSwapOp(Job &job, uint source1, uint source2, uint target1, uint target2) {
    vector<uint> *A = job.info.A;

    double deltaTriangles = 0;
    const uint n = G1AdjLists[source1].size();
    uint neighbor1, neighbor2;
    for(uint i = 0; i < n; ++i){
        for(uint j = i+1; j < n; ++j){
            neighbor1 = G1AdjLists[source1][i];
            neighbor2 = G1AdjLists[source1][j];
            if(G1Matrix[neighbor1][neighbor2]){
                //G1 has a triangle
                if(G2Matrix[target1][(*A)[neighbor1]] and G2Matrix[target1][(*A)[neighbor2]] and G2Matrix[(*A)[neighbor1]][(*A)[neighbor2]]){
                    //G2 HAD a triangle
                    deltaTriangles -= 1;
                }

                if((G2Matrix[target2][(*A)[neighbor1]] and G2Matrix[target2][(*A)[neighbor2]] and G2Matrix[(*A)[neighbor1]][(*A)[neighbor2]])
                || (neighbor1 == source2 and G2Matrix[target2][target1] and G2Matrix[target2][(*A)[neighbor2]] and G2Matrix[target1][(*A)[neighbor2]])
                || (neighbor2 == source2 and G2Matrix[target2][(*A)[neighbor1]] and G2Matrix[target2][target1] and G2Matrix[(*A)[neighbor1]][target1])) {
                    //G2 GAINS a triangle
                    deltaTriangles += 1;
                }
            }
         }
    }
    const uint m = G1AdjLists[source2].size();
    for(uint i = 0; i < m; ++i){
        for(uint j = i+1; j < m; ++j){
            neighbor1 = G1AdjLists[source2][i];
            neighbor2 = G1AdjLists[source2][j];
            if(G1Matrix[neighbor1][neighbor2]){
                //G1 has a triangle
                if(G2Matrix[target2][(*A)[neighbor1]] and G2Matrix[target2][(*A)[neighbor2]] and G2Matrix[(*A)[neighbor1]][(*A)[neighbor2]]){
                    //G2 HAD a triangle
                    deltaTriangles -= 1;
                }

                if((G2Matrix[target1][(*A)[neighbor1]] and G2Matrix[target1][(*A)[neighbor2]] and G2Matrix[(*A)[neighbor1]][(*A)[neighbor2]])
                   || (neighbor1 == source1 and G2Matrix[target1][target2] and G2Matrix[target1][(*A)[neighbor2]] and G2Matrix[target2][(*A)[neighbor2]])
                   || (neighbor2 == source1 and G2Matrix[target1][(*A)[neighbor1]] and G2Matrix[target1][target2] and G2Matrix[(*A)[neighbor1]][target2])){
                    //G2 GAINS a triangle
                    deltaTriangles += 1;
                }
            }
        }
    }
    return ((double)deltaTriangles/maxTriangles);
}

int SANA::squaredAligEdgesIncSwapOp(Job &job, uint source1, uint source2, uint target1, uint target2) {
    vector<uint> *A = job.info.A;

    int res = 0, diff;
    uint neighbor;
    const uint n = G1AdjLists[source1].size();
    uint i = 0;
    for (; i < n; ++i) {
        neighbor = G1AdjLists[source1][i];
        diff = SQRDIFF2(A, target1, neighbor);
        // assert(fabs(diff)<1100);
        res -= diff;// >0?diff:0;
        diff = SQRDIFF2(A, target2, neighbor);
        if (target2==(*A)[neighbor]){
                diff=0;
        }// assert(fabs(diff)<1100);
        res += diff;// >0?diff:0;
    }
    const uint m = G1AdjLists[source2].size();
    for (i = 0; i < m; ++i) {
        neighbor = G1AdjLists[source2][i];
        diff = SQRDIFF2(A, target2, neighbor);
        // assert(fabs(diff)<1100);
        res -= diff;// >0?diff:0;
        diff = SQRDIFF2(A, target1, neighbor);
        if (target1==(*A)[neighbor]){
                diff=0;
        }// assert(fabs(diff)<1100);
        res += diff;// >0?diff:0;
    }
    //  How to do for squared?
    // address case swapping between adjacent nodes with adjacent images:
    if(G1Matrix[source1][source2] and G2Matrix[target1][target2])
    {
        res += 2 * SQRDIFF2(A, target1,source2);
    }
    return res;
}

double SANA::WECIncSwapOp(Job &job, uint source1, uint source2, uint target1, uint target2) {
    vector<uint> *A = job.info.A;

    double res = 0;
    const uint n = G1AdjLists[source1].size();
    uint neighbor;
    for (uint j = 0; j < n; ++j) {
        neighbor = G1AdjLists[source1][j];
        if (G2Matrix[target1][(*A)[neighbor]]) {
            res -= wecSims[source1][target1];
            res -= wecSims[neighbor][(*A)[neighbor]];
        }
        if (G2Matrix[target2][(*A)[neighbor]]) {
            res += wecSims[source1][target2];
            res += wecSims[neighbor][(*A)[neighbor]];
        }
    }
    const uint m = G1AdjLists[source2].size();
    for (uint j = 0; j < m; ++j) {
        neighbor = G1AdjLists[source2][j];
        if (G2Matrix[target2][(*A)[neighbor]]) {
            res -= wecSims[source2][target2];
            res -= wecSims[neighbor][(*A)[neighbor]];
        }
        if (G2Matrix[target1][(*A)[neighbor]]) {
            res += wecSims[source2][target1];
            res += wecSims[neighbor][(*A)[neighbor]];
        }
    }
    //address case swapping between adjacent nodes with adjacent images:
#ifdef MULTI_PAIRWISE
    if (G1Matrix[source1][source2] > 0 and G2Matrix[target1][target2] > 0) {
#else
    if (G1Matrix[source1][source2] and G2Matrix[target1][target2]) {
#endif
        res += 2*wecSims[source1][target1];
        res += 2*wecSims[source2][target2];
    }
    return res;
}

double SANA::JSIncSwapOp(Job &job, uint source1, uint source2, uint target1, uint target2) {
    // NOTE: eval swap as two sources and then loop neighbours

    vector<uint> *A = job.info.A;
    double change = 0;

    uint source1OldAlingedEdges = alignedByNode[source1];
    uint source1AlignedEdges = 0;
    vector<uint> source1Neighbours = G1AdjLists[source1];
    uint source1TotalEdges = source1Neighbours.size();


    uint source2OldAlingedEdges = alignedByNode[source2];
    uint source2AlignedEdges = 0;
    vector<uint> source2Neighbours = G1AdjLists[source2];
    uint source2TotalEdges = source2Neighbours.size();

    // source 1 eval here

    // eval for source1 from sratch
    for (uint j = 0; j < source1TotalEdges; j++){
        uint neighbour = source1Neighbours[j];
        uint neighbourAlignedTo = (*A)[neighbour]; //find the node neighbour is mapped to
        // if(G2Matrix[target2][neighbourAlignedTo] == true){
        //     source1AlignedEdges += 1;
        // }
        source1AlignedEdges = G2Matrix[target2][neighbourAlignedTo];
    }
    alignedByNode[source1] = source1AlignedEdges;
    //update change here
    change += ((source1AlignedEdges - source1OldAlingedEdges)/(double)source1TotalEdges);


    // for each source neighbour update do iterative changes to the jsAlingedByNode vector
    // in each update get the G2mapping of neighbour and then check if edge was aligned by oldTarget to G2mapping and reduce score if newTarget to G2mapping doesnt exist
    // increase score if oldTarget to G2 mapping edge didnt exist but newTarget to G2 mapping does
    //no changes other wise
    for (uint j = 0; j < source1TotalEdges; j++){
        uint neighbour = source1Neighbours[j];
        uint neighbourAlignedTo = (*A)[neighbour]; //find the node neighbour is mapped to
        uint neighbourOldAlignedEdges = alignedByNode[neighbour];
        uint neighbourTotalEdges = G1AdjLists[neighbour].size();
        if (std::find (source1Neighbours.begin(), source1Neighbours.end(), neighbour) == source1Neighbours.end()){
            alignedByNode[neighbour] -= G2Matrix[target1][neighbourAlignedTo];
            alignedByNode[neighbour] += G2Matrix[target2][neighbourAlignedTo];
            //update newJsSum here
            change += ((alignedByNode[neighbour] - neighbourOldAlignedEdges)/(double)neighbourTotalEdges);
        }
    }

    // source 2 eval here

    // eval for source2 from sratch
    for (uint j = 0; j < source2TotalEdges; j++){
        uint neighbour = source2Neighbours[j];
        uint neighbourAlignedTo = (*A)[neighbour]; //find the node neighbour is mapped to
        source2AlignedEdges += G2Matrix[target1][neighbourAlignedTo];
    }
    alignedByNode[source2] = source2AlignedEdges;
    //update change here
    change += ((source2AlignedEdges - source2OldAlingedEdges)/(double)source2TotalEdges);


    // for each source neighbour update do iterative changes to the jsAlingedByNode vector
    // in each update get the G2mapping of neighbour and then check if edge was aligned by oldTarget to G2mapping and reduce score if newTarget to G2mapping doesnt exist
    // increase score if oldTarget to G2 mapping edge didnt exist but newTarget to G2 mapping does
    //no changes other wise
    for (uint j = 0; j < source2TotalEdges; j++){
        uint neighbour = source2Neighbours[j];
        uint neighbourAlignedTo = (*A)[neighbour]; //find the node neighbour is mapped to
        uint neighbourOldAlignedEdges = alignedByNode[neighbour];
        uint neighbourTotalEdges = G1AdjLists[neighbour].size();
        if (std::find (source1Neighbours.begin(), source1Neighbours.end(), neighbour) == source1Neighbours.end()){
            alignedByNode[neighbour] -= G2Matrix[target2][neighbourAlignedTo];
            alignedByNode[neighbour] += G2Matrix[target1][neighbourAlignedTo];
            //update newJsSum here
            change += ((alignedByNode[neighbour] - neighbourOldAlignedEdges)/(double)neighbourTotalEdges);
        }
    }

    // //eval for common neighbours here
    // vector<uint> source1source2commonneighbours(source1Neighbours.size() + source2Neighbours.size());

    return change;

}

double SANA::EWECIncSwapOp(Job &job, uint source1, uint source2, uint target1, uint target2) {
    double score = 0;
    score = (EWECSimCombo(job, source1, target2)) + (EWECSimCombo(job, source2, target1)) - (EWECSimCombo(job, source1, target1)) - (EWECSimCombo(job, source2, target2));
    if(G1Matrix[source1][source2] and G2Matrix[target1][target2]){
        score += ewec->getScore(ewec->getColIndex(target1, target2), ewec->getRowIndex(source1, source2))/(g1Edges); //correcting for missed edges when swapping 2 adjacent pairs
    }
    return score;
}

int SANA::ncIncSwapOp(Job &job, uint source1, uint source2, uint target1, uint target2) {
    int change = 0;
    if(trueA[source1] == target1) change -= 1;
    if(trueA[source2] == target2) change -= 1;
    if(trueA[source1] == target2) change += 1;
    if(trueA[source2] == target1) change += 1;
    return change;
}

double SANA::localScoreSumIncSwapOp(Job &job, vector<vector<float> > const & sim, uint const & source1, uint const & source2, uint const & target1, uint const & target2) {
    return sim[source1][target2] - sim[source1][target1] + sim[source2][target1] - sim[source2][target2];
}

double SANA::edgeDifferenceIncChangeOp(Job &job, uint source, uint oldTarget, uint newTarget) {
   vector<uint> *A = job.info.A;
   double edgeDifferenceIncDiff = 0;
   double c = 0;

   vector<uint> &adjList = G1AdjLists[source];
   for (uint i = 0; i < adjList.size(); ++i) {
       uint node2 = adjList[i];

       double y = -abs(G1FloatWeights[source][node2] - G2FloatWeights[oldTarget][(*A)[node2]])
                  - c;
       double t = edgeDifferenceIncDiff + y;
       c = (t - edgeDifferenceIncDiff) - y;
       edgeDifferenceIncDiff = t;

       uint node2Target = node2 == source ? newTarget : (*A)[node2];
       y = +abs(G1FloatWeights[source][node2] - G2FloatWeights[newTarget][node2Target])
                  - c;
       t = edgeDifferenceIncDiff + y;
       c = (t - edgeDifferenceIncDiff) - y;
       edgeDifferenceIncDiff = t;
   }
   return edgeDifferenceIncDiff;
}

double SANA::edgeRatioIncChangeOp(Job &job, uint source, uint oldTarget, uint newTarget) {
   vector<uint> *A = job.info.A;
   double edgeRatioIncDiff = 0;
   double c = 0;
   vector<uint> &adjList = G1AdjLists[source];
   for (uint i = 0; i < adjList.size(); ++i) {
       uint node2 = adjList[i];
       double r = getRatio(G1FloatWeights[source][node2], G2FloatWeights[oldTarget][(*A)[node2]]);
       double y = -r - c;
       double t = edgeRatioIncDiff + y;
       c = (t - edgeRatioIncDiff) - y;
       edgeRatioIncDiff = t;

       uint node2Target = node2 == source ? newTarget : (*A)[node2];
       r=getRatio(G1FloatWeights[source][node2],G2FloatWeights[newTarget][node2Target]);
       y = +r - c;
       t = edgeRatioIncDiff + y;
       c = (t - edgeRatioIncDiff) - y;
       edgeRatioIncDiff = t;
   }

   return edgeRatioIncDiff;
}

double SANA::edgeDifferenceIncSwapOp(Job &job, uint source1, uint source2, uint target1, uint target2) {
    vector<uint> *A = job.info.A;
    if (source1 == source2) {
        return 0;
    }

    // Subtract source1-target1
    // Add source1-target2
    double edgeDifferenceIncDiff = 0;
    double c = 0;
    vector<uint> &adjList = G1AdjLists[source1];
    for (uint i = 0; i < adjList.size(); ++i) {
        uint node2 = adjList[i];
        double y = -abs(G1FloatWeights[source1][node2] - G2FloatWeights[target1][(*A)[node2]])
                  - c;
        double t = edgeDifferenceIncDiff + y;
        c = (t - edgeDifferenceIncDiff) - y;
        edgeDifferenceIncDiff = t;

        uint node2Target = 0;
        if (node2 == source1) {
            node2Target = target2;
        } else if (node2 == source2) {
            node2Target = target1;
        } else {
            node2Target = (*A)[node2];
        }
        y = +abs(G1FloatWeights[source1][node2] - G2FloatWeights[target2][node2Target])
           - c;
        t = edgeDifferenceIncDiff + y;
        c = (t - edgeDifferenceIncDiff) - y;
        edgeDifferenceIncDiff = t;
   }

    // Subtract source2-target2
    // Add source2-target1
    vector<uint> &adjList2 = G1AdjLists[source2];
    for (uint i = 0; i < adjList2.size(); ++i) {
        uint node2 = adjList[i];
        if (node2 == source1) continue;

        double y = -abs(G1FloatWeights[source2][node2] - G2FloatWeights[target2][(*A)[node2]])
                  - c;
        double t = edgeDifferenceIncDiff + y;
        c = (t - edgeDifferenceIncDiff) - y;
        edgeDifferenceIncDiff = t;

        uint node2Target = 0;
        if (node2 == source2) {
            node2Target = target1;
        } else {
            node2Target = (*A)[node2];
        }
        y = +abs(G1FloatWeights[source2][node2] - G2FloatWeights[target1][node2Target])
           - c;
        t = edgeDifferenceIncDiff + y;
        c = (t - edgeDifferenceIncDiff) - y;
        edgeDifferenceIncDiff = t;
   }

    return edgeDifferenceIncDiff;
}

double SANA::edgeRatioIncSwapOp(Job &job, uint source1, uint source2, uint target1, uint target2) {
    vector<uint> *A = job.info.A;
    if (source1 == source2) {
        return 0;
    }

    double edgeRatioIncDiff = 0;
    double c = 0;
    vector<uint> &adjList = G1AdjLists[source1];
    for (uint i = 0; i < adjList.size(); ++i) {
        uint node2 = adjList[i];
        double r = getRatio(G1FloatWeights[source1][node2], G2FloatWeights[target1][(*A)[node2]]);
        double y = -r - c;
        double t = edgeRatioIncDiff + y;
        c = (t - edgeRatioIncDiff) - y;
        edgeRatioIncDiff = t;

        uint node2Target = 0;
        if (node2 == source1) {
            node2Target = target2;
        } else if (node2 == source2) {
            node2Target = target1;
        } else {
            node2Target = (*A)[node2];
        }
        r = getRatio(G1FloatWeights[source1][node2], G2FloatWeights[target2][node2Target]);
        y = +r - c;
        t = edgeRatioIncDiff + y;
        c = (t - edgeRatioIncDiff) - y;
        edgeRatioIncDiff = t;
   }

    // Subtract source2-target2
    // Add source2-target1
    vector<uint> &adjList2 = G1AdjLists[source2];
    for (uint i = 0; i < adjList2.size(); ++i) {
        uint node2 = adjList[i];
        if (node2 == source1) continue;
        double r = getRatio(G1FloatWeights[source2][node2], G2FloatWeights[target2][(*A)[node2]]);
        double y = -r - c;
        double t = edgeRatioIncDiff + y;
        c = (t - edgeRatioIncDiff) - y;
        edgeRatioIncDiff = t;

        uint node2Target = 0;
        if (node2 == source2) {
            node2Target = target1;
        } else {
            node2Target = (*A)[node2];
        }
        r = getRatio(G1FloatWeights[source2][node2], G2FloatWeights[target1][node2Target]);
        y = +r - c;
        t = edgeRatioIncDiff + y;
        c = (t - edgeRatioIncDiff) - y;
        edgeRatioIncDiff = t;
   }

    return edgeRatioIncDiff;
}

inline uint SANA::G1RandomUnlockedNode(Job &job, uint source1) {
    if(multipartite == 1){
        return G1RandomUnlockedNode(job);
    } else {
        // Checking node type and returning one with same type
        #ifdef REINDEX
            bool isGene = source1 < (uint) G1->unlockedGeneCount;
            if(isGene)
                return G1RandomUnlockedGeneDist(job.gen);
            else
                return G1->unlockedGeneCount + G1RandomUnlockedmiRNADist(job.gen);
        #else
            bool isGene = G1->nodeTypes[source1] == Graph::NODE_TYPE_GENE;
            if(isGene){
                int index = G1RandomUnlockedGeneDist(job.gen);
                return G1->geneIndexList[index];
            }
            else{
                int index =  G1RandomUnlockedmiRNADist(job.gen);
                return G1->miRNAIndexList[index];
            }
        #endif
    }
}
