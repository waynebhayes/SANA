#include <sys/unistd.h>
#include <chrono>
#include <iostream>
#include <thread>
#include <string>
#include <vector>
#include <utility>
#include <fstream>
#include <iomanip>
#include <stdexcept>
#include <unordered_set>
#include <algorithm>
#include <random>
#include <cmath>
#include <limits>
#include <cassert>
#include <csignal>
#include <cstdio>

#include "SANAThree.hpp"
#include "../measures/SquaredEdgeScore.hpp"
#include "../utils/utils.hpp"
#include "../utils/randomSeed.hpp"
#include "../Report.hpp"
#include "../utils/Stats.hpp"

double static inline acceptingProbability(double energyInc, double temperature) {
    if (temperature == 0.) return energyInc >= 0;
    return energyInc >= 0 ? 1 : exp(energyInc / temperature);
}

bool SANAThree::saveAligAndExitOnInterruption = false;
bool SANAThree::saveAligAndContOnInterruption = false;
SANAThree::SANAThree(const Graph* G1, const Graph* G2, double TInitial, double TDecay, double maxSeconds,
        long long maxIterations, double tolerance, bool addHillClimbing, const MeasureCombination* MC,
        const string& scoreAggrStr, const Alignment& optionalStartAlig, const string& outputFileName,
        const string& localScoresFileName, unsigned threadNumber):
    Method(G1, G2, "SANAThree_" + MC->toString()),
    hillClimbing(addHillClimbing),
    needEC(MC->getWeight("ec") > 0),
    needEM(MC->getWeight("emin") > 0),
    needER(MC->getWeight("er") > 0),
    tolerance(tolerance),
    maxSeconds(maxSeconds),
    maxIterations(maxIterations),
    threadNumber(threadNumber),
    MC(MC),
    startingAlignment(optionalStartAlig),
    outputFileName(outputFileName),
    localScoresFileName(localScoresFileName),
    n1(G1->getNumNodes()),
    n2(G2->getNumNodes()),
    m1(G1->getNumEdges()),
    m2(G2->getNumEdges()),
    tInitial(TInitial),
    tDecay(TDecay) {
    // This should never happen, and if it does, it is 100% user error.
    if (threadNumber >= n1 / 2) {
        throw runtime_error(
            "You should not request more threads than you have G1 nodes.");
    }

    if (tolerance > 0) {
        if (maxIterations > 0 or maxSeconds > 0)
            throw runtime_error(
                "To use iterations or time, first set \"-tolerance 0\" on the command line (NOT RECOMMENDED!)");
    }

    currentScore = 0.;
    alignment.reserve(n1);
    lockedHoles.resize(n2, false);

    if (scoreAggrStr != "sum") {
        cerr << "SANA 3.0 alpha does not yet support score aggregation methods other than weighted"
                "sums.\n If you really need a complex aggregation, either wait for a new version or"
                "run the old version." << endl;
    }

    generator = mt19937(getRandomSeed());
    randomReal = uniform_real_distribution<>(0, 1);


    // NODE COLOR SYSTEM initialization

    assert(G1->numColors() <= G2->numColors());
    constexpr bool COL_DBG = true; //print stats about color/neighbor type probabilities

    vector<uint> numSwapNeighborsByG1Color(G1->numColors(), 0);
    vector<uint> numChangeNeighborsByG1Color(G1->numColors(), 0);
    uint totalNbrCount = 0;
    for (uint g1Id = 0; g1Id < G1->numColors(); g1Id++) {
        string colName = G1->getColorName(g1Id);
        if (not G2->hasColor(colName))
            throw runtime_error("G1 nodes colored " + colName + " cannot be matched to any G2 nodes");
        uint c1 = G1->numNodesWithColor(g1Id);
        uint c2 = G2->numNodesWithColor(G2->getColorId(colName));
        if (c1 > c2)
            throw runtime_error("there are " + to_string(c1) + " G1 nodes colored "
                                + colName + " but only " + to_string(c2) + " such nodes in G2");
        uint numSwapNbrs = c1 * (c1 - 1) / 2, numChangeNbrs = c1 * (c2 - c1);
        numSwapNeighborsByG1Color[g1Id] = numSwapNbrs;
        numChangeNeighborsByG1Color[g1Id] = numChangeNbrs;
        uint numNbrs = numSwapNbrs + numChangeNbrs;
        totalNbrCount += numNbrs;
        if (COL_DBG) {
            cerr << "color " << colName << " has " << numSwapNbrs << " swap nbrs and "
                    << numChangeNbrs << " change nbrs (" << numNbrs << " total)" << endl;
            if (numNbrs == 0) cerr << "color " << colName << " is inactive" << endl;
        }
    }
    if (COL_DBG) cerr << "alignments have " << totalNbrCount << " nbrs in total" << endl;
    if (totalNbrCount == 0)
        throw runtime_error(
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
        actColToChangeProb.push_back(numChangeNbrs / (double) numNbrs);
        actColToG1ColId.push_back(g1Id);
    }
    //due to rounding errors, the last number may not be exactly 1, so we correct it
    actColToAccumProbCutpoint.back() = 1;
    if (COL_DBG) {
        cerr << "Active colors:" << endl;
        vector<vector<string> > colTable;
        colTable.push_back({"id", "name", "color P", "accum P", "change-P", "swap-P"});
        for (uint i = 0; i < actColToG1ColId.size(); i++) {
            string name = G1->getColorName(actColToG1ColId[i]);
            double colP = actColToAccumProbCutpoint[i] - (i > 0 ? actColToAccumProbCutpoint[i - 1] : 0);
            colTable.push_back({
                to_string(i), name, to_string(colP), to_string(actColToAccumProbCutpoint[i]),
                to_string(actColToChangeProb[i]), to_string(1 - actColToChangeProb[i])
            });
        }
        printTable(colTable, 4, cerr);
        cerr << endl;
    }

    //init g2NodeToActColId. For each node, we do the following transformations:
    //g2Node -> g2ColorId -> g1ColorId -> actColId
    vector<uint> g2ToG1ColorIdMap = G2->myColorIdsToOtherGraphColorIds(*G1);
    INVALID_ACTIVE_COLOR_ID = n1;
    vector<uint> g1ColIdToActColId(G1->numColors(), INVALID_ACTIVE_COLOR_ID);
    for (uint i = 0; i < actColToG1ColId.size(); i++) {
        g1ColIdToActColId[actColToG1ColId[i]] = i;
    }
    g2NodeToActColId = vector<uint>(n2, INVALID_ACTIVE_COLOR_ID);
    for (uint g2Node = 0; g2Node < n2; g2Node++) {
        uint g2ColorId = G2->getNodeColor(g2Node);
        uint g1ColorId = g2ToG1ColorIdMap[g2ColorId];
        if (g1ColorId == Graph::INVALID_COLOR_ID) continue; //no node in G1 has this color
        g2NodeToActColId[g2Node] = g1ColIdToActColId[g1ColorId];
    }
    //things initialized in initDataStructures because they depend on the starting alignment
    //they have the same size for every run, so we can allocate the size here
    actColToUnassignedG2Nodes = vector<vector<uint> >(actColToG1ColId.size());
}

