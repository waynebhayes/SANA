#include "LPSMeasure.hpp"
#include <stdexcept>
using namespace std;

LPS::LPS(const Graph* G1, const Graph* G2, const string& modeStr)
    : Measure(G1, G2, "lps_" + modeStr) {
    if (modeStr != "l" && modeStr != "i" && modeStr != "s")
        throw invalid_argument("LPS mode must be l, i or s");
    mode = modeStr[0];

    if (G1->isDirected() || G2->isDirected())
        throw runtime_error("LPS: directed graphs not supported");
    for (uint i = 0; i < G1->getNumNodes(); i++)
        if (G1->hasSelfLoop(i))
            throw runtime_error("LPS: G1 has self-loops; remove them first");
    for (uint i = 0; i < G2->getNumNodes(); i++)
        if (G2->hasSelfLoop(i))
            throw runtime_error("LPS: G2 has self-loops; remove them first");

    double n = G1->getNumNodes();
    omega = n * (n - 1) / 2.0;
    e1 = G1->getNumEdges();

    // LPS_L and LPS_S are undefined when G1 is edgeless or complete
    if (mode != 'i' && (e1 == 0 || e1 == omega))
        throw runtime_error("LPS_L/LPS_S undefined: G1 is edgeless or complete");
}

LPS::~LPS() {}

// term = x / (2*d), with the convention 0 when d == 0.
static inline double term(double x, double d) { return d == 0 ? 0.0 : x / (2.0 * d); }

double LPS::eval(const Alignment& A) {
    double TP = A.computeNumAlignedEdges(*G1, *G2);
    double eHat = G2->numEdgesInNodeInducedSubgraph(*A.getVector());
    double FP = eHat - TP;
    double TN = omega - e1 - FP;
    double L = term(TP, e1) + term(TN, omega - e1);
    double I = term(TP, eHat) + term(TN, omega - eHat);

    switch (mode) {
        case 'l': return L;
        case 'i': return I;
        default:  return 0.5 * (L + I);
    }
}