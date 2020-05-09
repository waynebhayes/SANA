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
#include "../measures/NodeCorrectness.hpp"
#include "../measures/SymmetricEdgeCoverage.hpp"
#include "../measures/localMeasures/Sequence.hpp"
#include "../measures/EdgeExposure.hpp"
#include "../measures/MultiS3.hpp"
#include "../utils/utils.hpp"
#include "../Report.hpp"

using namespace std;

//static fields
bool SANA::interrupt = false;
bool SANA::saveAlignment = false;

uint SANA::INVALID_ACTIVE_ID;

SANA::SANA(const Graph* G1, const Graph* G2,
        double TInitial, double TDecay, double t, bool usingIterations, bool addHillClimbing,
        MeasureCombination* MC, const string& scoreAggrStr, const Alignment& startA,
        const string& outputFileName, const string& localScoresFileName):
                Method(G1, G2, "SANA_"+MC->toString()),
                startA(startA),
                addHillClimbing(addHillClimbing),
                TInitial(TInitial), TDecay(TDecay),
                wasBadMove(false),
                usingIterations(usingIterations),
                MC(MC),
                outputFileName(outputFileName),
                localScoresFileName(localScoresFileName) {
    initTau();
    n1 = G1->getNumNodes(), n2 = G2->getNumNodes();
    g1Edges = G1->getNumEdges(), g2Edges = G2->getNumEdges();
    g1WeightedEdges = G1->getTotalEdgeWeight(), g2WeightedEdges = G2->getTotalEdgeWeight();
    pairsCount = n1 * (n1 + 1) / 2;

    if      (scoreAggrStr == "sum")       scoreAggr = ScoreAggregation::sum;
    else if (scoreAggrStr == "product")   scoreAggr = ScoreAggregation::product;
    else if (scoreAggrStr == "inverse")   scoreAggr = ScoreAggregation::inverse;
    else if (scoreAggrStr == "max")       scoreAggr = ScoreAggregation::max;
    else if (scoreAggrStr == "min")       scoreAggr = ScoreAggregation::min;
    else if (scoreAggrStr == "maxFactor") scoreAggr = ScoreAggregation::maxFactor;
    else throw runtime_error("unknown score aggregation: "+scoreAggrStr);

    //random number generation
    gen = mt19937(getRandomSeed());
    randomReal = uniform_real_distribution<>(0, 1);

    //temperature schedule
    if (usingIterations) maxIterations = (uint)(t);
    else minutes = t;
    initializedIterPerSecond = false;
    pBadBuffer = vector<double> (PBAD_CIRCULAR_BUFFER_SIZE, 0);

    //objective function
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
    try { wecWeight = MC->getWeight("wec"); }
    catch(...) { wecWeight = 0; }
    try { ewecWeight = MC->getWeight("ewec"); }
    catch(...) { ewecWeight = 0; }
    try {
        needNC      = false;
        ncWeight    = MC->getWeight("nc");
        Measure* nc = MC->getMeasure("nc");
        trueA       = ((NodeCorrectness*) nc)->getMappingforNC();
        needNC      = true;
    } catch(...) {
        ncWeight = 0;
        trueA    = {static_cast<uint>(G2->getNumNodes()), 1};
    }
    localWeight = MC->getSumLocalWeight();

    //indicate which variables need to be maintained incrementally
    needAligEdges        = icsWeight > 0 or ecWeight > 0 or s3Weight > 0 or wecWeight > 0 or secWeight > 0 or mecWeight > 0;
    needEd               = edWeight > 0; //edge difference
    needEr               = erWeight > 0; //edge ratio
    needSquaredAligEdges = sesWeight > 0; //SES
    needExposedEdges     = eeWeight > 0 or ms3Weight > 0; //EE; if needMS3, might use EE as denom
    needMS3              = ms3Weight > 0;
    needInducedEdges     = s3Weight > 0 or icsWeight > 0;
    needJs               = jsWeight > 0;
    needWec              = wecWeight > 0;
    needEwec             = ewecWeight>0;
    needSec              = secWeight > 0;
    needLocal            = localWeight > 0;
#ifndef MULTI_PAIRWISE
    needSquaredAligEdges = false;
    needExposedEdges     = false;
    needMS3              = false;
#endif
    if (needWec) {
        Measure* wec                     = MC->getMeasure("wec");
        LocalMeasure* m                  = ((WeightedEdgeConservation*) wec)->getNodeSimMeasure();
        vector<vector<float> >* wecSimsP = m->getSimMatrix();
        wecSims                          = (*wecSimsP);
    }
    if (needLocal) {
        sims              = MC->getAggregatedLocalSims();
        localSimMatrixMap = MC->getLocalSimMap();
        localWeight       = 1; //the values in the sim Matrix 'sims' have already been scaled by the weight
    } else {
        localWeight = 0;
    }

#ifdef CORES
#ifdef UNWEIGHTED_CORES
    numPegSamples = vector<unsigned long>(n1, 0);
    pegHoleFreq = Matrix<unsigned long>(n1, n2);
#endif
    weightedPegHoleFreq_pBad = Matrix<double>(n1, n2);
    totalWeightedPegWeight_pBad = vector<double>(n1, 0);
    weightedPegHoleFreq_1mpBad = Matrix<double>(n1, n2);
    totalWeightedPegWeight_1mpBad = vector<double>(n1, 0);
#endif

    //other execution options
    dynamicTDecay         = false;
    constantTemp          = false;
    enableTrackProgress   = true;
    iterationsPerStep     = 10000000;
    avgEnergyInc          = -0.00001; //to track progress

    //things initialized in initDataStructures
    //they have the same size for every run, so we can allocate the size here
    assignedNodesG2 = vector<bool> (n2);
    unassignedG2NodesByColor  = vector<vector<uint> > (G1->numColors());

    // NODE COLOR SYSTEM initlialization
    if (G1->numColors() > G2->numColors())
        throw runtime_error("some G1 nodes have a color non-existent in G2, "
                            "so there is no valid alignment");
    else if (G1->numColors() < G2->numColors())
        cerr<<"Warning: some G2 nodes have a color non-existent in G1, "
            <<"so some G2 nodes won't be part of any valid alignment"<<endl;

    vector<uint> numSwapNeighborsByColor(G1->numColors(), 0);
    vector<uint> numChangeNeighborsByColor(G1->numColors(), 0);
    uint totalAligNbrs = 0;
    for (uint id = 0; id < G1->numColors(); id++) {
        string colorName = G1->getColorName(id);
        if (!G2->hasColor(colorName))
            throw runtime_error("G1 nodes colored "+colorName+" cannot be matched to any G2 nodes");
        uint c1 = G1->numNodesWithColor(id);
        uint c2 = G2->numNodesWithColor(G2->getColorId(colorName));
        if (c1 > c2) throw runtime_error("there are "+to_string(c1)+" G1 nodes colored "
                    +colorName+" but only "+to_string(c2)+" such nodes in G2");  
        numSwapNeighborsByColor[id] = c1*(c1-1)/2;
        numChangeNeighborsByColor[id] = c1*(c2-c1);
        uint numNbrs = numSwapNeighborsByColor[id] + numChangeNeighborsByColor[id];
        totalAligNbrs += numNbrs;
        // debug info:
        cerr<<"color "<<colorName<<" has "<<numSwapNeighborsByColor[id]<<" swap nbrs and "
            <<numChangeNeighborsByColor[id]<<" change nbrs ("<<numNbrs<<" total)"<<endl;
    }
    if (totalAligNbrs == 0) throw runtime_error(
            "there is a unique valid alignment, so running SANA is pointless");
    //debug info:
    cerr<<"an alignment has "<<totalAligNbrs<<" nbrs in total"<<endl;

    //init activeColorIds, changeProbByColor, and colorAccumProbCutpoints
    for (uint id = 0; id < G1->numColors(); id++) {
        uint numNbrs = numSwapNeighborsByColor[id] + numChangeNeighborsByColor[id];
        if (numNbrs == 0) { 
            cerr<<"color "<<G1->getColorName(id)<<" (id "<<id<<") is inactive"<<endl;
            continue; //inactive color (e.g., a locked pair)
        }
        activeColorIds.push_back(id);
        changeProbByColor.push_back(numChangeNeighborsByColor[id]/ (double) numNbrs);
        double colorProb = numNbrs / (double) totalAligNbrs;
        double accumProb = colorProb +
                (colorAccumProbCutpoints.empty() ? 0 : colorAccumProbCutpoints.back());
        colorAccumProbCutpoints.push_back(accumProb);
        //debug info:
        cerr<<"color "<<G1->getColorName(id)<<" (id "<<id<<") has prob "<<colorProb
            <<" (accumulated prob is now up to "<<accumProb<<")"<<endl;
    }
    //due to rounding errors, the last number may not be exactly 1, so we correct it
    colorAccumProbCutpoints.back() = 1;

    //init g2NodeToActiveColorId
    vector<uint> g2ToG1ColorIdMap = G2->myColorIdsToOtherGraphColorIds(*G1);
    INVALID_ACTIVE_ID = n1;
    vector<uint> g1ColorIdToActiveId(G1->numColors(), INVALID_ACTIVE_ID);
    for (uint i = 0; i < activeColorIds.size(); i++) {
        g1ColorIdToActiveId[activeColorIds[i]] = i;
    }
    g2NodeToActiveColorId = vector<uint> (n2, INVALID_ACTIVE_ID);
    for (uint g2Node = 0; g2Node < n2; g2Node++) {
        uint g2ColorId = G2->nodeColors[g2Node];
        uint g1ColorId = g2ToG1ColorIdMap[g2ColorId];
        if (g1ColorId == Graph::INVALID_COLOR_ID) {
            g2NodeToActiveColorId[g2Node] = INVALID_ACTIVE_ID;
        } else {
            g2NodeToActiveColorId[g2Node] = g1ColorIdToActiveId[g1ColorId];
        }
    }
}
SANA::~SANA() {}

