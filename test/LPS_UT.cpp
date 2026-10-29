// test/LPS_UT.cpp -- unit tests for the LPS measures (LPS_L, LPS_I, LPS_S).
//
// Run with:   make test tg=LPS_UT        (or `make test_all`)
// Requires:   src/measures/LPSMeasure.cpp listed in MEASURES_SRCS in the Makefile.

#include "gtest/gtest.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <numeric>
#include <random>
#include <stdexcept>
#include <string>
#include <vector>

#include "../src/Graph.hpp"
#include "../src/Alignment.hpp"
#include "../src/measures/LPSMeasure.hpp"

using namespace std;

// Everything below is in an anonymous namespace so that `make test_all`, which links
// all *_UT.cpp files into one binary, cannot hit duplicate-symbol errors.
namespace {

const double EPS = 1e-9;

typedef array<uint, 2> Edge;
typedef vector<Edge> EdgeList;
typedef vector<uint> Mapping;

Edge e(uint a, uint b) { Edge x; x[0] = a; x[1] = b; return x; }

// Graph with nodes "0".."n-1" (explicit names so isolated nodes are kept).
Graph makeGraph(uint n, const EdgeList& edges, bool directed = false) {
    vector<string> names;
    for (uint i = 0; i < n; i++) names.push_back(to_string(i));
    return Graph(directed, "g", "", edges, names, {}, {});
}

double score(const Graph& G1, const Graph& G2, const Mapping& a, const string& mode) {
    LPS m(&G1, &G2, mode);
    return m.eval(Alignment(a));
}

// Brute-force confusion counts over every node pair of G1. Independent of
// Alignment::computeNumAlignedEdges and Graph::numEdgesInNodeInducedSubgraph.
struct Counts { double TP, TN, FP, FN, omega, e1, eHat; };

Counts bruteForce(const Graph& G1, const Graph& G2, const Mapping& a) {
    Counts c = {0, 0, 0, 0, 0, 0, 0};
    uint n = G1.getNumNodes();
    for (uint u = 0; u < n; u++)
        for (uint v = u + 1; v < n; v++) {
            bool in1 = G1.hasEdge(u, v);
            bool in2 = G2.hasEdge(a[u], a[v]);
            if (in1 && in2) c.TP++;
            else if (!in1 && !in2) c.TN++;
            else if (in1 && !in2) c.FN++;
            else c.FP++;
        }
    c.omega = c.TP + c.TN + c.FP + c.FN;
    c.e1    = c.TP + c.FN;   // edges of G1
    c.eHat  = c.TP + c.FP;   // edges of G2 induced by the image (a is injective)
    return c;
}

// term = x / (2d), 0 when d == 0 (same convention as LPS::eval)
double term(double x, double d) { return d > 0 ? x / (2.0 * d) : 0.0; }
double oracleL(const Counts& c) { return term(c.TP, c.e1)   + term(c.TN, c.omega - c.e1); }
double oracleI(const Counts& c) { return term(c.TP, c.eHat) + term(c.TN, c.omega - c.eHat); }
double oracleS(const Counts& c) { return 0.5 * (oracleL(c) + oracleI(c)); }

Mapping inverse(const Mapping& a) {
    Mapping inv(a.size());
    for (uint i = 0; i < a.size(); i++) inv[a[i]] = i;
    return inv;
}

EdgeList randomEdges(uint n, double p, mt19937& rng) {
    uniform_real_distribution<double> U(0.0, 1.0);
    EdgeList edges;
    for (uint u = 0; u < n; u++)
        for (uint v = u + 1; v < n; v++)
            if (U(rng) < p) edges.push_back(e(u, v));
    return edges;
}

} // namespace

// ------------------------------------------------------------ hand-computed values

