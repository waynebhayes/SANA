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
#include "../measures/localMeasures/LocalMeasure.hpp"
#include "../measures/Measure.hpp"
#include "../measures/MeasureCombination.hpp"
#include "../utils/randomSeed.hpp"
#include "../utils/ParetoFront.hpp"
#include "../measures/ExternalWeightedEdgeConservation.hpp"

#define UNWEIGHTED_CORES 1
#ifdef MULTI_PAIRWISE
#define PARAMS int aligEdges, int g1Edges, int inducedEdges, int g2Edges, double TCSum, int localScoreSum, int n1, double wecSum, double jsSum, double ewecSum, int ncSum, unsigned int trueA_back, double g1WeightedEdges, double g2WeightedEdges, int squaredAligEdges, int exposedEdgesNumer, int MS3Numer, double edSum, double erSum, uint pairsCount
#else
#define PARAMS int aligEdges, int g1Edges, int inducedEdges, int g2Edges, double TCSum, int localScoreSum, int n1, double wecSum, double jsSum, double ewecSum, int ncSum, unsigned int trueA_back, double edSum, double erSum, uint pairsCount
#endif

class SANA: public Method {

public:
    SANA(Graph* G1, Graph* G2,
      double TInitial, double TDecay, double t, bool usingIterations, bool addHillClimbing, MeasureCombination* MC, string& objectiveScore
#ifdef MULTI_PAIRWISE
	, string& startAligName
#endif
    );
    ~SANA();

    Alignment run();
    unordered_set<vector<uint>*>* paretoRun(const string &fileName);
    void describeParameters(ostream& stream);
    string fileNameSuffix(const Alignment& A);

    void enableRestartScheme(double minutesNewAlignments, uint iterationsPerStep,
        uint numCandidates, double minutesPerCandidate, double minutesFinalist);
    
    //set temperature decay dynamically
    void setDynamicTDecay();

    double elapsedEstimate = 0;
    int order = 0;

    //returns the number of iterations until it stagnates when not using temperture
    void constantTempIterations(long long int iterTarget);
    Alignment hillClimbingAlignment(Alignment startAlignment, long long int idleCountTarget);
    Alignment hillClimbingAlignment(long long int idleCountTarget);

    //returns an approximation of the the logarithm in base e of the size of the search space
    double logOfSearchSpaceSize();
    
    string startAligName = "";
    void prune(string& startAligName);

    //set the file names passed in args in case we want to store the alignment on the fly
    void setOutputFilenames(string outputFileName, string localMeasuresFileName);

    void setTInitial(double t);
    void setTFinal(double t);

    //requires TInitial and TFinal to be already initialized
    void setTDecayFromTempRange();

    double getPBad(double temp, double maxTime = 1.0, int logLevel = 1); //0 for no output, 2 for verbose
    list<pair<double, double>> ipsList;
private:
    friend class Ameur; //it needs to read the PBad buffer
    friend class StatisticalTest;
    //temperature schedule
    double TInitial;
    double TFinal;
    double TDecay;

    //circular pBad buffer
    const int PBAD_CIRCULAR_BUFFER_SIZE = 10000;
    vector<double> pBadBuffer;
    int numPBadsInBuffer;
    int pBadBufferIndex;
    double pBadBufferSum;
    double trueAcceptingProbability();
    double slowTrueAcceptingProbability();


    int maxTriangles = 0;

    //store whether or not most recent move was bad
    bool wasBadMove = false;

    //data structures for the networks
    uint n1;
    uint n2;
    double g1Edges; //stored as double because it appears in division
    uint pairsCount; // number of combinations of (g1_node_x, g1_node_y) including
                     // a pair that includes the same node (g1_node_x, g1_node_x)
#ifdef MULTI_PAIRWISE
    double g1WeightedEdges;
    double g2WeightedEdges;
#endif
    double g2Edges; //stored as double because it appears in division
    Matrix<MATRIX_UNIT> G1Matrix;
    Matrix<MATRIX_UNIT> G2Matrix;
    vector<vector<uint> > G1AdjLists;
    vector<vector<uint> > G2AdjLists;

    Matrix<float>& G1FloatWeights;
    Matrix<float>& G2FloatWeights;

    void initTau(void);
    vector<uint> unLockedNodesG1;
    int multipartite;
    //random number generation
    mt19937 gen;
    uniform_int_distribution<> G1RandomNode;
    uniform_int_distribution<> G1RandomUnlockedNodeDist;
    uniform_int_distribution<> G2RandomUnassignedNode;
    uniform_real_distribution<> randomReal;
    uniform_int_distribution<> G1RandomUnlockedGeneDist;
    uniform_int_distribution<> G1RandomUnlockedmiRNADist;
    uniform_int_distribution<> G2RandomUnassignedGeneDist;
    uniform_int_distribution<> G2RandomUnassignedmiRNADist;


