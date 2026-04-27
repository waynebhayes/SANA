#include <sys/unistd.h>
#include <chrono>
#include <iostream>
#include <thread>
#include <string>
#include <vector>
#include <iomanip>
#include <stdexcept>
#include <random>
#include <cmath>
#include <thread>
#include <mutex>
#include <cassert>
#include <csignal>
#include <cstdio>

#include "SANAThree.hpp"
#include "BatchHarvester.hpp"
#include "../Report.hpp"

#define DEBUG_HARVESTER 0

/// Returns the probability that a bad move will be accepted.
///
/// # Parameters:
///
/// const double energyInc: the score improvement of a potential change.
///
/// const double temperature: the annealing temperature, higher temperature -> bad change more likely
double static inline acceptingProbability(const double energyInc, const double temperature) {
    // Float comparisons are usually a bad practice, this case is fine -ML
    if (temperature == 0.) {
        return energyInc >= 0; // Bool coerced into 0.0 and 1.0
    }

    if (energyInc >= 0.) {
        return 1.0;
    }

    const double exponent = energyInc / temperature;
    // That is, if the exponent would produce a subnormal number, truncate final answer to zero.
    if (exponent < -700) {
        return 0.0;
    }

    return exp(exponent);
}

#define SCORE_BATCH_SIZE 31 // TODO: Borrowed from SANA 2.0. This should probably be looked at -ML
SANAThree::BatchHarvester::BatchHarvester(const unsigned threadNumber, SANAThree &SANA, unsigned long long bufferSize):
    parent(SANA),
    daughterNum(threadNumber),
    startedRequests(0),
    finishedRequests(0),
    currentState(State::pause),
    daughtersPaused(0),
    daughtersTerminated(0),
    totalScore(0.0),
    totalPBad(0.0),
    numTotalPBad(0),
    temperature(0.),
    scoreCircBuffer(SCORE_BATCH_SIZE),
    pBadCircBuffer(bufferSize) {
    unique_lock<mutex> stateLock(stateMutex);

    // Seed an initial generator to provide seeds for the daughter threads.
    mt19937_64 generator(random_device{}());

    // Debug
    cerr << "HI THERE! BatchHarvester is using " << threadNumber << " threads\n";
    if (threadNumber == 0)
        throw runtime_error("Thread number must be > 0");

    // Cast out the necessary number of threads
    threadVector.reserve(threadNumber);
    for (unsigned i = 0; i < threadNumber; ++i) {
        threadVector.emplace_back(&BatchHarvester::daughterFunction, this, generator());
    }

    // Wait for all daughter to pause before exiting constructor.
    waitMom.wait(stateLock, [this]{return daughtersPaused == daughterNum;});
    #if DEBUG_HARVESTER
        assert(daughtersPaused == daughterNum);
    #endif
}

// I greatly dislike destructors that are nontrivial, but I do not see a way around it. -ML
SANAThree::BatchHarvester::~BatchHarvester() {
    unique_lock<mutex> stateLock(stateMutex); // Lock state access

    // The order here is extremely important or else you will get a data race.
    // My recommendation is that you do not touch this unless you really know what you are doing.
    // -ML

    // Set the state to termination and wake up any paused daughter threads.
    activateDaughters(State::terminate);

    // Wait for all daughters to terminate
    waitMom.wait(stateLock, [this]{return daughtersTerminated == daughterNum;});

    // This thread should only be successfully woken if all daughters are terminated
    // If not, then we have a memory leak.
    assert(daughtersTerminated == daughterNum);

    // Join all threads
    for (thread& t: threadVector)
        t.join();
}

