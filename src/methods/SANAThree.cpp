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
    MC(MC),
    startingAlignment(optionalStartAlig),
    outputFileName(outputFileName),
    localScoresFileName(localScoresFileName),
    tInitial(TInitial),
    tDecay(TDecay),
    pegColorToHoleColor(G1->myColorIdsToOtherGraphColorIds(*G2)),
    holeLocks(n2)
{
    if (batchSize % CHUNK_SIZE != 0) {
        throw runtime_error("batchSize needs to be a multiple of CHUNK_SIZE!");
    }
    // This should never happen, and if it does, it is 100% user error.
    if (threadNumber >= n1 / 2) {
        throw runtime_error(
            "You should not request more threads than half the network size of G1."
        );
    }

    if (tolerance > 0) {
        if (maxIterations > 0 or maxSeconds > 0)
            throw runtime_error(
                "To use iterations or time, first set \"-tolerance 0\" on the command line (NOT RECOMMENDED)");
    }

    for (size_t i = 0; i < n2; ++i) {
        holeLocks[i].clear(std::memory_order_relaxed);
    }

    currentScore = 0.;


    // An old feature that I did not see much use of. It should be reimplemented with the Measure
    // refactor. -ML
    if (scoreAggrStr != "sum") {
        cerr << "SANA 3.5 does not yet support score aggregation methods other than weighted"
                "sums.\n If you really need a complex aggregation, either wait for a new version or"
                "run the old version." << endl;
    }

    // NODE COLOR SYSTEM initialization
    // TODO: Preferred Holes
    // Preferred Holes is like non-exclusive colors. Each peg has a number of holes that it would
    // like to be matched which. For example: peg 1 might prefer holes A, B, and C while peg 2 might
    // prefer holes B and D.

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

    resetAlignment();

    threadPool = new BatchHarvester{threadNumber, *this, batchSize / 2};
}

SANAThree::~SANAThree() {delete threadPool;}

