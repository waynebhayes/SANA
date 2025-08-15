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
    // Ensures all threads are terminated before deconstruction.
    _calculatorsOn = false;
    startBatch.notify_all();
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

    _inputRequests = 0;
    _outputRequests = 0;
    startBatch.notify_all();

    requestLock.lock();
    requestProcessed.wait(requestLock, [this] {return _outputRequests == _parent.batchSize;});
    return {totalEnergy / _parent.batchSize, totalPBad / _parent.batchSize};
}

void SANAThree::CalculatorHandler::_mainLoop() {
    // See comment about unique_locks in submitRequest
    unique_lock<mutex> requestLock (_requestSystem);

    while (_calculatorsOn) {
        startBatch.wait(requestLock, [this] {return _inputRequests < _parent.batchSize || !_calculatorsOn;});
        if (!_calculatorsOn) {
            requestLock.unlock();
            return;
        }
        changeRequest currentRequest = _parent.chooseNextRequest();
        _inputRequests++;
        requestLock.unlock();

        _assessChange(currentRequest);

        const double pBad = acceptingProbability(currentRequest.energyInc, temperature);
        requestLock.lock();
        _parent.implementLastRequest(pBad, currentRequest);
        totalPBad += pBad;
        totalEnergy += _parent.currentScore;
        _outputRequests++;
        requestLock.unlock();
        requestProcessed.notify_one();
        requestLock.lock();
    }
}

void SANAThree::CalculatorHandler::_assessMove(changeRequest &input) const {
    // This is a hack, MC should be providing this information, not SANA!!
    if (_parent.needEC) {
        input.energyInc = EdgeCorrectness::getIncChangeOp(input.peg1, input.hole1, input.hole2, _parent.alignment)
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
        input.energyInc = EdgeCorrectness::getIncSwapOp(input.peg1, input.peg2, input.hole1, input.hole2, _parent.alignment)
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
