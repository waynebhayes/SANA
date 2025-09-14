#ifndef SANATHREE_HPP
#define SANATHREE_HPP
#include <mutex>
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
#include "../utils/Misc.hpp"
#include "../utils/CircularBuffer.hpp"
#include "../Graph.hpp"

using namespace std;

// Big Picture TODO list by priority:
// 0.) Fix Happy Batches System
// 1.) Refactor and cleanly implement the stationary node system from 2.0
// 2.) Individual node KE system
// 3.) Multi-pairwise SANA reimplemented
// If you have ideas for any of this, my element is @malongo:matrix.org
// -Marcus



class SANAThree: public Method {
    class BatchHarvester;
public:
    SANAThree(const Graph* G1, const Graph* G2, double TInitial, double TDecay, double maxSeconds,
        long long maxIterations, double tolerance, bool addHillClimbing, const MeasureCombination* MC,
        const string& scoreAggrStr, const Alignment& optionalStartAlig, const string& outputFileName,
        const string& localScoresFileName, unsigned threadNumber);
    ~SANAThree() override;

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

    unsigned pBadsInBuffer() const;

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

    // Convenience variables
    const uint64_t n1, n2, m1, m2;

    // Control variables, keep constant -Marcus
    const bool hillClimbing, needEC, needEM, needER;
    const double tolerance;
    const double maxSeconds;
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

    BatchHarvester *threadPool;

    // Set-up function
    void initDataStructures();

    // Main run function and variables
    uint64_t totalMovesCalculated;
    uint64_t totalMovesAccepted;
    uint64_t totalSwapsCalculated;
    uint64_t totalSwapsAccepted;
    void runIterations();
    void runConfidenceIntervals();
    void runHillClimbing();

    void scramble();

    // THE REQUEST SYSTEM

    uniform_real_distribution<> randomReal;
    vector<vector<unsigned>> colorUnassignedNodes;
    // Keeps track of the total number of alignments, swaps, and moves we have access to as changes
    uint64_t numAdjacentAlignments;
    uint64_t numSwaps;
    vector<uint64_t> swapsPerColor;
    vector<uint64_t> movesPerColor;

    // These mess with these mutexes.
    mutex scoreMutex;
        atomic<double> currentScore;
    mutex alignmentMutex;
        Alignment alignment;
        vector<bool> holeLocks;
    changeRequest chooseNextRequest(mt19937_64 &generator);
    double implementLastRequest(double pBad, const changeRequest &input, mt19937_64 &generator); // Returns if accepted or rejected

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



#endif // SANATHREE_HPP