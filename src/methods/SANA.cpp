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
bool SANA::saveAligAndExitOnInterruption = false;
bool SANA::saveAligAndContOnInterruption = false;
uint SANA::INVALID_ACTIVE_COLOR_ID;

SANA::SANA(const Graph* G1, const Graph* G2,
        double TInitial, double TDecay, double maxSeconds, long long int maxIterations, bool addHillClimbing,
        MeasureCombination* MC, const string& scoreAggrStr, const Alignment& startA,
        const string& outputFileName, const string& localScoresFileName):
                Method(G1, G2, "SANA_"+MC->toString()),
                startA(startA),
                addHillClimbing(addHillClimbing),
                TInitial(TInitial), TDecay(TDecay),
                wasBadMove(false),
                maxSeconds(maxSeconds),
                maxIterations(maxIterations),
                MC(MC),
                outputFileName(outputFileName),
                localScoresFileName(localScoresFileName) {
    initTau();
    n1 = G1->getNumNodes(); n2 = G2->getNumNodes();
    g1Edges = G1->getNumEdges(); g2Edges = G2->getNumEdges();
    g1TotalWeight = G1->getTotalEdgeWeight(); g2TotalWeight = G2->getTotalEdgeWeight();
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
    if (maxIterations > 0 and maxSeconds > 0)
        throw runtime_error("use only one of maxIterations or maxSeconds");
    else if (maxIterations <= 0 and maxSeconds <= 0)
        throw runtime_error("exactly one of maxIterations and maxSeconds must be > 0");
    useIterations = maxIterations > 0;

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
        needNC = false;
        ncWeight = MC->getWeight("nc");
        Measure* nc = MC->getMeasure("nc");
        trueAWithValidCountAppended = ((NodeCorrectness*) nc)->getMappingforNC();
        needNC = true;
    } catch(...) {
        ncWeight = 0;
        trueAWithValidCountAppended = {static_cast<uint>(G2->getNumNodes()), 1};
    }
    localWeight = MC->getSumLocalWeight();

    //indicate which variables need to be maintained incrementally
    needAligEdges        = icsWeight > 0 or ecWeight > 0 or s3Weight > 0 or wecWeight > 0 or secWeight > 0 or mecWeight > 0;
    needEd               = edWeight > 0; //edge difference
    needEr               = erWeight > 0; //edge ratio
    needSquaredAligEdges = sesWeight > 0; //SES
    needExposedEdges     = eeWeight > 0 or MultiS3::denominator_type == MultiS3::ee_global; //EE; if needMS3, might use EE as denom
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
        vector<vector<float>>* wecSimsP = m->getSimMatrix();
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
    coreScoreData = CoreScoreData(n1, n2);
#endif

    //other execution options
    dynamicTDecay         = false;
    constantTemp          = false;
    enableTrackProgress   = true;
    iterationsPerStep     = 10000000;
    avgEnergyInc          = -0.00001; //to track progress

    // NODE COLOR SYSTEM initialization
    assert(G1->numColors() <= G2->numColors());
    const bool COL_DBG = true; //print stats about color/neighbor type probabilities

    vector<uint> numSwapNeighborsByG1Color(G1->numColors(), 0);
    vector<uint> numChangeNeighborsByG1Color(G1->numColors(), 0);
    uint totalNbrCount = 0;
    for (uint g1Id = 0; g1Id < G1->numColors(); g1Id++) {
        string colName = G1->getColorName(g1Id);
        if (not G2->hasColor(colName))
            throw runtime_error("G1 nodes colored "+colName+" cannot be matched to any G2 nodes");
        uint c1 = G1->numNodesWithColor(g1Id);
        uint c2 = G2->numNodesWithColor(G2->getColorId(colName));
        if (c1 > c2) throw runtime_error("there are "+to_string(c1)+" G1 nodes colored "
                    +colName+" but only "+to_string(c2)+" such nodes in G2");  
        uint numSwapNbrs = c1*(c1-1)/2, numChangeNbrs = c1*(c2-c1);
        numSwapNeighborsByG1Color[g1Id] = numSwapNbrs;
        numChangeNeighborsByG1Color[g1Id] = numChangeNbrs;
        uint numNbrs = numSwapNbrs + numChangeNbrs;
        totalNbrCount += numNbrs;
        if (COL_DBG) {
            cerr<<"color "<<colName<<" has "<<numSwapNbrs<<" swap nbrs and "
                <<numChangeNbrs<<" change nbrs ("<<numNbrs<<" total)"<<endl;
            if (numNbrs == 0) cerr<<"color "<<colName<<" is inactive"<<endl;
        }
    }
    if (COL_DBG) cerr<<"alignments have "<<totalNbrCount<<" nbrs in total"<<endl;
    if (totalNbrCount == 0) throw runtime_error(
            "there is a unique valid alignment, so running SANA is pointless");

    //init active color data structures
    for (uint g1Id = 0; g1Id < G1->numColors(); g1Id++) {
        uint numChangeNbrs = numChangeNeighborsByG1Color[g1Id];
        uint numNbrs = numChangeNbrs + numSwapNeighborsByG1Color[g1Id];
        if (numNbrs == 0) continue; //inactive color
        double colorProb = numNbrs / (double) totalNbrCount;
        double accumProb = colorProb +
                (actColToAccumProbCutpoint.empty() ? 0 : actColToAccumProbCutpoint.back());
        assert(accumProb <= 1 + 1e-14); // allow for a bit of numerical roundoff error
        actColToAccumProbCutpoint.push_back(accumProb);
        actColToChangeProb.push_back(numChangeNbrs/ (double) numNbrs);
        actColToG1ColId.push_back(g1Id);
    }
    //due to rounding errors, the last number may not be exactly 1, so we correct it
    actColToAccumProbCutpoint.back() = 1;
    if (COL_DBG) {
        cerr<<"Active colors:"<<endl;
        vector<vector<string>> colTable;
        colTable.push_back({"id","name","color P","accum P","change-P","swap-P"});
        for (uint i = 0; i < actColToG1ColId.size(); i++) {
            string name = G1->getColorName(actColToG1ColId[i]);
            double colP = actColToAccumProbCutpoint[i] - (i>0 ? actColToAccumProbCutpoint[i-1] : 0);
            colTable.push_back({to_string(i), name, to_string(colP), to_string(actColToAccumProbCutpoint[i]), 
                                to_string(actColToChangeProb[i]), to_string(1-actColToChangeProb[i])});
        }
        printTable(colTable, 4, cerr);
        cerr<<endl;
    }

    //init g2NodeToActColId. For each node, we do the following transformations:
    //g2Node -> g2ColorId -> g1ColorId -> actColId
    vector<uint> g2ToG1ColorIdMap = G2->myColorIdsToOtherGraphColorIds(*G1);
    INVALID_ACTIVE_COLOR_ID = n1;
    vector<uint> g1ColIdToActColId(G1->numColors(), INVALID_ACTIVE_COLOR_ID);
    for (uint i = 0; i < actColToG1ColId.size(); i++) {
        g1ColIdToActColId[actColToG1ColId[i]] = i;
    }
    g2NodeToActColId = vector<uint> (n2, INVALID_ACTIVE_COLOR_ID);
    for (uint g2Node = 0; g2Node < n2; g2Node++) {
        uint g2ColorId = G2->nodeColors[g2Node];
        uint g1ColorId = g2ToG1ColorIdMap[g2ColorId];
        if (g1ColorId == Graph::INVALID_COLOR_ID) continue; //no node in G1 has this color
        g2NodeToActColId[g2Node] = g1ColIdToActColId[g1ColorId];
    }
    //things initialized in initDataStructures because they depend on the starting alignment
    //they have the same size for every run, so we can allocate the size here
    assignedNodesG2 = vector<bool> (n2);
    totalInducedWeight = vector<uint> (n2,0);
    actColToUnassignedG2Nodes = vector<vector<uint>> (actColToG1ColId.size());
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

    //initialize assignedNodesG2 (the size was already set in the constructor)
    for (uint i = 0; i < n2; i++) assignedNodesG2[i] = false;
    for (uint i = 0; i < n1; i++) assignedNodesG2[alig[i]] = true;
    //initialize actColToUnassignedG2Nodes (the size was already set in the constructor)
    for (uint i = 0; i < actColToUnassignedG2Nodes.size(); i++)
        actColToUnassignedG2Nodes[i].clear();
    for (uint g2Node = 0; g2Node < n2; g2Node++) {
        if (assignedNodesG2[g2Node]) continue;
        uint actColId = g2NodeToActColId[g2Node];
        if (actColId != INVALID_ACTIVE_COLOR_ID) {
            actColToUnassignedG2Nodes[actColId].push_back(g2Node);
        }
    }

    if (needAligEdges or needSec) aligEdges = alig.numAlignedEdges(*G1, *G2);
    if (needEd) edSum = EdgeDifference::getEdgeDifferenceSum(G1, G2, alig);
    if (needEr) erSum = EdgeRatio::getEdgeRatioSum(G1, G2, alig);
    if (needSquaredAligEdges) squaredAligEdges =
            ((SquaredEdgeScore*) MC->getMeasure("ses"))->numSquaredAlignedEdges(alig);
    if (needExposedEdges) EdgeExposure::numer = 
        EdgeExposure::numExposedEdges(alig, *G1, *G2);//- EdgeExposure::getMaxEdge();
    if (needMS3) {
        MultiS3::numer = ((MultiS3*) MC->getMeasure("ms3"))->computeNumer(alig);
        MultiS3::denom = ((MultiS3*) MC->getMeasure("ms3"))->computeDenom(alig);
	EL_k = MultiS3::EL_k;
	ER_k = MultiS3::ER_k;
	RU_k = MultiS3::RU_k;
	RA_k = MultiS3::RA_k;
	RO_k = MultiS3::RO_k;
	for (uint i = 0; i < n1; i++) {
	    totalInducedWeight[alig[i]] = 0;
	    for(uint j=0;j<n1;j++) if(i!=j) {
		assert(alig[i]>=0 && alig[i] < n2);
		assert(alig[j]>=0 && alig[j] < n2);
		assert(G2->getEdgeWeight(alig[i], alig[j]) == G2->getEdgeWeight(alig[j], alig[i]));
		totalInducedWeight[alig[i]] += G2->getEdgeWeight(alig[i], alig[j]);
	    }
	}
	for (uint i = 0; i < n1; i++){assert(totalInducedWeight[alig[i]] <= G2->totalWeight[alig[i]]);}
    }
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
        ncSum       = (nc->eval(alig))*trueAWithValidCountAppended.back(); 
    }
    currentScore = eval(alig);
    A = alig.asVector();
    timer.start();
}


