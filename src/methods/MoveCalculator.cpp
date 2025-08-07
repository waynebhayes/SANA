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

#define WAIT_TIME 2 // I am uncertain of how long this should be, but it shouldn't be very long.

SANAThree::CalculatorHandler::CalculatorHandler(const unsigned threadNumber, SANAThree &SANA):
    _extraThreads(threadNumber),
    _parent(SANA) {
    _calculatorsOn = true;
    _requestBalance = 0;

    if (threadNumber == 0) return;
    // else
    _threadVector.reserve(threadNumber);
    for (unsigned i = 0; i < threadNumber; ++i) {
        _threadVector.emplace_back(&CalculatorHandler::_mainLoop, this);
    }
}

SANAThree::CalculatorHandler::~CalculatorHandler(){
    if (_extraThreads == 0) return;

    // Ensures all threads are terminated before deconstruction.
    _calculatorsOn = false;
    requestSubmitted.notify_all();
    for (thread& t: _threadVector) {
        t.join();
    }
}

void SANAThree::CalculatorHandler::submitRequest(changeRequest input) {
    // Unique_locking this is horrible overkill for a function this simple, but in case someone
    // chooses to mess with this in the future, I have safety proofed it. The compiler will
    // optimize it out anyway.
    unique_lock<mutex> inputLock (_scoringQueueMutex, defer_lock);

    assert(_extraThreads > 0 && "You should not be submitting requests to CalculatorHandler if you have no extra threads");
    _requestBalance++;
    inputLock.lock();
    _scoringQueue.push(input);
    inputLock.unlock();
    requestSubmitted.notify_one();
}

SANAThree::changeRequest SANAThree::CalculatorHandler::extractRequest() {
    assert(_requestBalance > 0 && "You should not be extracting more requests from CalculatorHandler than you submitted!");
    _requestBalance--;

    // See comment about unique_locks in submitRequest
    unique_lock<mutex> outputLock (_decisionQueueMutex);

    // Explanation: basically, this single line tells the thread to block until decisionQueue is no
    // longer empty and wake up to test this only whenever the outputLock mutex is unlocked.
    // Ideally, _decisionQueue should never be empty. If it is, you're not using enough thread
    requestProcessed.wait(outputLock, [this] {return !_decisionQueue.empty();});
    const auto request = _decisionQueue.front();
    _decisionQueue.pop();
    return request;
}

void SANAThree::CalculatorHandler::_mainLoop() {
    // See comment about unique_locks in submitRequest
    unique_lock<mutex> inputLock (_scoringQueueMutex, defer_lock);
    unique_lock<mutex> outputLock (_decisionQueueMutex, defer_lock);

    while (_calculatorsOn) {
        inputLock.lock();
        requestSubmitted.wait(inputLock, [this] {return !_scoringQueue.empty() || !_calculatorsOn;});
        if (!_calculatorsOn) {break;}
        changeRequest currentRequest = _scoringQueue.front();
        _scoringQueue.pop();
        inputLock.unlock();
        _assessChange(currentRequest);
        outputLock.lock();
        _decisionQueue.push(currentRequest);
        outputLock.unlock();
        requestProcessed.notify_one();
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