void SANAThree::initDataStructures() {
    auto assignedNodesG2 = vector<char> (n2);

    Alignment alig;
    if (startingAlignment.size() != 0) alig = startingAlignment;
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

    currentScore = MC->eval(alig);
    alignment = alig.asVector();
}

Alignment SANAThree::runUsingIterations() {
    cerr << "Warning, direct public access to SANA's different run types is being phased out." << endl;
    cerr << "Next time, pretty please use the base SANA run function with appropriate parameters." << endl;
    cerr << "If you spot this message, please contact Marcus at @malongo:matrix.org and let him" << endl;
    cerr << "know the test/parameters you ran to receive this message." << endl;

    return run();
}

Alignment SANAThree::runUsingConfidenceIntervals() {
    cerr << "Warning, direct public access to SANA's different run types is being phased out." << endl;
    cerr << "Next time, pretty please use the base SANA run function with appropriate parameters." << endl;
    cerr << "If you spot this message, please contact Marcus at @malongo:matrix.org and let him" << endl;
    cerr << "know the test/parameters you ran to receive this message." << endl;

    return run();
}

// TODO:
// This is terribly outdated. I'll buy a (soft) cider for anyone who takes it upon themselves to
// make a better version, but this relatively low priority.
void SANAThree::describeParameters(ostream &stream) const {
    stream << "Temperature goldilocks:" << endl;
    stream << "T_initial: " << tInitial << endl;
    stream << "T_decay: " << tDecay << endl;
    stream << "Optimize: " << endl;
    MC->printWeights(stream);
    stream << "Max iterations: " << maxIterations << endl;
    stream << "Execution time: " << maxSeconds << "s" << endl;
}

