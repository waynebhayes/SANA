#ifndef BATCHHARVESTER_HPP
#define BATCHHARVESTER_HPP

#define CHUNK_SIZE 4096

#include <condition_variable>

#include "SANAThree.hpp"

class SANAThree::BatchHarvester {
    // The internal state of the Harvester.
    // This is the primary way to communicate to the daughter threads what they should be doing.
    enum struct State {
        pause, // All daughters should be sleeping
        anneal, // All threads should compute a combined batchNumber of requests
        equilibrate, // All threads should compute requests until equilibrium is reached
        terminate // All threads should terminate
    };
public:
    BatchHarvester(unsigned threadNumber, SANAThree &SANA, unsigned long long bufferSize);
    ~BatchHarvester();

    // Collect a batch from BatchHarvester for annealing purposes. Note that this WILL mutate the
    // alignment (which is its job, after all). Returns the average score and average pBad of the
    // batch.
    ScoreWithPBad collectBatch(double temperature);

    // Find the equilibrium pBad of the annealing process at a certain temperature, timing out if
    // timeoutSeconds is exceeded.
    // This will also mutate the alignment.
    double runUntilEquilibrium(double temperature, unsigned timeoutSeconds);

private:
    SANAThree &parent;

    const unsigned daughterNum;

    // threadVector is the "fishing line" to pull each thread back in for termination.
    vector<thread> threadVector;

    atomic_uint64_t startedRequests; // changeRequests that have begun
    atomic_uint64_t finishedRequests; // changeRequests that have finished processing

    // State System
    // All indented members below should only be accessed if stateMutex is held.
    // Yes, even the member functions!
    // I really wish C++ had a way to enforce this at compile-time -ML
    mutex stateMutex;
        uint64_t activationID = 0;
        State currentState;
        condition_variable waitDaughters;
        condition_variable waitMom;

        unsigned daughtersPaused;
        unsigned daughtersTerminated; // Technically redundant, but we have the padding room we can use for clarity of purpose

        // Activate the daughter threads with a certain state.
        void activateDaughters(const State state) {
            currentState = state;
            daughtersPaused = 0;
            ++activationID;
            waitDaughters.notify_all();
        }

        double totalScore; // Total score of a batch; Used for calculating score averages
        double totalPBad; // Total pBad of a batch; Used for calculating pBad averages
        uint64_t numTotalPBad; // Number of pBad encountered; Used for calculating pBad averages
        void resetCounters() {
            startedRequests.store(0);
            finishedRequests.store(0);

            totalScore = 0;
            totalPBad = 0;
            numTotalPBad = 0;
        }

        double temperature;

    // This mutex is vestigial from an older version of SANAThree, but I found no easy way to
    // remove it for the temperature equilibrium calculation. In the future, it should be removed
    // from the class for a massive potential speedup. My recommendation is a message passing system
    // where the Mom thread handles the buffers.
    mutex bufferMutex;
        CircularBuffer<double> scoreCircBuffer;
        CircularBuffer<double> pBadCircBuffer;
        void resetBuffers() {
            pBadCircBuffer.resetBuffer();
            scoreCircBuffer.resetBuffer();
        }

    // All daughters of the BatchHarvestor are controlled from this loop and these functions.
    // This function operates as a state-machine for each thread, though with a shared state across
    // all daughters. -ML
    void daughterFunction(uint64_t seed);

    // Calculate the expected results of a request.
    double assessRequest(const ChangeRequest& currentRequest) const {
        if (currentRequest.twoPegs)
            return assessSwap_(currentRequest);
        return assessMove_(currentRequest);
    }
    double assessMove_(const ChangeRequest &input) const; // One pin
    double assessSwap_(const ChangeRequest &input) const; // Two pins
};

#endif //MOVECALCULATOR_HPP