double SANAThree::BatchHarvester::runUntilEquilibrium(double temperature, unsigned timeoutSeconds) {
    unique_lock<mutex> stateLock(stateMutex); // Held

    resetCounters();

    #if DEBUG_HARVESTER
        // Are we in a valid state? Did someone forget to clean up?
        assert(currentState == State::pause);
        assert(daughtersPaused == daughterNum);
        assert(daughtersTerminated == 0);
    #endif

    // bufferLock and stateLock should generally NOT be held at the same time.
    stateLock.unlock();

    unique_lock<mutex> bufferLock(bufferMutex);
        resetBuffers();
    bufferLock.unlock();

    stateLock.lock();

    // Set all daughters loose to equilibrate at this temperature.
    this->temperature = temperature;
    activateDaughters(State::equilibrate);

    bool timedOut = false;
    const auto originalTime = chrono::steady_clock::now();
    // Wait for all daughters to pause unless we time out.
    if(!waitMom.wait_for(stateLock, chrono::seconds(timeoutSeconds), [this]{return daughtersPaused == daughterNum;})) {
        // Clean up if we time out.
        timedOut = true;
        currentState = State::pause;
        // Wait for all daughters to pause.
        waitMom.wait(stateLock, [this]{return daughtersPaused == daughterNum;});
    }
    #if DEBUG_HARVESTER
        // We should be safe to unpause and unlock the state
        assert(currentState == State::pause);
        assert(daughtersPaused == daughterNum);
    #endif

    // Unlock the state, as no more state modifications will be done.
    stateLock.unlock();

    // Terminal output of answer.
    bufferLock.lock();
    const chrono::duration<double> equilibriumDuration = chrono::steady_clock::now() - originalTime;
    const double pBadAnswer = pBadCircBuffer.accurateAverage();
    cout<<"> getEquilibriumPBadAtTemp("<<temperature<<") = "<<pBadAnswer<<" (score: "<<parent.currentScore<<")"
    <<" (time: "<<equilibriumDuration.count()<<"s)"
    << (timedOut? "[TIMED OUT]" : "")
    <<" iterations = "<< finishedRequests
    <<", ips = "<< static_cast<double>(finishedRequests) / equilibriumDuration.count() <<endl
    <<"****************************************"<<endl<<endl;

    return pBadAnswer;
}

SANAThree::ScoreWithPBad SANAThree::BatchHarvester::collectBatch(double temperature) {
    unique_lock<mutex> stateLock(stateMutex); // Held whenever not waiting

    #if DEBUG_HARVESTER
        // Are we in a valid state? Did someone forget to clean up?
        assert(currentState == State::pause);
        assert(daughtersPaused == daughterNum);
        assert(daughtersTerminated == 0);
    #endif

    // Reset global counters and outputs.
    resetCounters();

    // Send daughters loose at this temperature annealing.
    this->temperature = temperature;
    activateDaughters(State::anneal);

    // Wait for all daughters to pause.
    waitMom.wait(stateLock, [this]{return daughtersPaused == daughterNum;});

    #if DEBUG_HARVESTER
        // We should be safe to unpause and unlock the state
        assert(currentState == State::pause);
        assert(daughtersPaused == daughterNum);
        assert(finishedRequests == parent.batchSize);
        assert(startedRequests >= parent.batchSize);
    #endif

    if (numTotalPBad == 0)
        ++numTotalPBad;
    const double averageScore = totalScore / finishedRequests.load();
    const double averagePBad = totalPBad / numTotalPBad;

    return {averageScore, averagePBad};
}


