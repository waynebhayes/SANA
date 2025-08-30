#include <sys/unistd.h>
#include <chrono>
#include <iostream>
#include <thread>
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
#include <csignal>
#include <cstdio>
#include <unistd.h>

#include "SANAThree.hpp"
#include "../measures/SymmetricSubstructureScore.hpp"
#include "../measures/JaccardSimilarityScore.hpp"
#include "../measures/InducedConservedStructure.hpp"
#include "../measures/EdgeCorrectness.hpp"
#include "../measures/EdgeDifference.hpp"
#include "../measures/EdgeRatio.hpp"
#include "../measures/EdgeMin.hpp"
#include "../measures/EdgeGeoMean.hpp"
#include "../measures/SquaredEdgeScore.hpp"
#include "../measures/WeightedEdgeConservation.hpp"
#include "../measures/NodeCorrectness.hpp"
#include "../measures/SymmetricEdgeCoverage.hpp"
#include "../measures/localMeasures/Sequence.hpp"
#include "../measures/EdgeExposure.hpp"
#include "../measures/MultiS3.hpp"
#include "../measures/FMeasure.hpp"
#include "../utils/utils.hpp"
#include "../Report.hpp"

double static inline acceptingProbability(const double energyInc, const double temperature) {
    if (temperature == 0.) return energyInc >= 0;
    return energyInc >= 0 ? 1 : exp(energyInc / temperature);
}

SANAThree::CalculatorHandler::CalculatorHandler(const unsigned threadNumber, SANAThree &SANA):
    _extraThreads(threadNumber),
    _parent(SANA){
    _calculatorsOn = true;
    temperature = 0;
    totalEnergy = 0;
    totalPBad = 0;

    _inputRequests = _parent.batchSize;
    _outputRequests = _parent.batchSize;

    if (threadNumber == 0) throw runtime_error("Thread number must be > 0");
    _threadVector.reserve(threadNumber);
    for (unsigned i = 0; i < threadNumber; ++i) {
        _threadVector.emplace_back(&CalculatorHandler::_mainLoop, this);
    }
}

SANAThree::CalculatorHandler::~CalculatorHandler(){
    unique_lock<mutex> requestLock (_requestSystem);
    // Ensures all threads are terminated before deconstruction.
    _calculatorsOn = false;
    startBatch.notify_all();
    requestLock.unlock();
    for (thread& t: _threadVector) {
        t.join();
    }
}

SANAThree::batchOutput SANAThree::CalculatorHandler::collectBatch(double temperature) {
    // Unique_locking this is horrible overkill for a function this simple, but in case someone
    // chooses to mess with this in the future, I have safety proofed it. The compiler will
    // optimize it out anyway.
    unique_lock<mutex> requestLock (_requestSystem, defer_lock);

    this->temperature = temperature;
    totalEnergy = 0.;
    totalPBad = 0.;

    requestLock.lock();
    _inputRequests = 0;
    _outputRequests = 0;
    _pBadTotal = 0;
    startBatch.notify_all();

    requestsFinished.wait(requestLock);
    if (_pBadTotal == 0) _pBadTotal = 1;
    return {totalEnergy / _parent.batchSize, totalPBad / _pBadTotal};
}

void SANAThree::CalculatorHandler::_mainLoop() {
    // See comment about unique_locks in submitRequest
    unique_lock<mutex> requestLock (_requestSystem);

    bool on; // To prevent double accessing this variable when unnecessary. We want compiler to cache it in certain scenarios
    startBatch.wait(requestLock, [this, &on]{
        on = _calculatorsOn;
        return _inputRequests < _parent.batchSize || !on;
    });
    while (on) {
        changeRequest currentRequest = _parent.chooseNextRequest();
        _inputRequests++;
        requestLock.unlock();

        _assessChange(currentRequest);

        const double pBad = acceptingProbability(currentRequest.energyInc, temperature);
        requestLock.lock();
        _parent.implementLastRequest(pBad, currentRequest);
        if (currentRequest.energyInc < 0) {
            totalPBad += pBad;
            _pBadTotal++;
        }
        totalEnergy += _parent.currentScore;
        _outputRequests++;
        on = _calculatorsOn;
        if (_inputRequests >= _parent.batchSize and on) {
            if (_outputRequests >= _parent.batchSize) requestsFinished.notify_one();
            startBatch.wait(requestLock);
            on = _calculatorsOn;
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
    if (_parent.needEC) {
        input.energyInc = aligEdgesIncMoveOp(input.peg1, input.hole1, input.hole2,
                                            _parent.alignment, _parent.G1, _parent.G2, _parent.m1)
                          * _parent.MC->getWeight("ec");
    }
    if (_parent.needEM) {
        input.energyInc += EdgeMin::getIncChangeOp(input.peg1, input.hole1, input.hole2, _parent.alignment)
                           * _parent.MC->getWeight("emin");
    }
    if (_parent.needER) {
        input.energyInc += EdgeRatio::getIncChangeOp(input.peg1, input.hole1, input.hole2, _parent.alignment)
                           * _parent.MC->getWeight("er");
    }
}

void SANAThree::CalculatorHandler::_assessSwap(changeRequest &input) const {
    // This is a hack, MC should be providing this information, not SANA!!
    if (_parent.needEC) {
        input.energyInc = aligEdgesIncSwapOp(input.peg1, input.peg2, input.hole1, input.hole2,
                                            _parent.alignment, _parent.G1, _parent.G2, _parent.m1)
                          * _parent.MC->getWeight("ec");
    }
    if (_parent.needEM) {
        input.energyInc += EdgeMin::getIncSwapOp(input.peg1, input.peg2, input.hole1, input.hole2, _parent.alignment)
                           * _parent.MC->getWeight("emin");
    }
    if (_parent.needER) {
        input.energyInc += EdgeRatio::getIncSwapOp(input.peg1, input.peg2, input.hole1, input.hole2, _parent.alignment)
                           * _parent.MC->getWeight("er");
    }
}


