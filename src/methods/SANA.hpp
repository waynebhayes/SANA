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
#include "../utils/ParetoFront.hpp"
#include "../measures/ExternalWeightedEdgeConservation.hpp"

using namespace std;

//can this be a static constexpr SANA variable?
#define UNWEIGHTED_CORES

#ifdef MULTI_PAIRWISE
#define PARAMS int aligEdges, int g1Edges, int inducedEdges, int g2Edges, int localScoreSum, int n1, double wecSum, double jsSum, double ewecSum, int ncSum, unsigned int trueA_back, double g1WeightedEdges, double g2WeightedEdges, int squaredAligEdges, int exposedEdgesNumer, int MS3Numer, double edSum, double erSum, uint pairsCount
#else
#define PARAMS int aligEdges, int g1Edges, int inducedEdges, int g2Edges, int localScoreSum, int n1, double wecSum, double jsSum, double ewecSum, int ncSum, unsigned int trueA_back, double edSum, double erSum, uint pairsCount
#endif

class SANA: public Method {

public:
    SANA(Graph* G1, Graph* G2, double TInitial, double TDecay, double t, bool usingIterations, bool addHillClimbing,
      MeasureCombination* MC, const string& objectiveScore, const string& startAligName);
    ~SANA();

    Alignment run();
    unordered_set<vector<uint>*>* paretoRun(const string &fileName);
    void describeParameters(ostream& stream);
    string fileNameSuffix(const Alignment& A) const;
    
    //set temperature decay dynamically
    void setDynamicTDecay();

    double elapsedEstimate = 0;
    int order = 0;

    //returns the number of iterations until it stagnates when not using temperture
    void constantTempIterations(long long int iterTarget);
    Alignment hillClimbingAlignment(Alignment startAlignment, long long int idleCountTarget);
    Alignment hillClimbingAlignment(long long int idleCountTarget);

    //returns an approximation of the logarithm in base e of the size of the search space
    double logOfSearchSpaceSize();
    
    string startAligName = "";

    //set the file names passed in args in case we want to store the alignment on the fly
    void setOutputFilenames(string outputFileName, string localMeasuresFileName);

    void setTInitial(double t);
    void setTFinal(double t);

    //requires TInitial and TFinal to be already initialized
    void setTDecayFromTempRange();

    double getPBad(double temp, double maxTime = 1.0, int logLevel = 1); //0 for no output, 2 for verbose
    list<pair<double, double>> ipsList;

    //interrupt handling stuff
    static bool interrupt;
    static bool saveAlignment;

private:
    friend class Ameur; //it needs to read the PBad buffer
    friend class StatisticalTest;
    //temperature schedule
    double TInitial, TFinal, TDecay;

    //circular pBad buffer
    const int PBAD_CIRCULAR_BUFFER_SIZE = 10000;
    vector<double> pBadBuffer;
    int numPBadsInBuffer;
    int pBadBufferIndex;
    double pBadBufferSum;
    double trueAcceptingProbability();
    double slowTrueAcceptingProbability();

    //store whether or not most recent move was bad
    bool wasBadMove = false;

    //data structures for the networks
    uint n1, n2;
    double g1Edges, g2Edges; //stored as double because they appear in division
    uint pairsCount; // number of combinations of (g1_node_x, g1_node_y) including
                     // a pair that includes the same node (g1_node_x, g1_node_x)
    double g1WeightedEdges, g2WeightedEdges;

    void initTau(void);

    //random number generation
    mt19937 gen;
    uniform_int_distribution<> G1RandomNode;
    uniform_real_distribution<> randomReal;

    double minutes = 0;
    bool usingIterations;
    uint maxIterations = 0;
    uint iterationsPerformed = 0;
    uint oldIterationsPerformed = 0;
    double oldTimeElapsed = 0;

    //to compute TDecay dynamically
    //vector holds "ideal" temperature values at certain execution times
    bool dynamicTDecay;
    vector<double> tau;
    double SANAtime;

    double Temperature;
    double temperatureFunction(long long int iter, double TInitial, double TDecay);
    double acceptingProbability(double energyInc, double Temperature);

    double TrimCoreScores(Matrix<unsigned long>& Freq, vector<unsigned long>& numPegSamples);
    double TrimCoreScores(Matrix<double>& Freq, vector<double>& totalPegWeight);

    bool initializedIterPerSecond;
    double iterPerSecond;
    double getIterPerSecond();
    void initIterPerSecond();

    //data structures allocated in constructor and initialized in initDataStructures,
    //so do not allocate them again in initDataStructures
    vector<bool> *assignedNodesG2;
    vector<vector<uint> > *unassignedG2NodesByColor;
    vector<uint> *A;
        
