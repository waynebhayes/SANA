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
#include <unistd.h>

#include "SANAThree.hpp"
#include "BatchHarvester.hpp"
#include "../Report.hpp"

#define DEBUG_HARVESTER 0

double static inline acceptingProbability(const double energyInc, const double temperature) {
    if (temperature == 0.) return energyInc >= 0;
    return energyInc >= 0 ? 1 : exp(energyInc / temperature);
}

#define SCORE_BATCH_SIZE 31
SANAThree::BatchHarvester::BatchHarvester(const unsigned threadNumber, SANAThree &SANA, unsigned long long bufferSize):
    parent(SANA),
    daughterNum(threadNumber),
    temperature(0.),
    startedRequests(0),
    finishedRequests(0),
    numPBad(0),
    scoreBuffer(SCORE_BATCH_SIZE),
    pBadBuffer(bufferSize) {

    unique_lock<mutex> stateLock(stateMutex); // Overkill, but I am trying to set a good example to you youngsters
    currentState = State::pause; // We DO NOT want threads to start calculations yet.
    daughtersPaused = 0;
    daughtersTerminated = 0;
    stateLock.unlock();

    unique_lock<mutex> outputLock(outputMutex);
    totalEnergy = 0.; // Not strictly required, every function will reset these as necessary anyway
    totalPBad = 0.;
    outputLock.unlock();

    numPBad = 0;

    mt19937_64 generator(random_device{}());

    if (threadNumber == 0) throw runtime_error("Thread number must be > 0");
    threadVector.reserve(threadNumber);
    stateLock.lock();
    for (unsigned i = 0; i < threadNumber; ++i) {
        threadVector.emplace_back(&BatchHarvester::daughterFunction, this, generator());
    }
    handlerCondition.wait(stateLock, [this]{return daughtersPaused == daughterNum;});

#if DEBUG_HARVESTER
    assert(daughtersPaused == daughterNum);
#endif
}

SANAThree::BatchHarvester::~BatchHarvester(){
    unique_lock<mutex> stateLock(stateMutex); // Lock state access
    currentState = State::terminate; // Set the state to class termination
    if (daughtersPaused > 0) { // Check if any daughters are paused and wake them if they are
        daughtersPaused = 0;
        pausedCondition.notify_all();
    }
    handlerCondition.wait(stateLock, [this]{return daughtersTerminated == daughterNum;}); // Wait for all daughters to terminate
    assert(daughtersTerminated == daughterNum); // This thread should only be woken if all daughters are terminated
    for (thread& t: threadVector) t.join(); // Join all threads
}

double SANAThree::BatchHarvester::runUntilEquilibrium(double temperature, unsigned timeoutSeconds) {
    unique_lock<mutex> stateLock(stateMutex);
    unique_lock<mutex> outputLock(outputMutex, defer_lock);

#if DEBUG_HARVESTER
    // Are we in a valid state? Did someone forget to clean up?
    assert(currentState == State::pause);
    assert(daughtersPaused == daughterNum);
    assert(daughtersTerminated == 0);
    stateLock.unlock();
    // Reset everything in case someone somehow collected a traditional annealing batch before this equilibrium run
    outputLock.lock();
    scoreBuffer.resetBuffer();
    pBadBuffer.resetBuffer();
    outputLock.unlock();
    stateLock.lock();
#endif

    chrono::duration<double> duration;
    bool timedOut = false;

    this->temperature.store(temperature);
    currentState = State::equilibrate;
    daughtersPaused = 0;
    pausedCondition.notify_all();
    const auto originalTime = chrono::steady_clock::now();
    if (!handlerCondition.wait_for(stateLock, chrono::seconds(timeoutSeconds), [this]{return daughtersPaused == daughterNum;})) {
        // Clean up for an early wake.
        duration = chrono::steady_clock::now() - originalTime;
        timedOut = true;
        currentState = State::pause;
        handlerCondition.wait(stateLock, [this]{return daughtersPaused == daughterNum;});
    }
    else {
        duration = chrono::steady_clock::now() - originalTime;
    }
#if DEBUG_HARVESTER
    // We should be safe to unpause and unlock the state
    assert(currentState == State::pause);
    assert(daughtersPaused == daughterNum);
#endif
    stateLock.unlock();

    outputLock.lock();
    // Terminal output
    const double answer = recentPBadTrue();
    cout<<"> getEquilibriumPBadAtTemp("<<temperature<<") = "<<answer<<" (score: "<<parent.currentScore<<")"
    <<" (time: "<<duration.count()<<"s)"
    << (timedOut? "[TIMED OUT]" : "")
    <<" iterations = "<< finishedRequests
    <<", ips = "<< static_cast<double>(finishedRequests) / duration.count() <<endl
    <<"****************************************"<<endl<<endl;

    // Clean up
    startedRequests.store(0);
    finishedRequests.store(0);
    scoreBuffer.resetBuffer();
    pBadBuffer.resetBuffer();

    return answer;
}

