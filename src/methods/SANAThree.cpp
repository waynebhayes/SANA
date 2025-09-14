#include <sys/unistd.h>
#include <chrono>
#include <iostream>
#include <thread>
#include <string>
#include <vector>
#include <fstream>
#include <iomanip>
#include <stdexcept>
#include <algorithm>
#include <random>
#include <cmath>
#include <limits>
#include <cassert>
#include <csignal>
#include <cstdio>

#include "SANAThree.hpp"
#include "BatchHarvester.hpp"

#include "../measures/SquaredEdgeScore.hpp"
#include "../utils/utils.hpp"
#include "../Report.hpp"
#include "../utils/Stats.hpp"

bool SANAThree::saveAligAndExitOnInterruption = false;
bool SANAThree::saveAligAndContOnInterruption = false;
SANAThree::SANAThree(const Graph* G1, const Graph* G2, double TInitial, double TDecay, double maxSeconds,
        long long maxIterations, double tolerance, bool addHillClimbing, const MeasureCombination* MC,
        const string& scoreAggrStr, const Alignment& optionalStartAlig, const string& outputFileName,
        const string& localScoresFileName, unsigned threadNumber):
    Method(G1, G2, "SANAThree_" + MC->toString()),
    n1(G1->getNumNodes()),
    n2(G2->getNumNodes()),
    m1(G1->getNumEdges()),
    m2(G2->getNumEdges()),
    hillClimbing(addHillClimbing),
    needEC(MC->getWeight("ec") > 0),
    needEM(MC->getWeight("emin") > 0),
    needER(MC->getWeight("er") > 0),
    tolerance(tolerance),
    maxSeconds(maxSeconds),
    maxIterations(maxIterations),
    batchSize(max(n1,n2)),
    threadNumber(threadNumber),
    MC(MC),
    startingAlignment(optionalStartAlig),
    outputFileName(outputFileName),
    localScoresFileName(localScoresFileName),
    tInitial(TInitial),
    tDecay(TDecay),
    holeLocks(n2, false) {
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

    if (scoreAggrStr != "sum") {
        cerr << "SANA 3.0 alpha does not yet support score aggregation methods other than weighted"
                "sums.\n If you really need a complex aggregation, either wait for a new version or"
                "run the old version." << endl;
    }


    randomReal = uniform_real_distribution<>(0, 1);

    // NODE COLOR SYSTEM initialization

    assert(G1->numColors() <= G2->numColors());


    swapsPerColor.reserve(G1->numColors() + 1);
    movesPerColor.reserve(G1->numColors() + 1);
    swapsPerColor.push_back(0);
    movesPerColor.push_back(0);
    numSwaps = 0;
    numAdjacentAlignments  = 0;
    for (uint g1Id = 0; g1Id < G1->numColors(); g1Id++) {
        string colName = G1->getColorName(g1Id);
        if (not G2->hasColor(colName))
            throw runtime_error("G1 nodes colored " + colName + " cannot be matched to any G2 nodes");
        unsigned c1 = G1->numNodesWithColor(g1Id);
        unsigned c2 = G2->numNodesWithColor(G2->getColorId(colName));

        if (c1 > c2)
            throw runtime_error("there are " + to_string(c1) + " G1 nodes colored "
                                + colName + " but only " + to_string(c2) + " such nodes in G2");
        const uint64_t numSwapNeighbors = c1 * (c1 - 1) / 2;
        const uint64_t numMoveNeighbors = c1 * (c2 - c1);
        const uint64_t numNeighbors = numSwapNeighbors + numMoveNeighbors;

        numSwaps += numSwapNeighbors;
        swapsPerColor.push_back(numSwaps);
        numAdjacentAlignments += numSwapNeighbors + numMoveNeighbors;
        movesPerColor.push_back(numAdjacentAlignments - numSwaps);
        if (true) {
            cerr << "SANAThree:: color " << colName << " has " << numSwapNeighbors << " possible swaps and "
                    << numMoveNeighbors << " possible moves (" << numNeighbors << " total)" << endl;
            if (numNeighbors == 0) cerr << "color " << colName << " is inactive" << endl;
        }
    }
    if (true) {
        cerr << "SANAThree:: Alignments have " << numAdjacentAlignments << " possible changes in total" << endl;
        cerr << "SANAThree:: There are " << numSwaps << " possible swaps and " << numAdjacentAlignments - numSwaps << " possible moves." << endl;
    }
    if (numAdjacentAlignments == 0)
        throw runtime_error(
            "There is a unique valid alignment, so running SANA is pointless");

    //things initialized in initDataStructures because they depend on the starting alignment
    //they have the same size for every run, so we can allocate the size here
    colorUnassignedNodes = vector<vector<uint>>(swapsPerColor.size());

    totalMovesCalculated = 0;
    totalMovesAccepted = 0;
    totalSwapsCalculated = 0;
    totalSwapsAccepted = 0;
    initDataStructures();
    threadPool = new BatchHarvester {threadNumber, *this, batchSize / 2};
}