Alignment SANA::run() {
    initDataStructures();
    setInterruptSignal();

#ifndef MULTI_PAIRWISE
    getIterPerSecond(); // this takes several seconds of CPU time; don't do it during multi-only-iterations.
#endif
    long long int maxIters = useIterations ? maxIterations
                                           : (long long int) (getIterPerSecond()*maxSeconds);
    double leeway = 2;
    double maxSecondsWithLeeway = maxSeconds * leeway;

    long long int iter;
    for (iter = 0; iter <= maxIters; iter++) {
        Temperature = temperatureFunction(iter, TInitial, TDecay);
        SANAIteration();
        if (saveAligAndExitOnInterruption) break;
        if (saveAligAndContOnInterruption) printReportOnInterruption();
        if (iter%iterationsPerStep == 0) {
            trackProgress(iter, maxIters);
            if (not useIterations and timer.elapsed() > maxSecondsWithLeeway
                and currentScore-previousScore < 0.005) break;
            previousScore = currentScore;
        }
    }
    trackProgress(iter, maxIters);
    cout<<"Performed "<<iter<<" total iterations\n";
    if (addHillClimbing) performHillClimbing(10000000LL); //arbitrarily chosen, probably too big.

#ifdef CORES
    Report::saveCoreScore(*G1, *G2, A, this, coreScoreData, outputFileName);
#endif

    return A;
}


void SANA::performHillClimbing(long long int idleCountTarget) {
    long long int iter = 0;
    Temperature = 0;
    numPBadsInBuffer = pBadBufferSum = pBadBufferIndex = 0; 

    cout << "Beginning Final Pure Hill Climbing Stage" << endl;
    Timer T;
    T.start();
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
    cout<<"Hill climbing took "<<T.elapsedString()<<"s"<<endl;
}

void SANA::describeParameters(ostream& sout) const {
    sout << "Temperature schedule:" << endl;
    sout << "T_initial: " << TInitial << endl;
    sout << "T_decay: " << TDecay << endl;
    sout << "Optimize: " << endl;
    MC->printWeights(sout);
    if (useIterations) sout << "Max iterations: " << maxIterations << endl;
    else sout << "Execution time: " << maxSeconds << "s" << endl;
}

string SANA::fileNameSuffix(const Alignment& Al) const {
    return "_" + extractDecimals(eval(Al),3);
}

double SANA::temperatureFunction(long long int iter, double TInitial, double TDecay) {
    if (constantTemp) return TInitial;
    double fraction;
    if (useIterations) fraction = iter / (double) maxIterations;
    else fraction = iter / (maxSeconds * getIterPerSecond());
    return TInitial * exp(-TDecay * fraction);
}

double SANA::acceptingProbability(double energyInc, double Temperature) {
    return energyInc >= 0 ? 1 : exp(energyInc/Temperature);
}

double SANA::incrementalMeanPBad() {
    return pBadBufferSum/(double) numPBadsInBuffer;
}

double SANA::slowMeanPBad() {
    double sum = 0;
    for (int i = 0; i < numPBadsInBuffer; i++) sum += pBadBuffer[i];
    return sum/(double) numPBadsInBuffer;
}

double SANA::eval(const Alignment& Al) const { return MC->eval(Al); }

void sigIntHandler(int s) {
    string line;
    int c = -1;
    do {
        cout<<"Select an option (0 - 3):"<<endl<<"  (0) Do nothing and continue"<<endl<<"  (1) Exit"<<endl
            <<"  (2) Save Alignment and Exit"<<endl<<"  (3) Save Alignment and Continue"<<endl<<">> ";
        cin >> c;
        if (cin.eof()) exit(0);
        if (cin.fail()) {
            c = -1;
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
        }
        if      (c == 0) cout<<"Continuing..."<<endl;
        else if (c == 1) exit(0);
        else if (c == 2) SANA::saveAligAndExitOnInterruption = true;
        else if (c == 3) SANA::saveAligAndContOnInterruption = true;
    } while (c < 0 || c > 3);    
}
void SANA::setInterruptSignal() {
    saveAligAndExitOnInterruption = false;
    struct sigaction sigInt;
    sigInt.sa_handler = sigIntHandler;
    sigemptyset(&sigInt.sa_mask);
    sigInt.sa_flags = 0;
    sigaction(SIGINT, &sigInt, NULL);
}
void SANA::printReportOnInterruption() {
    saveAligAndContOnInterruption = false; //reset value
    string timestamp = string(currentDateTime()); //necessary to make it not const
    std::replace(timestamp.begin(), timestamp.end(), ' ', '_');
    string outFile = outputFileName+"_"+timestamp;
    string localFile = localScoresFileName+"_"+timestamp;
    Report::saveReport(*G1, *G2, A, *MC, this, outFile, true);
    Report::saveLocalMeasures(*G1, *G2, A, *MC, this, localFile);
#ifdef CORES
    Report::saveCoreScore(*G1, *G2, A, this, coreScoreData, outputFileName);
#endif
    cout << "Alignment saved. SANA will now continue." << endl;
}

void SANA::SANAIteration() {
    ++iterationsPerformed;
    uint actColId = randActiveColorIdWeightedByNumNbrs();
    double p = randomReal(gen);
    if (p < actColToChangeProb[actColId]) {
        performChange(actColId);
    } else {
        performSwap(actColId);
    }
}

uint SANA::numActiveColors() const {
    return actColToChangeProb.size();
}

uint SANA::randActiveColorIdWeightedByNumNbrs() {
    if (numActiveColors() == 1) return 0; //optimized special case: monochromatic graphs
    double p = randomReal(gen);
    if (numActiveColors() == 2) //optimized special case: bichromatic graphs
        return (p < actColToAccumProbCutpoint[0] ? 0 : 1);

    //general case: use binary search to optimizie for the case with many active colors
    auto iter = lower_bound(actColToAccumProbCutpoint.begin(), actColToAccumProbCutpoint.end(), p);
    assert(iter != actColToAccumProbCutpoint.end());
    return iter - actColToAccumProbCutpoint.begin();
}

uint SANA::randomG1NodeWithActiveColor(uint actColId) const {
    uint g1ColId = actColToG1ColId[actColId];
    uint randIndex = randInt(0, G1->nodeGroupsByColor[g1ColId].size()-1);
    return G1->nodeGroupsByColor[g1ColId][randIndex];
} 