void SANAThree::BatchHarvester::daughterFunction(uint64_t seed) {
    mt19937_64 generator(seed);
    auto randomReal = uniform_real_distribution<>(0, 1);

    unique_lock<mutex> bufferLock(bufferMutex, defer_lock); // Defer
    unique_lock<mutex> stateLock(stateMutex); // Held

    // To prevent needing more mutex use, we collect these stats on the stack and then forward them
    // to the Mom thread when we pause
    double collectedScore = 0.0;
    double collectedPBad = 0.0;
    uint64_t processedPBad = 0;

    // Store these locally on the stack.
    double temperature = this->temperature;
    const unsigned batchSize = parent.batchSize;

    // Main state loop.
    // After every operation is complete, the function returns to check the current state of the Harvester
    while (true) {
    // CRITICAL: We always enter this switch statement with the stateLock held -ML
    switch (currentState) {

        // For this state, all daughters pause operations until Mom tells them to start again.
        case State::pause: {

            // Give the Mom thread our results
            totalScore += collectedScore;
            totalPBad += collectedPBad;
            numTotalPBad += processedPBad;

            // If all daughter threads are paused, then the Mom thread is alerted
            if (++daughtersPaused == daughterNum)
                waitMom.notify_all();

            uint64_t lastActivation = activationID;

            // Now we wait until we told that daughter threads should be unpaused.
            //
            // A condition is required here because the C++ standard does not guarantee that a
            // thread will never be woken up without our code asking for it. This is called a
            // spurious wakeup. Originally, the condition was currentState != State::pause.
            //
            // So why activationID instead? My code is sometimes so fast that the threads will
            // finish a batch before all of them have been woken up. This is bad, because it means
            // we return to the pause state before the sleepy thread can wake up, which means it
            // never does a state machine loop to increment daughtersPaused which means Mom is never
            // woken up. So we cannot rely on pause to tell threads to wake up, we need a different
            // indicator. This is called the ABA data race. The solution is to have a counter we can
            // check that will never return to the same state.
            // -ML
            waitDaughters.wait(stateLock, [this, lastActivation]{
                return activationID != lastActivation;
            });

            // Reset our daughter stats.
            collectedScore = 0.0;
            collectedPBad = 0.0;
            processedPBad = 0;

            // Fetch the current temperature.
            temperature = this->temperature;

            continue;
        }

        // For this state, the daughters process requests as granted to them by the SANA class.
        // This means calculating the expected change to the score, the resulting pBad, and then
        // implementing the change safely if it is accepted. The daughters process exactly one batch
        // of requests, as determined by the batchSize of the SANA class. Then they self-halt and
        // notify the Mom thread that unpaused them that the batch is finished.
        case State::anneal: {
            stateLock.unlock();

            while (true) {
                // Break if we've met the quota.
                uint64_t newStartedRequests = startedRequests.fetch_add(CHUNK_SIZE);
                if (newStartedRequests >= batchSize) {
                    break;
                }

                for (size_t i = 0; i < CHUNK_SIZE; ++i) {
                    // Part 1, ask the SANA class for a request to process
                    ChangeRequest currentRequest = parent.chooseNextRequest(generator);

                    // Part 2, assess request
                    double energyInc = assessRequest(currentRequest);
                    const double pBad = acceptingProbability(energyInc, temperature);

                    // Part 3, implement (or don't implement) request
                    const double newScore = parent.implementLastRequest(pBad, energyInc, currentRequest, generator, randomReal);

                    // Part 4, record the results
                    collectedScore += newScore;
                    if (energyInc < 0.0) {
                        collectedPBad += pBad;
                        ++processedPBad;
                    }
                }
                finishedRequests.fetch_add(CHUNK_SIZE);
            }

            stateLock.lock();
            if (currentState != State::terminate) {
                currentState = State::pause;
            }
            continue;
        }

        // This state is very similar to anneal, but we record results in circular buffers and
        // otherwise process requests indefinitely or until equilibrium is reached.
        case State::equilibrate: {
            stateLock.unlock();

            double newScore = 0.0;
            for (size_t i = 0; i < CHUNK_SIZE; ++i) {
                // Part 1, select request to process
                ChangeRequest currentRequest = parent.chooseNextRequest(generator);

                // Part 2, assess request
                const double energyInc = assessRequest(currentRequest);
                const double pBad = acceptingProbability(energyInc, temperature);

                // Part 3, implement (or don't implement) request
                newScore = parent.implementLastRequest(pBad, energyInc, currentRequest, generator, randomReal);

                // Part 4, record the results
                bufferLock.lock();
                if (energyInc < 0) {
                    pBadCircBuffer.insert(pBad);
                }
                bufferLock.unlock();
            }

            uint64_t newFinishedRequests = finishedRequests.fetch_add(CHUNK_SIZE);
            if (newFinishedRequests % batchSize == 0) {
                bufferLock.lock();
                scoreCircBuffer.insert(newScore);
                if (scoreCircBuffer.isFull() && !scoreCircBuffer.trendingUpwards()) {
                    bufferLock.unlock();
                    stateLock.lock();
                    if (currentState != State::terminate) {
                        currentState = State::pause;
                        continue; // We can do this because stateLock() is held.
                    }
                    stateLock.unlock();
                }
                else {
                    bufferLock.unlock();
                }
            }

            stateLock.lock();
            continue;
        }

        // Termination states
        case State::terminate: {
            if (++daughtersTerminated == daughterNum) waitMom.notify_all();
            return;
        }
        default:
            throw runtime_error("Unknown harvester state. Either I messed up or someone touched my code -ML");
    }
    }
}