SANAThree::~SANAThree() {delete threadPool;}

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

void SANAThree::initDataStructures() {
    auto assignedNodesG2 = vector<char> (n2);

    if (startingAlignment.size() != 0) alignment = startingAlignment;
    else alignment = Alignment::randomColorRestrictedAlignment(*G1, *G2);

    //init holeToColorID. For each node, we do the following transformations:
    //g2Node -> g2ColorId -> g1ColorId -> actColId
    vector<uint> g2ToG1ColorIdMap = G2->myColorIdsToOtherGraphColorIds(*G1);
    auto holeToColorID = vector<uint>(n2, n1);
    for (uint g2Node = 0; g2Node < n2; g2Node++) {
        uint g2ColorId = G2->getNodeColor(g2Node);
        uint color = g2ToG1ColorIdMap[g2ColorId];
        if (color == Graph::INVALID_COLOR_ID) continue; //no node in G1 has this color
        holeToColorID[g2Node] = color;
    }

    for (uint i = 0; i < n2; i++) assignedNodesG2[i] = false;
    for (uint i = 0; i < n1; i++) assignedNodesG2[alignment[i]] = true;
    //initialize actColToUnassignedG2Nodes (the size was already set in the constructor)
    for (auto & colorUnassignedNode : colorUnassignedNodes)
        colorUnassignedNode.clear();
    for (uint g2Node = 0; g2Node < n2; g2Node++) {
        if (assignedNodesG2[g2Node]) continue;
        uint actColId = holeToColorID[g2Node];
        if (actColId != n1) {
            colorUnassignedNodes[actColId].push_back(g2Node);
        }
    }

    currentScore = MC->eval(alignment);
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

unsigned SANAThree::pBadsInBuffer() const {return threadPool->pBadsInBuffer();}

double SANAThree::getEquilibriumPBadAtTemp(double temperature, unsigned timeoutSeconds) const {
    return threadPool->runUntilEquilibrium(temperature, timeoutSeconds);
}

Alignment SANAThree::run() {
    setInterruptSignal();

    if (tolerance > 0)
        runConfidenceIntervals();
    else
        runIterations();

    if (hillClimbing) runHillClimbing();

    cout<<"Calculated "<< totalMovesCalculated <<" moves\n";
    cout<<"Calculated "<< totalSwapsCalculated <<" swaps\n";
    cout<<"Calculated "<< totalMovesCalculated + totalSwapsCalculated <<" total iterations\n";
    cout<<"Accepted "<< totalMovesAccepted <<" moves\n";
    cout<<"Accepted "<< totalSwapsAccepted <<" swaps\n";
    cout<<"Accepted "<< totalMovesAccepted + totalSwapsAccepted <<" total iterations\n";

    return alignment;
}

#define LEEWAY 1.75
#define temperatureFunction(f, i, d) ((i) * exp(-(d) * (f)))
void SANAThree::runIterations() {
    double maxSecondsWithLeeway;
    long long unsigned maxBatches;
    unsigned batchesPerStep;
    TimerTrue T;
    T.start();
    double iterationsPerSecond;
    {
        unsigned batches = 0;
        while (T.elapsed() < 1.) {
            threadPool->collectBatch(0.);
            batches++;
        }
        iterationsPerSecond = batchSize * batches / T.elapsed();
        batchesPerStep = ceil(batches * 10 / T.elapsed());
    }
    if (maxSeconds > 0) {
        maxBatches = ceil(maxSeconds * iterationsPerSecond / batchSize);
        maxSecondsWithLeeway = maxSeconds * LEEWAY;
    }
    else {
        maxBatches = 1 + maxIterations / batchSize;
        maxSecondsWithLeeway = 0;
    }
    totalMovesCalculated = 0;
    totalMovesAccepted = 0;
    totalSwapsCalculated = 0;
    totalSwapsAccepted = 0;
    initDataStructures();
    threadPool->resetBuffers();
    T.start();
    long long unsigned iter = 0;
    double temperature = tInitial;
    for (; iter < maxBatches; iter += 1) {
        temperature = temperatureFunction(static_cast<double>(iter)/static_cast<double>(maxBatches),
                                                 tInitial, tDecay);
        const batchOutput output = threadPool->collectBatch(temperature);
        if (saveAligAndExitOnInterruption) break;
        if (saveAligAndContOnInterruption) printReportOnInterruption();
        if (iter % batchesPerStep == 0) {
            currentScore = MC->eval(alignment);
            trackProgress(iter * batchSize, static_cast<double>(iter)/static_cast<double>(maxBatches), T.elapsed(),
                temperature, output.averagePBad);
        }
        if (maxSecondsWithLeeway != 0. and T.elapsed() > maxSecondsWithLeeway) break;
    }
    trackProgress(iter * batchSize, static_cast<double>(iter)/static_cast<double>(maxBatches), T.elapsed(),
                temperature, 0.);
}

// All of these are purely heuristic -Wayne (I think, at least -Marcus)
// TODO: This should be refactored! And the tab characters replaced with spaces!
#define MAX_TAU_STEP 0.01
#define MIN_TAU_STEP 0.001
#define MIN_BATCHES 30
#define HAPPY_BATCHES MIN(10000, (int)(m1+m2))
#define MIN_CONFIDENCE 0.99999
#define TOL_SAFETY_MARGIN 1.07 // empirically this seems to cut failure rates to below 5%.
void SANAThree::runConfidenceIntervals() {
    TimerTrue T;

    // TODO: make all of these changeable on the command line
    unsigned batch = 0;
    double tau, tauStep = MAX_TAU_STEP; // dynamically made smaller or bigger as necessary
    assert(tolerance > 0);
    double tolPerStep = tolerance * (tauStep) / TOL_SAFETY_MARGIN;
    double confidence = 1-pow(tolPerStep, 1.5); // empirically works well.
    if(confidence < MIN_CONFIDENCE) confidence = MIN_CONFIDENCE; // doesn't add much CPU to increase confidence.

    bool verbose = true;
    if(verbose) printf("SANAThree::runConfidenceIntervals Parameters: batchSize %llu confidence %g tolerance per step %g\n",
	batchSize, confidence, tolPerStep);

    STAT *scoreBatchMeans = StatAlloc(0, 0.0, 0.0, false, false);
    STAT *pBadBatchMeans = StatAlloc(0, 0.0, 0.0, false, false);

    // TODO: add batchesPerStep from runIterations for a re-eval of score
    long int lastBatchCount=0;
    double previousScore = currentScore;
    double temperature = 0;
    totalMovesCalculated = 0;
    totalMovesAccepted = 0;
    totalSwapsCalculated = 0;
    totalSwapsAccepted = 0;
    initDataStructures();
    threadPool->resetBuffers();
    T.start();
    for (tau = 0; tau <= 1; tau += tauStep) {
	    int batchesPerTemperature = 0;
        temperature = temperatureFunction(tau, tInitial, tDecay);

	    // Now the "inner loop"
	    bool satisfied = false;
	    while(!satisfied) {
	        if (saveAligAndExitOnInterruption) break;
	        if (saveAligAndContOnInterruption) printReportOnInterruption();

	        const batchOutput output = threadPool->collectBatch(temperature);

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
        currentScore = MC->eval(alignment);
        trackProgress(batch * batchSize, tau, T.elapsed(), temperature, threadPool->recentPBadTrue(), batchesPerTemperature,
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
    trackProgress(batch * batchSize, tau, T.elapsed(), temperature, threadPool->recentPBadQuick());
}

// I stole this duration from SANA proper. I will repeat the comment there that this is
// "arbitrarily chosen, probably too long". Regardless, shouldn't we be using the actual hillClimbing
// class?? TODO: fix this
// -Marcus
#define HILLCLIMB_DURATION 10000000000u
void SANAThree::runHillClimbing() {
    Timer T;
    T.start();

    const unsigned long runTime = 1 + HILLCLIMB_DURATION / batchSize;
    unsigned long long iter = 0;
    for (; iter < runTime; iter++) {
        threadPool->collectBatch(0.);
        currentScore = MC->eval(alignment);
    }
    cout<<"Hill climbing took "<<T.elapsedString()<<"s"<<endl;
}

SANAThree::changeRequest SANAThree::chooseNextRequest(mt19937_64 &generator) {
    unique_lock<mutex> lockAlignmentAndHoles(alignmentMutex, defer_lock);

    // Request parameters
    bool twoPegs;

    unsigned peg1;
    unsigned peg2 = -1; // Garbage allocation so that an exception will occur if not set and then used
    unsigned hole1;
    unsigned hole2;

    unsigned peg1colorID;
    unsigned peg2colorID = -1;
    unsigned hole2unassignedID = -1;

    unsigned color = 0;

    while (true) {
        uint64_t alignmentNumber = randIndex_64(numAdjacentAlignments, generator);

        // Swap Logic
        if (alignmentNumber < numSwaps) {
            twoPegs = true;

            // Find the active color
            {
                auto colorIter = upper_bound(swapsPerColor.begin(), swapsPerColor.end(), alignmentNumber) - 1;
                color = colorIter - swapsPerColor.begin();
                alignmentNumber = alignmentNumber - *colorIter;
            }

            // We consider each swap possibility of this color to be ordered in the following way:
            // (0, 1), (0, 2), ..., (0, n), (1, 2), ..., (n-1, n)
            // Ignoring symmetrical options (it's the same swap, after all), this means that the first
            // (n - 1) indices for these options have peg1 = 0, then the next (n - 2) for these options
            // have peg1 = 1 and so on. What this next complicated bit does is transform an index into
            // these possibilities into the actual possibility.

            // Enclosed in brackets so that these temporary variables keep in scope.
            {
                const unsigned numColorPegs = G1->numNodesWithColor(color);
                // Calculating peg1. There is a quadratic inequality I use that derives from the fact that
                // cumulativeSwaps(peg1) <= alignmentNumber < cumulativeSwaps(peg1 + 1). Do the math
                // if you are confused, it's a good exercise.
                const auto n = static_cast<double>(numColorPegs);
                const auto Ad = static_cast<double>(alignmentNumber); // Narrowing cast, yuck!
                const double discriminant = (2. * n - 1) * (2. * n - 1) - 8. * Ad;
                peg1colorID = 1U + static_cast<unsigned>(ceil((2. * n - 1. - sqrt(discriminant)) / 2.));

                // These are the possibilities that have already been counted for all first pegs that
                // are less than the current one
                uint64_t cumulativeSwaps = peg1colorID * (2ULL * numColorPegs - peg1colorID - 1ULL) / 2;

                // Because any conversion from a double to an unsigned is sus, we ensure that we
                // do indeed fulfill the inequality of:
                // cumSwaps(peg1) <= alignmentNumber < cumSwaps(peg1  + 1)
                while (alignmentNumber < cumulativeSwaps) {
                    cumulativeSwaps -= numColorPegs - peg1colorID;
                    peg1colorID--;
                }
                while (alignmentNumber >= cumulativeSwaps + numColorPegs - peg1colorID - 1) {
                    cumulativeSwaps += numColorPegs - peg1colorID - 1;
                    peg1colorID++;
                }

                // peg2's ID is just the offset from alignmentNumber - cumSwaps(peg1) and then offset
                // again by peg1 + 1;
                peg2colorID = static_cast<unsigned>(alignmentNumber - cumulativeSwaps + peg1colorID + 1);
            }

            peg1 = (*G1->getNodesWithColor(color))[peg1colorID];
            peg2 = (*G1->getNodesWithColor(color))[peg2colorID];
            lockAlignmentAndHoles.lock();
            hole1 = alignment[peg1];
            hole2 = alignment[peg2];
        }
        // Move Logic
        else {
            twoPegs = false;

            alignmentNumber -= numSwaps;

            // Find the active color
            {
                auto colorIter = upper_bound(movesPerColor.begin(), movesPerColor.end(), alignmentNumber) - 1;
                color = colorIter - movesPerColor.begin();
                alignmentNumber = alignmentNumber - *colorIter;
            }

            // YES, IT IS REALLY THIS EASY FOR THE MOVE CASE COMPARED TO THE SWAP CASE
            const unsigned numUnassignedHoles = colorUnassignedNodes.at(color).size();
            peg1colorID = alignmentNumber / numUnassignedHoles;
            hole2unassignedID = alignmentNumber % numUnassignedHoles;

            peg1 = (*G1->getNodesWithColor(color))[peg1colorID];
            lockAlignmentAndHoles.lock();
            hole1 = alignment[peg1];
            hole2 = colorUnassignedNodes[color][hole2unassignedID];
        }
        // I.E., if invalid, reroll. Rejection sampling, google it
        if (holeLocks[hole1] || holeLocks[hole2]) {
            lockAlignmentAndHoles.unlock();
            continue;
        }

        holeLocks[hole1] = holeLocks[hole2] = true;
        return {twoPegs, peg1, peg2, hole1, hole2, hole2unassignedID, color, 0.0};
    }
}

// This probably deserves a rework. I am uncomfortable with the current implementation and pushing
// it all off into its own function has solved this only marginally. It works, but there has got to
// be a less intrusive way to do this!
double SANAThree::implementLastRequest(double pBad, const changeRequest &input, mt19937_64 &generator) {
    unique_lock<mutex> lockAlignmentAndHoles(alignmentMutex);

    holeLocks[input.hole1] = holeLocks[input.hole2] = false;
    if (input.twoPegs) totalSwapsCalculated++;
    else totalMovesCalculated++;

    if (randomReal(generator) >= pBad) return currentScore;

    if (input.twoPegs) {
        alignment.swap(input.peg1, input.peg2);
        totalSwapsAccepted++;
    }
    else {
        alignment.set(input.peg1, input.hole2);
        colorUnassignedNodes[input.color][input.hole2unassignedID] = input.hole1; // Move
        totalMovesAccepted++;
    }

    const double val = currentScore.load() + input.energyInc;
    currentScore.store(val);
    return val;
}

// TODO
// This function should be written in C++, not C
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

    printf("%lld (%.5g%%,%.1fs): score = %.3g ips = %.5g, P(%.3g) = %.3g", iter, 100*fractionTime,
        elapsedTime, currentScore.load(), ips, temperature, lastAvgPBad);
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
    struct sigaction sigInt{};
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