//initialize data structures specific to the starting alignment
//everything that is alignment-independent should be initialized in the
//constructor instead
//even for data structures initialized here, any space allocation for them
//should be done in the constructor, not here, to avoid memory leaks
void SANA::initDataStructures() {
    iterationsPerformed = 0;
    numPBadsInBuffer = pBadBufferSum = pBadBufferIndex = 0;

    Alignment alig;
    if (startA.size() != 0) alig = startA;
    else alig = Alignment::randomColorRestrictedAlignment(*G1, *G2);

    //initialize assignedNodesG2 (the space was already allocated in the constructor)
    for (uint i = 0; i < n2; i++) assignedNodesG2[i] = false;
    for (uint i = 0; i < n1; i++) assignedNodesG2[alig[i]] = true;
    //initialize unassignedG2NodesByColor (the space was already allocated in the constructor)
    for (uint i = 0; i <  unassignedG2NodesByColor.size(); i++)
        unassignedG2NodesByColor[i].clear();
    for (uint g2Node = 0; g2Node < n2; g2Node++) {
        if (assignedNodesG2[g2Node]) continue;
        uint actInd = g2NodeToActiveColorId[g2Node];
        if (actInd != INVALID_ACTIVE_ID) {
            unassignedG2NodesByColor[actInd].push_back(g2Node);
        }
    }

    if (needAligEdges or needSec) aligEdges = alig.numAlignedEdges(*G1, *G2);
    if (needEd) edSum = EdgeDifference::getEdgeDifferenceSum(G1, G2, alig);
    if (needEr) erSum = EdgeRatio::getEdgeRatioSum(G1, G2, alig);
    if (needSquaredAligEdges) squaredAligEdges =
            ((SquaredEdgeScore*) MC->getMeasure("ses"))->numSquaredAlignedEdges(alig);
    if (needExposedEdges) EdgeExposure::numer = 
            EdgeExposure::numExposedEdges(A, *G1, *G2) - EdgeExposure::getMaxEdge();
    if (needMS3) MultiS3::numer =
            ((MultiS3*) MC->getMeasure("ms3"))->computeNumer(alig);
    if (needInducedEdges) inducedEdges = G2->numEdgesInNodeInducedSubgraph(alig.asVector());
    if (needLocal) {
        localScoreSum = 0;
        for (uint i = 0; i < n1; i++) localScoreSum += sims[i][alig[i]];
        localScoreSumMap.clear();
    }
    if (needWec) {
        Measure* wec    = MC->getMeasure("wec");
        double wecScore = wec->eval(alig);
        wecSum          = wecScore*2*g1Edges;
    }
    if (needJs) {
        Measure* js = MC->getMeasure("js");
        jsSum       = js->eval(alig);
        alignedByNode = JaccardSimilarityScore::getAlignedByNode(G1, G2, alig);
    }
    if (needEwec) {
        ewec    = (ExternalWeightedEdgeConservation*)(MC->getMeasure("ewec"));
        ewecSum = ewec->eval(alig);
    }
    if (needNC) {
        Measure* nc = MC->getMeasure("nc");
        ncSum       = (nc->eval(alig))*trueA.back();
    }
    currentScore = eval(alig);
    A = alig.asVector();
    timer.start();
}

uint SANA::randColorWeightedByNumNbrs() {
    if (activeColorIds.size() == 1) return 0; //optimized special case: monochromatic graphs
    else if (activeColorIds.size() == 2) { //optimized special case: bichromatic graphs
        return (randomReal(gen) < colorAccumProbCutpoints[0] ? 0 : 1);
    } else { //general case
        double p = randomReal(gen);
        //use binary search to optimizie for the case with many (non-unique) colors
        //could be optimized to avoid the std::lower_bound call for the mono and bichromatic cases
        auto iter = lower_bound(colorAccumProbCutpoints.begin(), colorAccumProbCutpoints.end(), p);
        if (iter == colorAccumProbCutpoints.end()) throw runtime_error(
            "random number between 0 and 1 is beyond the last color cutpoint, which should be 1");
        uint index = iter - colorAccumProbCutpoints.begin();
        return index;
    }
}

/*The following is designed so that every single node from both networks
  is printed at least once. First, we find for every single node (across
  BOTH networks) what it's *highest* score is with a partner in the other
  network. Once we compute every node's highest score, we then go and find
  the smallest such score, and call it Smin. This defines the minimum score
  that we want to output, and it guarantees that every node appears in at
  least one aligned node-pair according to this score.  We output this Smin. */
double SANA::TrimCoreScores(Matrix<unsigned long>& Freq, vector<unsigned long>& numPegSamples) {
    uint n1 = Freq.size(), n2 = Freq[0].size();
    vector<double> high1(n1,0.0);
    vector<double> high2(n2,0.0);
    for (uint i=0; i<n1; i++) {
        double denom =  1.0 / (double)numPegSamples[i];
        for (uint j=0;j<n2; j++) {
            double score = Freq[i][j] * denom;
            if (score > high1[i]) high1[i] = score;
            if (score > high2[j]) high2[j] = score;
        }
    }
    double Smin = high1[0];
    for (uint i=0;i<n1;i++) if (high1[i] < Smin) Smin = high1[i];
    for (uint j=0;j<n2;j++) if (high2[j] < Smin) Smin = high2[j];
    return Smin;
}
double SANA::TrimCoreScores(Matrix<double>& Freq, vector<double>& totalPegWeight) {
    uint n1 = Freq.size(), n2 = Freq[0].size();
    vector<double> high1(n1,0.0);
    vector<double> high2(n2,0.0);
    for (uint i=0; i<n1; i++) {
        double denom =  1.0 / (double)totalPegWeight[i];
        for (uint j=0;j<n2; j++) {
            double score = Freq[i][j] * denom;
            if (score > high1[i]) high1[i] = score;
            if (score > high2[j]) high2[j] = score;
        }
    }
    double Smin = high1[0];
    for (uint i=0;i<n1;i++) if (high1[i] < Smin) Smin = high1[i];
    for (uint j=0;j<n2;j++) if (high2[j] < Smin) Smin = high2[j];
    return Smin;
}

Alignment SANA::run() {
    initDataStructures();
    setInterruptSignal();

    cerr<<"usingIterations = "<<usingIterations<<endl;
    long long int maxIters = usingIterations ? ((long long int)(maxIterations))*10000000 
                                             : (long long int) (getIterPerSecond()*minutes*60);
    double leeway = 2;
    double maxTime = usingIterations ? -1 : minutes * 60 * leeway;

    long long int iter;
    for (iter = 0; iter <= maxIters; ++iter) {
        Temperature = temperatureFunction(iter, TInitial, TDecay);
        SANAIteration();
        if (interrupt) break; //set from interruption
        if (saveAlignment) printReport(); //set from interruption
        if (iter%iterationsPerStep == 0) {
            trackProgress(iter, maxIters);
            if (maxTime != -1 and timer.elapsed() > maxTime and currentScore-previousScore < 0.005) break;
            previousScore = currentScore;
        }
    }
    trackProgress(iter, maxIters);
    cout<<"Performed "<<iter<<" total iterations\n";

    if (addHillClimbing) performHillClimbing(10000000LL); //arbitrarily chosen, probably too big.

#ifdef CORES
    const bool PRINT_CORES = false;
    if (PRINT_CORES) {
#ifdef UNWEIGHTED_CORES
        double SminUnW = TrimCoreScores(pegHoleFreq,numPegSamples);
        cout << "Smin_UnW "<< SminUnW << " ";
#endif
        double Smin_pBad =TrimCoreScores(weightedPegHoleFreq_pBad, totalWeightedPegWeight_pBad);
        double Smin_1mpBad =TrimCoreScores(weightedPegHoleFreq_1mpBad, totalWeightedPegWeight_1mpBad);
        cout << "Smin_pBad "<< Smin_pBad << " Smin_(1-pBad) " << Smin_1mpBad << endl;

        printf("######## core frequencies#########\n");
        printf("p1 p2");
#ifdef UNWEIGHTED_CORES
        printf(" unwgtd");
#endif
        printf("wpB w1_pB\n");
        for (uint i=0; i<n1; i++) for (uint j=0; j<n2; j++) {
#ifdef UNWEIGHTED_CORES
            double unweightdedScore = pegHoleFreq[i][j]/(double)numPegSamples[i];
#endif
            double weightedScore_pBad = weightedPegHoleFreq_pBad[i][j]/totalWeightedPegWeight_pBad[i];
            double weightedScore_1mpBad = weightedPegHoleFreq_1mpBad[i][j]/totalWeightedPegWeight_1mpBad[i];
            const double MIN_CORE_SCORE = 1e-4;
            if (
#ifdef UNWEIGHTED_CORES
            unweightdedScore  >= max(MIN_CORE_SCORE,SminUnW) ||
#endif
            weightedScore_pBad >= max(MIN_CORE_SCORE,Smin_pBad) ||
            weightedScore_1mpBad >= max(MIN_CORE_SCORE,Smin_1mpBad)) {
                printf(
#ifdef UNWEIGHTED_CORES
                "%s %s %.6f %.6f %.6f\n",
#else
                "%s %s %.6f %.6f\n",
#endif
                (G1->nodeNames)[i].c_str(), (G2->nodeNames)[j].c_str(),
#ifdef UNWEIGHTED_CORES
                unweightdedScore,
#endif
                weightedScore_pBad, weightedScore_1mpBad);
            }
        }
    }
#endif // cores

    return A;
}

