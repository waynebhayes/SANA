#ifndef SANATHREE_HPP
#define SANATHREE_HPP
#include <csignal>
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

class SANAThree: public Method {
    class BatchHarvester;
public:
    SANAThree(const Graph* G1, const Graph* G2, double TInitial, double TDecay, double maxSeconds,
        long long maxIterations, double tolerance, bool addHillClimbing, const MeasureCombination* MC,
        const string& scoreAggrStr, const Alignment& optionalStartAlig, const string& outputFileName,
        const string& localScoresFileName, unsigned threadNumber);
    ~SANAThree() override;

    // The public run function. This will start annealing.
    Alignment run() override;

    // Compatibility functions, the intent is for these to be completely removed -ML
    Alignment runUsingIterations();
    Alignment runUsingConfidenceIntervals();

    // Equilibrium mode takes in a temperature and a max number of seconds to run and performs a faux
    // annealing at that temperature until we either reach equilibrium or we timeout, then returns
    // the most recent average pBad. Note that because of the buffersd required, it is MUCH slower
    // than the usual run mode.
    double getEquilibriumPBadAtTemp(double temperature, unsigned timeoutSeconds) const;

    void describeParameters(ostream& stream) const override;
    string fileNameSuffix(const Alignment& A) const override;

    // This is an odd set-up inherited from SANA 2 given the CLI inputs are tinitial and tdecay. -ML
    void setTInitial(const double t) {tInitial = t;}
    void setTFinal(const double t) {tFinal = t;}
    //requires TInitial and TFinal to be already initialized
    void setTDecayFromTempRange() {tDecay = -log(tFinal/tInitial);}

private:

    // CUSTOM DATA TYPES

    // A ChangeRequest is the standard memory structure used for calculating and then applying a
    // particular change to the alignment. There are two types: a move and a swap. A move
    // moves peg1 from hole1 to an empty hole2. A swap swaps peg1 to hole2 and peg2 to hole1.
    // It must always be true that while the ChangeRequest exists, peg1 must be in hole1 and peg2
    // must be in hole2. If not, the Alignment will end up tearing.
    // It is possible that using alignas(32) or alignas(64) might increase speed by preventing
    // false sharing by threads on a cache line, but it is currently untested. -ML
    struct ChangeRequest {
        // We might be able to reduce this to 16 bytes by using peg1 != peg2 as our twoPegs
        // indicator. Given the alignas results, this might or might not be a good idea. -ML
        const bool twoPegs;

        const unsigned peg1;
        const unsigned peg2;
        const unsigned hole1;
        const unsigned hole2;
    };

    // This is used instead of a pair to prevent confusion of the score and pBad result.
    struct ScoreWithPBad {
        const double score;
        const double pBad;
    };

    // Convenience variables
    // CONVENIENCE VARIABLES
    const uint64_t n1, n2, m1, m2;

    // CONTROL VARIABLES
    // Some of these we can probably get rid of -ML
    const double tolerance;
    const double maxSeconds;
    const uint64_t maxIterations;
    const uint64_t batchSize; // MUST BE A MULTIPLE OF CHUNK_SIZE!
    const unsigned threadNumber;
    const bool hillClimbing;
    const MeasureCombination *const MC;
    const Alignment startingAlignment; // Give an empty alignment for a scramble
    const string outputFileName;
    const string localScoresFileName;

    // These can't be made constant because SANAThree is required to calculate these values
    // if they weren't known at program start.
    double tInitial;
    double tFinal{};
    double tDecay;

    // The thread pool that calculates batches for us.
    // We need to decide on an alternative to a raw pointer but which won't create a circular dependency -ML
    BatchHarvester *threadPool;

    atomic<double> currentScore;
    Alignment alignment;

    vector<unsigned> pegColorToHoleColor;

    // RUN FUNCTIONS
    void resetAlignment();
    uint64_t runIterations();
    uint64_t runConfidenceIntervals();
    uint64_t runHillClimbing();

    // THE REQUEST SYSTEM

    // holeLocks should never be accessed directly, please use tryToLockHoles and releaseHoles
    // to ensure the holes are locked and released atomically and in the canonical order.
    vector<atomic_flag> holeLocks;
    bool tryToLockHoles(unsigned hole1, unsigned hole2);
    void releaseHoles(unsigned hole1, unsigned hole2);

    // Generate a valid request that contains no peg or hole that is
    ChangeRequest chooseNextRequest(mt19937_64 &generator);
    // Interpret a request based on its calculations
    double implementLastRequest(ScoreWithPBad calculations, const ChangeRequest &input, mt19937_64 &generator, uniform_real_distribution<> &dist); // Returns if accepted or rejected

    // TRACKING SYSTEM
    void trackProgress(long long unsigned iter, double fractionTime, double elapsedTime,
        double temperature, double lastAvgPBad, unsigned batches = 0, double batchScore = 0., double batchPbad = 0.) const;

    // INTERRUPT HANDLER
    static void interruptionUserInput();
    static void setInterruptSignal();
    void printReportOnInterruption() const;
public:
    static volatile std::sig_atomic_t userInterrupted;
    static bool saveAligAndExitOnInterruption;
    static bool saveAligAndContOnInterruption;
};



#endif // SANATHREE_HPP
