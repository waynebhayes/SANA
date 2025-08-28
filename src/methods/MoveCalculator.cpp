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

    requestProcessed.wait(requestLock, [this] {return _outputRequests == _parent.batchSize;});
    if (_pBadTotal == 0) _pBadTotal = 1;
    return {totalEnergy / _parent.batchSize, totalPBad / _pBadTotal};
}

void SANAThree::CalculatorHandler::_mainLoop() {
    // See comment about unique_locks in submitRequest
    unique_lock<mutex> requestLock (_requestSystem);

    startBatch.wait(requestLock, [this] {return _inputRequests < _parent.batchSize || !_calculatorsOn;});
    while (_calculatorsOn) {
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
        requestProcessed.notify_one();
        startBatch.wait(requestLock, [this] {return _inputRequests < _parent.batchSize || !_calculatorsOn;});
    }
}

static inline double aligEdgesIncChangeOp(uint peg, uint oldHole, uint newHole, Alignment &alignment, const Graph *G1, const Graph *G2, unsigned denominator) {
    int res = 0;
    if (G1->hasSelfLoop(peg)) {
        if (G2->hasSelfLoop(oldHole)) res-=G2->getEdgeWeight(oldHole, oldHole);
        if (G2->hasSelfLoop(newHole)) res+=G2->getEdgeWeight(newHole, newHole);
    }
    for (uint nbr : *G1->getAdjList(peg)) if (nbr != peg) {
	res -= G2->getEdgeWeight(oldHole, alignment[nbr]);
	res += G2->getEdgeWeight(newHole, alignment[nbr]);
    }
    if(G1->directed) for (uint nbr : *G1->getInjList(peg)) if (nbr != peg) {
	res -= G2->getEdgeWeight(alignment[nbr],oldHole);
	res += G2->getEdgeWeight(alignment[nbr],newHole);
    }
    return static_cast<double>(res) / denominator;
}

static inline double aligEdgesIncSwapOp(uint peg1, uint peg2, uint hole1, uint hole2, Alignment &alignment, const Graph *G1, const Graph *G2, unsigned denominator) {
#ifdef WEIGHT
    throw runtime_error("SANATwo::aligEdgesIncSwapOp should not be called with WEIGHT");
    return 0;
#else
    int res = 0;
    if (G1->hasSelfLoop(peg1)) {
        if (G2->hasSelfLoop(hole1)) res-=G2->getEdgeWeight(hole1, hole1);
        if (G2->hasSelfLoop(hole2)) res+=G2->getEdgeWeight(hole2, hole2);
    }
    for (uint nbr : *G1->getAdjList(peg1)) if (nbr != peg1) {
	res -= G2->getEdgeWeight(hole1, alignment[nbr]);
	res += G2->getEdgeWeight(hole2, alignment[nbr]);
    }
    if(G1->directed) for (uint nbr : *G1->getInjList(peg1)) if (nbr != peg1) {
	res -= G2->getEdgeWeight(alignment[nbr],hole1);
	res += G2->getEdgeWeight(alignment[nbr],hole2);
    }

    if (G1->hasSelfLoop(peg2)) {
        if (G2->hasSelfLoop(hole2)) res-=G2->getEdgeWeight(hole2, hole2);
        if (G2->hasSelfLoop(hole1)) res+=G2->getEdgeWeight(hole1, hole1);
    }
    for (uint nbr : *G1->getAdjList(peg2)) if (nbr != peg2) {
	res -= G2->getEdgeWeight(hole2, alignment[nbr]);
	res += G2->getEdgeWeight(hole1, alignment[nbr]);
    }
    if(G1->directed) for (uint nbr : *G1->getInjList(peg2)) if (nbr != peg2) {
	res -= G2->getEdgeWeight(alignment[nbr],hole2);
	res += G2->getEdgeWeight(alignment[nbr],hole1);
    }

    //address the case where we are swapping between adjacent nodes with adjacent images:
#if defined(MULTI_PAIRWISE) || defined(MULTI_MPI)
    //why set the least-significant bit to 0?
    //this kind of bit manipulation needs a comment clarification -Nil
    res += (-1 << 1) & (G1->getEdgeWeight(peg1, peg2) +
                        G2->getEdgeWeight(hole1, hole2));
#else
    if                 (G1->hasEdge(peg1, peg2) and G2->hasEdge(hole1, hole2)) res += 2;
    if(G1->directed) if(G1->hasEdge(peg2, peg1) and G2->hasEdge(hole2, hole1)) res += 2;
#endif
    return res / denominator;
#endif // WEIGHT
}

void SANAThree::CalculatorHandler::_assessMove(changeRequest &input) const {
    // This is a hack, MC should be providing this information, not SANA!!
    if (_parent.needEC) {
        input.energyInc = aligEdgesIncChangeOp(input.peg1, input.hole1, input.hole2,
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