SANAThree::batchOutput SANAThree::BatchHarvester::collectBatch(double temperature) {
    unique_lock<mutex> stateLock(stateMutex);
    unique_lock<mutex> outputLock(outputMutex, defer_lock);

#if DEBUG_HARVESTER
    // Are we in a valid state? Did someone forget to clean up? This entire section can be removed if it is never
    assert(currentState == State::pause);
    assert(daughtersPaused == daughterNum);
    assert(daughtersTerminated == 0);
    stateLock.unlock();
    assert(startedRequests == 0); // Atomics don't require locks
    assert(finishedRequests == 0);
    outputLock.lock();
    assert(totalEnergy == 0);
    assert(totalPBad == 0);
    assert(numPBad == 0);
    outputLock.unlock();
    stateLock.lock();
#endif

    this->temperature.store(temperature);
    currentState = State::anneal;
    daughtersPaused = 0;
    pausedCondition.notify_all();
    handlerCondition.wait(stateLock, [this]{return daughtersPaused == daughterNum;});
#if DEBUG_HARVESTER
    // We should be safe to unpause and unlock the state
    assert(currentState == State::pause);
    assert(daughtersPaused == daughterNum);
    assert(finishedRequests == parent.batchSize);
    assert(startedRequests >= parent.batchSize);
#endif
    // ReSharper disable once CppDFAUnreachableCode
    stateLock.unlock();

    outputLock.lock();
    startedRequests.store(0);
    finishedRequests.store(0);
    if (numPBad == 0) ++numPBad;
    const double averageScore = totalEnergy / parent.batchSize;
    const double averagePBad = totalPBad / numPBad;
    totalEnergy = 0;
    totalPBad = 0;
    numPBad = 0;

    return {averageScore, averagePBad};
}


void SANAThree::BatchHarvester::daughterFunction(uint64_t seed) {
    mt19937_64 generator(seed);

    unique_lock<mutex> outputLock(outputMutex, defer_lock);
    unique_lock<mutex> stateLock(stateMutex);

    // After every operation is complete, the function returns to check the current state of the Harvester
    while (true) switch (currentState) { // Always enter with state locked

        case State::anneal: {
            stateLock.unlock();
            if (startedRequests++ >= parent.batchSize) {
                stateLock.lock();
                currentState = State::pause;
                continue;
            }
            // Part 1, select request to process
            changeRequest currentRequest = parent.chooseNextRequest(generator);

            // Part 2, assess request
            assessChange(currentRequest);
            const double pBad = acceptingProbability(currentRequest.energyInc, temperature);

            // Part 3, implement (or don't implement) request
            const double newScore = parent.implementLastRequest(pBad, currentRequest, generator);

            // Part 4, record the results
            outputLock.lock();
            totalEnergy += newScore;
            if (currentRequest.energyInc < 0) {
                totalPBad += pBad;
                pBadBuffer.insert(pBad);
                ++numPBad;
            }
            outputLock.unlock();

            ++finishedRequests;
            stateLock.lock();
            continue;
        }

        case State::equilibrate: {
            stateLock.unlock();
            ++startedRequests;
            // Part 1, select request to process
            changeRequest currentRequest = parent.chooseNextRequest(generator);

            // Part 2, assess request
            assessChange(currentRequest);
            const double pBad = acceptingProbability(currentRequest.energyInc, temperature);

            // Part 3, implement (or don't implement) request
            const double newScore = parent.implementLastRequest(pBad, currentRequest, generator);

            // Part 4, record the results
            stateLock.lock();
            if (currentState != State::equilibrate) continue; // Esoteric race condition, honestly, user skill issue if
            // it is encountered. More threads would have to be
            // assigned by the user than nodes in either G1 or G2.
            // This is a performance hit that makes me unhappy, but I
            // feel obligated to accommodate it. -Marcus
            stateLock.unlock();
            outputLock.lock();
            if (currentRequest.energyInc < 0) {
                pBadBuffer.insert(pBad);
            }
            if (++finishedRequests % parent.batchSize == 0) {
                scoreBuffer.insert(newScore);
                if (scoreBuffer.isFull() && !scoreBuffer.trendingUpwards()) {
                    stateLock.lock();
                    currentState = State::pause;
                    stateLock.unlock();
                }
            }
            outputLock.unlock();

            stateLock.lock();
            continue;
        }

        case State::pause: {
                if (++daughtersPaused == daughterNum) handlerCondition.notify_all();
                pausedCondition.wait(stateLock, [this]{return daughtersPaused == 0;});
            continue;
        }

        case State::terminate: {
            if (++daughtersTerminated == daughterNum) handlerCondition.notify_all();
            return;
        }

        default: throw runtime_error("Unknown harvester state. Either I messed up or someone touched my code -Marcus");
    }

}


