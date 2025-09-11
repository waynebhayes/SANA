#include <sys/unistd.h>
#include <chrono>
#include <iostream>
#include <thread>
#include <string>
#include <vector>
#include <fstream>
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
#include "../measures/EdgeRatio.hpp"
#include "../measures/EdgeMin.hpp"
#include "../measures/SquaredEdgeScore.hpp"
#include "../Report.hpp"

double static inline acceptingProbability(const double energyInc, const double temperature) {
    if (temperature == 0.) return energyInc >= 0;
    return energyInc >= 0 ? 1 : exp(energyInc / temperature);
}

#define SCORE_BATCH_SIZE 31
SANAThree::CalculatorHandler::CalculatorHandler(const unsigned threadNumber, SANAThree &SANA, unsigned long long bufferSize):
    daughterNum(threadNumber),
    parent(SANA),
    scoreBuffer(SCORE_BATCH_SIZE),
    pBadBuffer(bufferSize) {

    calculatorsOn = true;
    collectBatches = true;
    temperature = 0.;
    totalEnergy = 0.;
    totalPBad = 0.;

    inputRequests = parent.batchSize;
    outputRequests = parent.batchSize;

    pBadTotal = 0;

    if (threadNumber == 0) throw runtime_error("Thread number must be > 0");
    threadVector.reserve(threadNumber);
    for (unsigned i = 0; i < threadNumber; ++i) {
        threadVector.emplace_back(&CalculatorHandler::_mainLoop, this);
    }
}

SANAThree::CalculatorHandler::~CalculatorHandler(){
    unique_lock<mutex> requestLock (requestMutex);
    // Ensures all threads are terminated before deconstruction.
    calculatorsOn = false;
    startBatch.notify_all();
    requestLock.unlock();
    for (thread& t: threadVector) {
        t.join();
    }
}

double SANAThree::CalculatorHandler::runUntilEquilibrium(const double temperature, unsigned timeoutSeconds) {
    unique_lock<mutex> requestLock (requestMutex, defer_lock);
    unique_lock<mutex> bufferLock (bufferMutex, defer_lock);

    this->temperature = temperature;

    bufferLock.lock();
    scoreBuffer.resetBuffer();
    pBadBuffer.resetBuffer();
    bufferLock.unlock();

    requestLock.lock();
    collectBatches = false;
    inputRequests = 0;
    outputRequests = 0;
    startBatch.notify_all();
    requestLock.unlock();

    const auto originalTime = chrono::steady_clock::now();
    const auto timeoutTime = originalTime + chrono::seconds(timeoutSeconds);
    bool timedOut = false;

    bufferLock.lock();
    unsigned iter = SCORE_BATCH_SIZE - 1;
    do {
        if (equilibriumCheck.wait_until(bufferLock, timeoutTime) == cv_status::timeout) {
            timedOut = true;
            break;
        }
        ++iter;
        equilibriumCheck.notify_one();
    } while (scoreBuffer.trendingUpwards());
    collectBatches = true;

    const double answer = recentPBadTrue();
    scoreBuffer.resetBuffer();
    pBadBuffer.resetBuffer();


    const chrono::duration<double> duration = chrono::steady_clock::now() - originalTime;
    cout<<"> getEquilibriumPBadAtTemp("<<temperature<<") = "<<answer<<" (score: "<<parent.currentScore<<")"
    <<" (time: "<<duration.count()<<"s)"
    << (timedOut? "[TIMED OUT]" : "")
    <<" iterations = "<<iter * parent.batchSize
    <<", ips = "<< (iter * parent.batchSize) / duration.count() <<endl
    <<"****************************************"<<endl<<endl;

    return answer;
}