void SANA::performHillClimbing(long long int idleCountTarget) {
    long long int iter = 0;
    Temperature = 0;
    numPBadsInBuffer = pBadBufferSum = pBadBufferIndex = 0; 

    cout << "Beginning Final Pure Hill Climbing Stage" << endl;
    Timer hillTimer;
    hillTimer.start();
    uint idleCount = 0;
    while(idleCount < idleCountTarget) {
        if (iter%iterationsPerStep == 0) trackProgress(iter);
        double oldScore = currentScore;
        SANAIteration();
        if (abs(oldScore-currentScore) < 0.00001) ++idleCount;
        else idleCount = 0;
        ++iter;
    }
    trackProgress(iter);
    cout<<"Hill climbing took "<<hillTimer.elapsedString()<<endl;
}

void SANA::describeParameters(ostream& sout) {
    sout << "Temperature schedule:" << endl;
    sout << "T_initial: " << TInitial << endl;
    sout << "T_decay: " << TDecay << endl;
    sout << "Optimize: " << endl;
    MC->printWeights(sout);
    if (!usingIterations) sout << "Execution time: " << minutes << "m" << endl;
    else sout << "Iterations Run: " << maxIterations << "00,000,000" << endl; //it's in hundred millions
}

string SANA::fileNameSuffix(const Alignment& Al) const {
    return "_" + extractDecimals(eval(Al),3);
}

double SANA::temperatureFunction(long long int iter, double TInitial, double TDecay) {
    if (constantTemp) return TInitial;
    double fraction;
    if (usingIterations) fraction = iter / (100000000.0 * maxIterations);
    else fraction = iter / (minutes * 60.0 * getIterPerSecond());
    return TInitial * exp(-TDecay * fraction);
}

double SANA::acceptingProbability(double energyInc, double Temperature) {
    return energyInc >= 0 ? 1 : exp(energyInc/Temperature);
}

double SANA::trueAcceptingProbability() {
    return pBadBufferSum/(double) numPBadsInBuffer;
}

//trueAcceptingProbability can give incorrect probabilities (even negative) if the pbads in the buffer are small enough
//due to accumulated precision errors of adding and subtracting tiny values
double SANA::slowTrueAcceptingProbability() {
    double sum = 0;
    for (int i = 0; i < numPBadsInBuffer; i++) sum += pBadBuffer[i];
    return sum/(double) numPBadsInBuffer;
}

double SANA::eval(const Alignment& Al) const { return MC->eval(Al); }

void sigIntHandler(int s) {
    string line;
    int c = -1;
    do {
        cerr<<"Select an option (0 - 3):"<<endl<<"  (0) Do nothing and continue"<<endl<<"  (1) Exit"<<endl
            <<"  (2) Save Alignment and Exit"<<endl<<"  (3) Save Alignment and Continue"<<endl<<">> ";
        cin >> c;
        if (cin.eof()) exit(0);
        if (cin.fail()) {
            c = -1;
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
        }
        if      (c == 0) cerr << "Continuing..." << endl;
        else if (c == 1) exit(0);
        else if (c == 2) SANA::interrupt = true;
        else if (c == 3) SANA::saveAlignment = true;
    } while (c < 0 || c > 3);    
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
    Report::saveReport(*G1, *G2, A, *MC, this, out, false);
    Report::saveLocalMeasures(*G1, *G2, A, *MC, this, local);
    cout << "Alignment saved. SANA will now continue." << endl;
}

void SANA::SANAIteration() {
    ++iterationsPerformed;
    uint index = randColorWeightedByNumNbrs();
    uint colorId = activeColorIds[index];
    double p = randomReal(gen);
    if (p < changeProbByColor[index]) {
        if (G2->numNodesWithColor(colorId) <= 1) { //this shouldn't happen, but happened once
            //printing this to trace back the bug if it happens again
            //remove nested if after bug is fixed
            cerr<<"p: "<<p<<" index: "<<index<<" colorId: "<<colorId
                <<"changeProbByColor[index]: "<<changeProbByColor[index]<<endl;
            G1->debugPrint();
            G2->debugPrint();
        }
        performChange(colorId);
    } else {
        performSwap(colorId);
    }
}

void SANA::performChange(uint colorId) {
    assert(G2->numNodesWithColor(colorId) > 1);
    
    uint source = G1->getRandomNodeWithColor(colorId);
    uint oldTarget = A[source];
    uint unassignedVecIndex = randInt(0, unassignedG2NodesByColor[colorId].size()-1);
    uint newTarget = unassignedG2NodesByColor[colorId][unassignedVecIndex];

    //added this dummy initialization to shut compiler warning -Nil
    unsigned oldOldTargetDeg = 0, oldNewTargetDeg = 0, oldMs3Denom = 0;

    if (needMS3) {
        oldOldTargetDeg = MultiS3::totalDegrees[oldTarget];
        oldNewTargetDeg = MultiS3::totalDegrees[newTarget];
        oldMs3Denom = MultiS3::denom;
    }
    int newAligEdges           = (needAligEdges or needSec) ? aligEdges + aligEdgesIncChangeOp(source, oldTarget, newTarget) : -1;
    double newEdSum            = needEd ? edSum + edgeDifferenceIncChangeOp(source, oldTarget, newTarget) : -1;
    double newErSum            = needEr ? erSum + edgeRatioIncChangeOp(source, oldTarget, newTarget) : -1;
    double newSquaredAligEdges = needSquaredAligEdges ? squaredAligEdges + squaredAligEdgesIncChangeOp(source, oldTarget, newTarget) : -1;
    double newExposedEdgesNumer= needExposedEdges ? EdgeExposure::numer + exposedEdgesIncChangeOp(source, oldTarget, newTarget) : -1;
    double newMS3Numer         = needMS3 ? MultiS3::numer + MS3IncChangeOp(source, oldTarget, newTarget) : -1;
    int newInducedEdges        = needInducedEdges ? inducedEdges + inducedEdgesIncChangeOp(source, oldTarget, newTarget) : -1;
    double newLocalScoreSum    = needLocal ? localScoreSum + localScoreSumIncChangeOp(sims, source, oldTarget, newTarget) : -1;
    double newWecSum           = needWec ? wecSum + WECIncChangeOp(source, oldTarget, newTarget) : -1;
    double newJsSum            = needJs ? jsSum + JSIncChangeOp(source, oldTarget, newTarget) : -1;
    double newEwecSum          = needEwec ? ewecSum + EWECIncChangeOp(source, oldTarget, newTarget) : -1;
    double newNcSum            = needNC ? ncSum + ncIncChangeOp(source, oldTarget, newTarget) : -1;

    map<string, double> newLocalScoreSumMap;
    if (needLocal) {
        newLocalScoreSumMap = map<string, double>(localScoreSumMap);
        for (auto &item : newLocalScoreSumMap)
            item.second += localScoreSumIncChangeOp(localSimMatrixMap[item.first], source, oldTarget, newTarget);
    }

    double newCurrentScore = 0;
    bool makeChange = scoreComparison(newAligEdges, newInducedEdges,
            newLocalScoreSum, newWecSum, newJsSum, newNcSum, newCurrentScore, newEwecSum,
            newSquaredAligEdges, newExposedEdgesNumer, newMS3Numer, newEdSum, newErSum);

#ifdef CORES
    // Statistics on the emerging core alignment.
    // only update pBad if is nonzero; reuse previous nonzero pBad if the current one is zero.
    uint betterHole = wasBadMove ? oldTarget : newTarget;

    double pBad = trueAcceptingProbability();
    if (pBad <= 0 || myNan(pBad)) pBad = LOW_PBAD_LIMIT;
#ifdef UNWEIGHTED_CORES
    numPegSamples[source]++;
    pegHoleFreq[source][betterHole]++;
#endif
    totalWeightedPegWeight_pBad[source] += pBad;
    weightedPegHoleFreq_pBad[source][betterHole] += pBad;
    totalWeightedPegWeight_1mpBad[source] += 1-pBad;
    weightedPegHoleFreq_1mpBad[source][betterHole] += 1-pBad;
#endif

    if (makeChange) {
        A[source] = newTarget;
        unassignedG2NodesByColor[colorId][unassignedVecIndex] = oldTarget;
        assignedNodesG2[oldTarget] = false;
        assignedNodesG2[newTarget] = true;
        aligEdges                     = newAligEdges;
        edSum                         = newEdSum;
        erSum                         = newErSum;
        inducedEdges                  = newInducedEdges;
        localScoreSum                 = newLocalScoreSum;
        wecSum                        = newWecSum;
        ewecSum                       = newEwecSum;
        ncSum                         = newNcSum;
        if (needLocal) localScoreSumMap = newLocalScoreSumMap;
        currentScore                  = newCurrentScore;
        EdgeExposure::numer           = newExposedEdgesNumer;
        squaredAligEdges              = newSquaredAligEdges;
        MultiS3::numer                = newMS3Numer;
    }
    else if (needMS3) {
        MultiS3::totalDegrees[oldTarget] = oldOldTargetDeg;
        MultiS3::totalDegrees[newTarget] = oldNewTargetDeg;
        MultiS3::denom = oldMs3Denom;
    }
}