    //initializes the data structures specific to a starting alignment
    void initDataStructures(const Alignment& startA);

    bool addHillClimbing; //for post-run hill climbing

    //objective function
    MeasureCombination* MC;
    double eval(const Alignment& A) const;
    double ecWeight, edWeight, erWeight, s3Weight, icsWeight, wecWeight, jsWeight, secWeight,
           ncWeight, localWeight, mecWeight, sesWeight, eeWeight, ms3Weight, ewecWeight;


    //this should be refactored so that the return parameter is not the 9th one out of 15
    bool scoreComparison(double newAligEdges, double newInducedEdges,
        double newLocalScoreSum, double newWecSum, 
        double newJsSum, double newNcSum, double& newCurrentScore, 
        double newEwecSum, double newSquaredAligEdges, double newExposedEdgesNumer, 
        double newMS3Numer, double newEdgeDifferenceSum, double newEdgeRatioSum);

    enum class Score{sum, product, inverse, max, min, maxFactor, pareto};
    Score score;

    //For pareto mode
    uint paretoInitial, paretoCapacity, paretoIterations, paretoThreads;
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
    vector<uint> trueA;
    int ncIncChangeOp(uint source, uint oldTarget, uint newTarget);
    int ncIncSwapOp(uint source1, uint source2, uint target1, uint target2);

    //to evaluate wec incrementally
    bool needWec;
    double wecSum;
    vector<vector<float> > wecSims;
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
    map<string, double>* localScoreSumMap;
    vector<vector<float> > sims;

#ifdef CORES
#ifdef UNWEIGHTED_CORES
    Matrix<unsigned long> pegHoleFreq;
    vector<unsigned long> numPegSamples; // number of times this node in g1 was sampled.
#endif
    Matrix<double> weightedPegHoleFreq_pBad; // weighted by 1-pBad
    vector<double> totalWeightedPegWeight_pBad;
    Matrix<double> weightedPegHoleFreq_1mpBad; // weighted by 1-pBad
    vector<double> totalWeightedPegWeight_1mpBad;
#endif

    map<string, vector<vector<float> > > localSimMatrixMap;
    double localScoreSumIncChangeOp(vector<vector<float> > const & sim, uint const & source, uint const & oldTarget, uint const & newTarget);
    double localScoreSumIncSwapOp(vector<vector<float> > const & sim, uint const & source1, uint const & source2, uint const & target1, uint const & target2);

    //other execution options
    bool constantTemp; //tempertare does not decrease as a function of iteration
    bool enableTrackProgress; //shows output periodically
    void trackProgress(long long int i, bool end = false);
    double avgEnergyInc;

    static long long _maxExecutionIterations;

    Alignment simpleRun(const Alignment& A, double maxExecutionSeconds,
        long long int& iter);
    Alignment simpleRun(const Alignment& A, long long int maxExecutionIterations,
        long long int& iter);
    Alignment simpleRun(const Alignment& startA, double maxExecutionSeconds, long long int maxExecutionIterations,
        long long int& iter);
    unordered_set<vector<uint>*>* simpleParetoRun(const Alignment& A, double maxExecutionSeconds,
        long long int& iter, const string &fileName);
    unordered_set<vector<uint>*>* simpleParetoRun(const Alignment& A, long long int maxExecutionIterations,
        long long int& iter, const string &fileName);

    double currentScore;
    double previousScore;
    double energyInc;
    void SANAIteration();
    void performChange(uint colorId);
    void performSwap(uint colorId);

    Timer timer;

    //Control+C during execution offers options
    //exit, save alignment and exit, save alignment and continue
    static void setInterruptSignal(); 

    void printReport(); //print out reports from inside SANA
    string outputFileName = "sana";
    string localScoresFileName = "sana";

    Alignment getStartingAlignment();

    string getFolder();
    string haveFolder();
    string mkdir(const std::string& file);
    tuple<int, double, int, double, double, double> regress(double start, double end, int amount);

