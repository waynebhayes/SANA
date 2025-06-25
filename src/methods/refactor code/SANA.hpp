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
#include <limits>
#if LIBWAYNE
#include "../utils/Misc.hpp"
#include "../utils/Stats.hpp"
#include "../utils/CircBuf.hpp"
#endif

using namespace std;


class SANA: public Method {

public:
    /* PUBLIC VARIABLES */

    static bool saveAligAndExitOnInterruption;
    static bool saveAligAndContOnInterruption;

    list<pair<double, double>> ipsList;

    /* PUBLIC FUNCTIONS */

    SANA(const Graph* G1, const Graph* G2, double TInitial, double TDecay, double maxSeconds,
         long long maxIterations, double tolerance, bool addHillClimbing, MeasureCombination* MC,
         const string& scoreAggrStr, const Alignment& optionalStartAlig,
         const string& outputFileName, const string& localScoresFileName);
    ~SANA();

    Alignment run();
    Alignment runUsingIterations(); // TODO
    Alignment runUsingConfidenceIntervals();
    void describeParameters(ostream& stream) const;
    string fileNameSuffix(const Alignment& A) const;

    void setTInitial(double t);
    void setTFinal(double t);

    // Requires TInitial and TFinal to be already initialized
    void setTDecayFromTempRange();

    double getEquilibriumPBadAtTemp(double temp, double maxTimeInS = 1.0, int logLevel = 1); // 0 for no output, 2 for verbose

    //Defining infinity for f_beta
    double inf = std::numeric_limits<double>::infinity();

    /* TODO: PUBLIC FUNCTIONS */

    void setDynamicTDecay(); // Set temperature decay dynamically
    void setMultiOnly(); // I have no idea what this is.

private:

    /* PRIVATE VARIABLES */

    // Runtime Settings //
    bool yesHillClimbing; // (Private) Do we add hill climbing to the end of the solution?
    bool yesTrackProgress = true; // (Private) Do we give progress reports?
    bool constantTemp = false; // (Private)
    string outputFileName; // (Private) Filename and path for output.
    string localScoresFileName; // (Private) ???
    Alignment startAlignment;

    // Graph information
    uint n1; // (Private) Number of nodes in G1.
    uint n2; // (Private) Number of nodes in G2.
    uint m1; // (Private) Number of edges in G1.
    uint m2; // (Private) Number of edges in G2.

    // Temperature Variables
    double TInitial; // (Private) Initial temperature
    double TDecay; // (Private) Temperature decay
    double TFinal; // (Private) Final temperature
    double tolerance; // Tolerance for confidence intervals.



    /* PRIVATE FUNCTIONS */





    /* PRIVATE NESTED CLASSES */

    class Drone {
    public:
        Drone();
    private:
        Graph* G1;
        Graph* G2;
    };

    class Queen {
    public:
        friend class Drone;

        vector<Drone> daughters; // (Public) Like a bee, the SANA-Queen has a number of busy bees

        Queen(vector<Drone> daughters, mutex, );
        ~Queen();

        array<double, 2>

    private:
        bool single

    };




    /* TODO: PRIVATE FUNCTIONS */


};

#endif /* SANA_HPP */
