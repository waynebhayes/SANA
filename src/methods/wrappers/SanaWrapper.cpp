#include "SanaWrapper.hpp"

SanaWrapper::SanaWrapper(const Graph *G1, const Graph *G2, double TInitial, double TDecay, double maxSeconds,
        long long maxIterations, double tolerance, bool addHillClimbing, MeasureCombination *MC,
        const string &scoreAggrStr, const Alignment &optionalStartAlig,
        const string &outputFileName, const string &localScoresFileName):
        Method(G1, G2, "SANAWrapper_" + MC->toString()),
        modern(G1, G2, TInitial, TDecay, maxSeconds, maxIterations, tolerance, addHillClimbing, MC,
               scoreAggrStr, optionalStartAlig, outputFileName, localScoresFileName, THREAD_NUMBER) {
}
Alignment SanaWrapper::run() {
    return modern.run();
}

Alignment SanaWrapper::runUsingIterations() {
    return modern.runUsingIterations();
}
Alignment SanaWrapper::runUsingConfidenceIntervals() {
    return modern.runUsingConfidenceIntervals();
}

void SanaWrapper::describeParameters(ostream& stream) const {
    modern.describeParameters(stream);
}

string SanaWrapper::fileNameSuffix(const Alignment& A) const {
    return modern.fileNameSuffix(A);
}

void SanaWrapper::setTInitial(const double t) {
    modern.setTInitial(t);
}

void SanaWrapper::setTFinal(const double t) {
    modern.setTFinal(t);
}

void SanaWrapper::setTDecayFromTempRange() {
    modern.setTDecayFromTempRange();
}
