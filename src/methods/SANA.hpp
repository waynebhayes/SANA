#ifndef SANA_HPP
#define SANA_HPP
#include "Method.hpp"
#include <map>
#include <tuple>
#include <random>
#include "../measures/localMeasures/LocalMeasure.hpp"
#include "../measures/Measure.hpp"
#include "../measures/MeasureCombination.hpp"
#include "../utils/randomSeed.hpp"
#include "../utils/ParetoFront.hpp"
#include "../measures/ExternalWeightedEdgeConservation.hpp"

class SANA: public Method {

public:
    SANA(Graph* G1, Graph* G2,
      double TInitial, double TDecay, double t, bool usingIterations, bool addHillClimbing, MeasureCombination* MC, string& objectiveScore
#ifdef WEIGHTED
	, string& startAligName
#endif
    );
    ~SANA(){}

    Alignment run();
    unordered_set<vector<ushort>*>* paretoRun();
    void describeParameters(ostream& stream);
    string fileNameSuffix(const Alignment& A);

    void enableRestartScheme(double minutesNewAlignments, uint iterationsPerStep,
        uint numCandidates, double minutesPerCandidate, double minutesFinalist);

    //set temperature schedule automatically
    void searchTemperaturesByLinearRegression();
    void searchTemperaturesByStatisticalTest();
    void setTDecayAutomatically();
    //to compute TDecay automatically
    //returns a value of lambda such that with this TInitial, temperature reaches
    //0 after a certain number of minutes
    double solveTDecay();

    void setAcceptableTInitial();
    void setAcceptableTFinal();
    void setAcceptableTFinalFromManualTInitial();
    double findAcceptableTInitial(double temperature);
    double findAcceptableTFinal(double temperature);
    double findAcceptableTFinalFromManualTInitial(double temperature);

    //set temperature decay dynamically
    void setDynamicTDecay();

    double simpleSearchTInitial();

    double elapsedEstimate = 0;
    int order = 0;

    //returns the number of iterations until it stagnates when not using temperture
    void hillClimbingIterations(long long int iterTarget);
    Alignment hillClimbingAlignment(Alignment startAlignment, long long int idleCountTarget);
    Alignment hillClimbingAlignment(long long int idleCountTarget);

    //returns an approximation of the the logarithm in base e of the size of the search space
    double searchSpaceSizeLog();
    string startAligName = "";
    void prune(string& startAligName);
    vector<vector<ulong>> getCoreFreq();
    vector<ulong> getCoreCount();

private:
    int maxTriangles = 0;

    //Temperature Boundaries. Use these after the tinitial has been determined
    double lowerTBound = 0;
    double upperTBound = 0;

    //data structures for the networks
    uint n1;
    uint n2;
    double g1Edges; //stored as double because it appears in division
#ifdef WEIGHTED
    double g1WeightedEdges;
    double g2WeightedEdges;
#endif
    double g2Edges; //stored as double because it appears in division
#ifdef WEIGHTED
    vector<vector<ushort> > G1AdjMatrix;
    vector<vector<ushort> > G2AdjMatrix;
#else
    vector<vector<bool> > G1AdjMatrix;
    vector<vector<bool> > G2AdjMatrix;
#endif
    vector<vector<ushort> > G1AdjLists;
    vector<vector<ushort> > G2AdjLists;

    void initTau(void);
    vector<ushort> unLockedNodesG1;
    bool nodesHaveType = false;
    //random number generation
    mt19937 gen;
    uniform_int_distribution<> G1RandomNode;
    uniform_int_distribution<> G1RandomUnlockedNodeDist;
    uniform_int_distribution<> G2RandomUnassignedNode;
    uniform_real_distribution<> randomReal;
    uniform_int_distribution<> G1RandomUnlockedGeneDist;
    uniform_int_distribution<> G1RandomUnlockedmiRNADist;
    // uniform_int_distribution<> G2RandomUnlockedGeneDist;
    // uniform_int_distribution<> G2RandomUnlockedmiRNADist;