void SANA::performChange(uint actColId) {
    uint peg = randomG1NodeWithActiveColor(actColId);
    uint oldHole = A[peg];
    uint numUnassigWithCol = actColToUnassignedG2Nodes[actColId].size();
    uint unassignedVecIndex = randInt(0, numUnassigWithCol-1);
    uint newHole = actColToUnassignedG2Nodes[actColId][unassignedVecIndex];

    assert(numUnassigWithCol > 0);
    assert(oldHole != newHole);
    // assert(G1->getColorName(G1->getNodeColor(peg)) == G2->getColorName(G2->getNodeColor(oldHole)));
    // assert(G2->getNodeColor(newHole) == G2->getNodeColor(oldHole));

    //added this dummy initialization to shut compiler warning -Nil
   unsigned saveOldHoleDeg = 0, saveNewHoleDeg = 0, oldMs3Denom = 0, oldMs3Numer =0;

    if (needMS3) {
        saveOldHoleDeg = MultiS3::shadowDegree[oldHole];
        saveNewHoleDeg = MultiS3::shadowDegree[newHole];
        oldMs3Denom = MultiS3::denom;
        oldMs3Numer = MultiS3::numer;
    }
    int newAligEdges           = (needAligEdges or needSec) ? aligEdges + aligEdgesIncChangeOp(peg, oldHole, newHole) : -1;
    double newEdSum            = needEd ? edSum + edgeDifferenceIncChangeOp(peg, oldHole, newHole) : -1;
    double newErSum            = needEr ? erSum + edgeRatioIncChangeOp(peg, oldHole, newHole) : -1;
    double newSquaredAligEdges = needSquaredAligEdges ? squaredAligEdges + squaredAligEdgesIncChangeOp(peg, oldHole, newHole) : -1;
    double newExposedEdgesNumer= needExposedEdges ? EdgeExposure::numer + exposedEdgesIncChangeOp(peg, oldHole, newHole) : -1;
    double newMS3Numer         = needMS3 ? MultiS3::numer + MS3IncChangeOp(peg, oldHole, newHole) : -1;
    int newInducedEdges        = needInducedEdges ? inducedEdges + inducedEdgesIncChangeOp(peg, oldHole, newHole) : -1;
    double newLocalScoreSum    = needLocal ? localScoreSum + localScoreSumIncChangeOp(sims, peg, oldHole, newHole) : -1;
    double newWecSum           = needWec ? wecSum + WECIncChangeOp(peg, oldHole, newHole) : -1;
    double newJsSum            = needJs ? jsSum + JSIncChangeOp(peg, oldHole, newHole) : -1;
    double newEwecSum          = needEwec ? ewecSum + EWECIncChangeOp(peg, oldHole, newHole) : -1;
    double newNcSum            = needNC ? ncSum + ncIncChangeOp(peg, oldHole, newHole) : -1;

    map<string, double> newLocalScoreSumMap;
    if (needLocal) {
        newLocalScoreSumMap = map<string, double>(localScoreSumMap);
        for (auto &item : newLocalScoreSumMap)
            item.second += localScoreSumIncChangeOp(localSimMatrixMap[item.first], peg, oldHole, newHole);
    }

    double newCurrentScore = 0;
    double pBad = scoreComparison(newAligEdges, newInducedEdges,
            newLocalScoreSum, newWecSum, newJsSum, newNcSum, newCurrentScore, newEwecSum,
            newSquaredAligEdges, newExposedEdgesNumer, newMS3Numer, newEdSum, newErSum);
    bool makeChange;
    //if(newCurrentScore == currentScore) makeChange = false; else // if it ain't broke, don't fix it
    makeChange = randomReal(gen) < pBad;

#ifdef CORES
    // Statistics on the emerging core alignment.
    // only update pBad if is nonzero; reuse previous nonzero pBad if the current one is zero.
    uint betterHole = wasBadMove ? oldHole : newHole;

    double meanPBad = incrementalMeanPBad(); // maybe we should use the *actual* pBad of *this* move?
    if (meanPBad <= 0 || myNan(meanPBad)) meanPBad = LOW_PBAD_LIMIT_FOR_CORES;

    coreScoreData.incChangeOp(peg, betterHole, pBad, meanPBad);
#endif

    if (makeChange) {
        A[peg] = newHole;
        actColToUnassignedG2Nodes[actColId][unassignedVecIndex] = oldHole;
        assignedNodesG2[oldHole] = false;
        assignedNodesG2[newHole] = true;
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
        MultiS3::shadowDegree[oldHole] = saveOldHoleDeg;
        MultiS3::shadowDegree[newHole] = saveNewHoleDeg;
        MultiS3::denom = oldMs3Denom;
        MultiS3::numer = oldMs3Numer;
    }
#if 0
    uint correct = ((MultiS3*)MC->getMeasure("ms3"))->computeNumer(A);
    if(MultiS3::numer==correct)cerr<<'N';else{cerr<<"\nnumer "<<MultiS3::numer<<" off "<<(int)(MultiS3::numer-correct);MultiS3::numer=correct;}
    correct = ((MultiS3*)MC->getMeasure("ms3"))->computeDenom(A);
    if(MultiS3::denom==correct)cerr<<'D';else{cerr<<"\ndenom "<<MultiS3::denom<<" off "<<(int)(MultiS3::denom-correct);MultiS3::denom=correct;}
#endif
}

void SANA::performSwap(uint actColId) {
    uint peg1 = randomG1NodeWithActiveColor(actColId);
    
    uint peg2;
    for (uint i = 0; i < 100; i++) { //each attempt has >=50% chance of success
        peg2 = randomG1NodeWithActiveColor(actColId);
        if (peg1 != peg2) break;
    }

    uint hole1 = A[peg1], hole2 = A[peg2];
    
    assert(peg1 != peg2);
    assert(hole1 != hole2);
    // assert(G1->getNodeColor(peg1) == G1->getNodeColor(peg2));
    // assert(G2->getNodeColor(hole1) == G2->getNodeColor(hole2));
    // assert(G1->getColorName(G1->getNodeColor(peg1)) == G2->getColorName(G2->getNodeColor(hole1)));

    //added this dummy initialization to shut compiler warning -Nil
    unsigned oldHole1Deg = 0, oldHole2Deg = 0, oldMs3Denom = 0;

    if (needMS3) {
        oldHole1Deg = MultiS3::shadowDegree[hole1];
        oldHole2Deg = MultiS3::shadowDegree[hole2];
        oldMs3Denom = MultiS3::denom;
    }

    int newAligEdges           = (needAligEdges or needSec) ? aligEdges + aligEdgesIncSwapOp(peg1, peg2, hole1, hole2) : -1;
    double newSquaredAligEdges = needSquaredAligEdges ? squaredAligEdges + squaredAligEdgesIncSwapOp(peg1, peg2, hole1, hole2) : -1;
    double newExposedEdgesNumer= needExposedEdges ? EdgeExposure::numer + exposedEdgesIncSwapOp(peg1, peg2, hole1, hole2) : -1;
    double newMS3Numer         = needMS3 ? MultiS3::numer + MS3IncSwapOp(peg1, peg2, hole1, hole2) : -1;
    double newWecSum           = needWec ? wecSum + WECIncSwapOp(peg1, peg2, hole1, hole2) : -1;
    double newJsSum            = needJs ? jsSum + JSIncSwapOp(peg1, peg2, hole1, hole2) : -1;
    double newEwecSum          = needEwec ? ewecSum + EWECIncSwapOp(peg1, peg2, hole1, hole2) : -1;
    double newNcSum            = needNC ? ncSum + ncIncSwapOp(peg1, peg2, hole1, hole2) : -1;
    double newLocalScoreSum    = needLocal ? localScoreSum + localScoreSumIncSwapOp(sims, peg1, peg2, hole1, hole2) : -1;
    double newEdSum            = needEd ? edSum + edgeDifferenceIncSwapOp(peg1, peg2, hole1, hole2) : -1;
    double newErSum            = needEr ? erSum + edgeRatioIncSwapOp(peg1, peg2, hole1, hole2) : -1;

    map<string, double> newLocalScoreSumMap;
    if (needLocal) {
        newLocalScoreSumMap = map<string, double>(localScoreSumMap);
        for (auto &item : newLocalScoreSumMap)
            item.second += localScoreSumIncSwapOp(localSimMatrixMap[item.first], peg1, peg2, hole1, hole2);
    }

    double newCurrentScore = 0;
    double pBad = scoreComparison(newAligEdges, inducedEdges, newLocalScoreSum,
                newWecSum, newJsSum, newNcSum, newCurrentScore, newEwecSum, newSquaredAligEdges,
                newExposedEdgesNumer, newMS3Numer, newEdSum, newErSum);
    bool makeChange;
    //if(newCurrentScore == currentScore) makeChange = false; else // if it ain't broke, don't fix it
    makeChange = randomReal(gen) < pBad;

#ifdef CORES
        // Statistics on the emerging core alignment.
        // only update pBad if it's nonzero; reuse previous nonzero pBad if the current one is zero.
        double meanPBad = incrementalMeanPBad(); // maybe we should use the *actual* pBad of *this* move?
        if (meanPBad <= 0 || myNan(meanPBad)) meanPBad = LOW_PBAD_LIMIT_FOR_CORES;

        uint betterDest1 = wasBadMove ? hole1 : hole2;
        uint betterDest2 = wasBadMove ? hole2 : hole1;

        coreScoreData.incSwapOp(peg1, peg2, betterDest1, betterDest2, pBad, meanPBad);
#endif

    if (makeChange) {
        A[peg1]          = hole2;
        A[peg2]          = hole1;
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
        MultiS3::shadowDegree[hole1] = oldHole1Deg;
        MultiS3::shadowDegree[hole2] = oldHole2Deg;
        MultiS3::denom = oldMs3Denom;
    }
}