static inline double aligEdgesIncMoveOp(const Graph::Node &peg1, const Graph::Node &hole1, const Graph::Node &hole2,
    Alignment &alignment, uint64_t totalEdges, bool directed) {

#ifdef WEIGHT
    cerr << "EdgeCorrectness should not be used with weight. It is optimized for unweighted graphs and makes incompatible assumptions." <<endl;
    throw runtime_error("Bad measure used with WEIGHT compiler setting.");
#else
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
#endif
}

static inline double aligEdgesIncSwapOp(const Graph::Node &peg1, const Graph::Node &peg2, const Graph::Node &hole1, const Graph::Node &hole2,
    Alignment &alignment, uint64_t totalEdges, bool directed) {
#ifdef WEIGHT
    cerr << "EdgeCorrectness should not be used with weight. It is optimized for unweighted graphs and makes incompatible assumptions." <<endl;
    throw runtime_error("Bad measure used with WEIGHT compiler setting.");
#else
    /*
     * Marcus compiler optimizations for multithreading:
     * Do NOT call alignment[] for the same index twice in a row. You might believe "oh, the compiler will just optimize
     * it away, its no biggy!" NO, IT WILL NOT, because the alignment can vary while this calculation is ongoing. So the
     * compiler is going to believe it has to fetch the value again in case it has changed. Now, we don't actually care
     * if it has changed, but the compiler has no way to know that unless we tell it by manually specifying that YES,
     * we want to use the same value twice, code transparency be damned.
     */
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
    for (const auto &edge: peg2.adjList) {
        const unsigned nbrHole = alignment[edge.first];
        result -= hole2.adjList.count(nbrHole);
        result += hole1.adjList.count(nbrHole);
    }
    // self-loop correction
    if (peg2.adjList.count(peg2.nodeID)) {
        result -= hole1.adjList.count(hole2.nodeID); // Subtract erroneously counted edge
        result += hole1.adjList.count(hole1.nodeID); // Add correct edge
    }
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
        if (peg2.adjList.count(peg1.nodeID)) { // peg1 <-> peg2
            result += hole1.adjList.count(hole2.nodeID);
            result -= hole1.adjList.count(hole1.nodeID);
            result -= hole2.adjList.count(hole2.nodeID);
            result += hole2.adjList.count(hole1.nodeID);
        }
        else { // peg1 -> peg2
            result += hole1.adjList.count(hole2.nodeID);
            result += hole2.adjList.count(hole1.nodeID);
            result -= hole2.adjList.count(hole2.nodeID);
        }}
        else if (peg2.adjList.count(peg1.nodeID)) { // peg1 <- peg2
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
#endif
}

void SANAThree::BatchHarvester::assessMove(changeRequest &input) const {
    // This is a hack, MC should be providing this information, not SANA!!
    const Graph::Node &peg1 = parent.G1->deliverNode(input.peg1);
    const Graph::Node &hole1 = parent.G2->deliverNode(input.hole1);
    const Graph::Node &hole2 = parent.G2->deliverNode(input.hole2);
    if (parent.needEC) {
        input.energyInc = aligEdgesIncMoveOp(peg1, hole1, hole2, parent.alignment, parent.m1, parent.G1->directed)
                          * parent.MC->getWeight("ec");
    }
}

void SANAThree::BatchHarvester::assessSwap(changeRequest &input) const {
    // This is a hack, MC should be providing this information, not SANA!!
    const Graph::Node &peg1 = parent.G1->deliverNode(input.peg1);
    const Graph::Node &peg2 = parent.G1->deliverNode(input.peg2);
    const Graph::Node &hole1 = parent.G2->deliverNode(input.hole1);
    const Graph::Node &hole2 = parent.G2->deliverNode(input.hole2);
    if (parent.needEC) {
        input.energyInc = aligEdgesIncSwapOp(peg1, peg2, hole1, hole2, parent.alignment, parent.m1, parent.G1->directed)
                          * parent.MC->getWeight("ec");
    }
}