void SANA::performSwap(uint colorId) {
    assert(G1->numNodesWithColor(colorId) > 1);
    uint source1 = G1->getRandomNodeWithColor(colorId);
    
    uint source2;
    for (uint i = 0; i < 1000; i++) {
        source2 = G1->getRandomNodeWithColor(colorId);
        if (source1 != source2) break;
    }
    assert(source1 != source2);
    uint target1 = A[source1], target2 = A[source2];
    
    //added this dummy initialization to shut compiler warning -Nil
    unsigned oldTarget1Deg = 0, oldTarget2Deg = 0, oldMs3Denom = 0;

    if (needMS3) {
        oldTarget1Deg = MultiS3::totalDegrees[target1];
        oldTarget2Deg = MultiS3::totalDegrees[target2];
        oldMs3Denom = MultiS3::denom;
    }

    int newAligEdges           = (needAligEdges or needSec) ? aligEdges + aligEdgesIncSwapOp(source1, source2, target1, target2) : -1;
    double newSquaredAligEdges = needSquaredAligEdges ? squaredAligEdges + squaredAligEdgesIncSwapOp(source1, source2, target1, target2) : -1;
    double newExposedEdgesNumer= needExposedEdges ? EdgeExposure::numer + exposedEdgesIncSwapOp(source1, source2, target1, target2) : -1;
    double newMS3Numer         = needMS3 ? MultiS3::numer + MS3IncSwapOp(source1, source2, target1, target2) : -1;
    double newWecSum           = needWec ? wecSum + WECIncSwapOp(source1, source2, target1, target2) : -1;
    double newJsSum            = needJs ? jsSum + JSIncSwapOp(source1, source2, target1, target2) : -1;
    double newEwecSum          = needEwec ? ewecSum + EWECIncSwapOp(source1, source2, target1, target2) : -1;
    double newNcSum            = needNC ? ncSum + ncIncSwapOp(source1, source2, target1, target2) : -1;
    double newLocalScoreSum    = needLocal ? localScoreSum + localScoreSumIncSwapOp(sims, source1, source2, target1, target2) : -1;
    double newEdSum            = needEd ? edSum + edgeDifferenceIncSwapOp(source1, source2, target1, target2) : -1;
    double newErSum            = needEr ? erSum + edgeRatioIncSwapOp(source1, source2, target1, target2) : -1;

    map<string, double> newLocalScoreSumMap;
    if (needLocal) {
        newLocalScoreSumMap = map<string, double>(localScoreSumMap);
        for (auto &item : newLocalScoreSumMap)
            item.second += localScoreSumIncSwapOp(localSimMatrixMap[item.first], source1, source2, target1, target2);
    }

    double newCurrentScore = 0;
    bool makeChange = scoreComparison(newAligEdges, inducedEdges, newLocalScoreSum,
                newWecSum, newJsSum, newNcSum, newCurrentScore, newEwecSum, newSquaredAligEdges,
                newExposedEdgesNumer, newMS3Numer, newEdSum, newErSum);

#ifdef CORES
        // Statistics on the emerging core alignment.
        // only update pBad if it's nonzero; reuse previous nonzero pBad if the current one is zero.
        double pBad = trueAcceptingProbability();
        if (pBad <= 0 || myNan(pBad)) pBad = LOW_PBAD_LIMIT;

        uint betterDest1 = wasBadMove ? target1 : target2;
        uint betterDest2 = wasBadMove ? target2 : target1;
#ifdef UNWEIGHTED_CORES
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
        A[source1]          = target2;
        A[source2]          = target1;
        aligEdges           = newAligEdges;
        edSum               = newEdSum;
        erSum               = newErSum;
        localScoreSum       = newLocalScoreSum;
        wecSum              = newWecSum;
        ewecSum             = newEwecSum;
        ncSum               = newNcSum;
        currentScore        = newCurrentScore;
        squaredAligEdges    = newSquaredAligEdges;
        EdgeExposure::numer = newExposedEdgesNumer;
        MultiS3::numer      = newMS3Numer;
        if (needLocal) localScoreSumMap = newLocalScoreSumMap;
    } else if (needMS3) {
        MultiS3::totalDegrees[target1] = oldTarget1Deg;
        MultiS3::totalDegrees[target2] = oldTarget2Deg;
        MultiS3::denom = oldMs3Denom;
    }
}