Alignment SANAThree::run() {
    setInterruptSignal();

    uint64_t iter;

    // Tolerance > 0 is considered the default setting, as runConfidenceInterval has a
    // self-adjusting temperature schedule.
    if (tolerance > 0)
        iter = runConfidenceIntervals();
    else
        iter = runIterations();

    if (hillClimbing) iter += runHillClimbing();

    cout<<"Calculated "<<iter<<" total iterations."<<endl;

    return alignment;
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

double SANAThree::getEquilibriumPBadAtTemp(double temperature, unsigned timeoutSeconds) const {
    return threadPool->runUntilEquilibrium(temperature, timeoutSeconds);
}

// TODO:
// This is terribly outdated. I'll buy a (soft) cider for anyone who takes it upon themselves to
// make a better version, but this relatively low priority. -ML
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

void SANAThree::resetAlignment() {
    if (startingAlignment.numOfPegs() == 0)
        alignment = Alignment::randomColorRestrictedAlignment(*G1, *G2);
    else alignment = Alignment(startingAlignment);

    currentScore = MC->eval(alignment);
}

bool SANAThree::saveAligAndExitOnInterruption = false;

bool SANAThree::saveAligAndContOnInterruption = false;

#define SECONDS_PER_REPORT 30
#define temperatureFunction(f, i, d) ((i) * exp(-(d) * (f)))
// runIterations has two modes: maxSeconds mode and maxIterations mode. They work nearly identically,
// but change the standard we use for the temperature schedule to break our loop.
uint64_t SANAThree::runIterations() {
    // Control variables
    const bool isTimerBound = (maxSeconds > 0);
    const uint64_t maxBatches = !isTimerBound ? 1 + maxIterations / batchSize : 0;

    uint64_t batches = 0;

    double lastReportTime = 0.0;
    double fractionComplete = 0.0;
    resetAlignment();
    TimerTrue T;
    T.start();
    while (fractionComplete < 1.0) {
        // Resolve interrupts
        if (userInterrupted) interruptionUserInput();
        if (saveAligAndContOnInterruption) printReportOnInterruption();
        if (saveAligAndExitOnInterruption) break;

        const double temperature = temperatureFunction(fractionComplete, tInitial, tDecay);
        const ScoreWithPBad output = threadPool->collectBatch(temperature);
        ++batches;

        const double elapsed = T.elapsed();
        fractionComplete = isTimerBound ? elapsed / maxSeconds : static_cast<double>(batches) / maxBatches;
        if (elapsed - lastReportTime >= SECONDS_PER_REPORT || fractionComplete >= 1.0) {
            currentScore = MC->eval(alignment);
            lastReportTime = elapsed;
            trackProgress(batches * batchSize, fractionComplete, elapsed, temperature, output.pBad);
        }
    }
    return batches * batchSize;
}

// All of these are purely heuristic -WH
// TODO: Marcus suggests that this be put in a config document for more advanced settings
#define MAX_TAU_STEP 0.01
#define MIN_TAU_STEP 0.001
#define MIN_BATCHES 30
#define MIN_CONFIDENCE 0.99999  // doesn't add much CPU to increase confidence. -WH
#define TOL_SAFETY_MARGIN 1.07 // empirically this seems to cut failure rates to below 5%. - WH
#define VERBOSE true
uint64_t SANAThree::runConfidenceIntervals() {
    // TODO: We should try using CircularSTATBuffer for this for a rolling average rather
    // than resetting the stat buffer entirely after an arbitrary number of batches. It
    // would also require less confusing logic. -ML

    TimerTrue T;
    T.start();

    const double tolPerStep = tolerance * MAX_TAU_STEP / TOL_SAFETY_MARGIN;
    const double confidence = max(MIN_CONFIDENCE, 1 - pow(tolPerStep, 1.5)); // empirically works well. - WH
    const uint64_t happyBatches = min<uint64_t>(10000u, m1 + m2);

    if (VERBOSE) {
        printf("SANAThree::runConfidenceIntervals Parameters: batchSize %" PRIu64 " confidence %g tolerance per step %g\n",
            batchSize, confidence, tolPerStep);
    }

    // These come from a C library, so we have to manually allocate and free rather than using a
    // smart pointer. The CircularSTATBuffer does not have this problem. -ML
    STAT *scoreBatchMeans = StatAlloc(0, 0.0, 0.0, false, false);
    STAT *pBadBatchMeans = StatAlloc(0, 0.0, 0.0, false, false);

    uint64_t totalBatches = 0;

    double recentPBad = 0.0;
    double previousStageScore = currentScore.load();
    double temperature = 0.0;
    double tauStep = MAX_TAU_STEP; // dynamically made smaller or bigger as necessary -WH

    resetAlignment();

    // We set through the temperature schedule gradually, with the possibility of staying longer at
    // certain temperature if they are "productive" (that is, the score and pBad keep changing,
    // indicating that progress towards a better region of the solution space is still being made.)
    // The tau system was manually tuned by Dr.WH. It uses floats because sometimes we want to take
    // a smaller step to the final temperature.
    for (double tau = 0; tau <= 1; tau += tauStep) {
        uint64_t stageBatches = 0;
        uint64_t previousStageBatches = 0;
        temperature = temperatureFunction(tau, tInitial, tDecay);

        // Inner loop that defines the temperature stage.
        // The conditions for exiting are too complex for a simple while expression
        while (true) {
            // RESOLVE INTERRUPTS
            if (userInterrupted) interruptionUserInput();
            if (saveAligAndContOnInterruption) printReportOnInterruption();
            if (saveAligAndExitOnInterruption) break;

            // PROCESS BATCH
            const ScoreWithPBad output = threadPool->collectBatch(temperature);
            ++totalBatches; ++stageBatches;
            StatAddSample(scoreBatchMeans, output.score);
            StatAddSample(pBadBatchMeans, output.pBad);
            recentPBad = output.pBad;

            // TODO: Report mid-stage results without MIN_BATCHES
            if (stageBatches % MIN_BATCHES == 0) {
                currentScore = MC->eval(alignment);
            }

            // This is a hidden cast from int to uint64_t.
            // Yet another good reason to use CircularSTATBuffer, which doesn't use signed integers
            // for an unsigned quantity. -ML
            const uint64_t numSamples = StatNumSamples(scoreBatchMeans);

            // Continue loop if we do not have enough samples to make a judgement.
            if (numSamples < MIN_BATCHES) {continue;}

            // LOOP BREAK DECISION CODE

            const double meanScore = StatMean(scoreBatchMeans);
            const double meanPBad = StatMean(pBadBatchMeans);

            // The user specifies a *relative* tolerance on the FINAL score... but we don't know what the final
            // score will be. Thus, early on when the score is low and pBad is high, we punt to using (effectively)
            // an absolute tolerance by multiplying the tolerance by pBad. Then, as the score increases and
            // surpasses pBad, transition to a genuine relative tolerance by multiplying by the score.
            double relativeMultiplier = MAX(meanScore, meanPBad);
            // HOWEVER, we also slowly decrease the tolerance (by slowly increasing the Interval), because
            // sometimes we can get "stuck" for a VERY long time at one temperature because the score
            // is fluctuating too much. Let's not get stuck too long.
            relativeMultiplier *= 1+log(stageBatches);

            const double scoreInterval = tolPerStep * relativeMultiplier;
            const double pBadInterval = tolPerStep * relativeMultiplier;

            const bool scoreConfident = StatConfInterval(scoreBatchMeans, confidence) < scoreInterval;
            const bool pBadConfident = StatConfInterval(pBadBatchMeans, confidence) < pBadInterval;

            // Break if we are satisfied
            if (scoreConfident && pBadConfident) {break;}

            // Continue unless we have been stuck in this stage for too long.
            if (numSamples < happyBatches) {continue;}

            // ADJUST HEURISTICS

            // Reset the batch system if the score is increasing steadily, otherwise it can't "converge" without
            // an ENORMOUS number of batches to compensate for the "bias" that occurs in early batches. -WH
            // This is why I would rather use a circular buffer. -ML
            if (meanScore > previousStageScore) {
                if(VERBOSE) {
                    printf(" ++++> temp %.4g, batchMeanScore %.3f (pBad %.3g) still increasing after %lu batches; reset batches and continue\n",
                    temperature, meanScore, StatConfInterval(pBadBatchMeans, confidence), numSamples);
                    fflush(stdout);
                }
                previousStageScore = meanScore;
                previousStageBatches = 0;
                StatReset(scoreBatchMeans);
                StatReset(pBadBatchMeans);
                continue;
            }

            // If the score keeps decreasing, tauStep gets reduced
            if (tauStep > MIN_TAU_STEP && numSamples >= happyBatches + previousStageBatches) {
                if(VERBOSE) {
                    printf(" ----> %d batches, avg score %g decreased at tau %g; reduce next tauStep from %g",
                           StatNumSamples(scoreBatchMeans), StatMean(scoreBatchMeans), tau, tauStep);
                }
                tauStep = max(MIN_TAU_STEP, tauStep * 2.0/3.0);
                if(VERBOSE) {
                    printf(" to %g and backtrack to tau %g\n", tauStep, tau);
                    fflush(stdout);
                }
                previousStageBatches = numSamples;
            }
        }

        // CLEAN UP FOR NEXT TEMPERATURE STAGE

        currentScore = MC->eval(alignment);
        trackProgress(totalBatches * batchSize, tau, T.elapsed(), temperature, recentPBad, stageBatches,
                      StatMean(scoreBatchMeans), StatMean(pBadBatchMeans));

        // Marcus says: My assumption is that this logic is meant to skip over regions of inactivity,
        // particularly if tauStep has shrunk, but it is not how I would design it.
        if(tauStep < MAX_TAU_STEP) {
            if(StatNumSamples(scoreBatchMeans) < happyBatches) {
                if(VERBOSE) {
                    printf(" *****> doing OK at tau %g & %d batches; increasing tauStep from %g",
                        tau, StatNumSamples(scoreBatchMeans), tauStep);
                }
                tauStep = min(MAX_TAU_STEP, tauStep * 3);
                if(VERBOSE) {
                    printf(" to %g\n", tauStep);
                    fflush(stdout);
                }
            }
            else if(StatMean(scoreBatchMeans) < previousStageScore) {
                if(VERBOSE) {
                    printf(" !!!!!> score %g is stuck below previous %g; skip region by increasing tauStep from %g",
                        StatMean(scoreBatchMeans), previousStageScore, tauStep);
                }
                tauStep = min(MAX_TAU_STEP, tauStep * 10);
                if(VERBOSE) {
                    printf(" to %g\n", tauStep);
                    fflush(stdout);
                }
            }
        }

        previousStageScore = StatMean(scoreBatchMeans);
        StatReset(scoreBatchMeans);
        StatReset(pBadBatchMeans);
    }

    // FINISHED CALCULATIONS

    cout<<"Performed "<<totalBatches<<" total batches\n";
    trackProgress(totalBatches * batchSize, 1, T.elapsed(), temperature, recentPBad);

    StatFree(scoreBatchMeans);
    StatFree(pBadBatchMeans);

    return totalBatches * batchSize;
}

// I stole this duration from SANA proper. I will repeat the comment there that this is
// "arbitrarily chosen, probably too long". -ML
#define HILLCLIMB_DURATION 10000000000ull
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

SANAThree::ChangeRequest SANAThree::chooseNextRequest(mt19937_64 &generator) {
    // TODO: Decisions must be made about calculation speed vs function bias.
    // As this is currently written, each swap has twice the probability of being selected as a move.
    // Is this acceptable? I do not know, we need to perform tests on this. -ML

    // Loop while we look for a valid adjacent alignment.
    while (true) {
        bool twoPegs = true;

        unsigned peg1 = randIndex_64(n1, generator);
        unsigned hole1 = alignment.pegToHole(peg1);

        // We reroll until we find a hole that is not hole1.
        // TODO: Preferred Hole System
        // We could easily select hole2 from the preferred holes of peg1.
        bool foundHoleFlag = false;
        unsigned hole2;
        for (size_t i = 0; i < 20; ++i) {
            unsigned pegColor = G1->getNodeColor(peg1);
            unsigned holeColor = pegColorToHoleColor[pegColor];
            unsigned holeColorNum = G2->getNodesWithColor(holeColor)->size();
            hole2 = G2->getNodesWithColor(holeColor)->at(randIndex_64(holeColorNum, generator));
            if (hole1 != hole2) {
                foundHoleFlag = true;
                break; // At times like this, I miss Rust's loop labels and loop assignments.
            }
        }
        if (!foundHoleFlag) {
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

double SANAThree::implementLastRequest(const ScoreWithPBad calculations, const ChangeRequest &input, mt19937_64 &generator, uniform_real_distribution<> &dist) {
    if (dist(generator) >= calculations.pBad) {
        releaseHoles(input.hole1, input.hole2);
        return currentScore.load(memory_order_relaxed);
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
    // So this is acceptable, as long as we don't drift too far. Synchronizing is too expensive. -ML
    const double val = currentScore.load(memory_order_relaxed) + calculations.score;
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
    const double ips = static_cast<double>(iter - lastIterations) / (elapsedTime - oldTimeElapsed);
    oldTimeElapsed = elapsedTime;
    lastIterations = iter;

    printf("%lld (%.5g%%,%.1fs): score = %.3g ips = %.5g, P(%.3g) = %.3g", iter, 100*fractionTime,
        elapsedTime, currentScore.load(), ips, temperature, lastAvgPBad);
    if(batches) printf(" batches %d bSc %.3g, bpBad %.3g", batches, batchScore, batchPbad);
    printf("\n");
    fflush(stdout);
}

// INTERRUPTION CODE
volatile sig_atomic_t SANAThree::userInterrupted = 0;
void sigHandlerThree(const int s) {
    if (s == SIGINT || s == SIGTERM || s == SIGHUP) {
        SANAThree::userInterrupted = 1;
    }
}
void SANAThree::interruptionUserInput() {
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