// G1 = path 0-1-2-3, G2 = 4-cycle, identity alignment.
// TP=3 TN=2 FP=1 FN=0, |Omega|=6, |E1|=3, |Ehat|=4
TEST(LPSTest, PathVsCycleHandComputed) {
    Graph G1 = makeGraph(4, EdgeList{e(0,1), e(1,2), e(2,3)});
    Graph G2 = makeGraph(4, EdgeList{e(0,1), e(1,2), e(2,3), e(3,0)});
    Mapping a = {0, 1, 2, 3};

    EXPECT_NEAR(3.0/6 + 2.0/6,           score(G1, G2, a, "l"), EPS);   // 0.83333
    EXPECT_NEAR(3.0/8 + 2.0/4,           score(G1, G2, a, "i"), EPS);   // 0.875
    EXPECT_NEAR(0.5 * (5.0/6 + 7.0/8),   score(G1, G2, a, "s"), EPS);   // 0.854166...
}

TEST(LPSTest, PerfectAlignmentScoresOne) {
    Graph G1 = makeGraph(3, EdgeList{e(0,1), e(1,2)});
    Graph G2 = makeGraph(3, EdgeList{e(0,1), e(1,2)});
    Mapping a = {0, 1, 2};
    EXPECT_NEAR(1.0, score(G1, G2, a, "l"), EPS);
    EXPECT_NEAR(1.0, score(G1, G2, a, "i"), EPS);
    EXPECT_NEAR(1.0, score(G1, G2, a, "s"), EPS);
}

// G1 aligned into a larger G2 whose induced subgraph is isomorphic to G1.
TEST(LPSTest, PerfectAlignmentIntoLargerG2ScoresOne) {
    Graph G1    = makeGraph(3, EdgeList{e(0,1), e(1,2)});
    Graph G2big = makeGraph(5, EdgeList{e(0,1), e(1,2), e(2,3), e(3,4)});
    Mapping b = {1, 2, 3};   // path 1-2-3 inside the path 0-1-2-3-4
    EXPECT_NEAR(1.0, score(G1, G2big, b, "l"), EPS);
    EXPECT_NEAR(1.0, score(G1, G2big, b, "i"), EPS);
    EXPECT_NEAR(1.0, score(G1, G2big, b, "s"), EPS);
}

// Induced G2 subgraph is complete => LPS_I's TN denominator is 0 (0/0 -> term 0).
// G1 = path 0-1-2, G2 = triangle 0-1-2 plus pendant 3.
// TP=2 TN=0 FP=1, |Omega|=3, |E1|=2, |Ehat|=3
TEST(LPSTest, ZeroDenominatorInInducedGraphStaysFinite) {
    Graph G1 = makeGraph(3, EdgeList{e(0,1), e(1,2)});
    Graph G2 = makeGraph(4, EdgeList{e(0,1), e(1,2), e(0,2), e(2,3)});
    Mapping a = {0, 1, 2};

    EXPECT_NEAR(2.0/4,                  score(G1, G2, a, "l"), EPS);   // 0.5
    EXPECT_NEAR(2.0/6,                  score(G1, G2, a, "i"), EPS);   // 0.33333
    EXPECT_NEAR(0.5 * (0.5 + 2.0/6),    score(G1, G2, a, "s"), EPS);   // 0.416666...

    double s = score(G1, G2, a, "s");
    EXPECT_FALSE(std::isnan(s));
    EXPECT_FALSE(std::isinf(s));
}

// LPS_I is well defined for an edgeless G1 (its denominators use |Ehat|, not |E1|).
// G1 = 3 isolated nodes, G2 = path 0-1-2. TP=0 TN=1 FP=2, |Omega|=3, |Ehat|=2
TEST(LPSTest, EdgelessG1IsDefinedForModeI) {
    Graph G1 = makeGraph(3, EdgeList());
    Graph G2 = makeGraph(3, EdgeList{e(0,1), e(1,2)});
    Mapping a = {0, 1, 2};
    EXPECT_NO_THROW(LPS(&G1, &G2, "i"));
    EXPECT_NEAR(0.5, score(G1, G2, a, "i"), EPS);
}