bool SANA::scoreComparison(double newAligEdges, double newInducedEdges,
        double newLocalScoreSum, double newWecSum, double newJsSum, double newNcSum, double& newCurrentScore,
        double newEwecSum, double newSquaredAligEdges, double newExposedEdgesNumer, double newMS3Numer,
        double newEdgeDifferenceSum, double newEdgeRatioSum) {
    bool makeChange = false;
    wasBadMove = false;
    double badProbability = 0;

    switch (scoreAggr) {
    case ScoreAggregation::sum:
    {
        newCurrentScore += ecWeight * (newAligEdges / g1Edges);
        newCurrentScore += edWeight * EdgeDifference::adjustSumToTargetScore(newEdgeDifferenceSum, pairsCount);
        newCurrentScore += erWeight * EdgeRatio::adjustSumToTargetScore(newEdgeRatioSum, pairsCount);
        newCurrentScore += s3Weight * (newAligEdges / (g1Edges + newInducedEdges - newAligEdges));
        newCurrentScore += icsWeight * (newAligEdges / newInducedEdges);
        newCurrentScore += secWeight * (newAligEdges / g1Edges + newAligEdges / g2Edges)*0.5;
        newCurrentScore += localWeight * (newLocalScoreSum / n1);
        newCurrentScore += wecWeight * (newWecSum / (2 * g1Edges));
        newCurrentScore += jsWeight * (newJsSum);
        newCurrentScore += ewecWeight * (newEwecSum);
        newCurrentScore += ncWeight * (newNcSum / trueA.back());
#ifdef MULTI_PAIRWISE
        newCurrentScore += mecWeight * (newAligEdges / (g1WeightedEdges + g2WeightedEdges));
        newCurrentScore += sesWeight * newSquaredAligEdges / (double)SquaredEdgeScore::getDenom();
        newCurrentScore += eeWeight * (1 - (newExposedEdgesNumer / (double)EdgeExposure::denom));
        if (MultiS3::_type==1) MultiS3::denom = newExposedEdgesNumer;
        newCurrentScore += ms3Weight * (double)newMS3Numer / (double)MultiS3::denom / (double)NUM_GRAPHS;
#endif
        energyInc = newCurrentScore - currentScore;
        wasBadMove = energyInc < 0;
        //using max and min here because with extremely low temps I was seeing invalid probabilities
        //note: I did not make this change for the other types of ScoreAggregation::  -Nil
        badProbability = max(0.0, min(1.0, exp(energyInc / Temperature)));
        makeChange = (energyInc >= 0 or randomReal(gen) <= badProbability);
        break;
    }
    case ScoreAggregation::product:
    {
        newCurrentScore = 1;
        newCurrentScore *= ecWeight * (newAligEdges / g1Edges);
        newCurrentScore *= s3Weight * (newAligEdges / (g1Edges + newInducedEdges - newAligEdges));
        newCurrentScore *= icsWeight * (newAligEdges / newInducedEdges);
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
    case ScoreAggregation::max:
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
    case ScoreAggregation::min:
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
    case ScoreAggregation::inverse:
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
    case ScoreAggregation::maxFactor:
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
    }

    //if (wasBadMove && (iterationsPerformed % 512 == 0 || (iterationsPerformed % 32 == 0))) {
    //the above will never be true in the case of iterationsPerformed never being changed so that it doesn't greatly
    // slow down the program if for some reason iterationsPerformed doesn't need to be changed.
    if (wasBadMove) { // I think Dillon was wrong above, just do it always - WH
        if (numPBadsInBuffer == PBAD_CIRCULAR_BUFFER_SIZE) {
            pBadBufferIndex = (pBadBufferIndex == PBAD_CIRCULAR_BUFFER_SIZE ? 0 : pBadBufferIndex);
            pBadBufferSum -= pBadBuffer[pBadBufferIndex];
            pBadBuffer[pBadBufferIndex] = badProbability;
        } else {
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
    if (G1->hasSelfLoop(source)) {
        if (G2->hasSelfLoop(oldTarget)) res--;
        if (G2->hasSelfLoop(newTarget)) res++;
    }
    for (uint nbr : G1->adjLists[source]) {
        if (nbr != source) {
            res -= G2->edgeWeight(oldTarget, A[nbr]);
            res += G2->edgeWeight(newTarget, A[nbr]);
        }
    }
    return res;
}

int SANA::aligEdgesIncSwapOp(uint source1, uint source2, uint target1, uint target2) {
#ifdef FLOAT_WEIGHTS
    return 0; //not applicable
#else
    int res = 0;
    if (G1->hasSelfLoop(source1)) {
        if (G2->hasSelfLoop(target1)) res--;
        if (G2->hasSelfLoop(target2)) res++;
    }
    for (uint nbr : G1->adjLists[source1]) {
        if (nbr != source1) {
            res -= G2->edgeWeight(target1, A[nbr]);
            res += G2->edgeWeight(target2, A[nbr]);
        }
    }
    if (G1->hasSelfLoop(source2)) {
        if (G2->hasSelfLoop(target2)) res--;
        if (G2->hasSelfLoop(target1)) res++;
    }
    for (uint nbr : G1->adjLists[source2]) {
        if (nbr != source2) {
            res -= G2->edgeWeight(target2, A[nbr]);
            res += G2->edgeWeight(target1, A[nbr]);
        }
    }

    //address case swapping between adjacent nodes with adjacent images:
#ifdef MULTI_PAIRWISE
    //why set the least-significant bit to 0?
    //this kind of bit manipulation needs a comment clarification -Nil
    res += (-1 << 1) & (G1->edgeWeight(source1, source2) +
                        G2->edgeWeight(target1, target2));
#else
    if (G1->hasEdge(source1, source2) and G2->hasEdge(target1, target2)) res += 2;
#endif

    return res;
#endif // FLOAT_WEIGHTS
}

static double getRatio(double w1, double w2) {
    double r;
    if (w1==0 && w2==0) r=1;
    else if (abs(w1)<abs(w2)) r=w1/w2;
    else r=w2/w1;
    // At this point, r can be in [-1,1], but we want it [0,1], so add 1 and divide by 2
    r = (r+1)/2;
    assert(r>=0 && r<=1);
    return r;
}

/* We swap the mapping of two nodes source1 and source2
 * We can first handle source1, then do the same with source2
 * Subtract old edge difference with edge (source1, target1)
 * Add new edge difference with edge (source1, target2) */
double SANA::edgeDifferenceIncSwapOp(uint source1, uint source2, uint target1, uint target2) {
    if (source1 == source2) return 0;
    // Handle source1
    double edgeDifferenceIncDiff = 0;
    double c = 0;
    for (uint node2 : G1->adjLists[source1]) {
        double y = -abs(G1->edgeWeight(source1, node2) - G2->edgeWeight(target1, A[node2])) - c;
        double t = edgeDifferenceIncDiff + y;
        c = (t - edgeDifferenceIncDiff) - y;
        edgeDifferenceIncDiff = t;

        // Determine the new target node for node2
        uint node2Target = 0;
        if (node2 == source1) node2Target = target2;
        else if (node2 == source2) node2Target = target1;
        else node2Target = A[node2];

        y = +abs(G1->edgeWeight(source1, node2) - G2->edgeWeight(target2, node2Target)) - c;
        t = edgeDifferenceIncDiff + y;
        c = (t - edgeDifferenceIncDiff) - y;
        edgeDifferenceIncDiff = t;
    }
    // Handle source2
    for (uint node2 : G1->adjLists[source2]) {
        if (node2 == source1) continue;
        double y = -abs(G1->edgeWeight(source2, node2) - G2->edgeWeight(target2, A[node2])) - c;
        double t = edgeDifferenceIncDiff + y;
        c = (t - edgeDifferenceIncDiff) - y;
        edgeDifferenceIncDiff = t;

        uint node2Target = (node2 == source2 ? target1 : A[node2]);
        y = +abs(G1->edgeWeight(source2, node2) - G2->edgeWeight(target1, node2Target)) - c;
        t = edgeDifferenceIncDiff + y;
        c = (t - edgeDifferenceIncDiff) - y;
        edgeDifferenceIncDiff = t;
    }
    return edgeDifferenceIncDiff;
}

double SANA::edgeRatioIncSwapOp(uint source1, uint source2, uint target1, uint target2) {
    if (source1 == source2) return 0;
    double edgeRatioIncDiff = 0;
    double c = 0;
    // Subtract source1-target1, add source1-target2
    for (uint node2 : G1->adjLists[source1]) {
        double r = getRatio(G1->edgeWeight(source1, node2), G2->edgeWeight(target1, A[node2]));
        double y = -r - c;
        double t = edgeRatioIncDiff + y;
        c = (t - edgeRatioIncDiff) - y;
        edgeRatioIncDiff = t;

        uint node2Target = 0;
        if (node2 == source1) node2Target = target2;
        else if (node2 == source2) node2Target = target1;
        else node2Target = A[node2];

        r = getRatio(G1->edgeWeight(source1, node2), G2->edgeWeight(target2, node2Target));
        y = r - c;
        t = edgeRatioIncDiff + y;
        c = (t - edgeRatioIncDiff) - y;
        edgeRatioIncDiff = t;
    }
   // Subtract source2-target2, add source2-target1
   for (uint node2 : G1->adjLists[source2]) {
        if (node2 == source1) continue;
        double r = getRatio(G1->edgeWeight(source2, node2), G2->edgeWeight(target2, A[node2]));
        double y = -r - c;
        double t = edgeRatioIncDiff + y;
        c = (t - edgeRatioIncDiff) - y;
        edgeRatioIncDiff = t;

        uint node2Target = (node2 == source2 ? target1 : A[node2]);
        r = getRatio(G1->edgeWeight(source2, node2), G2->edgeWeight(target1, node2Target));
        y = r - c;
        t = edgeRatioIncDiff + y;
        c = (t - edgeRatioIncDiff) - y;
        edgeRatioIncDiff = t;
    }
    return edgeRatioIncDiff;
}


double SANA::edgeDifferenceIncChangeOp(uint source, uint oldTarget, uint newTarget) {
    double edgeDifferenceIncDiff = 0;
    double c = 0;
    for (uint node2 : G1->adjLists[source]) {
        double y = -abs(G1->edgeWeight(source, node2) - G2->edgeWeight(oldTarget, A[node2])) - c;
        double t = edgeDifferenceIncDiff + y;
        c = (t - edgeDifferenceIncDiff) - y;
        edgeDifferenceIncDiff = t;

        uint node2Target = node2 == source ? newTarget : A[node2];
        y = +abs(G1->edgeWeight(source, node2) - G2->edgeWeight(newTarget, node2Target)) - c;
        t = edgeDifferenceIncDiff + y;
        c = (t - edgeDifferenceIncDiff) - y;
        edgeDifferenceIncDiff = t;
    }
    return edgeDifferenceIncDiff;
}

double SANA::edgeRatioIncChangeOp(uint source, uint oldTarget, uint newTarget) {
    double edgeRatioIncDiff = 0;
    double c = 0;
    for (uint node2 : G1->adjLists[source]) {
        double r = getRatio(G1->edgeWeight(source, node2), G2->edgeWeight(oldTarget, A[node2]));
        double y = -r - c;
        double t = edgeRatioIncDiff + y;
        c = (t - edgeRatioIncDiff) - y;
        edgeRatioIncDiff = t;

        uint node2Target = node2 == source ? newTarget : A[node2];
        r = getRatio(G1->edgeWeight(source, node2), G2->edgeWeight(newTarget, node2Target));
        y = r - c;
        t = edgeRatioIncDiff + y;
        c = (t - edgeRatioIncDiff) - y;
        edgeRatioIncDiff = t;
    }
    return edgeRatioIncDiff;
}

// UGLY GORY HACK BELOW!! Sometimes the edgeVal is crazily wrong, like way above 1,000, when it
// cannot possibly be greater than the number of networks we're aligning when MULTI_PAIRWISE is on.
// It happens only rarely, so here I ask if the edgeVal is less than 1,000; if it's less than 1,000
// then we assume it's OK, otherwise we just ignore this edge entirely and say the diff is 0.
// Second problem: even if the edgeVal is correct, I couldn't seem to figure out the difference
// between the value of this ladder and the ladder with one edge added or removed.  Mathematically
// it should be edgeVal^2 - (edgeVal+1)^2 which is (2e + 1), but for some reason I had to make
// it 2*(e+1).  That seemed to work better.  So yeah... big ugly hack.
static int _edgeVal;
#define SQRDIFF(i,j) ((_edgeVal=G2->edgeWeight(i, A[j])), 2*_edgeVal + 1)
int SANA::squaredAligEdgesIncChangeOp(uint source, uint oldTarget, uint newTarget) {
    int res = 0;
    for (uint nbr : G1->adjLists[source]) {
        // Account for uint edges? Or assume smaller graph is edge value 1?
        int diff = SQRDIFF(oldTarget, nbr);
        // assert(fabs(diff)<1100);
        res -= diff;// >0?diff:0;
        diff = SQRDIFF(newTarget, nbr);
        // assert(fabs(diff)<1100);
        res += diff;// >0?diff:0;
    }
    return res;
}

int SANA::squaredAligEdgesIncSwapOp(uint source1, uint source2, uint target1, uint target2) {
    int res = 0;
    for (uint nbr : G1->adjLists[source1]) {
        int diff = SQRDIFF(target1, nbr);
        // assert(fabs(diff)<1100);
        res -= diff;// >0?diff:0;
        diff = SQRDIFF(target2, nbr);
        if (target2 == A[nbr]) diff = 0;
        // assert(fabs(diff)<1100);
        res += diff;// >0?diff:0;
    }
    for (uint nbr : G1->adjLists[source2]) {
        int diff = SQRDIFF(target2, nbr);
        // assert(fabs(diff)<1100);
        res -= diff;// >0?diff:0;
        diff = SQRDIFF(target1, nbr);
        if (target1 == A[nbr]) diff = 0;
        // assert(fabs(diff)<1100);
        res += diff;// >0?diff:0;
    }
    // How to do for squared?
    // address case swapping between adjacent nodes with adjacent images:
    if (G1->hasEdge(source1, source2) and G2->hasEdge(target1, target2))
        res += 2 * SQRDIFF(target1,source2);
    return res;
}

int SANA::exposedEdgesIncChangeOp(uint source, uint oldTarget, uint newTarget) {
    int res = 0;
    for (uint nbr : G1->adjLists[source]) {
        if (not G2->hasEdge(oldTarget, A[nbr])) --res;
        if (not G2->hasEdge(newTarget, A[nbr])) ++res;
    }
    return res;
}

int SANA::exposedEdgesIncSwapOp(uint source1, uint source2, uint target1, uint target2) {
    int res = 0;
    for (uint nbr : G1->adjLists[source1]) {
        if (not G2->hasEdge(target1, A[nbr])) --res;
        if (not G2->hasEdge(target2, A[nbr])) ++res;
    }
    for (uint nbr : G1->adjLists[source2]) {
        if (not G2->hasEdge(target2, A[nbr])) --res;
        if (not G2->hasEdge(target1, A[nbr])) ++res;
    }
    return res;
}

// Return the change in NUMERATOR of MS3
int SANA::MS3IncChangeOp(uint source, uint oldTarget, uint newTarget) {
    if (MultiS3::_type==1) {
        //denom is ee, computed elsewhere
        int res = 0;
        for (uint nbr : G1->adjLists[source]) {
            int diff = G2->edgeWeight(oldTarget, A[nbr]) + 1;
            res -= (diff==1?0:diff);
            diff = G2->edgeWeight(newTarget, A[nbr]) + 1;
            res += (diff==1?0:diff);
        }
        return res;
    }
    int res = 0;
    unsigned oldOldTargetDeg = MultiS3::totalDegrees[oldTarget];
    unsigned oldNewTargetDeg = MultiS3::totalDegrees[newTarget];

    if (G1->hasSelfLoop(source)) {
        if (G2->hasSelfLoop(oldTarget)) --res;
        if (G2->hasSelfLoop(newTarget)) ++res;
    }
    for (uint nbr : G1->adjLists[source]) {
        if (nbr != source) {
            --MultiS3::totalDegrees[oldTarget];
            ++MultiS3::totalDegrees[newTarget];
            res -= G2->edgeWeight(oldTarget, A[nbr]);
            res += G2->edgeWeight(newTarget, A[nbr]);
        }
    }
    if (oldOldTargetDeg > 0 and !MultiS3::totalDegrees[oldTarget]) MultiS3::denom -= 1;
    if (oldNewTargetDeg > 0 and !MultiS3::totalDegrees[newTarget]) MultiS3::denom += 1;
    return res;
}

// Return change in NUMERATOR only
int SANA::MS3IncSwapOp(uint source1, uint source2, uint target1, uint target2) {
    if (MultiS3::_type==1) { //denom is ee
        int res = 0;
        for (uint nbr : G1->adjLists[source1]) {
            int diff = G2->edgeWeight(target1, A[nbr]) + 1;//SQRDIFF(target1, nbr);
            res -= (diff == 1 ? 0 : diff);
            diff = G2->edgeWeight(target2, A[nbr]) + 1;//SQRDIFF(target2, nbr);
            if (target2 == A[nbr]) diff = 0;
            res += (diff == 1 ? 0 : diff);
        }
        for (uint nbr : G1->adjLists[source2]) {
            int diff = G2->edgeWeight(target2, A[nbr]) + 1;//SQRDIFF(target2, nbr);
            res -= (diff == 1 ? 0 : diff);
            diff = G2->edgeWeight(target1, A[nbr]) + 1;//SQRDIFF(target1, nbr);
            if (target1 == A[nbr]) diff = 0;
            res += (diff == 1 ? 0 : diff);
        }
        if (G2->hasEdge(target1, target2) and  G1->hasEdge(source1, source2)) {
            int diff = G2->edgeWeight(target1, A[source2]) + 1;
            res += 2*(diff == 1 ? 0 : diff);
        }
        return res;
    }

    int res = 0;
    uint oldTarget1Deg = MultiS3::totalDegrees[target1];
    uint oldTarget2Deg = MultiS3::totalDegrees[target2];
    if (G1->hasSelfLoop(source1)) {
        if (G2->hasSelfLoop(target1)) --res;
        if (G2->hasSelfLoop(target2)) ++res;
    }
    if (G1->hasSelfLoop(source2)) {
        if (G2->hasSelfLoop(target1)) --res;
        if (G2->hasSelfLoop(target2)) ++res;
    }
    for (uint nbr : G1->adjLists[source1]) {
        if (nbr != source1) {
            --MultiS3::totalDegrees[target1];
            ++MultiS3::totalDegrees[target2];
            res -= G2->edgeWeight(target1, A[nbr]);
            res += G2->edgeWeight(target2, A[nbr]);
        }
    }
    for (uint nbr : G1->adjLists[source2]) {
        if (nbr != source1) {
            --MultiS3::totalDegrees[target2];
            ++MultiS3::totalDegrees[target1];
            res -= G2->edgeWeight(target2, A[nbr]);
            res += G2->edgeWeight(target1, A[nbr]);
        }
    }
    if (oldTarget1Deg > 0 && !MultiS3::totalDegrees[target1]) MultiS3::denom -= 1;
    if (oldTarget2Deg > 0 && !MultiS3::totalDegrees[target2]) MultiS3::denom += 1;
    return res;
}

int SANA::inducedEdgesIncChangeOp(uint source, uint oldTarget, uint newTarget) {
    int res = 0;
    for (uint nbr : G2->adjLists[oldTarget]) res -= assignedNodesG2[nbr];
    for (uint nbr : G2->adjLists[newTarget]) res += assignedNodesG2[nbr];
    res -= G2->edgeWeight(oldTarget, newTarget); //address case changing between adjacent nodes:
    return res;
}

double SANA::localScoreSumIncChangeOp(vector<vector<float>> const & sim, uint const & source, uint const & oldTarget, uint const & newTarget) {
    return sim[source][newTarget] - sim[source][oldTarget];
}

double SANA::localScoreSumIncSwapOp(vector<vector<float>> const & sim, uint const & source1, uint const & source2, uint const & target1, uint const & target2) {
    return sim[source1][target2] - sim[source1][target1] + sim[source2][target1] - sim[source2][target2];
}

double SANA::JSIncChangeOp(uint source, uint oldTarget, uint newTarget) {
    if (jsWeight == 0) return 0;

    //eval newJsSum
    //update alignedByNode with source and source neighbours using oldTarget and newTarget

    // eval for source from scratch
    uint sourceOldAlingedEdges = alignedByNode[source];
    uint sourceAlignedEdges = 0;
    vector<uint> sourceNeighbours = G1->adjLists[source];
    for (uint nbr : sourceNeighbours) {
        uint neighbourAlignedTo = A[nbr]; 
        sourceAlignedEdges += G2->edgeWeight(newTarget, neighbourAlignedTo);
    }
    alignedByNode[source] = sourceAlignedEdges;
    //update newJsSum
    uint sourceTotalEdges = sourceNeighbours.size();
    double change = ((sourceAlignedEdges - sourceOldAlingedEdges)/(double)sourceTotalEdges);

    // for each source neighbour update do iterative changes to the jsAlingedByNode vector
    // in each update get the G2mapping of neighbour and then check if edge was aligned by oldTarget to G2mapping and reduce score if newTarget to G2mapping doesnt exist
    // increase score if oldTarget to G2 mapping edge didnt exist but newTarget to G2 mapping does
    //no changes other wise
    for (uint nbr : sourceNeighbours) {
        uint neighbourAlignedTo = A[nbr];
        uint neighbourOldAlignedEdges = alignedByNode[nbr];
        uint neighbourTotalEdges = G1->adjLists[nbr].size();
        alignedByNode[nbr] -= G2->edgeWeight(oldTarget, neighbourAlignedTo);
        alignedByNode[nbr] += G2->edgeWeight(newTarget, neighbourAlignedTo);
        //update newJsSum
        change += ((alignedByNode[nbr] - neighbourOldAlignedEdges)/(double)neighbourTotalEdges);
    }
    return change;
}

double SANA::JSIncSwapOp(uint source1, uint source2, uint target1, uint target2) {
    if (jsWeight == 0) return 0;

    //eval swap as two sources and then loop neighbours

    uint source1OldAlingedEdges = alignedByNode[source1];
    uint source1AlignedEdges = 0;
    vector<uint> source1Neighbours = G1->adjLists[source1];
    uint source1TotalEdges = source1Neighbours.size();

    uint source2OldAlingedEdges = alignedByNode[source2];
    uint source2AlignedEdges = 0;
    vector<uint> source2Neighbours = G1->adjLists[source2];
    uint source2TotalEdges = source2Neighbours.size();

    // eval for source1 from sratch
    for (uint nbr : source1Neighbours) {
        uint neighbourAlignedTo = A[nbr];
        // if (G2->edgeWeight(target2, neighbourAlignedTo) == true) {
        //     source1AlignedEdges += 1;
        // }
        source1AlignedEdges = G2->edgeWeight(target2, neighbourAlignedTo);
    }
    alignedByNode[source1] = source1AlignedEdges;

    double change = ((source1AlignedEdges - source1OldAlingedEdges)/(double)source1TotalEdges);

    //for each source neighbour update do iterative changes to the jsAlingedByNode vector
    //in each update get the G2mapping of neighbour and then check if edge was aligned by
    //oldTarget to G2mapping and reduce score if newTarget to G2mapping doesnt exist
    //increase score if oldTarget to G2 mapping edge didnt exist but newTarget to G2 mapping does
    //no changes other wise
    for (uint nbr : source1Neighbours) {
        uint neighbourAlignedTo = A[nbr];
        uint neighbourOldAlignedEdges = alignedByNode[nbr];
        uint neighbourTotalEdges = G1->adjLists[nbr].size();
        if (std::find (source1Neighbours.begin(), source1Neighbours.end(), nbr) == source1Neighbours.end()) {
            alignedByNode[nbr] -= G2->edgeWeight(target1, neighbourAlignedTo);
            alignedByNode[nbr] += G2->edgeWeight(target2, neighbourAlignedTo);
            //update newJsSum
            change += ((alignedByNode[nbr] - neighbourOldAlignedEdges)/(double)neighbourTotalEdges);
        }
    }

    // eval for source2 from scratch
    for (uint nbr : source2Neighbours) {
        uint neighbourAlignedTo = A[nbr];
        source2AlignedEdges += G2->edgeWeight(target1, neighbourAlignedTo);
    }
    alignedByNode[source2] = source2AlignedEdges;
    change += ((source2AlignedEdges - source2OldAlingedEdges)/(double)source2TotalEdges);

    // for each source neighbour update do iterative changes to the jsAlingedByNode vector
    // in each update get the G2mapping of neighbour and then check if edge was aligned by oldTarget to G2mapping and reduce score if newTarget to G2mapping doesnt exist
    // increase score if oldTarget to G2 mapping edge didnt exist but newTarget to G2 mapping does
    //no changes other wise
    for (uint nbr : source2Neighbours) {
        uint neighbourAlignedTo = A[nbr];
        uint neighbourOldAlignedEdges = alignedByNode[nbr];
        uint neighbourTotalEdges = G1->adjLists[nbr].size();
        if (std::find (source1Neighbours.begin(), source1Neighbours.end(), nbr) == source1Neighbours.end()) {
            alignedByNode[nbr] -= G2->edgeWeight(target2, neighbourAlignedTo);
            alignedByNode[nbr] += G2->edgeWeight(target1, neighbourAlignedTo);
            //update newJsSum
            change += ((alignedByNode[nbr] - neighbourOldAlignedEdges)/(double)neighbourTotalEdges);
        }
    }

    //eval for common neighbours
    vector<uint> source1source2commonneighbours(source1Neighbours.size() + source2Neighbours.size());
    set_intersection(source1Neighbours.begin(), source1Neighbours.end(),
                     source2Neighbours.begin(), source2Neighbours.end(),
                     source1source2commonneighbours.begin());
    return change;
}

double SANA::WECIncChangeOp(uint source, uint oldTarget, uint newTarget) {
    double res = 0;
    for (uint nbr : G1->adjLists[source]) {
        if (G2->edgeWeight(oldTarget, A[nbr])) {
            res -= wecSims[source][oldTarget];
            res -= wecSims[nbr][A[nbr]];
        }
        if (G2->edgeWeight(newTarget, A[nbr])) {
            res += wecSims[source][newTarget];
            res += wecSims[nbr][A[nbr]];
        }
    }
    return res;
}

double SANA::WECIncSwapOp(uint source1, uint source2, uint target1, uint target2) {
    double res = 0;
    for (uint nbr : G1->adjLists[source1]) {
        if (G2->edgeWeight(target1, A[nbr])) {
            res -= wecSims[source1][target1];
            res -= wecSims[nbr][A[nbr]];
        }
        if (G2->edgeWeight(target2, A[nbr])) {
            res += wecSims[source1][target2];
            res += wecSims[nbr][A[nbr]];
        }
    }
    for (uint nbr : G1->adjLists[source2]) {
        if (G2->edgeWeight(target2, A[nbr])) {
            res -= wecSims[source2][target2];
            res -= wecSims[nbr][A[nbr]];
        }
        if (G2->edgeWeight(target1, A[nbr])) {
            res += wecSims[source2][target1];
            res += wecSims[nbr][A[nbr]];
        }
    }
    if (G1->hasEdge(source1, source2) and G2->hasEdge(target1, target2)) {
        res += 2*wecSims[source1][target1];
        res += 2*wecSims[source2][target2];
    }
    return res;
}

double SANA::EWECIncChangeOp(uint source, uint oldTarget, uint newTarget) {
    return EWECSimCombo(source, newTarget) - EWECSimCombo(source, oldTarget);
}

double SANA::EWECIncSwapOp(uint source1, uint source2, uint target1, uint target2) {
    double score = EWECSimCombo(source1, target2) + EWECSimCombo(source2, target1) 
                 - EWECSimCombo(source1, target1) - EWECSimCombo(source2, target2);
    if (G1->hasEdge(source1, source2) and G2->hasEdge(target1, target2)) {
        score += ewec->getScore(ewec->getColIndex(target1, target2), 
                                ewec->getRowIndex(source1, source2))/(g1Edges); //correcting for missed edges when swapping 2 adjacent pairs
    }
    return score;
}

double SANA::EWECSimCombo(uint source, uint target) {
    double score = 0;
    for (uint nbr : G1->adjLists[source]) {
        if (G2->edgeWeight(target, A[nbr])) {
            int e1 = ewec->getRowIndex(source, nbr);
            int e2 = ewec->getColIndex(target, A[nbr]);
            score+=ewec->getScore(e2,e1);
        }
    }
    return score/(2*g1Edges);
}

int SANA::ncIncChangeOp(uint source, uint oldTarget, uint newTarget) {
    int change = 0;
    if (trueA[source] == oldTarget) change -= 1;
    if (trueA[source] == newTarget) change += 1;
    return change;
}

int SANA::ncIncSwapOp(uint source1, uint source2, uint target1, uint target2) {
    int change = 0;
    if (trueA[source1] == target1) change -= 1;
    if (trueA[source2] == target2) change -= 1;
    if (trueA[source1] == target2) change += 1;
    if (trueA[source2] == target1) change += 1;
    return change;
}

void SANA::trackProgress(long long int i, long long int maxIters) {
    if (!enableTrackProgress) return;
    double fractionTime = maxIters == -1 ? 0 : i/(double)maxIters;
    double elapsedTime = timer.elapsed();
    uint iterationsElapsed = iterationsPerformed-oldIterationsPerformed;
    if (elapsedTime == 0) oldTimeElapsed = 0;
    double ips = (iterationsElapsed/(elapsedTime-oldTimeElapsed));
    oldTimeElapsed = elapsedTime;
    oldIterationsPerformed = iterationsPerformed;
    cout<<i/iterationsPerStep<<" ("<<100*fractionTime<<"%,"<<elapsedTime<<"s): score = "<<currentScore;
    cout<< " ips = "<<ips<<", P("<<Temperature<<") = "<<acceptingProbability(avgEnergyInc, Temperature);
    cout<<", pBad = "<<trueAcceptingProbability()<<endl;

    bool checkScores = true;
    if (checkScores) {
        double realScore = eval(A);
        if (fabs(realScore-currentScore) > 0.00001) {
            cerr<<"internal error: incrementally computed score ("<<currentScore;
            cerr<<") is not correct ("<<realScore<<")"<<endl;
            currentScore = realScore;
        }
    }

    //code for estimating dynamic TDecay. The dynamic method uses linear interpolation to obtain an
    //an "ideal" P(bad) as a basis for SANA runs. If the current P(bad) is significantly different from 
    //our "ideal" P(bad), then decay is either "sped up" or "slowed down"
    if (dynamicTDecay) {
        int NSteps = 100;
        double fractionTime = (timer.elapsed()/(minutes*60));
        double lowIndex = floor(NSteps*fractionTime);
        double highIndex = ceil(NSteps*fractionTime);
        double betweenFraction = NSteps*fractionTime - lowIndex;
        double PLow = tau[lowIndex];
        double PHigh = tau[highIndex];
        double PBetween = PLow + betweenFraction * (PHigh - PLow);

        // if the ratio if off by more than a few percent, adjust.
        double ratio = acceptingProbability(avgEnergyInc, Temperature) / PBetween;

        //dynamicTDecayTime is never initialized, so I don't think this works
        if (abs(1-ratio) >= .01 and
            (ratio < 1 or dynamicTDecayTime > .2)) { //don't speed it up too soon
            double shouldBe = -log(avgEnergyInc/(TInitial*log(PBetween)))/(dynamicTDecayTime);
            if (dynamicTDecayTime == 0 or shouldBe != shouldBe or shouldBe <= 0)
                shouldBe = TDecay * (ratio >= 0 ? ratio*ratio : 0.5);
            cout<<"TDecay "<<TDecay<<" too ";
            cout<<(ratio < 1 ? "fast" : "slow")<<" shouldBe "<<shouldBe;
            TDecay = sqrt(TDecay * shouldBe); //geometric mean
            cout<<"; try "<<TDecay<<endl;
        }
    }
}

void SANA::setTInitial(double t) { TInitial = t; }
void SANA::setTFinal(double t) { TFinal = t; }
void SANA::setTDecayFromTempRange() { TDecay = -log(TFinal/TInitial); }
void SANA::setDynamicTDecay() { dynamicTDecay = true; }

double SANA::getIterPerSecond() {
    if (not initializedIterPerSecond) initIterPerSecond();
    return iterPerSecond;
}

void SANA::initIterPerSecond() {
    initializedIterPerSecond = true;
    cout << "Determining iteration speed...." << endl;
    double totalIps = 0.0;
    int ipsListSize = 0;
    if (ipsList.size() != 0) {
        for (pair<double,double> ipsPair : ipsList) {
            if (TFinal <= ipsPair.first && ipsPair.first <= TInitial) {
                totalIps+=ipsPair.second;
                ipsListSize+=1;
            }
        }
        totalIps = totalIps / (double) ipsListSize;
    } else {
        cout << "Since temperature schedule is provided, ips will be "
             << "calculated using constantTempIterations" << endl;
        long long int iter = 1E6;
        constantTempIterations(iter - 1);
        double res = iter/timer.elapsed();
        totalIps = res;
    }
    cout << "SANA does " << long(totalIps) << " iterations per second on average" << endl;
    iterPerSecond = totalIps;

    //what is this? can it be removed? -Nil
    uint integralMin = minutes;
    string folder = "cache-pbad/"+MC->toString()+"/progress_"+to_string(integralMin)+"/";
    string file = folder+G1->getName()+"_"+G2->getName()+"_0.csv";
    ofstream header(file);
    header<<"time,score,avgEnergyInc,Temperature,realTemp,pBad,lower,higher,timer"<<endl;
    header.close();
}

void SANA::constantTempIterations(long long int iterTarget) {
    initDataStructures();
    long long int iter;
    for (iter = 1; iter < iterTarget ; ++iter) {
        if (iter%iterationsPerStep == 0) trackProgress(iter);
        SANAIteration();
    }
    trackProgress(iter);
}

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
    initDataStructures(); //this initializes the timer and resets the pBad buffer
    bool verbose = (logLevel == 2); //print everything going on, for debugging purposes
    uint verbose_i = 0;
    if (verbose) cerr<<endl<<"****************************************"<<endl
                     <<"starting search for pBad for temp = "<<temp<<endl;
    while (not reachedEquilibrium) {
        SANAIteration();
        iter++;
        if (iter%sampleInterval == 0) {
            if (verbose) {
                cerr<<verbose_i<<" score: "<<currentScore<<" (avg pBad: "
                    <<slowTrueAcceptingProbability()<<")"<<endl;
                verbose_i++;
            }
            //circular buffer behavior
            //(since the buffer is tiny, the cost of shifting everything is negligible)
            scoreBuffer.push_back(currentScore);            
            if (scoreBuffer.size() > numScores) scoreBuffer.erase(scoreBuffer.begin());
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
                    cerr<<"scoreTrend = "<<scoreTrend<<endl;
                    if (reachedEquilibrium) {
                        cerr<<endl<<"Reached equilibrium"<<endl<<"scoreBuffer:"<<endl;
                        for (uint i = 0; i < scoreBuffer.size(); i++) cerr<<scoreBuffer[i]<<" ";
                        cerr<<endl;
                    }
                }
            }
            if (timer.elapsed() > maxTime) {
                if (verbose) {
                    cerr<<"ran out of time. scoreBuffer:"<<endl;
                    for (uint i = 0; i < scoreBuffer.size(); i++) cerr<<scoreBuffer[i]<<endl;
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
        cout<<"> getPBad("<<temp<<") = "<<pBadAvgAtEq<<" (score: "<<currentScore<<")";
        if (reachedEquilibrium) cout<<" (time: "<<timer.elapsed()<<"s)";
        else cout<<" (didn't detect eq. after "<<maxTime<<"s)";
        cout<<" iterations = "<<iter<<", ips = "<<nextIps<<endl;
        if (verbose) cerr<<"final result: "<<pBadAvgAtEq<<endl
                         <<"****************************************"<<endl<<endl;
    }

    //restore normal execution state
    constantTemp = false;
    enableTrackProgress = true;
    Temperature = TInitial;

    return pBadAvgAtEq;
}

void SANA::initTau(void) {
    /* tau = {
    1.000, 0.985, 0.970, 0.960, 0.950, 0.942, 0.939, 0.934, 0.928, 0.920,
    0.918, 0.911, 0.906, 0.901, 0.896, 0.891, 0.885, 0.879, 0.873, 0.867,
    0.860, 0.853, 0.846, 0.838, 0.830, 0.822, 0.810, 0.804, 0.794, 0.784,
    0.774, 0.763, 0.752, 0.741, 0.728, 0.716, 0.703, 0.690, 0.676, 0.662,
    0.647, 0.632, 0.616, 0.600, 0.584, 0.567, 0.549, 0.531, 0.514, 0.495,
    0.477, 0.458, 0.438, 0.412, 0.400, 0.381, 0.361, 0.342, 0.322, 0.303,
    0.284, 0.264, 0.246, 0.228, 0.210, 0.193, 0.177, 0.161, 0.145, 0.131,
    0.116, 0.104, 0.092, 0.081, 0.070, 0.061, 0.052, 0.044, 0.0375, 0.031,
    0.026, 0.0212, 0.0172, 0.0138, 0.011, 0.008, 0.006, 0.005, 0.004, 0.003,
    0.002, 0.001, 0.0003, 0.0001, 3e-5, 1e-6, 0, 0, 0, 0, 0}; */
    tau = {0.996738, 0.994914, 0.993865, 0.974899, 0.977274, 0.980926, 0.97399,  0.970583, 0.967492, 0.962373,
           0.953197, 0.954104, 0.951387, 0.953532, 0.948492, 0.939501, 0.939128, 0.932902, 0.912378, 0.896011,
           0.89535,  0.88642,  0.874628, 0.856721, 0.855782, 0.838483, 0.820407, 0.784303, 0.771297, 0.751457,
           0.735902, 0.676393, 0.633939, 0.604872, 0.53482,  0.456856, 0.446905, 0.377708, 0.337258, 3.04e-01,
           0.280585, 0.240093, 1.95e-01, 1.57e-01, 1.21e-01, 1.00e-01, 8.04e-02, 5.95e-02, 4.45e-02, 3.21e-02,
           1.81e-02, 1.82e-02, 1.12e-02, 7.95e-03, 4.82e-03, 3.73e-03, 2.11e-03, 1.41e-03, 9.69e-04, 6.96e-04,
           5.48e-04, 4.20e-04, 4.00e-04, 3.50e-04, 3.10e-04, 2.84e-04, 2.64e-04, 1.19e-04, 8.16e-05, 7.22e-05,
           6.16e-05, 4.46e-05, 3.36e-05, 2.66e-05, 1.01e-05, 9.11e-06, 4.09e-06, 3.96e-06, 3.43e-06, 3.12e-06,
           2.46e-06, 2.02e-06, 1.85e-06, 1.72e-06, 1.10e-06, 9.13e-07, 8.65e-07, 8.21e-07, 7.26e-07, 6.25e-07,
           5.99e-07, 5.42e-07, 8.12e-08, 4.16e-08, 6.56e-09, 9.124e-10, 6.1245e-10, 3.356e-10, 8.124e-11, 4.587e-11};
}