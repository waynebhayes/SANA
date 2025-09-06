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
    while (on) { // Request system should be locked while this check is made, too complicated to explain why
        // Part 1, generate request
        // (We enter this part locked!)
        changeRequest currentRequest = parent.chooseNextRequest();
        inputRequests++;
        requestLock.unlock();

        // Part 2, assess request and calculate pBad
        _assessChange(currentRequest);
        const double pBad = acceptingProbability(currentRequest.energyInc, temperature);

        // Part 3, implement request
        requestLock.lock();
        parent.implementLastRequest(pBad, currentRequest);
        requestLock.unlock();

        // Part 4, update stats
        bufferLock.lock();
        outputRequests++;
        if (!collectBatches) { // For equilibrium, ts is so cringe, we should probably be doing this switch with an inherited class -Marcus
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
        if (currentRequest.energyInc < 0) {
            pBadBuffer.insert(pBad);
            totalPBad += pBad;
            pBadTotal++;
        }
        totalEnergy += parent.currentScore;
        bufferLock.unlock();
        requestLock.lock();
        if (inputRequests >= parent.batchSize and ((on = calculatorsOn))) {
            if (outputRequests >= parent.batchSize) requestsFinished.notify_all();
            startBatch.wait(requestLock);
            on = calculatorsOn;
        }
    }
}

static inline double aligEdgesIncMoveOp(uint peg, uint oldHole, uint newHole, Alignment &alignment, const Graph *G1, const Graph *G2, unsigned denominator) {
    int res = 0;
    if (G1->hasSelfLoop(peg)) {
        if (G2->hasSelfLoop(oldHole))
            res-=G2->getEdgeWeight(oldHole, oldHole);
        if (G2->hasSelfLoop(newHole))
            res+=G2->getEdgeWeight(newHole, newHole);
    }
    for (uint nbrPeg : *G1->getAdjList(peg)) if (nbrPeg != peg) {
        const unsigned nbrHole = alignment[nbrPeg];
        const int def = G2->getEdgeWeight(oldHole, nbrHole);
        res -= def;
        const int sur = G2->getEdgeWeight(newHole, nbrHole);
        res += sur;
    }
    if(G1->directed)
        for (uint nbrPeg : *G1->getInjList(peg)) if (nbrPeg != peg) {
            const unsigned nbrHole = alignment[nbrPeg];
            const int def = G2->getEdgeWeight(nbrHole, oldHole);
            res -= def;
            const int sur = G2->getEdgeWeight(nbrHole, newHole);
            res += sur;
    }
    return static_cast<double>(res) / denominator;
}

static inline double aligEdgesIncSwapOp(uint peg1, uint peg2, uint hole1, uint hole2, Alignment &alignment, const Graph *G1, const Graph *G2, unsigned denominator) {
    int result = 0;

    // TODO: weight check

    /*
     * Marcus compiler optimizations for multithreading:
     * Do NOT call alignment[] for the same index twice in a row. You might believe "oh, the compiler will just optimize
     * it away, its no biggy!" NO, IT WILL NOT, because the alignment can vary while this calculation is ongoing. So the
     * compiler is going to believe it has to fetch the value again in case it has changed. Now, we don't actually care
     * if it has changed, but the compiler has no way to know that unless we tell it by manually specifying that YES,
     * we want to use the same value twice, code transparency be damned.
     */

    // Peg 1 changes
    if (G1->hasSelfLoop(peg1)) {
        if (G2->hasSelfLoop(hole1))
            result-=G2->getEdgeWeight(hole1, hole1);
        if (G2->hasSelfLoop(hole2))
            result+=G2->getEdgeWeight(hole2, hole2);
    }
    for (const uint nbrPeg : *G1->getAdjList(peg1)) if (nbrPeg != peg1) {
        const unsigned nbrHole = alignment[nbrPeg];
        const int def = G2->getEdgeWeight(hole1, nbrHole);
        result -= def;
        const int sur = G2->getEdgeWeight(hole2, nbrHole);
        result += sur;
    }
    // Peg 2 changes
    if (G1->hasSelfLoop(peg2)) {
        if (G2->hasSelfLoop(hole2))
            result-=G2->getEdgeWeight(hole2, hole2);
        if (G2->hasSelfLoop(hole1))
            result+=G2->getEdgeWeight(hole1, hole1);
    }
    for (const uint nbrPeg : *G1->getAdjList(peg2)) if (nbrPeg != peg2) {
        const unsigned nbrHole = alignment[nbrPeg];
        const int deficit = G2->getEdgeWeight(hole2, nbrHole);
        result -= deficit;
        const int sur = G2->getEdgeWeight(hole1, nbrHole);
        result += sur;
    }
    // Fix for double counting
    if (G1->hasEdge(peg1, peg2) and G2->hasEdge(hole1, hole2))
        result += 2;

    // Same thing again, but backwards
    if(G1->directed) {
        for (const uint nbrPeg : *G1->getInjList(peg1)) if (nbrPeg != peg1) {
            const unsigned nbrHole = alignment[nbrPeg];
            const int def = G2->getEdgeWeight(nbrHole, hole1);
            result -= def;
            const int sur = G2->getEdgeWeight(nbrHole, hole2);
            result += sur;
        }
        for (const uint nbrPeg : *G1->getInjList(peg2)) if (nbrPeg != peg2) {
            const unsigned nbrHole = alignment[nbrPeg];
            const int deficit = G2->getEdgeWeight(nbrHole, hole2);
            result -= deficit;
            const int sur = G2->getEdgeWeight(nbrHole, hole1);
            result += sur;
        }
        if (G1->hasEdge(peg2, peg1) and G2->hasEdge(hole2, hole1))
            result += 2;
    }

    return static_cast<double>(result) / denominator;
}

void SANAThree::CalculatorHandler::_assessMove(changeRequest &input) const {
    // This is a hack, MC should be providing this information, not SANA!!
    if (parent.needEC) {
        input.energyInc = aligEdgesIncMoveOp(input.peg1, input.hole1, input.hole2,
                                            parent.alignment, parent.G1, parent.G2, parent.m1)
                          * parent.MC->getWeight("ec");
    }
    if (parent.needEM) {
        input.energyInc += EdgeMin::getIncChangeOp(input.peg1, input.hole1, input.hole2, parent.alignment)
                           * parent.MC->getWeight("emin");
    }
    if (parent.needER) {
        input.energyInc += EdgeRatio::getIncChangeOp(input.peg1, input.hole1, input.hole2, parent.alignment)
                           * parent.MC->getWeight("er");
    }
}

void SANAThree::CalculatorHandler::_assessSwap(changeRequest &input) const {
    // This is a hack, MC should be providing this information, not SANA!!
    if (parent.needEC) {
        input.energyInc = aligEdgesIncSwapOp(input.peg1, input.peg2, input.hole1, input.hole2,
                                            parent.alignment, parent.G1, parent.G2, parent.m1)
                          * parent.MC->getWeight("ec");
    }
    if (parent.needEM) {
        input.energyInc += EdgeMin::getIncSwapOp(input.peg1, input.peg2, input.hole1, input.hole2, parent.alignment)
                           * parent.MC->getWeight("emin");
    }
    if (parent.needER) {
        input.energyInc += EdgeRatio::getIncSwapOp(input.peg1, input.peg2, input.hole1, input.hole2, parent.alignment)
                           * parent.MC->getWeight("er");
    }
}


