#ifndef SANA_HPP
#define SANA_HPP
#include <map>
#include "Method.hpp"
#include <random>
#include "../measures/localMeasures/LocalMeasure.hpp"
#include "../measures/Measure.hpp"
#include "../measures/MeasureCombination.hpp"
#include "../utils/randomSeed.hpp"
#include "../measures/MultiSymmetricEdgeCoverage.hpp"
#include "../measures/ConservedInteractionQuality.hpp"
class SANA: public Method {

public:
    SANA(Graph* G1, Graph* G2,
        double TInitial, double TDecay, double t, MeasureCombination* MC, string& objectiveScore);
    SANA(Graph* G1, Graph* G2,
        double TInitial, double TDecay, uint i, MeasureCombination* MC, string& objectiveScore);
    SANA(vector<Graph>* GV, Graph* SN,
        double TInitial, double TDecay, double t, MeasureCombination* MC, string& objectiveScore);

    ~SANA();

    Alignment run();
    MultiAlignment runMA();
    MultiAlignment RunAndPrintTime();
    void describeParameters(ostream& stream);
    string fileNameSuffix(const Alignment& A);

    void enableRestartScheme(double minutesNewAlignments, uint iterationsPerStep,
        uint numCandidates, double minutesPerCandidate, double minutesFinalist);

    //set temperature schedule automatically
    void setTemperatureScheduleAutomatically();
    void setTInitialAutomatically();
    void setTDecayAutomatically();

	//set temperature decay dynamically
	void setDynamicTDecay();




    //returns the number of iterations until it stagnates when not using temperture
    long long unsigned int hillClimbingIterations(long long unsigned int idleCountTarget);

    //returns an approximation of the the logarithm in base e of the size of the search space
    double searchSpaceSizeLog();

private:

    //data structures for the networks
    uint n1;
    uint n2;
    double g1Edges; //stored as double because it appears in division
	double g2Edges; //stored as double because it appears in division
    vector<vector<bool> > G1AdjMatrix;
    vector<vector<bool> > G2AdjMatrix;
    vector<vector<ushort> > G1AdjLists;
    vector<vector<ushort> > G2AdjLists;

    //Just for multimode
    vector<Graph>* graphs;
    Graph* shadowNetwork;
    vector<vector<bool> > SNAdjMatrix;
    vector<vector<vector<bool> >> AdjMatrices;
    bool nodesHaveType = false;
    double MAexecTime;
    MultiAlignment getStartingMultiAlignment();
    //multiSANAIteration
    vector<ushort> unLockedNodesG1;
    vector< uniform_int_distribution<>> RandomUnlockedNodeDists;
    vector< uniform_int_distribution<>> SNRandomUnassignedNodeDists;
    void MultiSANAIteration();
    MultiSymmetricEdgeCoverage MSEC;
    ConservedInteractionQuality CIQ;
    vector< vector<ushort>> coverageMatrix;
    vector< vector<ushort>> denominatorMatrix;

    //random number generation
    mt19937 gen;
    uniform_int_distribution<> RandomG1Choice;
    uniform_int_distribution<> G1RandomNode;
    uniform_int_distribution<> G1RandomUnlockedNodeDist;
    uniform_int_distribution<> G2RandomUnassignedNode;
    uniform_real_distribution<> randomReal;
    uniform_int_distribution<> G1RandomUnlockedGeneDist;
    uniform_int_distribution<> G1RandomUnlockedmiRNADist;
    ushort G1RandomUnlockedNode();
    ushort G1RandomUnlockedNode(uint source1); // used in nodes-have-type because
    ushort G1RandomUnlockedNode_Fast();
    ushort G2RandomUnlockedNode(uint target1);
    ushort G2RandomUnlockedNode_Fast();

    //temperature schedule
    double TInitial;
    double TDecay;
    double minutes = 0;
    uint maxIterations = 0;
    long long unsigned int iterationCount = 0;
    const double TInitialScaling = 0.00004;
    const double TDecayScaling = 0.00000001;
	//to compute TDecay dynamically
	//vector holds "ideal" temperature values at certain execution times
	bool dynamic_tdecay;
	vector<double> tau;