    //Mostly for pareto front, to hold multiple alignments and scores
    unordered_map<string, int> mapScoresToIndexes();
    void prepareMeasureDataByAlignment();
    void insertCurrentAndPrepareNewMeasureDataByAlignment(vector<double> &addScores);
    vector<double> translateScoresToVector();
    void insertCurrentAlignmentAndData();
    void removeAlignmentData(vector<uint>* toRemove);
    void initializeParetoFront();
    vector<double> getMeasureScores(double newAligEdges, double newInducedEdges,
         double newLocalScoreSum, double newWecSum, double newJsSum, double newNcSum,
         double newEwecSum, double newSquaredAligEdges, double newExposedEdgesNumer,
	     double newMS3Numer, double newEdSum, double newErSum);
    bool dominates(vector<double> &left, vector<double> &right);
    void printParetoFront(const string &fileName);
    void deallocateParetoData();
    uint numOfMeasures;
    vector<string> measureNames;
    int currentMeasure;
    vector<double> currentScores;
    ParetoFront paretoFront;
    unordered_map<string, int> scoreNamesToIndexes;
    unordered_set<vector<uint>*>* storedAlignments;
    unordered_map<vector<uint>*, vector<bool>*> storedAssignedNodesG2;
    unordered_map<vector<uint>*, vector<vector<uint> >*> storedUnassignedG2NodesByColor;
    unordered_map<vector<uint>*, map<string, double>*> storedLocalScoreSumMap;
    unordered_map<vector<uint>*, int> storedAligEdges;
    unordered_map<vector<uint>*, int> storedSquaredAligEdges;
    unordered_map<vector<uint>*, int> storedExposedEdgesNumer;
    unordered_map<vector<uint>*, int> storedMS3Numer;
    unordered_map<vector<uint>*, int> storedInducedEdges;
    unordered_map<vector<uint>*, double> storedLocalScoreSum;
    unordered_map<vector<uint>*, double> storedWecSum;
    unordered_map<vector<uint>*, double> storedJsSum;
    unordered_map<vector<uint>*, double> storedEwecSum;
    unordered_map<vector<uint>*, int> storedNcSum;
    unordered_map<vector<uint>*, double> storedEdSum;
    unordered_map<vector<uint>*, double> storedErSum;
    unordered_map<vector<uint>*, double> storedCurrentScore;
    typedef double (*calc)(PARAMS);
    unordered_map<string, calc> measureCalculation;
    unordered_set<string> localScoreNames = {
        "edgec", "edged", "esim", "go", "graphlet",
        "graphletcosine", "graphletlgraal", "importance",
        "nodec", "noded", "sequence", "graphletnorm" };


    // Code related with parallel pareto run, these code will be later refactored along with the
    // rest of SANA's code.

    // This construct only contain properties that can't be shared between alignments
    struct AlignmentInfo {
        vector<uint> *A;
        vector<bool> *assignedNodesG2;
        vector<vector<uint> > *unassignedG2NodesByColor;
        map<string, double> *localScoreSumMap;
        int aligEdges;
        int squaredAligEdges;
	int exposedEdgesNumer;
        int MS3Numer;
        int inducedEdges;
        double wecSum;
        double jsSum;
        double ewecSum;
        double ncSum;
        double edSum, erSum;
        int localScoreSum;
        double currentScore;
        vector<double> currentScores;
        int currentMeasure;
    };

    struct Job {
        uint id;     // A job's ID equals to its index in the jobs vector.
                     // Given an id, you can access a job by jobs[id].
        AlignmentInfo info;

        long long int iterationsPerformed;
        double energyInc;
        double Temperature;
		vector<double> sampledProbability;

        // mt19937 is a random generator that is implemented with mutex.
        // Which means each thread should have an unique random generator.
        mt19937 gen;
    };
    vector<Job> jobs;
    void initializeJobs();

    const uint insertionsPerStepOfEachThread = 5;

    // There is no need to pass a iter to this function.
    //And perhaps to other functions like simpleRun, simpleParetoRun.
    unordered_set<vector<uint>*>* parallelParetoRun(const Alignment& A,
        long long int maxExecutionIterations, const string &fileName);
    unordered_set<vector<uint>*>* parallelParetoRun(const Alignment& A,
        double maxExecutionSeconds, const string &fileName);
    void startParallelParetoRunByIteration(Job &job, long long int maxExecutionIterations);
    void startParallelParetoRunByTime(Job &job, double maxExecutionSeconds);
    void trackProgress(Job &job);
    void parallelParetoSANAIteration(Job &job);
    void attemptInsertAlignment(Job &job);
    void assignRandomAlignment(Job &job);
    void copyAlignmentFromStorage(Job &job, vector<uint> *A);
    void storeAlignment(Job &job);
    void releaseAlignment(Job &job);
    void releaseAlignment();

    void performSwap(Job &job, uint colorId);
    void performChange(Job &job, uint colorId);
    int MS3IncChangeOp(Job &job, uint source, uint oldTarget, uint newTarget);
    int MS3IncSwapOp(Job &job, uint source1, uint source2, uint target1, uint target2);

