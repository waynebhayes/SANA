#ifndef NEWSANA_HPP
#define NEWSANA_HPP
#include <condition_variable>
#include <mutex>
#include <thread>
#include <map>
#include <ctime>
#include <random>
#include <list>
#include <limits>
#include <queue>
#include <set>

#include "Method.hpp"
#include "../measures/Measure.hpp"
#include "../measures/MeasureCombination.hpp"
#include "../measures/CoreScore.hpp"
#include "../utils/Misc.hpp"
#include "../utils/CircularBuffer.hpp"
#include "../Graph.hpp"

using namespace std;

// Big Picture TODO list by priority:
// 1.) Add thread number as a command line argument
// 2.) Refactor and cleanly implement the stationary node system from 2.0
// 3.) Fix happy batches system
// 4.) Figure out how to better track the statistics of the SA process without cluttering up the
// SANA class like in the 2.0
// 5.) Goldilocks functionality
// 6.) SANAWrapper retired
// 7.) Better thread error detection and/or experiment with more restrictive locking schemes
// 8.) Individual node KE system
// 9.) Multi-pairwise SANA reimplemented
// If you have ideas for any of this, my element is @malongo:matrix.org
// -Marcus



class SANAThree: public Method {

public:
    SANAThree(const Graph* G1, const Graph* G2, double TInitial, double TDecay, double maxSeconds,
        long long maxIterations, double tolerance, bool addHillClimbing, const MeasureCombination* MC,
        const string& scoreAggrStr, const Alignment& optionalStartAlig, const string& outputFileName,
        const string& localScoresFileName, unsigned threadNumber);
    ~SANAThree() override {delete threadPool;}

    Alignment run() override;

    // Compatibility functions, the intent is for this to get reworked - Marcus
    Alignment runUsingIterations();
    Alignment runUsingConfidenceIntervals();

    double getEquilibriumPBadAtTemp(double temperature, unsigned timeoutSeconds) const;

    void describeParameters(ostream& stream) const override;
    string fileNameSuffix(const Alignment& A) const override;

    void setTInitial(const double t) {tInitial = t;}
    void setTFinal(const double t) {tFinal = t;}

    //requires TInitial and TFinal to be already initialized
    void setTDecayFromTempRange() {tDecay = -log(tFinal/tInitial);}

    unsigned pBadsInBuffer() const {return threadPool->pBadsInBuffer();}

private:

    struct changeRequest {
        // Request properties
        const bool twoPegs;

        const unsigned peg1;
        const unsigned peg2;
        const unsigned hole1;
        const unsigned hole2;

        const unsigned hole2unassignedID;

        const unsigned color;

        // Request output
        double energyInc;

        changeRequest(bool two_pegs, unsigned peg1, unsigned peg2, unsigned hole1, unsigned hole2,
        unsigned hole2unassignedID, unsigned colorID, double energyInc):
            twoPegs(two_pegs), peg1(peg1), peg2(peg2), hole1(hole1), hole2(hole2), hole2unassignedID(hole2unassignedID),
            color(colorID) {
            this->energyInc = energyInc;
        }
    };

    struct batchOutput {
        const double averageScore;
        const double averagePBad;
    };

    // Multithread handler (yes, this is used even when single-threaded to prevent code bloat
    // even if it is slightly slower than dedicated single-threading, but also if you are single-
    // threading even after the refactor, then you clearly care more about accuracy than speed).
    // -Marcus
    class CalculatorHandler {
    public:
        // The handler starts the threads as soon as it is constructed and terminates them when it
        // is deconstructed. Therefore, it should only ever exist as a local object at the smallest
        // possible scope to ensure that the computer threads are not being hogged by a greedy SANA.
        // -Marcus
        CalculatorHandler(unsigned threadNumber, SANAThree &SANA, unsigned long long bufferSize);
        ~CalculatorHandler();

        // This is the proper way to interface with collectBatch
        // -Marcus
        batchOutput collectBatch(double temperature);

        double runUntilEquilibrium(double temperature, unsigned timeoutSeconds);

        double recentPBadQuick() const {return pBadBuffer.quickAverage();}

        double recentPBadTrue() {return pBadBuffer.accurateAverage();}

        unsigned pBadsInBuffer() const {return pBadBuffer.size();}

        void resetBuffers() {pBadBuffer.resetBuffer(); scoreBuffer.resetBuffer();}

    private:
        bool calculatorsOn;
        bool collectBatches;
        const unsigned daughterNum;

        double temperature;
        double totalEnergy;
        double totalPBad;

        uint64_t inputRequests;
        uint64_t outputRequests;
        uint64_t pBadTotal;

        SANAThree &parent;

        mutex requestMutex;
        mutex bufferMutex;

        condition_variable requestsFinished;
        condition_variable equilibriumCheck;
        condition_variable startBatch;

        CircularBuffer<double> scoreBuffer;
        CircularBuffer<double> pBadBuffer;

        vector<thread> threadVector;
        void _mainLoop();
        void _assessChange(changeRequest& currentRequest) const {
            if (currentRequest.twoPegs) _assessSwap(currentRequest);
            else _assessMove(currentRequest);
        }
        void _assessMove(changeRequest &input) const; // One pin
        void _assessSwap(changeRequest &input) const; // Two pins
    };

    // Convenience variables
    const uint64_t n1, n2, m1, m2;

    // Control variables, keep constant -Marcus
    const bool hillClimbing, needEC, needEM, needER;
    const double tolerance;
    const unsigned long long maxSeconds;
    const unsigned long long maxIterations;
    const unsigned long long batchSize;
    const unsigned threadNumber;
    const MeasureCombination *const MC;
    const Alignment startingAlignment; // Give an empty alignment for a scramble
    const string outputFileName;
    const string localScoresFileName;

    double tInitial;
    double tFinal{};
    double tDecay;

    CalculatorHandler *threadPool;

    // Set-up function
    void initDataStructures();

    // Main run function and variables
    Alignment alignment;
    double currentScore;
    uint64_t totalMovesCalculated;
    uint64_t totalMovesAccepted;
    uint64_t totalSwapsCalculated;
    uint64_t totalSwapsAccepted;
    void runIterations();
    void runConfidenceIntervals();
    void runHillClimbing();

    void scramble();

    // THE REQUEST SYSTEM

    mt19937_64 generator; // rng
    uniform_real_distribution<> randomReal;
    vector<vector<unsigned>> colorUnassignedNodes;
    // Keeps track of the total number of alignments, swaps, and moves we have access to as changes
    uint64_t numAdjacentAlignments;
    uint64_t numSwaps;
    vector<uint64_t> swapsPerColor;
    vector<uint64_t> movesPerColor;

    mutex checkHoleLock;
    vector<bool> holeLocks;

    changeRequest chooseNextRequest();
    void implementLastRequest(double pBad, const changeRequest &input);

    // TRACKING SYSTEM
    void trackProgress(long long unsigned iter, double fractionTime, double elapsedTime,
        double temperature, double lastAvgPBad, unsigned batches = 0, double batchScore = 0., double batchPbad = 0.) const;
    static void setInterruptSignal(); // Control+C during execution offers options
    void printReportOnInterruption() const;
public:
    // Interrupt handler
    // These need to be public to be set from the interruption handler
    static bool saveAligAndExitOnInterruption;
    static bool saveAligAndContOnInterruption;
};



#endif // NEWSANA_HPP