    uint G1RandomUnlockedNode();
    uint G1RandomUnlockedNode(uint source1); // used in nodes-have-type because
    uint G1RandomUnlockedNode_Fast();
    uint G2RandomUnlockedNode(uint target1);
    uint G2RandomUnlockedNode_Fast();

    double minutes = 0;
    bool usingIterations;
    uint maxIterations = 0;
    uint iterationsPerformed = 0;
    uint oldIterationsPerformed = 0;
    double oldTimeElapsed = 0;

    //to compute TDecay dynamically
    //vector holds "ideal" temperature values at certain execution times
    bool dynamic_tdecay;
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

    //data structures for the solution space search
    double changeProbability[2];
    vector<bool> *assignedNodesG2;
    vector<uint> *unassignedNodesG2;
    vector<uint> *unassignedmiRNAsG2;
    vector<uint> *unassignedgenesG2;
    
    vector<vector<uint>>* unassignedMultipartiteNodes;
    
    vector<uint>* A;
    //initializes all the necessary data structures for a new run
    void initDataStructures(const Alignment& startA);

    bool addHillClimbing; //for post-run hill climbing

    //objective function
    MeasureCombination* MC;
    double eval(const Alignment& A);
    bool scoreComparison(double newAligEdges, double newInducedEdges, double newTCSum, double newLocalScoreSum, double newWecSum, double newJsSum, double newNcSum, double& newCurrentScore, double newEwecSum, double newSquaredAligEdges, double newExposedEdgesNumer, double newMS3Numer, double newEdgeDifferenceSum, double newEdgeRatioSum);
    double ecWeight;
    double edWeight, erWeight;
    double s3Weight;
    double icsWeight;
    double wecWeight;
    double jsWeight;
    double secWeight;
    double ncWeight;
    double localWeight;
    double mecWeight;
    double sesWeight;
    double eeWeight;
    double ms3Weight;
    double ewecWeight;
    double TCWeight;

    enum class Score{sum, product, inverse, max, min, maxFactor, pareto};
    Score score;

    //For pareto mode
    unsigned int paretoInitial;
    unsigned int paretoCapacity;
    unsigned int paretoIterations;
    unsigned int paretoThreads;

    //restart scheme
    bool restart;
    //parameters
    double minutesNewAlignments;
    uint iterationsPerStep;
    uint numCandidates;
    double minutesPerCandidate;
    double minutesFinalist;
    //data structures
    uint newAlignmentsCount;
    vector<Alignment> candidates;
    vector<double> candidatesScores;
    //functions
    Alignment runRestartPhases();
    uint getLowestIndex() const;
    uint getHighestIndex() const;


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

    //to evaluate EC incrementally
    bool needSec;
    double secSum;

    //to evaluate S3 incrementally
    bool needInducedEdges;
    int inducedEdges = -1;    // This variable must be initialized as non-zero since it's passed
                              // to scoreComparison in performSwap as "newInducedEdges" which could
                              // make computation go wrong.
    int inducedEdgesIncChangeOp(uint source, uint oldTarget, uint newTarget);

    bool needTC;
    double TCSum;
    double TCIncChangeOp(uint source, uint oldTarget, uint newTarget);
    double TCIncSwapOp(uint source1, uint source2, uint target1, uint target2);

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
    bool needAlignedByNode;
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
#if UNWEIGHTED_CORES
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


    //algorithm
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
    void performChange(int type);
    void performSwap(int type);


    //others
    Timer timer;
    void setInterruptSignal(); //allows the execution to be paused with Control+C
    void printReport(); //print out reports from inside SANA
    string outputFileName = "sana";
    string localScoresFileName = "sana";

