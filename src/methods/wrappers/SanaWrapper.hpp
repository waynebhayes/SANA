#ifndef SANAWRAPPER_HPP
#define SANAWRAPPER_HPP
#include "../Method.hpp"
#include "../../measures/Measure.hpp"
#include "../../measures/MeasureCombination.hpp"
#include "../../utils/Misc.hpp"
#include "../SANA.hpp"
#include "../SANAThree.hpp"

using namespace std;

#ifdef THREADS
#define THREAD_NUMBER atoi(getenv("SANA_THREADS"))
#else
#define THREAD_NUMBER 1
#endif

class SanaWrapper: public Method {

public:
    SanaWrapper(const Graph *G1, const Graph *G2, double TInitial, double TDecay, double maxSeconds,
        long long maxIterations, double tolerance, bool addHillClimbing, MeasureCombination *MC,
        const string &scoreAggrStr, const Alignment &optionalStartAlig,
        const string &outputFileName, const string &localScoresFileName);

    ~SanaWrapper() override = default;

    Alignment run() override;
    Alignment runUsingIterations();
    Alignment runUsingConfidenceIntervals();

    void describeParameters(ostream& stream) const override;
    string fileNameSuffix(const Alignment& A) const override;

    static void setDynamicTDecay() {featureNotSupported("DynamicTDecay");}
    static void setMultiOnly() {featureNotSupported("multiple_iteration_only");}

    void setTInitial(double t);

    void setTFinal(double t);

    void setTDecayFromTempRange();

    double getEquilibriumPBadAtTemp(double temp, double maxTimeInS = 1.0, int logLevel = 1) {
        return modern.getEquilibriumPBadAtTemp(temp, maxTimeInS);
    }

private:
    SANAThree modern;

    static void featureNotSupported(const string& name) {
        cout << "At this stage, SANA 3.0 does not support " << name << "." << endl;
    }

    friend class GoldilocksMethod;
};


#endif //SANAWRAPPER_HPP