// TODO: Move this out and into EdgeCorrectness!
static inline double aligEdgesIncMoveOp(const Graph::Node &peg1, const Graph::Node &hole1,
    const Graph::Node &hole2, const Alignment &alignment, uint64_t totalEdges, bool directed) {

    #ifdef WEIGHT
        cerr << "EdgeCorrectness should not be used with weight. It is optimized for unweighted graphs and makes incompatible assumptions." <<endl;
        throw runtime_error("Bad measure used with WEIGHT compiler setting.");
    #endif
    int64_t result = 0;

    for (const auto &edge: peg1.adjList) {
        const unsigned nbrHole = alignment[edge.first];
        result -= hole1.adjList.count(nbrHole);
        result += hole2.adjList.count(nbrHole);
    }
    // self-loop correction
    if (peg1.adjList.count(peg1.nodeID)) {
        result -= hole2.adjList.count(hole1.nodeID); // Subtract erroneously counted edge
        result += hole2.adjList.count(hole2.nodeID); // Add correct edge
    }
    if(directed) {
        for (const auto& edge : peg1.injList) {
            const unsigned nbrHole = alignment[edge.first];
            result -= hole1.injList.count(nbrHole);
            result += hole2.injList.count(nbrHole);
        }
    }

    return static_cast<double>(result) / totalEdges;
}