    ushort G1RandomUnlockedNode();
    ushort G1RandomUnlockedNode(uint source1); // used in nodes-have-type because
    ushort G1RandomUnlockedNode_Fast();
    ushort G2RandomUnlockedNode(uint target1);
    ushort G2RandomUnlockedNode_Fast();

    //temperature schedule
    double TInitial;
    double TFinal;
    double TDecay;
    double minutes = 0;
    bool usingIterations;
    uint maxIterations = 0;
    uint iterationsPerformed = 0;
    const double TInitialScaling = 1;
    const double TDecayScaling = 1;
    //to compute TDecay dynamically
    //vector holds "ideal" temperature values at certain execution times
    bool dynamic_tdecay;
    vector<double> tau;
    double SANAtime;

    double T;
    double temperatureFunction(long long int iter, double TInitial, double TDecay);
    double acceptingProbability(double energyInc, double T);
    double trueAcceptingProbability();
    //to compute TInitial automatically
    //returns a value of TInitial such that the temperature is random
    double scoreForTInitial(double TInitial);
    bool isRandomTInitial(double TInitial, double highThresholdScore, double lowThresholdScore);
    double scoreRandom();
    //to compute TDecay automatically
    //returns a value of lambda such that with this TInitial, temperature reaches
    //0 after a certain number of minutes
    double searchTDecay(double TInitial, double minutes);
    double searchTDecay(double TInitial, uint iterations);

    bool initializedIterPerSecond;
    double iterPerSecond;
    double getIterPerSecond();
    void initIterPerSecond();

    vector<double> energyIncSample(double temp = 0.0);
    double expectedNumAccEInc(double temp, const vector<double>& energyIncSample);

    //data structures for the solution space search
    double changeProbability[2];
    vector<bool> *assignedNodesG2;
    vector<ushort> *unassignedNodesG2;
    vector<ushort>* A;
    //initializes all the necessary data structures for a new run
    void initDataStructures(const Alignment& startA);

    bool addHillClimbing; //for post-run hill climbing

    //objective function
    MeasureCombination* MC;
    double eval(const Alignment& A);
    bool scoreComparison(double newAligEdges, double newInducedEdges, double newTCSum, double newLocalScoreSum, double newWecSum, double newNcSum, double& newCurrentScore, double newEwecSum, double newSquaredAligEdges);
    double ecWeight;
    double s3Weight;
    double icsWeight;
    double wecWeight;
    double secWeight;
    double ncWeight;
    double localWeight;
    double mecWeight;
    double sesWeight;
    double ewecWeight;
    double TCWeight;

    enum class Score{sum, product, inverse, max, min, maxFactor, pareto};
    Score score;

    //For pareto mode
    int paretoInitial;
    int paretoCapacity;

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
    int aligEdgesIncChangeOp(ushort source, ushort oldTarget, ushort newTarget);
    int aligEdgesIncSwapOp(ushort source1, ushort source2, ushort target1, ushort target2);

    // to evaluate SES incrementally
    bool needSquaredAligEdges;
    int squaredAligEdges;
    int squaredAligEdgesIncChangeOp(ushort source, ushort oldTarget, ushort newTarget);
    int squaredAligEdgesIncSwapOp(ushort source1, ushort source2, ushort target1, ushort target2);

    //to evaluate EC incrementally
    bool needSec;
    double secSum;

    //to evaluate S3 incrementally
    bool needInducedEdges;
    int inducedEdges;
    int inducedEdgesIncChangeOp(ushort source, ushort oldTarget, ushort newTarget);

    bool needTC;
    double TCSum;
    double TCIncChangeOp(ushort source, ushort oldTarget, ushort newTarget);
    double TCIncSwapOp(ushort source1, ushort source2, ushort target1, ushort target2);

    //to evaluate nc incrementally
    bool needNC;
    int ncSum;
    vector<ushort> trueA;
    int ncIncChangeOp(ushort source, ushort oldTarget, ushort newTarget);
    int ncIncSwapOp(ushort source1, ushort source2, ushort target1, ushort target2);