string SANAThree::fileNameSuffix(const Alignment& Al) const {
    return "_" + extractDecimals(MC->eval(Al),3);
}

Alignment SANAThree::run() {
    initDataStructures();
    setInterruptSignal();

    // See CalculatorHandler comment as to why this is a local variable.
    CalculatorHandler threadPool{threadNumber - 1, *this};

    if (tolerance > 0)
        runConfidenceIntervals(threadPool);
    else
        runIterations(threadPool);

    if (hillClimbing) runHillClimbing(threadPool);

    return alignment;
}

// Why the big duration? Well, this is the least effortful part of the SANA process.
// Might as well do it in a way that REALLY scrambles the alignment.
#define SCRAMBLE_DURATION 1000000000ul
void SANAThree::scramble() {
    for (unsigned i = 0; i < min(static_cast<unsigned long>(n1 * n1), SCRAMBLE_DURATION); i++) {
        changeRequest request = chooseNextRequest();
        implementLastRequest(1., request);
    }
}

#define BATCH_SIZE (static_cast<long long unsigned>(max(n1,n2))) // This is probably too small.
#define LEEWAY 1.75
#define temperatureFunction(f, i, d) (i * exp(-d * f))
void SANAThree::runIterations(CalculatorHandler &threadPool) {
    double maxSecondsWithLeeway;
    long long unsigned maxBatches;
    unsigned batchesPerStep;
    TimerTrue T;
    T.start();
    double iterationsPerSecond;
    {
        unsigned batches = 0;
        while (T.elapsed() < 1.) {
            collectBatch(threadPool, 0.);
            batches++;
        }
        iterationsPerSecond = BATCH_SIZE * batches / T.elapsed();
        batchesPerStep = ceil(batches * 30 / T.elapsed());
    }
    if (maxSeconds > 0) {
        maxBatches = ceil(maxSeconds * iterationsPerSecond / BATCH_SIZE);
        maxSecondsWithLeeway = maxSeconds * LEEWAY;
    }
    else {
        maxBatches = 1 + maxIterations / BATCH_SIZE;
        maxSecondsWithLeeway = 0;
    }
    T.start();
    long long unsigned iter = 0;
    double temperature = tInitial;
    for (; iter < maxBatches; iter += 1) {
        temperature = temperatureFunction(static_cast<double>(iter)/static_cast<double>(maxBatches),
                                                 tInitial, tDecay);
        const batchOutput output = collectBatch(threadPool, temperature);
        if (saveAligAndExitOnInterruption) break;
        if (saveAligAndContOnInterruption) printReportOnInterruption();
        if (iter % batchesPerStep == 0) {
            trackProgress(iter * BATCH_SIZE, static_cast<double>(iter)/static_cast<double>(maxBatches), T.elapsed(),
                temperature, output.averagePBad);
        }
        if (maxSecondsWithLeeway != 0. and T.elapsed() > maxSecondsWithLeeway) break;
    }
    trackProgress(iter * BATCH_SIZE, static_cast<double>(iter)/static_cast<double>(maxBatches), T.elapsed(),
                temperature, 0.);
    cout<<"Performed "<<iter * BATCH_SIZE<<" total iterations\n";
}

