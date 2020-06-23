#ifndef SANA_HPP
#define SANA_HPP
#include "Method.hpp"
#include <map>
#include <tuple>
#include <mutex>
#include <chrono>
#include <ctime>
#include <random>
#include <list>
#include <utility>
#include <unordered_set>
#include "../measures/localMeasures/LocalMeasure.hpp"
#include "../measures/Measure.hpp"
#include "../measures/MeasureCombination.hpp"
#include "../utils/randomSeed.hpp"
#include "../measures/ExternalWeightedEdgeConservation.hpp"
#include "../measures/CoreScore.hpp"

using namespace std;


class SANA: public Method {

public:
    SANA(const Graph* G1, const Graph* G2, double TInitial, double TDecay, double maxSeconds, long long maxIterations,
        bool addHillClimbing, MeasureCombination* MC, const string& scoreAggrStr,
        const Alignment& optionalStartAlig, const string& outputFileName, const string& localScoresFileName);
    ~SANA();

    Alignment run();
    void describeParameters(ostream& stream) const;
    string fileNameSuffix(const Alignment& A) const;
    
    //set temperature decay dynamically
    void setDynamicTDecay();    

    void setTInitial(double t);
    void setTFinal(double t);

    //requires TInitial and TFinal to be already initialized
    void setTDecayFromTempRange();

    double getPBad(double temp, double maxTimeInS = 1.0, int logLevel = 1); //0 for no output, 2 for verbose
    list<pair<double, double>> ipsList;

private:
    Alignment startA;

    bool addHillClimbing; //for post-run hill climbing
    void performHillClimbing(long long int idleCountTarget);

    //temperature schedule
    double TInitial, TFinal, TDecay;

    //circular pBad buffer
    const int PBAD_CIRCULAR_BUFFER_SIZE = 10000;
    vector<double> pBadBuffer;
    int numPBadsInBuffer;
    int pBadBufferIndex;
    double pBadBufferSum;

    //may incorrect probabilities (even negative) if the pbads in the buffer are small enough
    //due to accumulated precision errors of adding and subtracting tiny values from pBadBufferSum
    double incrementalMeanPBad(); 

    //this function adds all the pBads in the buffer from scratch and divides at the end
    //this takes linear time instead of constant, hence the name
    double slowMeanPBad();

    //store whether or not most recent move was bad
    bool wasBadMove;

    //data structures for the networks
    uint n1, n2;
    double g1Edges, g2Edges; //stored as double because they appear in division
    uint pairsCount; // number of combinations of (g1_node_x, g1_node_y) including
                     // a pair that includes the same node (g1_node_x, g1_node_x)
    double g1WeightedEdges, g2WeightedEdges;

    //random number generation
    mt19937 gen;
    uniform_real_distribution<> randomReal;

    //execution time is delimited by either maxSeconds or maxIterations
    //exactly one of them can be > 0 
    bool useIterations;
    double maxSeconds;
    long long int maxIterations;

    uint iterationsPerformed = 0;
    uint oldIterationsPerformed = 0;
    double oldTimeElapsed = 0;

    //to compute TDecay dynamically
    //tau holds "ideal" temperature values at certain execution times
    void initTau();
    bool dynamicTDecay;
    vector<double> tau;
    double dynamicTDecayTime;

    double Temperature; //shouldn't be capitalized
    double temperatureFunction(long long int iter, double TInitial, double TDecay);
    double acceptingProbability(double energyInc, double Temperature);


    double iterPerSecond;
    double getIterPerSecond();
    bool initializedIterPerSecond;
    void initIterPerSecond();
    void constantTempIterations(long long int iterTarget);

    //initializes the data structures specific to the starting alignment
    //if startA is empty, a random alignment is used
    void initDataStructures();
    vector<uint> A;
    vector<bool> assignedNodesG2;