    //to evaluate wec incrementally
    bool needWec;
    double wecSum;
    vector<vector<float> > wecSims;
    double WECIncChangeOp(ushort source, ushort oldTarget, ushort newTarget);
    double WECIncSwapOp(ushort source1, ushort source2, ushort target1, ushort target2);

    //to evaluate ewec incrementally
    bool needEwec;
    ExternalWeightedEdgeConservation* ewec;
    double ewecSum;
    double EWECIncChangeOp(ushort source, ushort oldTarget, ushort newTarget);
    double EWECIncSwapOp(ushort source1, ushort source2, ushort target1, ushort target2);
    double EWECSimCombo(ushort source, ushort target);

    //to evaluate local measures incrementally
    bool needLocal;
    double localScoreSum;
    map<string, double>* localScoreSumMap = new map<string, double>;
    vector<vector<float> > sims;
    vector<vector<ulong> > coreFreq;
    vector<ulong> coreCount; // number of times this node in g1 was sampled.
    vector<vector<double> > weightedCoreFreq; // weighted by pBad below
    vector<double> totalCoreWeight; // sum of all pBads, for each node in G1.
    map<string, vector<vector<float> > > localSimMatrixMap;
    double localScoreSumIncChangeOp(vector<vector<float> > const & sim, ushort const & source, ushort const & oldTarget, ushort const & newTarget);
    double localScoreSumIncSwapOp(vector<vector<float> > const & sim, ushort const & source1, ushort const & source2, ushort const & target1, ushort const & target2);



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
    unordered_set<vector<ushort>*>* simpleParetoRun(const Alignment& A, double maxExecutionSeconds,
        long long int& iter);
    unordered_set<vector<ushort>*>* simpleParetoRun(const Alignment& A, long long int maxExecutionIterations,
        long long int& iter);

    double currentScore;
    double previousScore;
    double energyInc;
    vector<double> sampledProbability;
    void SANAIteration();
    void performChange(int type);
    void performSwap(int type);


    //others
    Timer timer;
    void setInterruptSignal(); //allows the execution to be paused with Control+C

    // Used to support locking
    Alignment getStartingAlignment();
    bool implementsLocking(){ return true; }

    double pForTInitial(double TInitial);
    double getPforTInitial(const Alignment& startA, double maxExecutionSeconds,
        long long int& iter);
    string getFolder();
    string haveFolder();
    string mkdir(const std::string& file);
    tuple<int, double, int, double, double, double> regress(double start, double end, int amount);


    //Mostly for pareto front, to hold multiple alignments and scores
    unordered_map<string, int> mapScoresToIndexes(vector<string> &measureNames);
    void prepareMeasureDataByAlignment();
    void insertCurrentAndPrepareNewMeasureDataByAlignment();
    void removeAlignmentData(vector<ushort>* toRemove);
    int numOfMeasures;
    ParetoFront paretoFront;
    vector<ushort>* newA;
    unordered_map<string, int> scoreNamesToIndexes;
    unordered_set<vector<ushort>*>* storedAlignments = new unordered_set<vector<ushort>*>;
    unordered_map<vector<ushort>*, vector<bool>*> storedAssignedNodesG2;
    unordered_map<vector<ushort>*, vector<ushort>*> storedUnassignedNodesG2;
    unordered_map<vector<ushort>*, int> storedAligEdges;
    unordered_map<vector<ushort>*, int> storedSquaredAligEdges;
    unordered_map<vector<ushort>*, int> storedInducedEdges;
    unordered_map<vector<ushort>*, double> storedLocalScoreSum;
    unordered_map<vector<ushort>*, double> storedWecSum;
    unordered_map<vector<ushort>*, double> storedEwecSum;
    unordered_map<vector<ushort>*, int> storedNcSum;
    unordered_map<vector<ushort>*, double> storedTCSum;
    unordered_map<vector<ushort>*, double> storedCurrentScore;
    unordered_map<vector<ushort>*, map<string, double>*> storedLocalScoreSumMap;

};

#endif