    bool scoreComparison(Job &job, double newAligEdges, double newInducedEdges,
        double newLocalScoreSum, double newWecSum,
        double newJssum, double newNcSum, double& newCurrentScore,
        double newEwecSum, double newSquaredAligEdges, double newExposedEdgesNumer,
        double newMS3Numer, double newEdgeDifferenceSum, double newEdgeRatioSum);

    int aligEdgesIncChangeOp(Job &job, uint source, uint oldTarget, uint newTarget);
    int squaredAligEdgesIncChangeOp(Job &job, uint source, uint oldTarget, uint newTarget);
	int exposedEdgesIncChangeOp(Job &job, uint source, uint oldTarget, uint newTarget);
    int inducedEdgesIncChangeOp(Job &job, uint source, uint oldTarget, uint newTarget);
    double localScoreSumIncChangeOp(Job &job, vector<vector<float> > const & sim, uint const & source, uint const & oldTarget, uint const & newTarget);
    double WECIncChangeOp(Job &job, uint source, uint oldTarget, uint newTarget);
    double JSIncChangeOp(Job &job, uint source, uint oldTarget, uint newTarget);
    double EWECSimCombo(Job &job, uint source, uint target);
    double EWECIncChangeOp(Job &job, uint source, uint oldTarget, uint newTarget);
    int ncIncChangeOp(Job &job, uint source, uint oldTarget, uint newTarget);
    double edgeDifferenceIncChangeOp(Job &job, uint source, uint oldTarget, uint newTarget);
    double edgeDifferenceIncSwapOp(Job &job, uint source1, uint source2, uint target1, uint target2);
    double edgeRatioIncChangeOp(Job &job, uint source, uint oldTarget, uint newTarget);
    double edgeRatioIncSwapOp(Job &job, uint source1, uint source2, uint target1, uint target2);

    int aligEdgesIncSwapOp(Job &job, uint source1, uint source2, uint target1, uint target2);
    int squaredAligEdgesIncSwapOp(Job &job, uint source1, uint source2, uint target1, uint target2);
	int exposedEdgesIncSwapOp(Job &job, uint source1, uint source2, uint target1, uint target2);
    double WECIncSwapOp(Job &job, uint source1, uint source2, uint target1, uint target2);
    double JSIncSwapOp(Job &job, uint source1, uint source2, uint target1, uint target2);
    double EWECIncSwapOp(Job &job, uint source1, uint source2, uint target1, uint target2);
    int ncIncSwapOp(Job &job, uint source1, uint source2, uint target1, uint target2);
    double localScoreSumIncSwapOp(Job &job, vector<vector<float> > const & sim, uint const & source1, uint const & source2, uint const & target1, uint const & target2);

    vector<double> translateScoresToVector(Job &job);
    double trueAcceptingProbability(Job &job);

    long long int sharedIter = 0;
    long long int iterOfLastTrackProgress = 0;
    mutex getJobMutex;

    chrono::steady_clock::time_point startTime;
    double getElapsedTime();


    // ***************************************************
    // NODE COLOR SYSTEM
    // ***************************************************
    /*mechanism for choosing a neighbor of an alignment uniformly at random
    this is done in steps:
    1. an "active" color is chosen randomly weighted by their "ramification"
    (number of change and swap neighbors using only nodes of that color)
    we say a color is "active" if it has at least one neighbor
    that is: it appears at least once in G1 and at least twice in G2
    in particular, "locked" pairs of nodes have inactive colors
    non-active colors do not need to be considered for choosing neighbors
    2. an operation between change and swap is chosen randomly weighted by their "ramification"
    (number of neighbors of that kind using only nodes of the chosen color)
    3. the source node(s) is (are) chosen randomly from G1 among the nodes of the chosen color  
    4. same with target node(s) */
 
    //ids *in G1* of the active colors
    vector<uint> activeColorIds;

    //position i contains the change probability for color with id activeColorIds[i]
    //a number between 0 and 1, depending on the number of swap and change neighbors
    //of that color
    vector<double> changeProbByColor;

    //position i contains the probability of choosing any of the colors with ids
    //activeColorIds[0]..activeColorIds[i]
    //storing the probs like this breaks the range [0,1] into segments for each
    //active color s.t. the length of each segment equals the probability of
    //chosing that color
    vector<double> colorAccumProbCutpoints; 

    //this allows us to implement the following efficiently:
    uint randColorWeightedByNumNbrs();

    //data structure used to initialize unassignedG2NodesByColor
    vector<uint> g2NodeToActiveColorId;
    static uint INVALID_ACTIVE_ID; //arbitrary value bigger than any valid G1 color id

};

#endif /* SANA_HPP */