SANAThree::batchOutput SANAThree::CalculatorHandler::collectBatch(const double temperature) {
    // Unique_locking this is horrible overkill for a function this simple, but in case someone
    // chooses to mess with this in the future, I have safety proofed it. The compiler will
    // optimize it out anyway.
    unique_lock<mutex> requestLock (requestMutex, defer_lock);
    unique_lock<mutex> bufferLock (bufferMutex, defer_lock);

    this->temperature = temperature;

    bufferLock.lock();
    totalEnergy = 0.;
    totalPBad = 0.;
    pBadTotal = 0;
    bufferLock.unlock();

    requestLock.lock();
    inputRequests = 0;
    outputRequests = 0;

    startBatch.notify_all();

    requestsFinished.wait(requestLock);
    bufferLock.lock();
    if (pBadTotal == 0) pBadTotal = 1;
    return {totalEnergy / parent.batchSize, totalPBad / pBadTotal};
}

void SANAThree::CalculatorHandler::_mainLoop() {
    // See comment about unique_locks in submitRequest
    unique_lock<mutex> bufferLock (bufferMutex, defer_lock);
    unique_lock<mutex> requestLock (requestMutex);

    bool on; // To prevent double accessing this variable when unnecessary. We want compiler to cache it in certain scenarios


    startBatch.wait(requestLock, [this, &on]{
        on = calculatorsOn;
        return inputRequests < parent.batchSize || !on || !collectBatches;
    });
    while (on) { // Request system should always be locked while this check is made, too complicated to explain why
        // Part 1, generate request
        // (We enter this part locked!)
        ++inputRequests;
        requestLock.unlock();
        changeRequest currentRequest = parent.chooseNextRequest();

        // Part 2, assess request and calculate pBad
        _assessChange(currentRequest);
        const double pBad = acceptingProbability(currentRequest.energyInc, temperature);

        // Part 3, implement request
        parent.implementLastRequest(pBad, currentRequest);

        // Part 4, update stats
        bufferLock.lock();
        ++outputRequests;
        if (!collectBatches) { // For equilibrium, ts is so cringe -Marcus
            if (currentRequest.energyInc < 0) pBadBuffer.insert(pBad);
            if (outputRequests % parent.batchSize == 0) {
                scoreBuffer.insert(parent.currentScore);
                if (scoreBuffer.isFull()) {
                    equilibriumCheck.notify_all();
                    // Pause this thread until calculation over
                    equilibriumCheck.wait(bufferLock);
                }
            }
            bufferLock.unlock();
            requestLock.lock();
            continue;
        }
        // Implicit else
        if (currentRequest.energyInc < 0) {
            pBadBuffer.insert(pBad);
            totalPBad += pBad;
            pBadTotal++;
        }
        totalEnergy += parent.currentScore;
        bufferLock.unlock();

        // Stop batch logic
        requestLock.lock();
        if (inputRequests >= parent.batchSize and ((on = calculatorsOn))) {
            if (outputRequests >= parent.batchSize) requestsFinished.notify_all();
            startBatch.wait(requestLock);
            on = calculatorsOn;
        }
    }
}



static inline double aligEdgesIncMoveOp(const Graph::Node &peg1, const Graph::Node &hole1, const Graph::Node &hole2,
    Alignment &alignment, uint64_t totalEdges, bool directed) {

#ifdef WEIGHT
    cerr << "EdgeCorrectness should not be used with weight. It is optimized for unweighted graphs and make incompatible assumptions." <<endl;
    throw runtime_error("Bad measure used with WEIGHT compiler setting.");
#else
    int result = 0;

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
    cerr << "EdgeCorrectness should not be used with weight. It is optimized for unweighted graphs and make incompatible assumptions." <<endl;
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
    int result = 0;

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

void SANAThree::CalculatorHandler::_assessMove(changeRequest &input) const {
    // This is a hack, MC should be providing this information, not SANA!!
    const Graph::Node &peg1 = parent.G1->deliverNode(input.peg1);
    const Graph::Node &hole1 = parent.G2->deliverNode(input.hole1);
    const Graph::Node &hole2 = parent.G2->deliverNode(input.hole2);
    if (parent.needEC) {
        input.energyInc = aligEdgesIncMoveOp(peg1, hole1, hole2, parent.alignment, parent.m1, parent.G1->directed)
                          * parent.MC->getWeight("ec");
    }
}

void SANAThree::CalculatorHandler::_assessSwap(changeRequest &input) const {
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