// All of these are purely heuristic -Wayne (I think, at least -Marcus)
// TODO: This should be refactored! And the tab characters replaced with spaces!
#define MAX_TAU_STEP 0.01
#define MIN_TAU_STEP 0.001
#define MIN_BATCHES 30
#define HAPPY_BATCHES MIN(10000, (int)(m1+m2))
#define MIN_CONFIDENCE 0.99999
#define TOL_SAFETY_MARGIN 1.07 // empirically this seems to cut failure rates to below 5%.
void SANAThree::runConfidenceIntervals(CalculatorHandler &threadPool) {
    TimerTrue T;
    T.start();

    // TODO: make all of these changeable on the command line
    unsigned batch = 0, batchSize = BATCH_SIZE;
    double tau, tauStep = MAX_TAU_STEP; // dynamically made smaller or bigger as necessary
    assert(tolerance > 0);
    double tolPerStep = tolerance * (tauStep) / TOL_SAFETY_MARGIN;
    double confidence = 1-pow(tolPerStep, 1.5); // empirically works well.
    if(confidence < MIN_CONFIDENCE) confidence = MIN_CONFIDENCE; // doesn't add much CPU to increase confidence.

    bool verbose = true;
    if(verbose) printf("SANAThree::runConfidenceIntervals Parameters: batchSize %d confidence %g tolerance per step %g\n",
	batchSize, confidence, tolPerStep);

    STAT *scoreBatchMeans = StatAlloc(0, 0.0, 0.0, false, false);
    STAT *pBadBatchMeans = StatAlloc(0, 0.0, 0.0, false, false);

    long int lastBatchCount=0;
    double lastPBad = 1.0;
    double previousScore = currentScore;
    double temperature;
    for (tau = 0; tau <= 1; tau += tauStep) {
	    int batchesPerTemperature = 0;
        temperature = temperatureFunction(tau, tInitial, tDecay);

	    // Now the "inner loop"
	    Boolean satisfied = false;
	    while(!satisfied) {
	        if (saveAligAndExitOnInterruption) break;
	        if (saveAligAndContOnInterruption) printReportOnInterruption();

	        const batchOutput output = collectBatch(threadPool, temperature);
	        lastPBad = output.averagePBad;

            ++batch; ++batchesPerTemperature;
		    StatAddSample(scoreBatchMeans, output.averageScore);
		    StatAddSample(pBadBatchMeans, output.averagePBad);

		    if(StatNumSamples(scoreBatchMeans)>=MIN_BATCHES){
		        double pBadInterval = tolPerStep;
		        double scoreInterval = pBadInterval;

		        // The user specifies a *relative* tolerance on the FINAL score... but we don't know what the final
		        // score will be. Thus, early on when the score is low and pBad is high, we punt to using (effectively)
		        // an absolute tolerance by multiplying the tolerance by pBad. Then, as the score increases and
		        // surpasses pBad, transition to a genuine relative tolerance by multiplying by the score.
		        double relativeMultiplier = MAX(StatMean(scoreBatchMeans), StatMean(pBadBatchMeans));

		        // HOWEVER, we also slowly decrease the tolerance (by slowly increasing the Interval), because
		        // sometimes we can get "stuck" for a VERY long time at one temperature because the score
		        // is fluctuating too much. Let's not get stuck too long.
		        relativeMultiplier *= (1+log(batchesPerTemperature));

		        scoreInterval *= relativeMultiplier;
		        pBadInterval *= relativeMultiplier;
		        if(StatConfInterval(scoreBatchMeans, confidence) < scoreInterval &&
			    StatConfInterval(pBadBatchMeans,  confidence) < pBadInterval) satisfied = true;
		        else if(StatNumSamples(scoreBatchMeans) >= HAPPY_BATCHES) {
			    // Reset the batch system if the score is increasing steadily, otherwise it can't "converge" without
			    // an ENORMOUS number of batches to compensate for the "bias" that occurs in early batches.
			    if(StatMean(scoreBatchMeans) > previousScore) { // adding + tolPerSstep/2 seems too much.
			        if(verbose)
				    printf(" ++++> temp %.4g, batchMeanScore %.3f (pBad %.3g) still increasing after %d batches; reset batches and continue\n",
				    temperature, StatMean(scoreBatchMeans),
				    StatConfInterval(pBadBatchMeans,  confidence), StatNumSamples(scoreBatchMeans));
			        fflush(stdout);
			        previousScore = StatMean(scoreBatchMeans);
			        lastBatchCount = 0; StatReset(scoreBatchMeans); StatReset(pBadBatchMeans);
			    } else if(tauStep>MIN_TAU_STEP && StatNumSamples(scoreBatchMeans) >= HAPPY_BATCHES+lastBatchCount) {
			        if(verbose)
				    printf(" ----> %d batches, avg score %g decreased at tau %g; reduce next tauStep from %g",
				        StatNumSamples(scoreBatchMeans), StatMean(scoreBatchMeans), tau, tauStep);
			        fflush(stdout);
			        // tau -= tauStep;
			        tauStep *= 2.0/3.0;
			        if(tauStep < MIN_TAU_STEP) tauStep = MIN_TAU_STEP;
			        // tau += tauStep;
			        if(verbose) printf(" to %g and backtrack to tau %g\n", tauStep, tau);
			        lastBatchCount = StatNumSamples(scoreBatchMeans);
			    }
		        }
		    }
	        }
        trackProgress(batch, tau, T.elapsed(), temperature, lastPBad, batchesPerTemperature,
                      StatMean(scoreBatchMeans), StatMean(pBadBatchMeans));
	    if(tauStep < MAX_TAU_STEP) {
	        if(StatNumSamples(scoreBatchMeans) < HAPPY_BATCHES) {
		    if(verbose) printf(" *****> doing OK at tau %g & %d batches; increasing tauStep from %g",
		        tau, StatNumSamples(scoreBatchMeans), tauStep);
		    tauStep *= 3;
		    if(tauStep > MAX_TAU_STEP) tauStep = MAX_TAU_STEP;
		    if(verbose) printf(" to %g\n", tauStep);
	        }
	        else if(StatMean(scoreBatchMeans) + 0.00 < previousScore) {
		    if(verbose) printf(" !!!!!> score %g is stuck below previous %g; skip region by increasing tauStep from %g",
		        StatMean(scoreBatchMeans), previousScore, tauStep);
		    fflush(stdout);
		    tauStep *= 10; // if the score is not increasing... skip this region
		    if(tauStep > MAX_TAU_STEP) tauStep = MAX_TAU_STEP;
		    if(verbose) printf(" to %g\n", tauStep);
	        }
	    }
	    previousScore = StatMean(scoreBatchMeans);
	    StatReset(scoreBatchMeans); StatReset(pBadBatchMeans);
    }
    cout<<"Performed "<<batch<<" total batches\n";
    trackProgress(batch * batchSize, tau, T.elapsed(), temperature, lastPBad);
}