    //objective function
    MeasureCombination* MC;
    double eval(const Alignment& A) const;
    double ecWeight, edWeight, erWeight, s3Weight, icsWeight, wecWeight, jsWeight, secWeight,
           ncWeight, localWeight, mecWeight, sesWeight, eeWeight, ms3Weight, ewecWeight;

    //this should be refactored so that the return parameter is not the 9th one out of 15
    // changed in June 2020 to return pBad, not the decision itself. -WH
    double scoreComparison(double newAligEdges, double newInducedEdges,
        double newLocalScoreSum, double newWecSum, 
        double newJsSum, double newNcSum, double& newCurrentScore, 
        double newEwecSum, double newSquaredAligEdges, double newExposedEdgesNumer, 
        double newMS3Numer, double newEdgeDifferenceSum, double newEdgeRatioSum);

    enum class ScoreAggregation{sum, product, inverse, max, min, maxFactor};
    ScoreAggregation scoreAggr;

    uint iterationsPerStep;

    //to evaluate EC incrementally
    bool needAligEdges;
    int aligEdges;
    int aligEdgesIncChangeOp(uint source, uint oldTarget, uint newTarget);
    int aligEdgesIncSwapOp(uint source1, uint source2, uint target1, uint target2);

    // to evaluate ED (edge difference score) incrementally
    bool needEd, needEr;
    double edSum, erSum;
    double edgeDifferenceIncChangeOp(uint source, uint oldTarget, uint newTarget);
    double edgeDifferenceIncSwapOp(uint source1, uint source2, uint target1, uint target2);
    double edgeRatioIncChangeOp(uint source, uint oldTarget, uint newTarget);
    double edgeRatioIncSwapOp(uint source1, uint source2, uint target1, uint target2);

    // to evaluate SES incrementally
    bool needSquaredAligEdges;
    int squaredAligEdges;
    int squaredAligEdgesIncChangeOp(uint source, uint oldTarget, uint newTarget);
    int squaredAligEdgesIncSwapOp(uint source1, uint source2, uint target1, uint target2);

	// to evaluate EE incrementally
    bool needExposedEdges;
    int exposedEdgesIncChangeOp(uint source, uint oldTarget, uint newTarget);
    int exposedEdgesIncSwapOp(uint source1, uint source2, uint target1, uint target2);
    
    // to evaluate MS3 incrementally
    bool needMS3;
    int MS3Numer;
    int MS3IncChangeOp(uint source, uint oldTarget, uint newTarget);
    int MS3IncSwapOp(uint source1, uint source2, uint target1, uint target2);

    //to evaluate SEC incrementally
    bool needSec;
    double secSum;

    //to evaluate S3 incrementally
    bool needInducedEdges;
    int inducedEdges = -1; // This variable must be initialized as non-zero since it's passed
                           // to scoreComparison in performSwap as "newInducedEdges" which could
                           // make computation go wrong.
    int inducedEdgesIncChangeOp(uint source, uint oldTarget, uint newTarget);

    //to evaluate nc incrementally
    bool needNC;
    int ncSum;
    vector<uint> trueAWithValidCountAppended;
    int ncIncChangeOp(uint source, uint oldTarget, uint newTarget);
    int ncIncSwapOp(uint source1, uint source2, uint target1, uint target2);

    //to evaluate wec incrementally
    bool needWec;
    double wecSum;
    vector<vector<float>> wecSims;
    double WECIncChangeOp(uint source, uint oldTarget, uint newTarget);
    double WECIncSwapOp(uint source1, uint source2, uint target1, uint target2);

    //to evaluate js incrementally
    bool needJs;
    double jsSum;
    vector<uint> alignedByNode;
    double JSIncChangeOp(uint source, uint oldTarget, uint newTarget);
    double JSIncSwapOp(uint source1, uint source2, uint target1, uint target2);

    //to evaluate ewec incrementally
    bool needEwec;
    ExternalWeightedEdgeConservation* ewec;
    double ewecSum;
    double EWECIncChangeOp(uint source, uint oldTarget, uint newTarget);
    double EWECIncSwapOp(uint source1, uint source2, uint target1, uint target2);
    double EWECSimCombo(uint source, uint target);