// returns pBad
double SANA::scoreComparison(double newAligEdges, double newInducedEdges,
        double newLocalScoreSum, double newWecSum, double newJsSum, double newNcSum, double& newCurrentScore,
        double newEwecSum, double newSquaredAligEdges, double newExposedEdgesNumer, double newMS3Numer,
        double newEdgeDifferenceSum, double newEdgeRatioSum) {
    wasBadMove = false;
    double pBad = 0;

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
        newCurrentScore += ncWeight * (newNcSum / trueAWithValidCountAppended.back());
#ifdef MULTI_PAIRWISE
        newCurrentScore += mecWeight * (newAligEdges / (g1TotalWeight + g2TotalWeight));
        newCurrentScore += sesWeight * newSquaredAligEdges / (double)SquaredEdgeScore::getDenom();
        newCurrentScore += eeWeight * (1 - (newExposedEdgesNumer / (double)EdgeExposure::denom));
         if (MultiS3::denominator_type==MultiS3::ee_global) MultiS3::denom = newExposedEdgesNumer;
        newCurrentScore += ms3Weight * (double)newMS3Numer / (double)MultiS3::denom / (double)MultiS3::Normalization_factor;//(double)NUM_GRAPHS;
#endif
        energyInc = newCurrentScore - currentScore;
        wasBadMove = energyInc < 0;
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
        newCurrentScore *= ncWeight * (newNcSum / trueAWithValidCountAppended.back());
        energyInc = newCurrentScore - currentScore;
        wasBadMove = energyInc < 0;
        break;
    }
    case ScoreAggregation::max:
    {
        // this is a terrible way to compute the max; we should loop through all of them and figure out which is the biggest
        // and in fact we haven't yet integrated icsWeight here yet, so assert so
        assert(icsWeight == 0.0);
        double energyInc = max(ncWeight* (newNcSum / trueAWithValidCountAppended.back() - ncSum / trueAWithValidCountAppended.back()), max(max(ecWeight*(newAligEdges / g1Edges - aligEdges / g1Edges), max(
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
        newCurrentScore += ncWeight * (newNcSum / trueAWithValidCountAppended.back());

        energyInc = newCurrentScore - currentScore;
        wasBadMove = energyInc < 0;
        break;
    }
    case ScoreAggregation::min:
    {
        // see comment above in max
        assert(icsWeight == 0.0);
        double energyInc = min(ncWeight* (newNcSum / trueAWithValidCountAppended.back() - ncSum / trueAWithValidCountAppended.back()), min(min(ecWeight*(newAligEdges / g1Edges - aligEdges / g1Edges), min(
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
        newCurrentScore += ncWeight * (newNcSum / trueAWithValidCountAppended.back());

        energyInc = newCurrentScore - currentScore; //is this even used?
        wasBadMove = energyInc < 0;
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
        newCurrentScore += ncWeight / (newNcSum / trueAWithValidCountAppended.back());

        energyInc = newCurrentScore - currentScore;
        wasBadMove = energyInc < 0;
        break;
    }
    case ScoreAggregation::maxFactor:
    {
        assert(icsWeight == 0.0);
        double maxScore = max(ncWeight*(newNcSum / trueAWithValidCountAppended.back() - ncSum / trueAWithValidCountAppended.back()), max(max(ecWeight*(newAligEdges / g1Edges - aligEdges / g1Edges), max(
            s3Weight*((newAligEdges / (g1Edges + newInducedEdges - newAligEdges) - (aligEdges / (g1Edges + inducedEdges - aligEdges)))),
            secWeight*0.5*(newAligEdges / g1Edges - aligEdges / g1Edges + newAligEdges / g2Edges - aligEdges / g2Edges))),
            max(localWeight*((newLocalScoreSum / n1) - (localScoreSum)),
            max(wecWeight*(newWecSum / (2 * g1Edges) - wecSum / (2 * g1Edges)), jsWeight*(newJsSum - jsSum)))));

        double minScore = min(ncWeight*(newNcSum / trueAWithValidCountAppended.back() - ncSum / trueAWithValidCountAppended.back()), min(min(ecWeight*(newAligEdges / g1Edges - aligEdges / g1Edges), min(
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
        newCurrentScore += ncWeight * (newNcSum / trueAWithValidCountAppended.back());

        energyInc = newCurrentScore - currentScore;
        wasBadMove = maxScore < -1 * minScore;
        break;
    }
    }
    //using max and min here because with extremely low temps I was seeing invalid probabilities
    //note: I did not make this change for the other types of ScoreAggregation::  -Nil
    //note2: I moved it down here to apply to all ScoreAggregation methods - WH
    if (energyInc >= 0) pBad = 1.0;
    else pBad = max(0.0, min(1.0, exp(energyInc / Temperature)));

    //if (wasBadMove && (iterationsPerformed % 512 == 0 || (iterationsPerformed % 32 == 0))) 
    //the above will never be true in the case of iterationsPerformed never being changed so that it doesn't greatly
    // slow down the program if for some reason iterationsPerformed doesn't need to be changed.
    if (wasBadMove) { // I think Dillon was wrong above, just do it always - WH
        if (numPBadsInBuffer == PBAD_CIRCULAR_BUFFER_SIZE) {
            pBadBufferIndex = (pBadBufferIndex == PBAD_CIRCULAR_BUFFER_SIZE ? 0 : pBadBufferIndex);
            pBadBufferSum -= pBadBuffer[pBadBufferIndex];
            pBadBuffer[pBadBufferIndex] = pBad;
        } else {
            pBadBuffer[pBadBufferIndex] = pBad;
            numPBadsInBuffer++;
        }
        pBadBufferSum += pBad;
        pBadBufferIndex++;
    }
    return pBad;
}

int SANA::aligEdgesIncChangeOp(uint peg, uint oldHole, uint newHole) {
    int res = 0;
    if (G1->hasSelfLoop(peg)) {
        if (G2->hasSelfLoop(oldHole)) res-=G2->getEdgeWeight(oldHole, oldHole);
        if (G2->hasSelfLoop(newHole)) res+=G2->getEdgeWeight(newHole, newHole);
    }
    for (uint nbr : G1->adjLists[peg]) {
        if (nbr != peg) {
            res -= G2->getEdgeWeight(oldHole, A[nbr]);
            res += G2->getEdgeWeight(newHole, A[nbr]);
        }
    }
    return res;
}

int SANA::aligEdgesIncSwapOp(uint peg1, uint peg2, uint hole1, uint hole2) {
#ifdef FLOAT_WEIGHTS
    return 0; //not applicable
#else
    int res = 0;
    if (G1->hasSelfLoop(peg1)) {
        if (G2->hasSelfLoop(hole1)) res-=G2->getEdgeWeight(hole1, hole1);
        if (G2->hasSelfLoop(hole2)) res+=G2->getEdgeWeight(hole2, hole2);
    }
    for (uint nbr : G1->adjLists[peg1]) {
        if (nbr != peg1) {
            res -= G2->getEdgeWeight(hole1, A[nbr]);
            res += G2->getEdgeWeight(hole2, A[nbr]);
        }
    }
    if (G1->hasSelfLoop(peg2)) {
        if (G2->hasSelfLoop(hole2)) res-=G2->getEdgeWeight(hole2, hole2);
        if (G2->hasSelfLoop(hole1)) res+=G2->getEdgeWeight(hole1, hole1);
    }
    for (uint nbr : G1->adjLists[peg2]) {
        if (nbr != peg2) {
            res -= G2->getEdgeWeight(hole2, A[nbr]);
            res += G2->getEdgeWeight(hole1, A[nbr]);
        }
    }

    //address the case where we are swapping between adjacent nodes with adjacent images:
#ifdef MULTI_PAIRWISE
    //why set the least-significant bit to 0?
    //this kind of bit manipulation needs a comment clarification -Nil
    res += (-1 << 1) & (G1->getEdgeWeight(peg1, peg2) +
                        G2->getEdgeWeight(hole1, hole2));
#else
    if (G1->hasEdge(peg1, peg2) and G2->hasEdge(hole1, hole2)) res += 2;
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

/* We swap the mapping of two nodes peg1 and peg2
 * We can first handle peg1, then do the same with peg2
 * Subtract old edge difference with edge (peg1, hole1)
 * Add new edge difference with edge (peg1, hole2) */
double SANA::edgeDifferenceIncSwapOp(uint peg1, uint peg2, uint hole1, uint hole2) {
    if (peg1 == peg2) return 0;
    // Handle peg1
    double edgeDifferenceIncDiff = 0;
    double c = 0;
    for (uint node2 : G1->adjLists[peg1]) {
        double y = -abs(G1->getEdgeWeight(peg1, node2) - G2->getEdgeWeight(hole1, A[node2])) - c;
        double t = edgeDifferenceIncDiff + y;
        c = (t - edgeDifferenceIncDiff) - y;
        edgeDifferenceIncDiff = t;

        // Determine the new target hole for node2
        uint node2Hole = 0;
        if (node2 == peg1) node2Hole = hole2;
        else if (node2 == peg2) node2Hole = hole1;
        else node2Hole = A[node2];

        y = +abs(G1->getEdgeWeight(peg1, node2) - G2->getEdgeWeight(hole2, node2Hole)) - c;
        t = edgeDifferenceIncDiff + y;
        c = (t - edgeDifferenceIncDiff) - y;
        edgeDifferenceIncDiff = t;
    }
    // Handle peg2
    for (uint node2 : G1->adjLists[peg2]) {
        if (node2 == peg1) continue;
        double y = -abs(G1->getEdgeWeight(peg2, node2) - G2->getEdgeWeight(hole2, A[node2])) - c;
        double t = edgeDifferenceIncDiff + y;
        c = (t - edgeDifferenceIncDiff) - y;
        edgeDifferenceIncDiff = t;

        uint node2Hole = (node2 == peg2 ? hole1 : A[node2]);
        y = +abs(G1->getEdgeWeight(peg2, node2) - G2->getEdgeWeight(hole1, node2Hole)) - c;
        t = edgeDifferenceIncDiff + y;
        c = (t - edgeDifferenceIncDiff) - y;
        edgeDifferenceIncDiff = t;
    }
    return edgeDifferenceIncDiff;
}

double SANA::edgeRatioIncSwapOp(uint peg1, uint peg2, uint hole1, uint hole2) {
    if (peg1 == peg2) return 0;
    double edgeRatioIncDiff = 0;
    double c = 0;
    // Subtract peg1-hole1, add peg1-hole2
    for (uint node2 : G1->adjLists[peg1]) {
        double r = getRatio(G1->getEdgeWeight(peg1, node2), G2->getEdgeWeight(hole1, A[node2]));
        double y = -r - c;
        double t = edgeRatioIncDiff + y;
        c = (t - edgeRatioIncDiff) - y;
        edgeRatioIncDiff = t;

        uint node2Hole = 0;
        if (node2 == peg1) node2Hole = hole2;
        else if (node2 == peg2) node2Hole = hole1;
        else node2Hole = A[node2];

        r = getRatio(G1->getEdgeWeight(peg1, node2), G2->getEdgeWeight(hole2, node2Hole));
        y = r - c;
        t = edgeRatioIncDiff + y;
        c = (t - edgeRatioIncDiff) - y;
        edgeRatioIncDiff = t;
    }
   // Subtract peg2-hole2, add peg2-hole1
   for (uint node2 : G1->adjLists[peg2]) {
        if (node2 == peg1) continue;
        double r = getRatio(G1->getEdgeWeight(peg2, node2), G2->getEdgeWeight(hole2, A[node2]));
        double y = -r - c;
        double t = edgeRatioIncDiff + y;
        c = (t - edgeRatioIncDiff) - y;
        edgeRatioIncDiff = t;

        uint node2Hole = (node2 == peg2 ? hole1 : A[node2]);
        r = getRatio(G1->getEdgeWeight(peg2, node2), G2->getEdgeWeight(hole1, node2Hole));
        y = r - c;
        t = edgeRatioIncDiff + y;
        c = (t - edgeRatioIncDiff) - y;
        edgeRatioIncDiff = t;
    }
    return edgeRatioIncDiff;
}


double SANA::edgeDifferenceIncChangeOp(uint peg, uint oldHole, uint newHole) {
    double edgeDifferenceIncDiff = 0;
    double c = 0;
    for (uint node2 : G1->adjLists[peg]) {
        double y = -abs(G1->getEdgeWeight(peg, node2) - G2->getEdgeWeight(oldHole, A[node2])) - c;
        double t = edgeDifferenceIncDiff + y;
        c = (t - edgeDifferenceIncDiff) - y;
        edgeDifferenceIncDiff = t;

        uint node2Hole = node2 == peg ? newHole : A[node2];
        y = +abs(G1->getEdgeWeight(peg, node2) - G2->getEdgeWeight(newHole, node2Hole)) - c;
        t = edgeDifferenceIncDiff + y;
        c = (t - edgeDifferenceIncDiff) - y;
        edgeDifferenceIncDiff = t;
    }
    return edgeDifferenceIncDiff;
}

double SANA::edgeRatioIncChangeOp(uint peg, uint oldHole, uint newHole) {
    double edgeRatioIncDiff = 0;
    double c = 0;
    for (uint node2 : G1->adjLists[peg]) {
        double r = getRatio(G1->getEdgeWeight(peg, node2), G2->getEdgeWeight(oldHole, A[node2]));
        double y = -r - c;
        double t = edgeRatioIncDiff + y;
        c = (t - edgeRatioIncDiff) - y;
        edgeRatioIncDiff = t;

        uint node2Hole = node2 == peg ? newHole : A[node2];
        r = getRatio(G1->getEdgeWeight(peg, node2), G2->getEdgeWeight(newHole, node2Hole));
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
#define SQRDIFF(i,j) ((_edgeVal=G2->getEdgeWeight(i, A[j])), 2*_edgeVal + 1)
int SANA::squaredAligEdgesIncChangeOp(uint peg, uint oldHole, uint newHole) {
    int res = 0;
    for (uint nbr : G1->adjLists[peg]) {
        // Account for uint edges? Or assume smaller graph is edge value 1?
        int diff = SQRDIFF(oldHole, nbr);
        // assert(fabs(diff)<1100);
        res -= diff;// >0?diff:0;
        diff = SQRDIFF(newHole, nbr);
        // assert(fabs(diff)<1100);
        res += diff;// >0?diff:0;
    }
    return res;
}

int SANA::squaredAligEdgesIncSwapOp(uint peg1, uint peg2, uint hole1, uint hole2) {
    int res = 0;
    for (uint nbr : G1->adjLists[peg1]) {
        int diff = SQRDIFF(hole1, nbr);
        // assert(fabs(diff)<1100);
        res -= diff;// >0?diff:0;
        diff = SQRDIFF(hole2, nbr);
        if (hole2 == A[nbr]) diff = 0;
        // assert(fabs(diff)<1100);
        res += diff;// >0?diff:0;
    }
    for (uint nbr : G1->adjLists[peg2]) {
        int diff = SQRDIFF(hole2, nbr);
        // assert(fabs(diff)<1100);
        res -= diff;// >0?diff:0;
        diff = SQRDIFF(hole1, nbr);
        if (hole1 == A[nbr]) diff = 0;
        // assert(fabs(diff)<1100);
        res += diff;// >0?diff:0;
    }
    // How to do for squared?
    // address case swapping between adjacent nodes with adjacent images:
    if (G1->hasEdge(peg1, peg2) and G2->hasEdge(hole1, hole2))
        res += 2 * SQRDIFF(hole1,peg2);
    return res;
}

int SANA::exposedEdgesIncChangeOp(uint peg, uint oldHole, uint newHole) {
    int res = 0;
    for (uint nbr : G1->adjLists[peg]) {
        if (not G2->hasEdge(oldHole, A[nbr])) --res;
        if (not G2->hasEdge(newHole, A[nbr])) ++res;
    }
    return res;
}

int SANA::exposedEdgesIncSwapOp(uint peg1, uint peg2, uint hole1, uint hole2) {
    int res = 0;
    for (uint nbr : G1->adjLists[peg1]) {
        if (not G2->hasEdge(hole1, A[nbr])) --res;
        if (not G2->hasEdge(hole2, A[nbr])) ++res;
    }
    for (uint nbr : G1->adjLists[peg2]) {
        if (not G2->hasEdge(hole2, A[nbr])) --res;
        if (not G2->hasEdge(hole1, A[nbr])) ++res;
    }
    return res;
}

// Return the change in NUMERATOR of MS3
int SANA::MS3IncChangeOp(uint peg, uint oldHole, uint newHole) {
    uint pegNeigh, holeNeigh, diff;
    int res = 0;
    switch (MultiS3::numerator_type){
        case MultiS3::ra_k:
        {
            const uint n = G1->adjLists[peg].size();
            for (uint i = 0; i < n; ++i) {
                pegNeigh = G1->adjLists[peg][i];
                res -= G2->getEdgeWeight(oldHole,A[pegNeigh]);
                res += G2->getEdgeWeight(newHole,A[pegNeigh]);
            }
        }
	break;
    
        case MultiS3::ra_global:
        {
            const uint n = G1->adjLists[peg].size();
            for (uint i = 0; i < n; ++i) {
                pegNeigh = G1->adjLists[peg][i];
                if (G1->getEdgeWeight(peg,pegNeigh)>0){
                    diff = G2->getEdgeWeight(oldHole,A[pegNeigh]) + 1;
                    if (diff <= 1){res -= 0;}
                    else if (G2->getEdgeWeight(oldHole,A[pegNeigh])>1){res --;}
                    else{res-=2;}

                    diff = G2->getEdgeWeight(newHole,A[pegNeigh]) + 1;
                    if (diff <= 1){res += 0;}
                    else if (G2->getEdgeWeight(newHole,A[pegNeigh])>1){res ++;}
                    else{res+=2;}
                }
            }
        }
	break;
        
        case MultiS3::la_k:
        {
            const uint n = G1->adjLists[peg].size();
            bool ladder = false;
            for (uint i = 0; i < n; ++i) {
                pegNeigh = G1->adjLists[peg][i];
                diff = G2->getEdgeWeight(oldHole,A[pegNeigh]) + 1;

                ladder = (diff>1?true:false);
                if (ladder){res -= 1;}
                diff = G2->getEdgeWeight(newHole,A[pegNeigh]) + 1;

                ladder = (diff>1?true:false);
                if (ladder){res += 1;}
            }
        }
	break;
            
        case MultiS3::la_global:
        {
            bool ladder = false;
            const uint n = G1->adjLists[peg].size();
            for (uint i=0;i<n;i++){
                pegNeigh = G1->adjLists[peg][i];
                if (G1->getEdgeWeight(pegNeigh,peg)>0){
                    diff = G2->getEdgeWeight(oldHole,A[pegNeigh]) + 1;
                            ladder = (diff > 1);
                    if (ladder and G2->getEdgeWeight(oldHole,A[pegNeigh])==1) {res --;}
                    diff = G2->getEdgeWeight(newHole,A[pegNeigh]) + 1;
                    ladder = (diff > 1);
                    if (ladder and G2->getEdgeWeight(newHole,A[pegNeigh])==1) {res ++;}
                }
            }
        }
	break;
            
        default:
        {
            //unsigned oldoldHoleDeg = MultiS3::shadowDegree[oldHole];
            //unsigned oldnewHoleDeg = MultiS3::shadowDegree[newHole];

            if (G1->hasSelfLoop(peg)) {
                if (G2->hasSelfLoop(oldHole)) --res;
                if (G2->hasSelfLoop(newHole)) ++res;
            }
            for (uint nbr : G1->adjLists[peg]) {
                if (nbr != peg) {
                    --MultiS3::shadowDegree[oldHole];
                    ++MultiS3::shadowDegree[newHole];
                    res -= G2->getEdgeWeight(oldHole, A[nbr]);
                    res += G2->getEdgeWeight(newHole, A[nbr]);
                }
            }
        }
	break;
    }

    const uint n1 = G1->getNumNodes();
    const uint n2 = G2->getNumNodes();
    vector<uint> whichPeg(n2, n1); // value of n1 represents not used
    for (uint i = 0; i < n1; ++i){
	// inverse of the alignment--but this is stupidly EXPENSIVE to do for every proposed move!
	// Instead, we should have an inverse alignment stored simultaneously, and have it updated
	// either explicitly or automatically using overload of the array operator. There is also
	// a reverse() member function but that'll be equally expensive. It should be stored and
	// incrementally updated and checked periodically just like we check incremental scores.
	whichPeg[A[i]] = i;
    }
    assert(whichPeg[newHole] == n1); // hole should be empty

    switch (MultiS3::denominator_type) {
        case MultiS3::mre_k:
        case MultiS3::rt_k:
        {
            uint numNeigh = G2->adjLists[oldHole].size();
            for (uint i = 0; i < numNeigh; i++){
                holeNeigh = G2->adjLists[oldHole][i];
                if (whichPeg[holeNeigh]<n1) {
		    assert(holeNeigh != newHole); // the new hole should be empty!
                    MultiS3::denom -= G2->getEdgeWeight(oldHole,holeNeigh);
		}
            }
	    numNeigh = G2->adjLists[newHole].size();
	    for (uint i =0; i < numNeigh; i++){
		holeNeigh = G2->adjLists[newHole][i];
		if (whichPeg[holeNeigh]<n1) {
		    if(holeNeigh == oldHole) assert(whichPeg[oldHole] == peg);
		    else MultiS3::denom += G2->getEdgeWeight(newHole,holeNeigh);
		}
	    }
	}
	break;
            
        case MultiS3::ee_k:
        {
            uint numNeigh = G2->adjLists[oldHole].size();
            for (uint i =0; i < numNeigh; i++){
                holeNeigh = G2->adjLists[oldHole][i];
                if (whichPeg[holeNeigh]<n1){
                    MultiS3::denom--;
                }
            }
            numNeigh = G1->adjLists[peg].size();
            for (uint i =0; i < numNeigh; i++){
                pegNeigh = G1->adjLists[peg][i];
                if (!G2->getEdgeWeight(A[pegNeigh],oldHole)){
                    MultiS3::denom--;
                }
                if (A[pegNeigh]!=newHole and !G2->getEdgeWeight(A[pegNeigh],newHole)){
                    MultiS3::denom++;
                }
            }
            numNeigh = G2->adjLists[newHole].size();
            for (uint i =0; i < numNeigh; i++){
                holeNeigh = G2->adjLists[newHole][i];
                if (whichPeg[holeNeigh]<n1 and peg!=whichPeg[holeNeigh] ){
                    MultiS3::denom++;
                }
            }
        }
	break;
            
        default:
	{
#if 0
	    if (oldoldHoleDeg > 0 and !MultiS3::shadowDegree[oldHole]) MultiS3::denom -= 1;
            if (oldnewHoleDeg > 0 and !MultiS3::shadowDegree[newHole]) MultiS3::denom += 1;
#else
            uint numNeigh = G2->adjLists[oldHole].size();
            for (uint i =0; i < numNeigh; i++){
                holeNeigh = G2->adjLists[oldHole][i];
                if (whichPeg[holeNeigh]<n1){
                    MultiS3::denom--;
                }
            }
            numNeigh = G1->adjLists[peg].size();
            for (uint i =0; i < numNeigh; i++){
                pegNeigh = G1->adjLists[peg][i];
                if (!G2->getEdgeWeight(A[pegNeigh],oldHole)){
                    MultiS3::denom--;
                }
                if (A[pegNeigh]!=newHole and !G2->getEdgeWeight(A[pegNeigh],newHole)){
                    MultiS3::denom++;
                }
            }
            numNeigh = G2->adjLists[newHole].size();
            for (uint i =0; i < numNeigh; i++){
                holeNeigh = G2->adjLists[newHole][i];
                if (whichPeg[holeNeigh]<n1 and peg!=whichPeg[holeNeigh] ){
                    MultiS3::denom++;
                }
            }  	    
#endif
	}
	break;
    }
    return res;
}

// Return change in NUMERATOR only
int SANA::MS3IncSwapOp(uint peg1, uint peg2, uint hole1, uint hole2) {
      uint pegNeigh, holeNeigh, diff;
      switch (MultiS3::numerator_type){
          case MultiS3::ra_k:
          {
              int res = 0;
              const uint n = G1->adjLists[peg1].size();
              uint i;
              for (i = 0; i < n; ++i) {
                  pegNeigh = G1->adjLists[peg1][i];
                  if (pegNeigh!=peg2) {
		      res -= G2->getEdgeWeight(hole1,A[pegNeigh]);
		      res += G2->getEdgeWeight(hole2,A[pegNeigh]);
		  }
              }
              const uint m = G1->adjLists[peg2].size();
              for (i = 0; i < m; ++i) {
                  pegNeigh = G1->adjLists[peg2][i];
                  if (pegNeigh!=peg1) {
		      res -= G2->getEdgeWeight(hole2,A[pegNeigh]);
		      res += G2->getEdgeWeight(hole1,A[pegNeigh]);
		  }
              }
              return res;
          }
              break;
              
          case MultiS3::ra_global:
          {
              int res = 0;
              const uint n = G1->adjLists[peg1].size();
              uint i = 0;
              bool ladder = false;
              for (; i < n; ++i) {
                  pegNeigh = G1->adjLists[peg1][i];
                  if (G1->getEdgeWeight(pegNeigh,peg1)>0){
                      diff = G2->getEdgeWeight(hole1,A[pegNeigh]) + 1;
                      ladder = (diff>1?true:false);
                      if (ladder){
                          res--;
                          if (G2->getEdgeWeight(hole1,A[pegNeigh]) == 1){res--;}
                      }

                      diff = G2->getEdgeWeight(hole2,A[pegNeigh]) + 1;
                      if (hole2!=A[pegNeigh]){
                          if (diff==2){res+=2;}
                          else if (diff>1){res++;}
                      }
                  }
              }
              if (G1->getEdgeWeight(peg1,peg2)==1){
                  res++;
                  if (G2->getEdgeWeight(hole2,hole1)==1){res++;}
                  else if (G2->getEdgeWeight(hole2,hole1)==0){res--;}
              }
              
              const uint m = G1->adjLists[peg2].size();
              for (i = 0; i < m; ++i) {
                  pegNeigh = G1->adjLists[peg2][i];
                  if (G1->getEdgeWeight(pegNeigh,peg2)>0){
                      diff = G2->getEdgeWeight(hole2,A[pegNeigh]) + 1;

                      ladder = (diff>1?true:false);
                      if (ladder and pegNeigh!=peg1 and G2->getEdgeWeight(hole2,A[pegNeigh])){
                          res--;
                          if (G2->getEdgeWeight(hole2,A[pegNeigh]) == 1){res--;}
                          }
                      
                      diff = G2->getEdgeWeight(hole1,A[pegNeigh]) + 1;
                      if (hole1!=A[pegNeigh]){
                          if (diff==2){res+=2;}
                          else if (diff>1){res++;}
                      }
                  }
              }
              return res;
          }
              break;
          case MultiS3::la_k:
          {
               int res = 0;
               const uint n = G1->adjLists[peg1].size();
               uint i = 0;
               bool ladder = false;
               for (; i < n; ++i) {
                   pegNeigh = G1->adjLists[peg1][i];
                   diff = G2->getEdgeWeight(hole1,A[pegNeigh]) + 1;

                   ladder = (diff>1?true:false);
                   diff = G2->getEdgeWeight(hole2,A[pegNeigh]) + 1;
                   if (hole2==A[pegNeigh]){
                       diff=0;
                   }

                   if (ladder and diff<=1){
                       res--;
                   }else if (not ladder and diff>1){
                       res++;
                   }
               }
               const uint m = G1->adjLists[peg2].size();
               for (i = 0; i < m; ++i) {
                   pegNeigh = G1->adjLists[peg2][i];
                   diff = G2->getEdgeWeight(hole2,A[pegNeigh]) + 1;

                   ladder = (diff>1?true:false);
                   diff = G2->getEdgeWeight(hole1,A[pegNeigh])+ 1;
                   if (hole1==A[pegNeigh]){
                       diff=0;
                   }

                   if (ladder and diff<=1){
                       res--;
                   }else if (not ladder and diff>1){
                       res++;
                   }
               }
               if(G2->getEdgeWeight(hole1,hole2) and G1->getEdgeWeight(peg1,peg2))
               {
                   diff = ( G2->getEdgeWeight(hole1,A[peg2]) + 1);

                   res += 2*(diff>1?1:0);
               }
               return res;
          }
              break;
          case MultiS3::la_global:
          {
              int res = 0;
              const uint n = G1->adjLists[peg1].size();
              uint i = 0;
              bool ladder = false;
              
              for (; i < n; ++i) {
                  pegNeigh = G1->adjLists[peg1][i];
                  if (G1->getEdgeWeight(pegNeigh,peg1)>0){
                      diff = G2->getEdgeWeight(hole1,A[pegNeigh]) + 1;

                      ladder = (diff>1?true:false);
                      if (ladder and G2->getEdgeWeight(hole1,A[pegNeigh]) == 1){res--;}
                      if (hole2!=A[pegNeigh] and G2->getEdgeWeight(hole2,A[pegNeigh])==1){res++;}
                  }
              }
              if (G2->getEdgeWeight(hole2,A[peg1])==1 and G1->getEdgeWeight(peg1,peg2)==1){res++;}
              
              const uint m = G1->adjLists[peg2].size();
              for (i = 0; i < m; ++i) {
                  pegNeigh = G1->adjLists[peg2][i];
                  if (G1->getEdgeWeight(pegNeigh,peg2)>0){
                      diff = G2->getEdgeWeight(hole2,A[pegNeigh]) + 1;

                      ladder = (diff>1?true:false);
                      if (ladder and pegNeigh!=peg1 and G2->getEdgeWeight(hole2,A[pegNeigh])== 1){res--;}
                      if (hole1!=A[pegNeigh] and G2->getEdgeWeight(hole1,A[pegNeigh])==1 and pegNeigh!=peg1){res++;}
                  }
              }
              return res;
          }
              break;
          default:
          {
              int res = 0;
              //uint oldhole1Deg = MultiS3::shadowDegree[hole1];
              //uint oldhole2Deg = MultiS3::shadowDegree[hole2];
              if (G1->hasSelfLoop(peg1)) {
                  if (G2->hasSelfLoop(hole1)) --res;
                  if (G2->hasSelfLoop(hole2)) ++res;
              }
              if (G1->hasSelfLoop(peg2)) {
                  if (G2->hasSelfLoop(hole1)) --res;
                  if (G2->hasSelfLoop(hole2)) ++res;
              }
              for (uint nbr : G1->adjLists[peg1]) {
                  if (nbr != peg1) {
                      --MultiS3::shadowDegree[hole1];
                      ++MultiS3::shadowDegree[hole2];
                      res -= G2->getEdgeWeight(hole1, A[nbr]);
                      res += G2->getEdgeWeight(hole2, A[nbr]);
                  }
              }
              for (uint nbr : G1->adjLists[peg2]) {
                  if (nbr != peg1) {
                      --MultiS3::shadowDegree[hole2];
                      ++MultiS3::shadowDegree[hole1];
                      res -= G2->getEdgeWeight(hole2, A[nbr]);
                      res += G2->getEdgeWeight(hole1, A[nbr]);
                  }
              }
//              if (oldhole1Deg > 0 && !MultiS3::shadowDegree[hole1]) MultiS3::denom -= 1;
//              if (oldhole2Deg > 0 && !MultiS3::shadowDegree[hole2]) MultiS3::denom += 1;
              return res;

          }
              break;
      }
    const uint n1 = G1->getNumNodes();
    const uint n2 = G2->getNumNodes();
    vector<uint> whichPeg(n2, n1);
    for (uint i = 0; i < n1; ++i) whichPeg[A[i]] = i; // inverse of the alignment
    switch (MultiS3::denominator_type){
      case MultiS3::rt_k:
        {
              uint numNeigh = G2->adjLists[hole1].size();
              for (uint i=0; i < numNeigh; i++){
                  holeNeigh = G2->adjLists[hole1][i];
                  if (holeNeigh!=hole2 && whichPeg[holeNeigh]<n1){
                      MultiS3::denom-=G2->getEdgeWeight(hole1,holeNeigh);
                      MultiS3::denom+=G2->getEdgeWeight(hole2,holeNeigh);
                  }
              }
              numNeigh = G2->adjLists[hole2].size();
              for (uint i =0; i < numNeigh; i++){
                  holeNeigh = G2->adjLists[hole2][i];
                  if (holeNeigh != hole1 && whichPeg[holeNeigh]<n1){
                      MultiS3::denom-=G2->getEdgeWeight(hole2,holeNeigh);
                      MultiS3::denom+=G2->getEdgeWeight(hole1,holeNeigh);
                   }
              }
          }
              break;
          case MultiS3::ee_k:
          {
              uint numNeigh = G2->adjLists[hole1].size();
              for (uint i =0; i < numNeigh; i++){
                  holeNeigh = G2->adjLists[hole1][i];
                  if (whichPeg[holeNeigh]<n1 and holeNeigh!=hole2 and G2->getEdgeWeight(holeNeigh,hole1)){
                      MultiS3::denom--;
                  }
              }
              numNeigh = G1->adjLists[peg1].size();
              for (uint i =0; i < numNeigh; i++){
                  pegNeigh = G1->adjLists[peg1][i];
                    if (pegNeigh!=peg2){
                        if (!G2->getEdgeWeight(A[pegNeigh],hole1)){
                            MultiS3::denom--;
                        }
                        if (!G2->getEdgeWeight(A[pegNeigh],hole2)){
                            MultiS3::denom++;
                        }
                    }
              }

              numNeigh = G2->adjLists[hole2].size();
              for (uint i =0; i < numNeigh; i++){
                  holeNeigh = G2->adjLists[hole2][i];
                  if (holeNeigh!=hole1 and whichPeg[holeNeigh]<n1 and peg1!=whichPeg[holeNeigh] and !G2->getEdgeWeight(holeNeigh,hole2)){
                      MultiS3::denom++;
                  }
              }
              numNeigh = G2->adjLists[hole2].size();
              for (uint i =0; i < numNeigh; i++){
                  holeNeigh = G2->adjLists[hole2][i];
                  if (holeNeigh!=hole1 and whichPeg[holeNeigh]<n1 and !G2->getEdgeWeight(hole2,holeNeigh)){
                      MultiS3::denom--;
                  }
              }
              
              numNeigh = G1->adjLists[peg2].size();
              for (uint i =0; i < numNeigh; i++){
                  pegNeigh = G1->adjLists[peg2][i];
                  if (pegNeigh!=peg1){
                      if (!G2->getEdgeWeight(A[pegNeigh],hole2)){
                          MultiS3::denom--;
                      }
                      if (!G2->getEdgeWeight(A[pegNeigh],hole1)){
                          MultiS3::denom++;
                      }
                  }
              }
              numNeigh = G2->adjLists[hole1].size();
              for (uint i =0; i < numNeigh; i++){
                  holeNeigh = G2->adjLists[hole1][i];
                  if ( G2->getEdgeWeight(hole1,holeNeigh)>0 and whichPeg[holeNeigh]<n1 and holeNeigh!=hole2 and peg2!=whichPeg[holeNeigh]){
                      MultiS3::denom++;
                  }
              }
        }
            break;
	default:
        {
#if 0
		if (oldhole1Deg > 0 && !MultiS3::shadowDegree[hole1]) MultiS3::denom -= 1;
        	if (oldhole2Deg > 0 && !MultiS3::shadowDegree[hole2]) MultiS3::denom += 1;	
#else
              uint numNeigh = G2->adjLists[hole1].size();
              for (uint i =0; i < numNeigh; i++){
                  holeNeigh = G2->adjLists[hole1][i];
                  if (whichPeg[holeNeigh]<n1 and holeNeigh!=hole2 and G2->getEdgeWeight(holeNeigh,hole1)){
                      MultiS3::denom--;
                  }
              }
              numNeigh = G1->adjLists[peg1].size();
              for (uint i =0; i < numNeigh; i++){
                  pegNeigh = G1->adjLists[peg1][i];
                    if (pegNeigh!=peg2){
                        if (!G2->getEdgeWeight(A[pegNeigh],hole1)){
                            MultiS3::denom--;
                        }
                        if (!G2->getEdgeWeight(A[pegNeigh],hole2)){
                            MultiS3::denom++;
                        }
                    }
              }

              numNeigh = G2->adjLists[hole2].size();
              for (uint i =0; i < numNeigh; i++){
                  holeNeigh = G2->adjLists[hole2][i];
                  if (holeNeigh!=hole1 and whichPeg[holeNeigh]<n1 and peg1!=whichPeg[holeNeigh] and !G2->getEdgeWeight(holeNeigh,hole2)){
                      MultiS3::denom++;
                  }
              }
              numNeigh = G2->adjLists[hole2].size();
              for (uint i =0; i < numNeigh; i++){
                  holeNeigh = G2->adjLists[hole2][i];
                  if (holeNeigh!=hole1 and whichPeg[holeNeigh]<n1 and !G2->getEdgeWeight(hole2,holeNeigh)){
                      MultiS3::denom--;
                  }
              }
              
              numNeigh = G1->adjLists[peg2].size();
              for (uint i =0; i < numNeigh; i++){
                  pegNeigh = G1->adjLists[peg2][i];
                  if (pegNeigh!=peg1){
                      if (!G2->getEdgeWeight(A[pegNeigh],hole2)){
                          MultiS3::denom--;
                      }
                      if (!G2->getEdgeWeight(A[pegNeigh],hole1)){
                          MultiS3::denom++;
                      }
                  }
              }
              numNeigh = G2->adjLists[hole1].size();
              for (uint i =0; i < numNeigh; i++){
                  holeNeigh = G2->adjLists[hole1][i];
                  if ( G2->getEdgeWeight(hole1,holeNeigh)>0 and whichPeg[holeNeigh]<n1 and holeNeigh!=hole2 and peg2!=whichPeg[holeNeigh]){
                      MultiS3::denom++;
                  }
              }
#endif
        }
    }
}

int SANA::inducedEdgesIncChangeOp(uint peg, uint oldHole, uint newHole) {
    int res = 0;
    for (uint nbr : G2->adjLists[oldHole]) res -= assignedNodesG2[nbr];
    for (uint nbr : G2->adjLists[newHole]) res += assignedNodesG2[nbr];
    res -= G2->getEdgeWeight(oldHole, newHole); //address case changing between adjacent nodes:
    return res;
}

double SANA::localScoreSumIncChangeOp(const vector<vector<float>>& sim, uint peg, uint oldHole, uint newHole) {
    return sim[peg][newHole] - sim[peg][oldHole];
}

double SANA::localScoreSumIncSwapOp(const vector<vector<float>>& sim, uint peg1, uint peg2, uint hole1, uint hole2) {
    return sim[peg1][hole2] - sim[peg1][hole1] + sim[peg2][hole1] - sim[peg2][hole2];
}

double SANA::JSIncChangeOp(uint peg, uint oldHole, uint newHole) {
    if (jsWeight == 0) return 0;

    //eval newJsSum
    //update alignedByNode with peg and peg neighbors using oldHole and newHole

    // eval for peg from scratch
    uint pegOldAlingedEdges = alignedByNode[peg];
    uint pegAlignedEdges = 0;
    vector<uint> pegNeighbors = G1->adjLists[peg];
    for (uint nbr : pegNeighbors) {
        uint neighborAlignedTo = A[nbr]; 
        pegAlignedEdges += G2->getEdgeWeight(newHole, neighborAlignedTo);
    }
    alignedByNode[peg] = pegAlignedEdges;
    //update newJsSum
    uint pegTotalEdges = pegNeighbors.size();
    double change = ((pegAlignedEdges - pegOldAlingedEdges)/(double)pegTotalEdges);

    // for each peg neighbor update do iterative changes to the jsAlingedByNode vector
    // in each update get the G2mapping of neighbor and then check if edge was aligned by oldHole to G2mapping and reduce score if newHole to G2mapping doesnt exist
    // increase score if oldHole to G2 mapping edge didnt exist but newHole to G2 mapping does
    //no changes other wise
    for (uint nbr : pegNeighbors) {
        uint neighborAlignedTo = A[nbr];
        uint neighborOldAlignedEdges = alignedByNode[nbr];
        uint neighborTotalEdges = G1->adjLists[nbr].size();
        alignedByNode[nbr] -= G2->getEdgeWeight(oldHole, neighborAlignedTo);
        alignedByNode[nbr] += G2->getEdgeWeight(newHole, neighborAlignedTo);
        //update newJsSum
        change += ((alignedByNode[nbr] - neighborOldAlignedEdges)/(double)neighborTotalEdges);
    }
    return change;
}

double SANA::JSIncSwapOp(uint peg1, uint peg2, uint hole1, uint hole2) {
    if (jsWeight == 0) return 0;

    //eval swap as two pegs and then loop neighbors

    uint peg1OldAlingedEdges = alignedByNode[peg1];
    uint peg1AlignedEdges = 0;
    vector<uint> peg1Neighbors = G1->adjLists[peg1];
    uint peg1TotalEdges = peg1Neighbors.size();

    uint peg2OldAlingedEdges = alignedByNode[peg2];
    uint peg2AlignedEdges = 0;
    vector<uint> peg2Neighbors = G1->adjLists[peg2];
    uint peg2TotalEdges = peg2Neighbors.size();

    // eval for peg1 from sratch
    for (uint nbr : peg1Neighbors) {
        uint neighborAlignedTo = A[nbr];
        // if (G2->getEdgeWeight(hole2, neighborAlignedTo) == true) {
        //     peg1AlignedEdges += 1;
        // }
        peg1AlignedEdges = G2->getEdgeWeight(hole2, neighborAlignedTo);
    }
    alignedByNode[peg1] = peg1AlignedEdges;

    double change = ((peg1AlignedEdges - peg1OldAlingedEdges)/(double)peg1TotalEdges);

    //for each peg neighbor update do iterative changes to the jsAlingedByNode vector
    //in each update get the G2mapping of neighbor and then check if edge was aligned by
    //oldHole to G2mapping and reduce score if newHole to G2mapping doesnt exist
    //increase score if oldHole to G2 mapping edge didnt exist but newHole to G2 mapping does
    //no changes other wise
    for (uint nbr : peg1Neighbors) {
        uint neighborAlignedTo = A[nbr];
        uint neighborOldAlignedEdges = alignedByNode[nbr];
        uint neighborTotalEdges = G1->adjLists[nbr].size();
        if (std::find (peg1Neighbors.begin(), peg1Neighbors.end(), nbr) == peg1Neighbors.end()) {
            alignedByNode[nbr] -= G2->getEdgeWeight(hole1, neighborAlignedTo);
            alignedByNode[nbr] += G2->getEdgeWeight(hole2, neighborAlignedTo);
            //update newJsSum
            change += ((alignedByNode[nbr] - neighborOldAlignedEdges)/(double)neighborTotalEdges);
        }
    }

    // eval for peg2 from scratch
    for (uint nbr : peg2Neighbors) {
        uint neighborAlignedTo = A[nbr];
        peg2AlignedEdges += G2->getEdgeWeight(hole1, neighborAlignedTo);
    }
    alignedByNode[peg2] = peg2AlignedEdges;
    change += ((peg2AlignedEdges - peg2OldAlingedEdges)/(double)peg2TotalEdges);

    // for each peg neighbor update do iterative changes to the jsAlingedByNode vector
    // in each update get the G2mapping of neighbor and then check if edge was aligned by oldHole to G2mapping and reduce score if newHole to G2mapping doesnt exist
    // increase score if oldHole to G2 mapping edge didnt exist but newHole to G2 mapping does
    //no changes other wise
    for (uint nbr : peg2Neighbors) {
        uint neighborAlignedTo = A[nbr];
        uint neighborOldAlignedEdges = alignedByNode[nbr];
        uint neighborTotalEdges = G1->adjLists[nbr].size();
        if (std::find (peg1Neighbors.begin(), peg1Neighbors.end(), nbr) == peg1Neighbors.end()) {
            alignedByNode[nbr] -= G2->getEdgeWeight(hole2, neighborAlignedTo);
            alignedByNode[nbr] += G2->getEdgeWeight(hole1, neighborAlignedTo);
            //update newJsSum
            change += ((alignedByNode[nbr] - neighborOldAlignedEdges)/(double)neighborTotalEdges);
        }
    }

    //eval for common neighbors
    vector<uint> peg1peg2commonneighbors(peg1Neighbors.size() + peg2Neighbors.size());
    set_intersection(peg1Neighbors.begin(), peg1Neighbors.end(),
                     peg2Neighbors.begin(), peg2Neighbors.end(),
                     peg1peg2commonneighbors.begin());
    return change;
}

double SANA::WECIncChangeOp(uint peg, uint oldHole, uint newHole) {
    double res = 0;
    for (uint nbr : G1->adjLists[peg]) {
        if (G2->getEdgeWeight(oldHole, A[nbr])) {
            res -= wecSims[peg][oldHole];
            res -= wecSims[nbr][A[nbr]];
        }
        if (G2->getEdgeWeight(newHole, A[nbr])) {
            res += wecSims[peg][newHole];
            res += wecSims[nbr][A[nbr]];
        }
    }
    return res;
}

double SANA::WECIncSwapOp(uint peg1, uint peg2, uint hole1, uint hole2) {
    double res = 0;
    for (uint nbr : G1->adjLists[peg1]) {
        if (G2->getEdgeWeight(hole1, A[nbr])) {
            res -= wecSims[peg1][hole1];
            res -= wecSims[nbr][A[nbr]];
        }
        if (G2->getEdgeWeight(hole2, A[nbr])) {
            res += wecSims[peg1][hole2];
            res += wecSims[nbr][A[nbr]];
        }
    }
    for (uint nbr : G1->adjLists[peg2]) {
        if (G2->getEdgeWeight(hole2, A[nbr])) {
            res -= wecSims[peg2][hole2];
            res -= wecSims[nbr][A[nbr]];
        }
        if (G2->getEdgeWeight(hole1, A[nbr])) {
            res += wecSims[peg2][hole1];
            res += wecSims[nbr][A[nbr]];
        }
    }
    if (G1->hasEdge(peg1, peg2) and G2->hasEdge(hole1, hole2)) {
        res += 2*wecSims[peg1][hole1];
        res += 2*wecSims[peg2][hole2];
    }
    return res;
}

double SANA::EWECIncChangeOp(uint peg, uint oldHole, uint newHole) {
    return EWECSimCombo(peg, newHole) - EWECSimCombo(peg, oldHole);
}

double SANA::EWECIncSwapOp(uint peg1, uint peg2, uint hole1, uint hole2) {
    double score = EWECSimCombo(peg1, hole2) + EWECSimCombo(peg2, hole1) 
                 - EWECSimCombo(peg1, hole1) - EWECSimCombo(peg2, hole2);
    if (G1->hasEdge(peg1, peg2) and G2->hasEdge(hole1, hole2)) {
        score += ewec->getScore(ewec->getColIndex(hole1, hole2), 
                                ewec->getRowIndex(peg1, peg2))/(g1Edges); //correcting for missed edges when swapping 2 adjacent pairs
    }
    return score;
}

double SANA::EWECSimCombo(uint peg, uint hole) {
    double score = 0;
    for (uint nbr : G1->adjLists[peg]) {
        if (G2->getEdgeWeight(hole, A[nbr])) {
            int e1 = ewec->getRowIndex(peg, nbr);
            int e2 = ewec->getColIndex(hole, A[nbr]);
            score+=ewec->getScore(e2,e1);
        }
    }
    return score/(2*g1Edges);
}

int SANA::ncIncChangeOp(uint peg, uint oldHole, uint newHole) {
    int change = 0;
    if (trueAWithValidCountAppended[peg] == oldHole) change -= 1;
    if (trueAWithValidCountAppended[peg] == newHole) change += 1;
    return change;
}

int SANA::ncIncSwapOp(uint peg1, uint peg2, uint hole1, uint hole2) {
    int change = 0;
    if (trueAWithValidCountAppended[peg1] == hole1) change -= 1;
    if (trueAWithValidCountAppended[peg2] == hole2) change -= 1;
    if (trueAWithValidCountAppended[peg1] == hole2) change += 1;
    if (trueAWithValidCountAppended[peg2] == hole1) change += 1;
    return change;
}

void SANA::trackProgress(long long int iter, long long int maxIters) {
    if (!enableTrackProgress) return;
    double fractionTime = maxIters == -1 ? 0 : iter/(double)maxIters;
    double elapsedTime = timer.elapsed();
    uint iterationsElapsed = iterationsPerformed-oldIterationsPerformed;
    if (elapsedTime == 0) oldTimeElapsed = 0;
    double ips = (iterationsElapsed/(elapsedTime-oldTimeElapsed));
    oldTimeElapsed = elapsedTime;
    oldIterationsPerformed = iterationsPerformed;
    cout<<iter/iterationsPerStep<<" ("<<100*fractionTime<<"%,"<<elapsedTime<<"s): score = "<<currentScore;
    cout<< " ips = "<<ips<<", P("<<Temperature<<") = "<<acceptingProbability(avgEnergyInc, Temperature);
    cout<<", pBad = "<<incrementalMeanPBad()<<endl;

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
        double fractionTime = (timer.elapsed()/maxSeconds);
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
    uint integralMin = maxSeconds/60.0;
    string folder = "cache-pbad/"+MC->toString()+"/progress_"+to_string(integralMin)+"/";
    string fileName = folder+G1->getName()+"_"+G2->getName()+"_0.csv";
    ofstream ofs(fileName);
    ofs<<"time,score,avgEnergyInc,Temperature,realTemp,pBad,lower,higher,timer"<<endl;
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
double SANA::getPBad(double temp, double maxTimeInS, int logLevel) {
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
                    <<slowMeanPBad()<<")"<<endl;
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
            if (timer.elapsed() > maxTimeInS) {
                if (verbose) {
                    cerr<<"ran out of time. scoreBuffer:"<<endl;
                    for (uint i = 0; i < scoreBuffer.size(); i++) cerr<<scoreBuffer[i]<<endl;
                    cerr<<endl;
                }
                break;
            }
        }
    }
    double pBadAvgAtEq = slowMeanPBad();
    double nextIps = (double)iter / (double)timer.elapsed();
    pair<double, double> nextPair (temp, nextIps);
    ipsList.push_back(nextPair);
    if (logLevel >= 1) {
        cout<<"> getPBad("<<temp<<") = "<<pBadAvgAtEq<<" (score: "<<currentScore<<")";
        if (reachedEquilibrium) cout<<" (time: "<<timer.elapsed()<<"s)";
        else cout<<" (didn't detect eq. after "<<maxSeconds<<"s)";
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