// TODO: And this, too!
static inline double aligEdgesIncSwapOp(const Graph::Node &peg1, const Graph::Node &peg2,
    const Graph::Node &hole1, const Graph::Node &hole2, const Alignment &alignment,
    const uint64_t totalEdges, const bool directed) {
    #ifdef WEIGHT
        cerr << "EdgeCorrectness should not be used with weight. It is optimized for unweighted graphs and makes incompatible assumptions." <<endl;
        throw runtime_error("Bad measure used with WEIGHT compiler setting.");
    #endif
    /*
     * Marcus compiler optimizations for multithreading:
     * Do NOT call alignment[] for the same index twice in a row. You might believe "oh, the
     * compiler will just optimize it away, its no biggy!" IT WILL NOT, because the alignment can
     * vary while this calculation is ongoing. So the compiler is going to believe it has to fetch
     * the value again in case it has changed. Now, we don't actually care if it has changed, but
     * the compiler has no way to know that unless we tell it by manually specifying that YES, we
     * want to use the same value twice, code transparency be damned.
     */
    int64_t result = 0;

    for (const auto &edge: peg1.adjList) {
        const unsigned nbrHole = alignment[edge.first];
        result -= hole1.adjList.count(nbrHole);
        result += hole2.adjList.count(nbrHole);
    }
    // Self-loop correction.
    if (peg1.adjList.count(peg1.nodeID)) {
        result -= hole2.adjList.count(hole1.nodeID); // Subtract erroneously counted edge
        result += hole2.adjList.count(hole2.nodeID); // Add correct edge
    }

    for (const auto &edge: peg2.adjList) {
        const unsigned nbrHole = alignment[edge.first];
        result -= hole2.adjList.count(nbrHole);
        result += hole1.adjList.count(nbrHole);
    }
    // Self-loop correction
    if (peg2.adjList.count(peg2.nodeID)) {
        result -= hole1.adjList.count(hole2.nodeID); // Subtract erroneously counted edge
        result += hole1.adjList.count(hole1.nodeID); // Add correct edge
    }

    //
    if(directed) {
        for (const auto &edge: peg1.injList) {
            const unsigned nbrHole = alignment[edge.first];
            result -= hole1.injList.count(nbrHole);
            result += hole2.injList.count(nbrHole);
        }
        for (const auto &edge: peg2.injList) {
            const unsigned nbrHole = alignment[edge.first];
            result -= hole2.injList.count(nbrHole);
            result += hole1.injList.count(nbrHole);
        }

        // Directed peg edge correction
        if (peg1.adjList.count(peg2.nodeID)) {
        // peg1 <-> peg2
            if (peg2.adjList.count(peg1.nodeID)) {
                result += hole1.adjList.count(hole2.nodeID);
                result -= hole1.adjList.count(hole1.nodeID);
                result -= hole2.adjList.count(hole2.nodeID);
                result += hole2.adjList.count(hole1.nodeID);
            }
            // peg1 -> peg2
            else {
                result += hole1.adjList.count(hole2.nodeID);
                result += hole2.adjList.count(hole1.nodeID);
                result -= hole2.adjList.count(hole2.nodeID);
            }
        }
        // peg1 <- peg2
        else if (peg2.adjList.count(peg1.nodeID)) {
            result += hole2.adjList.count(hole1.nodeID);
            result += hole1.adjList.count(hole2.nodeID);
            result -= hole1.adjList.count(hole1.nodeID);
        }
    }
    // Undirected case to correct peg1 <-> peg2 edge, which should result in zero delta.
    else if (peg1.adjList.count(peg2.nodeID)) {
        result += hole1.adjList.count(hole2.nodeID);
        result -= hole1.adjList.count(hole1.nodeID);
        result -= hole2.adjList.count(hole2.nodeID);
        result += hole2.adjList.count(hole1.nodeID);
    }
    return static_cast<double>(result) / totalEdges;
}

double SANAThree::BatchHarvester::assessMove_(const ChangeRequest &input) const {
    double energyInc = 0.0;

    // This is a hack, MC should be providing this information, not SANA!!
    const Graph::Node &peg1 = parent.G1->deliverNode(input.peg1);
    const Graph::Node &hole1 = parent.G2->deliverNode(input.hole1);
    const Graph::Node &hole2 = parent.G2->deliverNode(input.hole2);
    if (parent.needEC) {
        energyInc += parent.MC->getWeight("ec") *
            aligEdgesIncMoveOp(peg1, hole1, hole2, parent.alignment, parent.m1, parent.G1->directed);
    }

    return energyInc;
}

double SANAThree::BatchHarvester::assessSwap_(const ChangeRequest &input) const {
    double energyInc = 0.0;

    const Graph::Node &peg1 = parent.G1->deliverNode(input.peg1);
    const Graph::Node &peg2 = parent.G1->deliverNode(input.peg2);
    const Graph::Node &hole1 = parent.G2->deliverNode(input.hole1);
    const Graph::Node &hole2 = parent.G2->deliverNode(input.hole2);

    // TODO: This is a hack, MeasureCombination should be performing these calculations! -ML
    if (parent.needEC) {
        energyInc += parent.MC->getWeight("ec") *
            aligEdgesIncSwapOp(peg1, peg2, hole1, hole2, parent.alignment, parent.m1, parent.G1->directed);
    }
    if ()

    return energyInc;
}