// I stole this duration from SANA proper. I will repeat the comment there that this is
// "arbitrarily chosen, probably too long". Regardless, shouldn't we be using the actual hillClimbing
// class?? TODO: fix this
// -Marcus
#define HILLCLIMB_DURATION 10000000000u
void SANAThree::runHillClimbing(CalculatorHandler &threadPool) {
    Timer T;
    T.start();

    const unsigned long runTime = 1 + HILLCLIMB_DURATION / BATCH_SIZE;
    unsigned long long iter = 0;
    for (; iter < runTime; iter++) {
        collectBatch(threadPool, 0.);
    }
    cout<<"Hill climbing took "<<T.elapsedString()<<"s"<<endl;
}

SANAThree::batchOutput SANAThree::_singleThreadBatch(CalculatorHandler &threadPool, double temperature) {
    double scoreTotal = 0;
    double pBadTotal = 0;
    for (unsigned long i = 0; i < BATCH_SIZE; i++) {
        // Here, because we have no threads to spare, we are using a "should be private" CalculatorHandler
        // function. This is behavior that is not to be replicated in the rest of this code base
        // unless you have consulted me first.
        // -Marcus
        changeRequest request = chooseNextRequest();
        threadPool._assessChange(request);
        const double pBad = acceptingProbability(request.energyInc, temperature);
        implementLastRequest(pBad, request);

        // Update our totals.
        scoreTotal += currentScore;
        pBadTotal += pBad;
    }
    currentScore = MC->eval(alignment);

    // We handle the compiler the constructor here so that it knows it shouldn't be doing any copying
    return batchOutput{scoreTotal/BATCH_SIZE, pBadTotal/BATCH_SIZE};
}

