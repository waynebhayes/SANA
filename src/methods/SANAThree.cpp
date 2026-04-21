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

#include <cinttypes>

#include "BatchHarvester.hpp"

#include "../measures/SquaredEdgeScore.hpp"
#include "../utils/utils.hpp"
#include "../Report.hpp"
#include "../utils/Stats.hpp"

bool SANAThree::saveAligAndExitOnInterruption = false;
bool SANAThree::saveAligAndContOnInterruption = false;
SANAThree::SANAThree(const Graph* G1, const Graph* G2, const double TInitial, const double TDecay,
    const double maxSeconds, const long long maxIterations, const double tolerance,
    const bool addHillClimbing, const MeasureCombination* MC, const string& scoreAggrStr,
    const Alignment& optionalStartAlig, const string& outputFileName,
    const string& localScoresFileName, unsigned threadNumber):

    Method(G1, G2, "SANAThree_" + MC->toString()),
    n1(G1->getNumNodes()),
    n2(G2->getNumNodes()),
    m1(G1->getNumEdges()),
    m2(G2->getNumEdges()),
    tolerance(tolerance),
    maxSeconds(maxSeconds),
    maxIterations(maxIterations),
    batchSize((max<uint64_t>(CHUNK_SIZE * threadNumber * 2, G2->getNumEdges()) / CHUNK_SIZE) * CHUNK_SIZE),
    threadNumber(threadNumber),
    hillClimbing(addHillClimbing),
    needEC(MC->getWeight("ec") > 0),
    needEM(MC->getWeight("emin") > 0),
    needER(MC->getWeight("er") > 0),
    MC(MC),
    startingAlignment(optionalStartAlig),
    outputFileName(outputFileName),
    localScoresFileName(localScoresFileName),
    tInitial(TInitial),
    tDecay(TDecay),
    holeLocks(n2)
    {
    if (batchSize % CHUNK_SIZE != 0) {
        throw runtime_error("batchSize needs to be a multiple of CHUNK_SIZE!");
    }
    // This should never happen, and if it does, it is 100% user error.
    if (threadNumber >= n1 / 2) {
        throw runtime_error(
            "You should not request more threads than half the network size of G1.");
    }

    if (tolerance > 0) {
        if (maxIterations > 0 or maxSeconds > 0)
            throw runtime_error(
                "To use iterations or time, first set \"-tolerance 0\" on the command line (NOT RECOMMENDED!)");
    }

    for (size_t i = 0; i < n2; ++i) {
        holeLocks[i].clear(std::memory_order_relaxed);
    }

    currentScore = 0.;

    if (scoreAggrStr != "sum") {
        cerr << "SANA 3.0 alpha does not yet support score aggregation methods other than weighted"
                "sums.\n If you really need a complex aggregation, either wait for a new version or"
                "run the old version." << endl;
    }

    // NODE COLOR SYSTEM initialization
    // TODO: Preferred Holes

    assert(G1->numColors() <= G2->numColors());

    uint64_t numSwaps = 0;
    uint64_t numAdjacentAlignments  = 0;
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
        numAdjacentAlignments += numSwapNeighbors + numMoveNeighbors;
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

    if (startingAlignment.numOfPegs() == 0)
        alignment = Alignment::randomColorRestrictedAlignment(*G1, *G2);
    else
        alignment = startingAlignment;
    currentScore = MC->eval(alignment);

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

void SANAThree::resetAlignment() {
    if (startingAlignment.numOfPegs() == 0) alignment = Alignment::randomColorRestrictedAlignment(*G1, *G2);
    else alignment = startingAlignment;

    currentScore = MC->eval(alignment);
}


// TODO:
// This is terribly outdated. I'll buy a (soft) cider for anyone who takes it upon themselves to
// make a better version, but this relatively low priority. -Marcus
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

double SANAThree::getEquilibriumPBadAtTemp(double temperature, unsigned timeoutSeconds) const {
    return threadPool->runUntilEquilibrium(temperature, timeoutSeconds);
}

Alignment SANAThree::run() {
    setInterruptSignal();

    uint64_t iter;
    if (tolerance > 0)
        iter = runConfidenceIntervals();
    else
        iter = runIterations();

    if (hillClimbing) iter += runHillClimbing();

    cout<<"Calculated "<<iter<<" total iterations."<<endl;

    return alignment;
}

#define LEEWAY 1.75
#define temperatureFunction(f, i, d) ((i) * exp(-(d) * (f)))
uint64_t SANAThree::runIterations() {
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
    resetAlignment();
    T.start();
    long long unsigned batch = 0;
    double temperature = tInitial;
    for (; batch < maxBatches; batch += 1) {
        if (userInterrupted) {handleInterruption();}
        if (saveAligAndExitOnInterruption) break;
        if (saveAligAndContOnInterruption) printReportOnInterruption();

        temperature = temperatureFunction(static_cast<double>(batch)/static_cast<double>(maxBatches),
                                                 tInitial, tDecay);
        const batchOutput output = threadPool->collectBatch(temperature);
        if (batch % batchesPerStep == 0) {
            currentScore = MC->eval(alignment);
            trackProgress(batch * batchSize, static_cast<double>(batch)/static_cast<double>(maxBatches), T.elapsed(),
                temperature, output.averagePBad);
        }
        if (maxSecondsWithLeeway != 0. and T.elapsed() > maxSecondsWithLeeway) break;
    }
    trackProgress(batch * batchSize, static_cast<double>(batch)/static_cast<double>(maxBatches), T.elapsed(),
                temperature, 0.);
    return batch * batchSize;
}

// All of these are purely heuristic -Wayne (I think, at least -Marcus)
// TODO: This should be refactored! (Marcus also wants tab characters replaced with spaces, but WH likes tabs)
#define MAX_TAU_STEP 0.01
#define MIN_TAU_STEP 0.001
#define MIN_BATCHES 30
#define HAPPY_BATCHES MIN(10000, (int)(m1+m2))
#define MIN_CONFIDENCE 0.99999
#define TOL_SAFETY_MARGIN 1.07 // empirically this seems to cut failure rates to below 5%.
uint64_t SANAThree::runConfidenceIntervals() {
    TimerTrue T;

    // TODO: make all of these changeable on the command line
    unsigned batch = 0;
    double tau, tauStep = MAX_TAU_STEP; // dynamically made smaller or bigger as necessary
    assert(tolerance > 0);
    double tolPerStep = tolerance * (tauStep) / TOL_SAFETY_MARGIN;
    double confidence = 1-pow(tolPerStep, 1.5); // empirically works well.
    if(confidence < MIN_CONFIDENCE) confidence = MIN_CONFIDENCE; // doesn't add much CPU to increase confidence.

    bool verbose = true;
    if(verbose) printf("SANAThree::runConfidenceIntervals Parameters: batchSize %" PRIu64 " confidence %g tolerance per step %g\n",
	batchSize, confidence, tolPerStep);

    STAT *scoreBatchMeans = StatAlloc(0, 0.0, 0.0, false, false);
    STAT *pBadBatchMeans = StatAlloc(0, 0.0, 0.0, false, false);

    // TODO: add batchesPerStep from runIterations for a regular re-eval of score
    long int lastBatchCount=0;
    double previousScore = currentScore.load();
    double temperature = 0;
    resetAlignment();

    double lastPBad = 0.0;

    T.start();

    // Technical start of the loop, we do not indent for aesthetic reasons
    for (tau = 0; tau <= 1; tau += tauStep) {
	int batchesThisTemperature = 0;
        temperature = temperatureFunction(tau, tInitial, tDecay);

	// Now the "inner loop"
	bool satisfied = false;
	while(!satisfied) {
	    if (userInterrupted) {handleInterruption();}
	    if (saveAligAndExitOnInterruption) break;
	    if (saveAligAndContOnInterruption) printReportOnInterruption();

	    const batchOutput output = threadPool->collectBatch(temperature);
        ++batch; ++batchesThisTemperature;
	    StatAddSample(scoreBatchMeans, output.averageScore);
	    StatAddSample(pBadBatchMeans, output.averagePBad);
	    lastPBad = output.averagePBad;

	    if (batchesThisTemperature % MIN_BATCHES) {
	        currentScore = MC->eval(alignment);
	    }

	    if(StatNumSamples(scoreBatchMeans)>=MIN_BATCHES) {
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
		relativeMultiplier *= (1+log(batchesThisTemperature));

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
    trackProgress(batch * batchSize, tau, T.elapsed(), temperature, lastPBad, batchesThisTemperature,
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
    return batch * batchSize;
}

// I stole this duration from SANA proper. I will repeat the comment there that this is
// "arbitrarily chosen, probably too long". Regardless, shouldn't we be using the actual hillClimbing
// class?? TODO: fix this
// -Marcus
#define HILLCLIMB_DURATION 10000000000u
uint64_t SANAThree::runHillClimbing() {
    Timer T;
    T.start();

    const unsigned long runTime = 1 + HILLCLIMB_DURATION / batchSize;
    unsigned long long batch = 0;
    for (; batch < runTime; batch++) {
        threadPool->collectBatch(0.);
        currentScore = MC->eval(alignment);
    }
    cout<<"Hill climbing took "<<T.elapsedString()<<"s"<<endl;
    return batch * batchSize;
}

SANAThree::changeRequest SANAThree::chooseNextRequest(mt19937_64 &generator) {
    // Loop while we look for a valid adjacent alignment.
    while (true) {
        bool twoPegs = true;

        unsigned peg1 = randIndex_64(n1, generator);
        unsigned hole1 = alignment.pegToHole(peg1);

        // TODO: Preferred Hole System
        unsigned color = G1->getNodeColor(peg1);
        unsigned colorNum = G2->getNodesWithColor(color)->size();
        unsigned hole2 = G2->getNodesWithColor(color)->at(randIndex_64(colorNum, generator));
        if (hole1 == hole2) {
            continue;
        }
        unsigned peg2 = alignment.holeToPeg(hole2);

        if (peg2 == n1) {
            twoPegs = false;
        }

        if (!tryToLockHoles(hole1, hole2)) {
            continue;
        }

        if (alignment.pegToHole(peg1) != hole1 || alignment.holeToPeg(hole2) != peg2) {
            releaseHoles(hole1, hole2);
            continue;
        }

        return {twoPegs, peg1, peg2, hole1, hole2};
    }
}


bool SANAThree::tryToLockHoles(unsigned hole1, unsigned hole2) {
    if (hole1 == hole2) {
        return false;
    }

    // Philosopher's problem solution using a canonical locking order
    unsigned holeA, holeB;
    if (hole1 < hole2) {
        holeA = hole1;
        holeB = hole2;
    }
    else {
        holeA = hole2;
        holeB = hole1;
    }

    // test_and_set returns TRUE if the hole is already "locked"
    if (holeLocks[holeA].test_and_set(memory_order_acquire)) {
        return false;
    }
    if (holeLocks[holeB].test_and_set(memory_order_acquire)) {
        holeLocks[holeA].clear(memory_order_release); // Clear first "lock"
        return false;
    }

    // test_and_set "acquired" both "locks" successfully (this code is technically lockless)

    return true;
}

void SANAThree::releaseHoles(unsigned hole1, unsigned hole2) {
    if (hole1 == hole2) { // This should never happen.
        holeLocks[hole1].clear(memory_order_release);
        return;
    }

    // Philosopher's problem solution using a canonical locking order
    unsigned holeA, holeB;
    if (hole1 < hole2) {
        holeA = hole1;
        holeB = hole2;
    }
    else {
        holeA = hole2;
        holeB = hole1;
    }

    holeLocks[holeB].clear(memory_order_release);
    holeLocks[holeA].clear(memory_order_release);
}


double SANAThree::implementLastRequest(const double pBad, const double energyInc, const changeRequest &input, mt19937_64 &generator, uniform_real_distribution<> &dist) {
    if (dist(generator) >= pBad) {
        releaseHoles(input.hole1, input.hole2);
        return currentScore;
    }

    if (input.twoPegs) {
        alignment.swapPegs(input.peg1, input.peg2, input.hole1, input.hole2);
    }
    else {
        alignment.movePeg(input.peg1, input.hole1, input.hole2);
    }
    releaseHoles(input.hole1, input.hole2);

    // Yes, this causes a data race if two thread try to adjust the score at once. Too bad!
    // The energyInc is already stale and we've already got floating point error accumulation.
    // So this is acceptable, as long as we don't drift too far.
    const double val = currentScore.load(memory_order_relaxed) + energyInc;
    currentScore.store(val, memory_order_relaxed);
    return val;
}

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

volatile sig_atomic_t SANAThree::userInterrupted = 0;

void sigHandlerThree(const int s) {
    if (s == SIGINT || s == SIGTERM || s == SIGHUP) {
        SANAThree::userInterrupted = 1;
    }
}

void SANAThree::handleInterruption() {
    userInterrupted = 0; // Reset the flag

    int c = 3;
    do {
        cout << "\n--- SANA Interrupted ---" << endl;
        cout << "Select an option (0 - 3):" << endl
             << "  (0) Do nothing and continue" << endl
             << "  (1) Exit" << endl
             << "  (2) Save Alignment and Exit" << endl
             << "  (3) Save Alignment and Continue" << endl << ">> ";

        cin >> c;
        if (cin.eof()) {
            saveAligAndContOnInterruption = true;
            cin.clear();
            return;
        }
        if (cin.fail()) {
            c = -1;
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
        }

        if      (c == 0) cout << "Continuing..." << endl;
        else if (c == 1) exit(0);
        else if (c == 2) saveAligAndExitOnInterruption = true;
        else if (c == 3) saveAligAndContOnInterruption = true;

    } while (c < 0 || c > 3);
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
