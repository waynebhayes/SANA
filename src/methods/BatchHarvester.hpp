#ifndef BATCHHARVESTER_HPP
#define BATCHHARVESTER_HPP

#include <condition_variable>

#include "SANAThree.hpp"

class SANAThree::BatchHarvester {
    enum struct State {
        anneal,
        equilibrate,
        pause,
        terminate
    };
public:
    // The handler starts the threads as soon as it is constructed and terminates them when it
    // is deconstructed. Therefore, it should only ever exist as a local object at the smallest
    // possible scope to ensure that the computer threads are not being hogged by a greedy SANA.
    // -Marcus
    BatchHarvester(unsigned threadNumber, SANAThree &SANA, unsigned long long bufferSize);
    ~BatchHarvester();

    // This is the proper way to interface with collectBatch
    // -Marcus
    batchOutput collectBatch(double temperature);

    double runUntilEquilibrium(double temperature, unsigned timeoutSeconds);

    double recentPBadQuick() const {return pBadBuffer.quickAverage();}

    double recentPBadTrue() {return pBadBuffer.accurateAverage();}

    unsigned pBadsInBuffer() const {return pBadBuffer.size();}

    void resetBuffers() {pBadBuffer.resetBuffer(); scoreBuffer.resetBuffer();}

private:
    SANAThree &parent;

    vector<thread> threadVector;

    // State System
    const unsigned daughterNum;
    mutex stateMutex; // All indented members below should only be accessed if this mutex is locked.
        State currentState;
        unsigned daughtersPaused;
        unsigned daughtersTerminated;
        condition_variable pausedCondition;
        condition_variable handlerCondition;
    atomic<double> temperature; // Write only if stateMutex is locked.

    atomic_uint64_t startedRequests;
    atomic_uint64_t finishedRequests;

    // Output system
    mutex outputMutex; // All indented members below should only be accessed if this mutex is locked.
        double totalEnergy;
        double totalPBad;
        uint64_t numPBad;
        CircularBuffer<double> scoreBuffer;
        CircularBuffer<double> pBadBuffer;

    void daughterFunction(uint64_t seed);
    void assessChange(changeRequest& currentRequest) const {
        if (currentRequest.twoPegs) assessSwap(currentRequest);
        else assessMove(currentRequest);
    }
    void assessMove(changeRequest &input) const; // One pin
    void assessSwap(changeRequest &input) const; // Two pins
    bool allDaughtersPaused() const {return daughtersPaused == daughterNum;} // ONLY CALL IF STATE MUTEX IS HELD!
};

#endif //MOVECALCULATOR_HPP
