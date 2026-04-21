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

private:

    struct changeRequest {
        // Request properties
        const bool twoPegs;

        const unsigned peg1;
        const unsigned peg2;
        const unsigned hole1;
        const unsigned hole2;

        changeRequest(bool two_pegs, unsigned peg1, unsigned peg2, unsigned hole1, unsigned hole2):
            twoPegs(two_pegs), peg1(peg1), peg2(peg2), hole1(hole1), hole2(hole2) {
        }
    };

    struct batchOutput {
        const double averageScore;
        const double averagePBad;
    };

    // Convenience variables
    const uint64_t n1, n2, m1, m2;

    // Control variables, keep constant -Marcus
    const double tolerance;
    const double maxSeconds;
    const uint64_t maxIterations;
    const uint64_t batchSize; // MUST BE A MULTIPLE OF CHUNK_SIZE!!
    const unsigned threadNumber;
    const bool hillClimbing, needEC, needEM, needER;
    const MeasureCombination *const MC;
    const Alignment startingAlignment; // Give an empty alignment for a scramble
    const string outputFileName;
    const string localScoresFileName;

    double tInitial;
    double tFinal{};
    double tDecay;

    BatchHarvester *threadPool;

    atomic<double> currentScore;
    Alignment alignment;

    // Set-up function
    void resetAlignment();

    uint64_t runIterations();
    uint64_t runConfidenceIntervals();
    uint64_t runHillClimbing();

    // THE REQUEST SYSTEM
    // Hole locking system
    vector<atomic_flag> holeLocks;
    bool tryToLockHoles(unsigned hole1, unsigned hole2);
    void releaseHoles(unsigned hole1, unsigned hole2);

    changeRequest chooseNextRequest(mt19937_64 &generator);
    double implementLastRequest(double pBad, double energyInc, const changeRequest &input, mt19937_64 &generator, uniform_real_distribution<> &dist); // Returns if accepted or rejected

    // TRACKING SYSTEM
    void trackProgress(long long unsigned iter, double fractionTime, double elapsedTime,
        double temperature, double lastAvgPBad, unsigned batches = 0, double batchScore = 0., double batchPbad = 0.) const;

    static void handleInterruption();

    static void setInterruptSignal(); // Control+C during execution offers options
    void printReportOnInterruption() const;
public:
    // Interrupt handler
    static volatile std::sig_atomic_t userInterrupted;
    static bool saveAligAndExitOnInterruption;
    static bool saveAligAndContOnInterruption;
};



#endif // SANATHREE_HPP