SANAThree::batchOutput SANAThree::_multiThreadBatch(CalculatorHandler &threadPool, double temperature) {
    double scoreTotal = 0;
    double pBadTotal = 0;

    // A fresh set of requests for CalculatorHandler to chew on
    for (unsigned long i = 0; i < threadNumber - 1; i++) {
        // Feed the threadpool a new request
        threadPool.submitRequest(chooseNextRequest());
    }

    // The main loop
    for (unsigned long i = 0; i < BATCH_SIZE - (threadNumber - 1); i++) {
        // Feed the threadpool a new request
        threadPool.submitRequest(chooseNextRequest());

        // Extract and process a new request
        changeRequest outputRequest = threadPool.extractRequest();
        const double pBad = acceptingProbability(outputRequest.energyInc, temperature);
        implementLastRequest(pBad, outputRequest);

        // Update our totals
        scoreTotal += currentScore;
        pBadTotal += pBad;
    }

    // Clean up the requests to bring threadPool back to _requestBalance = 0;
    for (unsigned long i = 0; i < threadNumber - 1; i++) {
        // Extract and process a new request
        changeRequest outputRequest = threadPool.extractRequest();
        const double pBad = acceptingProbability(outputRequest.energyInc, temperature);
        implementLastRequest(pBad, outputRequest);

        // Update our totals
        scoreTotal += currentScore;
        pBadTotal += pBad;
    }

    // For a multi-threaded SA where the alignment is changing while energyInc is being calculated,
    // sometimes small errors in the score will accumulate. This update is to make sure we never get
    // *too* far out of sync.
    currentScore = MC->eval(alignment);

    // We handle the compiler the constructor here so that it knows it shouldn't be doing any copying
    return batchOutput{scoreTotal/BATCH_SIZE, pBadTotal/BATCH_SIZE};
}

SANAThree::changeRequest SANAThree::chooseNextRequest() {
    double p;

    // Request parameters with dummy values to shut up the linter.
    bool twoPegs;
    unsigned peg1 = 0;
    unsigned peg2 = -1;
    unsigned hole1 = 0;
    unsigned hole2 = 0;
    unsigned activeColorSANA = 0;
    unsigned unassignedVecIndex = 0;

    while (true) {
        // Part 1: Choose the active color
        switch (actColToChangeProb.size()) {
            case 0: throw runtime_error("Something was initialized incorrectly,"
                                        " there should always be an active color.\n"
                                        "Did you input a null graph?");
            case 1: activeColorSANA = 0; break;
            case 2:
                p = randomReal(generator);
                activeColorSANA = p < actColToAccumProbCutpoint[0] ? 0 : 1; break;
            default:
                // General case: generate a random p and then find where within the given bounds it falls.
                // These bounds are precalculated so that each range is proportional to the number of
                // nodes of that graph across the networks.
                p = randomReal(generator);
                auto iter = lower_bound(actColToAccumProbCutpoint.begin(), actColToAccumProbCutpoint.end(), p);
                assert(iter != actColToAccumProbCutpoint.end());
                activeColorSANA = iter - actColToAccumProbCutpoint.begin();
        }

        // Part 2: Choose a swap or a flop
        p = randomReal(generator);
        twoPegs = p < actColToChangeProb[activeColorSANA];

        const unsigned activeColorG1 = actColToG1ColId[activeColorSANA];

        for (unsigned i = 0; i < 15; i++) {
            peg1 = G1->getNodesWithColor(activeColorG1)->at(randUnsigned(0, G1->numNodesWithColor(activeColorG1) - 1, generator));
            hole1 = alignment[peg1];
            if (not lockedHoles[hole1]) break;
        }
        // If we can't find a peg with an unlocked hole, restart.
        if (lockedHoles[hole1]) continue;

        // Choose second hole and, if a swap, the second peg.
        if (twoPegs) {
            for (unsigned i = 0; i < 30; i++) {
                peg2 = G1->getNodesWithColor(activeColorG1)->at(randUnsigned(0, G1->numNodesWithColor(activeColorG1) - 1, generator));
                hole2 = alignment[peg2];
                if (not lockedHoles[hole2] && peg1 != peg2) break;
            }
            // If we can't find a peg in an unlocked hole that isn't peg1, restart
            if (lockedHoles[hole2] || peg1 == peg2) continue;
        }
        else {
            const unsigned numUnassignedWithColor = actColToUnassignedG2Nodes[activeColorSANA].size();

            // I am pretty sure this is an impossible case, but it doesn't hurt to double-check
            // -Marcus
            if (numUnassignedWithColor == 0) continue;
            for (unsigned i = 0; i < 30; i++) {
                unassignedVecIndex = randUnsigned(0, numUnassignedWithColor-1, generator);
                hole2 = actColToUnassignedG2Nodes[activeColorSANA][unassignedVecIndex];
                if (not lockedHoles[hole2]) break;
            }
            // If we can't find an unlocked hole
            if (lockedHoles[hole2]) continue;
        }

        // We have both holes, we can safely lock now.
        lockedHoles[hole1] = true;
        lockedHoles[hole2] = true;
        break;
    }
    return changeRequest{twoPegs, peg1, peg2, hole1, hole2, activeColorSANA, unassignedVecIndex};
}