// ------------------------------------------------------------ properties from the paper

// Strict monotonicity. With |V1| == |V2| the induced edge count is fixed, so more
// conserved edges must give a strictly larger score. G1 = G2 = path 0-1-2-3.
//   identity   : TP=3 TN=3 -> 1
//   [1,0,2,3]  : TP=2 TN=2 -> 2/3
//   [0,2,1,3]  : TP=1 TN=1 -> 1/3
TEST(LPSTest, StrictlyMonotonicInConservedStructure) {
    Graph G1 = makeGraph(4, EdgeList{e(0,1), e(1,2), e(2,3)});
    Graph G2 = makeGraph(4, EdgeList{e(0,1), e(1,2), e(2,3)});
    Mapping best = {0, 1, 2, 3}, mid = {1, 0, 2, 3}, worst = {0, 2, 1, 3};

    const char* modes[] = {"l", "i", "s"};
    for (int k = 0; k < 3; k++) {
        SCOPED_TRACE(string("mode ") + modes[k]);
        double sb = score(G1, G2, best, modes[k]);
        double sm = score(G1, G2, mid, modes[k]);
        double sw = score(G1, G2, worst, modes[k]);
        EXPECT_GT(sb, sm);
        EXPECT_GT(sm, sw);
    }
    EXPECT_NEAR(2.0 / 3, score(G1, G2, mid, "s"), EPS);
    EXPECT_NEAR(1.0 / 3, score(G1, G2, worst, "s"), EPS);

    // |E1| == |Ehat| here, so LPS_L and LPS_I coincide.
    EXPECT_NEAR(score(G1, G2, mid, "l"), score(G1, G2, mid, "i"), EPS);
    EXPECT_NEAR(score(G1, G2, worst, "l"), score(G1, G2, worst, "i"), EPS);
}

TEST(LPSTest, SIsArithmeticMeanOfLAndI) {
    Graph G1 = makeGraph(4, EdgeList{e(0,1), e(1,2), e(2,3)});
    Graph G2 = makeGraph(5, EdgeList{e(0,1), e(1,2), e(2,3), e(3,0), e(3,4)});
    Mapping a = {3, 0, 1, 2};
    double L = score(G1, G2, a, "l"), I = score(G1, G2, a, "i"), S = score(G1, G2, a, "s");
    EXPECT_NEAR(0.5 * (L + I), S, EPS);
}

// Symmetry for equal-size bijections:
//   LPS_I(G1,G2,a) == LPS_L(G2,G1,a^-1)   (and vice versa)
//   LPS_S(G1,G2,a) == LPS_S(G2,G1,a^-1)
TEST(LPSTest, SymmetryUnderSwappingNetworks) {
    Graph G1 = makeGraph(4, EdgeList{e(0,1), e(1,2), e(2,3)});
    Graph G2 = makeGraph(4, EdgeList{e(0,1), e(1,2), e(2,3), e(3,0)});
    Mapping a = {2, 0, 3, 1};
    Mapping inv = inverse(a);

    EXPECT_NEAR(score(G1, G2, a, "i"), score(G2, G1, inv, "l"), EPS);
    EXPECT_NEAR(score(G1, G2, a, "l"), score(G2, G1, inv, "i"), EPS);
    EXPECT_NEAR(score(G1, G2, a, "s"), score(G2, G1, inv, "s"), EPS);
}

// ------------------------------------------------------------ constructor guards

