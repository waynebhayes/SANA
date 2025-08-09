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

#include "Method.hpp"
#include "../measures/Measure.hpp"
#include "../measures/MeasureCombination.hpp"
#include "../measures/CoreScore.hpp"
#include "../utils/Misc.hpp"

using namespace std;

// Big Picture TODO list by priority:
// 0.) Add thread number as a command line argument
// 1.) Profile this new version and find if the main thread or the daughters are the bottleneck
// 2.) Refactor and cleanly implement the stationary node system from 2.0
// 3.) Fix happy batches system
// 4.) Refactor the node color system
// 5.) Figure out how to better track the statistics of the SA process without cluttering up the
// SANA class like in the 2.0
// 6.) Goldilocks functionality
// 7.) SANAWrapper retired
// 8.) Better thread error detection and/or experiment with more restrictive locking schemes
// 9.) Individual node KE system
// 10.) Multi-pairwise SANA reimplemented
// If you have ideas for any of this, my element is @malongo:matrix.org
// -Marcus



class SANAThree: public Method {

public:
    SANAThree(const Graph* G1, const Graph* G2, double TInitial, double TDecay, double maxSeconds,
        long long maxIterations, double tolerance, bool addHillClimbing, const MeasureCombination* MC,
        const string& scoreAggrStr, const Alignment& optionalStartAlig, const string& outputFileName,
        const string& localScoresFileName, unsigned threadNumber);
    ~SANAThree() override {}

    Alignment run() override;

    // Compatibility functions, the intent is for this to get reworked - Marcus
    Alignment runUsingIterations();
    Alignment runUsingConfidenceIntervals();
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
        const unsigned activeColorSANA;
        const unsigned unoccupiedHole2ID;

        // Request output
        double energyInc;

        // Constructor to initialize all members
        changeRequest(bool twoPegs, unsigned peg1, unsigned peg2,
                      unsigned hole1, unsigned hole2, unsigned activeColorSANA,
                      unsigned unoccupiedHole2ID)
            : twoPegs(twoPegs), peg1(peg1), peg2(peg2),
              hole1(hole1), hole2(hole2), activeColorSANA(activeColorSANA),
              unoccupiedHole2ID(unoccupiedHole2ID), energyInc(0.0) {
            energyInc = 0.0;
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
        CalculatorHandler(unsigned threadNumber, SANAThree &SANA);
        ~CalculatorHandler();

        // These are the proper getters and setters for CalculatorHandler. Please use this, minding
        // the below comment.
        // -Marcus
        void submitRequest(changeRequest input);
        changeRequest extractRequest();

        // This function is be considered PRIVATE, and hence the underscore. It is public ONLY for a
        // single use in _singleThreadBatch in the main class because we have no threads to spare.
        // Otherwise, this function should never be accessed directly by SANAThree unless you know
        // what you are doing or have consulted me.
        // TODO: make this a friend function
        void _assessChange(changeRequest &input) const{
            if (input.twoPegs) _assessSwap(input);
            else _assessMove(input);
        }
    private:
        bool _calculatorsOn;
        condition_variable requestSubmitted;
        condition_variable requestProcessed;
        const unsigned _extraThreads;
        int _requestBalance;
        SANAThree &_parent;
        mutex _scoringQueueMutex;
        mutex _decisionQueueMutex;
        queue<changeRequest> _scoringQueue;
        queue<changeRequest> _decisionQueue;
        vector<thread> _threadVector;
        void _mainLoop();
        void _assessMove(changeRequest &input) const; // One pin
        void _assessSwap(changeRequest &input) const; // Two pins
    };

    // Control variables, keep constant -Marcus
    const bool hillClimbing, needEC, needEM, needER;
    const double tolerance;
    const unsigned long long maxSeconds;
    const unsigned long long maxIterations;
    const unsigned threadNumber;
    const MeasureCombination *const MC;
    const Alignment startingAlignment; // Give an empty alignment for a scramble
    const string outputFileName;
    const string localScoresFileName;

    // Convenience variables
    const unsigned n1, n2, m1, m2;

    double tInitial;
    double tFinal;
    double tDecay;

    // Set-up function
    void initDataStructures();

    // Main run function and variables
    vector<unsigned> alignment;
    double currentScore;
    void runIterations(CalculatorHandler &threadPool);
    void runConfidenceIntervals(CalculatorHandler &threadPool);
    void runHillClimbing(CalculatorHandler &threadPool);

    void scramble();
    batchOutput collectBatch(CalculatorHandler &threadPool, const double temperature) {
        if (threadNumber == 1)
            return _singleThreadBatch(threadPool, temperature);
        return _multiThreadBatch(threadPool, temperature);
    }
    batchOutput _singleThreadBatch(CalculatorHandler &threadPool, double temperature);
    batchOutput _multiThreadBatch(CalculatorHandler &threadPool, double temperature);

    // THE REQUEST SYSTEM

    mt19937 generator; // rng
    uniform_real_distribution<> randomReal;

    // To mark holes as off limits for swaps or changes. Vector<bool> is too slow, so we shall
    // sacrifice space for speed.
    vector<char> lockedHoles;

    // This is not mine, which is probably why I wish to rewrite it, lol. -Marcus
    vector<double> actColToAccumProbCutpoint;
    vector<double> actColToChangeProb;
    vector<uint> actColToG1ColId;
    vector<vector<uint>> actColToUnassignedG2Nodes;
    vector<uint> g2NodeToActColId; //data structure used to initialize actColToUnassignedG2Nodes
    uint INVALID_ACTIVE_COLOR_ID; //arbitrary value bigger than any valid active color id

    changeRequest chooseNextRequest();
    void implementLastRequest(double pBad, const changeRequest &input);

    // Luxury functions
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