// This probably deserves a rework. I am uncomfortable with the current implementation and pushing
// it all off into its own function has solved this only marginally. It works, but there has got to
// be a less intrusive way to do this!
void SANAThree::implementLastRequest(double pBad, const changeRequest &input) {
    lockedHoles[input.hole1] = false;
    lockedHoles[input.hole2] = false;

    if (randomReal(generator) >= pBad) return;

    alignment[input.peg1] = input.hole2;
    if (input.twoPegs) alignment[input.peg2] = input.hole1; // Swap
    else actColToUnassignedG2Nodes[input.activeColorSANA][input.unoccupiedHole2ID] = input.hole1; // Move
    currentScore += input.energyInc;
}

// TODO
// This function should be written in C++, not C, and should give more useful statistics like in
// SANA 2.0
void SANAThree::trackProgress(long long unsigned iter, double fractionTime, double elapsedTime,
    double temperature, double lastAvgPBad, unsigned batches, double batchScore, double batchPbad) const {

    static double oldTimeElapsed = 0.;
    static long long unsigned lastIterations = 0;
    if (fractionTime == 0.) {
        oldTimeElapsed = 0;
        lastIterations = 0;
    }
    const double ips = (iter - lastIterations) / (elapsedTime - oldTimeElapsed);
    oldTimeElapsed = elapsedTime;
    lastIterations = iter;

    printf("%lld (%.5g%%,%.1fs): score = %.3g ips = %.2g, P(%.3g) = %.3g", iter, 100*fractionTime,
        elapsedTime, currentScore, ips, temperature, lastAvgPBad);
    if(batches) printf(" batches %d bSc %.3g, bpBad %.3g", batches, batchScore, batchPbad);
    printf("\n");
    fflush(stdout);
}

void sigHandlerThree(const int s) {
    string line;
    if(s == SIGINT) {
        int c = 3; // default to save and continue
        // probably an interactive ^C
        do {
            cout<<"Select an option (0 - 3):"<<endl<<"  (0) Do nothing and continue"<<endl<<"  (1) Exit"<<endl
            <<"  (2) Save Alignment and Exit"<<endl<<"  (3) Save Alignment and Continue"<<endl<<">> ";
            cin >> c;
            if (cin.eof()) { // hmm, assume ^D means "save and continue"
                SANAThree::saveAligAndContOnInterruption = true;
                cin.clear();
                return;
            }
            if (cin.fail()) {
                c = -1;
                cin.clear();
                cin.ignore(numeric_limits<streamsize>::max(), '\n');
            }
            if      (c == 0) cout<<"Continuing..."<<endl;
            else if (c == 1) exit(0);
            else if (c == 2) SANAThree::saveAligAndExitOnInterruption = true;
            else if (c == 3) SANAThree::saveAligAndContOnInterruption = true;
        } while (c < 0 || c > 3);
    }
    else if(s == SIGTERM) // probably sent via top(1), so save and exit
        SANAThree::saveAligAndExitOnInterruption = true;
    else // any other signal (eg USR1 or something unexpected), save and continue
        SANAThree::saveAligAndContOnInterruption = true;
}

void SANAThree::setInterruptSignal() {
    saveAligAndExitOnInterruption = false;
    struct sigaction sigInt;
    sigInt.sa_handler = sigHandlerThree;
    sigemptyset(&sigInt.sa_mask);
    sigInt.sa_flags = 0;
    sigaction(SIGINT, &sigInt, nullptr);
    sigaction(SIGTERM, &sigInt, nullptr);
    sigaction(SIGHUP, &sigInt, nullptr);
}

void SANAThree::printReportOnInterruption() const {
    saveAligAndContOnInterruption = false; //reset value
    auto timestamp = string(currentDateTime()); //necessary to make it not const
    std::replace(timestamp.begin(), timestamp.end(), ' ', '_');
    string outFile = outputFileName+"_"+timestamp;
    string localFile = localScoresFileName+"_"+timestamp;
    Report::saveReport(*G1, *G2, alignment, *MC, this, outFile, true);
    Report::saveLocalMeasures(*G1, *G2, alignment, *MC, this, localFile);
    cout << "Alignment saved. SANAThree will now continue." << endl;
}