    double T;
    double temperatureFunction(double iter, double TInitial, double TDecay);
    double acceptingProbability(double energyInc, double T);
    //to compute TInitial automatically
    //returns a value of TInitial such that the temperature is random
    double searchTInitial();
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

    vector<double> energyIncSample();
    double expectedNumAccEInc(double temp, const vector<double>& energyIncSample);

    //data structures for the solution space search
    double changeProbability;
    vector<bool> assignedNodesG2;
    vector<ushort> unassignedNodesG2;
    vector<ushort> A;
    vector<vector<ushort> > upAlign;
    vector<vector<ushort> > MA;
    vector<vector<bool>> assignedNodesG2Vector;
    vector<vector<ushort>> unassignedNodesG2Vector;
    //initializes all the necessary data structures for a new run
    void initDataStructures(const Alignment& startA);
    void initDataStructures(const MultiAlignment& startA);


    //objective function
    MeasureCombination* MC;
    double eval(const Alignment& A);
    double eval(const MultiAlignment& A);
    double CIQeval(const MultiAlignment& A);
    double MCeval(const MultiAlignment& A);
    bool scoreComparison(double newAligEdges, double newInducedEdges, double newLocalScoreSum, double newWecSum, double& newCurrentScore);
    bool scoreComparison(ushort source,ushort oldTarget,ushort newTarget,ushort randomIndex);
    bool scoreComparison(ushort source1,ushort source2,ushort target1,ushort target2,ushort randomIndex);
    bool CIQscoreComparison(ushort source,ushort oldTarget,ushort newTarget,ushort randomIndex);
    bool CIQscoreComparison(ushort source1,ushort source2,ushort target1,ushort target2,ushort randomIndex);

    double ecWeight;
    double s3Weight;
    double wecWeight;
    double secWeight;
    double localWeight;
    string score;

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

    //to evaluate SEC incrementally
    bool needSec;
    double secSum;

    //to evaluate S3 incrementally
    bool needInducedEdges;
    int inducedEdges;
    int inducedEdgesIncChangeOp(ushort source, ushort oldTarget, ushort newTarget);


    //to evaluate wec incrementally
    bool needWec;
    double wecSum;
    vector<vector<float> > wecSims;
    double WECIncChangeOp(ushort source, ushort oldTarget, ushort newTarget);
    double WECIncSwapOp(ushort source1, ushort source2, ushort target1, ushort target2);

    //to evaluate MSEC incrementally
    bool needMWEC;
    uint graphNodesSum;
    vector<short> randomChoice;
    uniform_int_distribution<> graphSelector;
    uint denominator;
    uint numerator;
    double CIQnumerator;
    double CIQdenominator;
    vector<vector<vector<ushort>>> edgeLists;
    //to evaluate local measures incrementally
    bool needLocal;
    double localScoreSum;
    vector<vector<float> > sims;
    double localScoreSumIncChangeOp(ushort source, ushort oldTarget, ushort newTarget);
    double localScoreSumIncSwapOp(ushort source1, ushort source2, ushort target1, ushort target2);



    //other execution options
    bool constantTemp; //tempertare does not decrease as a function of iteration
    bool enableTrackProgress; //shows output periodically
    void trackProgress(long long unsigned i);
    double avgEnergyInc;


    //algorithm
    Alignment simpleRun(const Alignment& A, double maxExecutionSeconds,
        long long unsigned int& iter);
    MultiAlignment simpleRun(const MultiAlignment& A, double maxExecutionSeconds,
        long long unsigned int& iter);
    Alignment simpleRun(const Alignment& A, long long unsigned int maxExecutionIterations,
        long long unsigned int& iter);
    double currentScore;
    double energyInc;
    void SANAIteration();
    void multiSANAIteration();
    void performChange();
    void performSwap();


    //others
    Timer timer;
    void setInterruptSignal(); //allows the execution to be paused with Control+C

    // Used to support locking
    Alignment getStartingAlignment();
    bool implementsLocking(){ return true; }
};

#endif