    //to evaluate local measures incrementally
    bool needLocal;
    double localScoreSum;
    map<string, double> localScoreSumMap;
    vector<vector<float>> sims;

    //to evaluate core scores    
#ifdef CORES
    constexpr double LOW_PBAD_LIMIT_FOR_CORES = 1e10;
    CoreScoreData coreScoreData;
#endif

    map<string, vector<vector<float>>> localSimMatrixMap;
    double localScoreSumIncChangeOp(const vector<vector<float>>& sim, uint source, uint oldTarget, uint newTarget);
    double localScoreSumIncSwapOp(const vector<vector<float>>& sim, uint source1, uint source2, uint target1, uint target2);

    //other execution options
    bool constantTemp; //tempertare does not decrease as a function of iteration
    bool enableTrackProgress; //shows output periodically
    void trackProgress(long long int iter, long long int maxIter = -1);
    double avgEnergyInc;

    double currentScore;
    double previousScore;
    double energyInc;
    void SANAIteration();
    void performChange(uint activeColorId);
    void performSwap(uint activeColorId);

    Timer timer;

    //Control+C during execution offers options
    //exit, save alignment and exit, save alignment and continue
    static void setInterruptSignal(); 
    void printReportOnInterruption();
public: //these need to be public to be set from the interruption handler
    static bool saveAligAndExitOnInterruption;
    static bool saveAligAndContOnInterruption;
private:

    string outputFileName;
    string localScoresFileName;

    Alignment getStartingAlignment();

    /* NODE COLOR SYSTEM
    We define the number of neighbors of a color as the number of change and swap neighbors
    of an alignment involving only nodes of that color. We say a color is "active" if it has at
    least one neighbor. Equivalently, a color is active if it appears at least once in G1 and
    at least twice in G2. An example of non-active color is a color used to "lock" two nodes together.

    The goal of the color system is to be able to choose a random neighbor of an alignment
    uniformly at random and efficiently. For this, inactive colors don't need to be considered

    colors that are active receive a new "active color" id, which is an index from 0 to the
    number of active color ids -1. This id is not the same as the id of these 
    colors in either G1 or G2, so colors normally have 3 different ids used as 
    indices in different data structures. SANA's data structures below operate on active color ids */
    uint numActiveColors() const;

    // The mechanism for choosing a neighbor of an alignment uniformly at random is done in 4 steps:
    // 1. an active color is chosen randomly weighted by their number of neighbors
    uint randActiveColorIdWeightedByNumNbrs(); //not const because of RNG

    /* Data structure to implement step 1. index i contains the accumulated probability of choosing
    any of the active colors with active color id <= i. The last value is 1 (by definition).
    Storing the probs like this breaks the range [0,1] into segments for each active color such that the
    length of segment i equals the probability of chosing active color i. To choose a color randomly,
    a random real is taken from [0, 1] and the color corresponding to the segment containing it is returned */
    vector<double> actColToAccumProbCutpoint; 

    //2. an operation between change and swap is chosen randomly weighted by their number of neighbors
    vector<double> actColToChangeProb;

    //3. the source node (or pair of source nodes, for a swap) are chosen randomly from G1 among the
    //nodes of the chosen color
    uint randomG1NodeWithActiveColor(uint actColId) const;
    vector<uint> actColToG1ColId; //to implement step 3.

    //4. same with target nodes
    vector<vector<uint>> actColToUnassignedG2Nodes;        

    //data structure used to initialize actColToUnassignedG2Nodes
    vector<uint> g2NodeToActColId;
    static uint INVALID_ACTIVE_COLOR_ID; //arbitrary value bigger than any valid active color id
 


    friend class Ameur; //it needs to read the PBad buffer
    friend class StatisticalTest;
};

#endif /* SANA_HPP */