TEST(LPSTest, ConstructorRejectsUnsupportedInputs) {
    Graph path3  = makeGraph(3, EdgeList{e(0,1), e(1,2)});
    Graph tri    = makeGraph(3, EdgeList{e(0,1), e(1,2), e(0,2)});
    Graph empty3 = makeGraph(3, EdgeList());
    Graph dir    = makeGraph(3, EdgeList{e(0,1)}, true);
    Graph loopy  = makeGraph(3, EdgeList{e(0,0), e(0,1)});

    EXPECT_THROW(LPS(&path3, &path3, "x"), std::exception);   // bad mode
    EXPECT_THROW(LPS(&dir, &path3, "s"), std::exception);     // directed G1
    EXPECT_THROW(LPS(&path3, &dir, "s"), std::exception);     // directed G2
    EXPECT_THROW(LPS(&loopy, &path3, "s"), std::exception);   // self-loop in G1
    EXPECT_THROW(LPS(&path3, &loopy, "s"), std::exception);   // self-loop in G2
    EXPECT_THROW(LPS(&empty3, &path3, "l"), std::exception);  // edgeless G1: LPS_L undefined
    EXPECT_THROW(LPS(&empty3, &path3, "s"), std::exception);
    EXPECT_THROW(LPS(&tri, &path3, "l"), std::exception);     // complete G1: LPS_L undefined
    EXPECT_THROW(LPS(&tri, &path3, "s"), std::exception);
}

TEST(LPSTest, ConstructorAcceptsValidInputs) {
    Graph path3 = makeGraph(3, EdgeList{e(0,1), e(1,2)});
    EXPECT_NO_THROW(LPS(&path3, &path3, "l"));
    EXPECT_NO_THROW(LPS(&path3, &path3, "i"));
    EXPECT_NO_THROW(LPS(&path3, &path3, "s"));
}

// ------------------------------------------------------------ randomized oracle

// 200 random graph pairs with random injective alignments (including |V1| < |V2|).
// Compares against the brute-force oracle, checks that the API pieces LPS::eval
// relies on agree with brute-force counts, and that scores stay in [0, 1].
TEST(LPSTest, RandomInstancesMatchBruteForceOracle) {
    mt19937 rng(12345);
    uniform_real_distribution<double> U(0.0, 1.0);

    for (int trial = 0; trial < 200; trial++) {
        SCOPED_TRACE("trial " + to_string(trial));

        uint n1 = 3 + rng() % 6;          // 3..8
        uint n2 = n1 + rng() % 4;         // n1..n1+3
        double omega = n1 * (n1 - 1) / 2.0;

        EdgeList E1;
        do { E1 = randomEdges(n1, 0.2 + 0.6 * U(rng), rng); }
        while (E1.empty() || (double)E1.size() == omega);   // LPS_L must be defined
        EdgeList E2 = randomEdges(n2, 0.2 + 0.6 * U(rng), rng);

        Graph G1 = makeGraph(n1, E1), G2 = makeGraph(n2, E2);

        Mapping perm(n2);
        iota(perm.begin(), perm.end(), 0);
        shuffle(perm.begin(), perm.end(), rng);
        Mapping a(perm.begin(), perm.begin() + n1);          // random injective alignment

        Counts c = bruteForce(G1, G2, a);
        Alignment A(a);

        // building blocks used by LPS::eval
        EXPECT_DOUBLE_EQ(omega, c.omega);
        EXPECT_DOUBLE_EQ(c.e1,   (double)G1.getNumEdges());
        EXPECT_DOUBLE_EQ(c.TP,   (double)A.computeNumAlignedEdges(G1, G2));
        EXPECT_DOUBLE_EQ(c.eHat, (double)G2.numEdgesInNodeInducedSubgraph(a));

        double L = score(G1, G2, a, "l");
        double I = score(G1, G2, a, "i");
        double S = score(G1, G2, a, "s");
        EXPECT_NEAR(oracleL(c), L, EPS);
        EXPECT_NEAR(oracleI(c), I, EPS);
        EXPECT_NEAR(oracleS(c), S, EPS);

        EXPECT_GE(L, -EPS); EXPECT_LE(L, 1 + EPS);
        EXPECT_GE(I, -EPS); EXPECT_LE(I, 1 + EPS);
        EXPECT_GE(S, -EPS); EXPECT_LE(S, 1 + EPS);
    }
}