    // Used to support locking
    Alignment getStartingAlignment();
    bool implementsLocking(){ return true; }

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
    vector<double> getMeasureScores(double newAligEdges, double newInducedEdges, double newTCSum,
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
    //vector<bool>* newAN = new vector<bool>(0);
    //vector<uint>* newUAN = new vector<uint>(0);
    //vector<uint>* newUmiRNA = new vector<uint>(0);
    //vector<uint>* newUG = new vector<uint>(0);
    unordered_map<string, int> scoreNamesToIndexes;
    unordered_set<vector<uint>*>* storedAlignments = new unordered_set<vector<uint>*>;
    unordered_map<vector<uint>*, vector<bool>*> storedAssignedNodesG2;
    unordered_map<vector<uint>*, vector<uint>*> storedUnassignedNodesG2;
    unordered_map<vector<uint>*, vector<uint>*> storedUnassignedmiRNAsG2;
    unordered_map<vector<uint>*, vector<uint>*> storedUnassignedgenesG2;
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
    unordered_map<vector<uint>*, double> storedTCSum;
    unordered_map<vector<uint>*, double> storedEdSum;
    unordered_map<vector<uint>*, double> storedErSum;
    unordered_map<vector<uint>*, double> storedCurrentScore;
    unordered_map<vector<uint>*, map<string, double>*> storedLocalScoreSumMap;
    typedef double (*calc)(PARAMS);
    unordered_map<string, calc> measureCalculation;
    unordered_set<string> localScoreNames = { "edgec", "edged", "esim", "go", "graphlet",
                                              "graphletcosine", "graphletlgraal", "importance",
                                              "nodec", "noded", "sequence", "graphletnorm" };


    // Code related with parallel pareto run, these code will be later refactored along with the
    // rest of SANA's code.

    // This construct only contain properties that can't be shared between alignments
    struct AlignmentInfo {
        vector<uint> *A;
        vector<bool> *assignedNodesG2;
        vector<uint> *unassignedNodesG2;
        vector<uint> *unassignedmiRNAsG2;
        vector<uint> *unassignedgenesG2;
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
        double TCSum;
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

    // There is no need to pass a iter to this function. And perhaps to other functions like simpleRun, simpleParetoRun.
    unordered_set<vector<uint>*>* parallelParetoRun(const Alignment& A, long long int maxExecutionIterations,
                                                    const string &fileName);
    unordered_set<vector<uint>*>* parallelParetoRun(const Alignment& A, double maxExecutionSeconds,
                                                    const string &fileName);
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

    void performSwap(Job &job, int type);
    void performChange(Job &job, int type);
    int MS3IncChangeOp(Job &job, uint source, uint oldTarget, uint newTarget);
    int MS3IncSwapOp(Job &job, uint source1, uint source2, uint target1, uint target2);

    uint G1RandomUnlockedNode(Job &job);
    uint G1RandomUnlockedNode(Job &job, uint source1);
    uint G1RandomUnlockedNode_Fast(Job &job);
    uint G2RandomUnlockedNode(Job &job, uint target1);
    uint G2RandomUnlockedNode_Fast(Job &job);

    int aligEdgesIncChangeOp(Job &job, uint source, uint oldTarget, uint newTarget);
    int squaredAligEdgesIncChangeOp(Job &job, uint source, uint oldTarget, uint newTarget);
	int exposedEdgesIncChangeOp(Job &job, uint source, uint oldTarget, uint newTarget);
    int inducedEdgesIncChangeOp(Job &job, uint source, uint oldTarget, uint newTarget);
    double TCIncChangeOp(Job &job, uint source, uint oldTarget, uint newTarget);
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
    double TCIncSwapOp(Job &job, uint source1, uint source2, uint target1, uint target2);
    int squaredAligEdgesIncSwapOp(Job &job, uint source1, uint source2, uint target1, uint target2);
	int exposedEdgesIncSwapOp(Job &job, uint source1, uint source2, uint target1, uint target2);
    double WECIncSwapOp(Job &job, uint source1, uint source2, uint target1, uint target2);
    double JSIncSwapOp(Job &job, uint source1, uint source2, uint target1, uint target2);
    double EWECIncSwapOp(Job &job, uint source1, uint source2, uint target1, uint target2);
    int ncIncSwapOp(Job &job, uint source1, uint source2, uint target1, uint target2);
    double localScoreSumIncSwapOp(Job &job, vector<vector<float> > const & sim, uint const & source1, uint const & source2, uint const & target1, uint const & target2);

    bool scoreComparison(Job &job, double newAligEdges, double newInducedEdges, double newTCSum,
                         double newLocalScoreSum, double newWecSum, double newJssum, double newNcSum, double& newCurrentScore,
                         double newEwecSum, double newSquaredAligEdges, double newExposedEdgesNumer, double newMS3Numer,
			 double newEdgeDifferenceSum, double newEdgeRatioSum);

    vector<double> translateScoresToVector(Job &job);
    double trueAcceptingProbability(Job &job);

    long long int sharedIter = 0;
    long long int iterOfLastTrackProgress = 0;
    mutex getJobMutex;


    chrono::steady_clock::time_point startTime;
    double getElapsedTime();
};

#endif
