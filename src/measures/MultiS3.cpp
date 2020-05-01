#include "MultiS3.hpp"
#include "EdgeExposure.hpp"
#include <cmath>

uint NUM_GRAPHS;
uint MultiS3::denom = 1;
uint MultiS3::numer = 1;
double MultiS3::_type = 0;
vector<uint> MultiS3::totalDegrees;

MultiS3::MultiS3(const Graph* G1, const Graph* G2, int type) : Measure(G1, G2, "ms3") {
#ifdef MULTI_PAIRWISE
    extern char *getetv(char*);
    char *s = getenv((char*)"NUM_GRAPHS");
    if (s) {
        assert(1 == sscanf(s, "%u",&NUM_GRAPHS));
    }
    else {
        cerr << "Warning: NUM_GRAPHS should be an environment variable; setting to 2 for now\n";
        NUM_GRAPHS = 2;
    }
    _type=0;//default
    _type = type;
    if (type==1) cout<<"Multi S3: denom = ee"<<endl;
    else if (type==0) cout<<"Multi S3: denom = default"<<endl;
    cout << "Multi S3: NUM_GRAPHS = " << NUM_GRAPHS << endl;
    degreesInit = false;
#endif
}

MultiS3::~MultiS3() {}

void MultiS3::setDenom(const Alignment& A) {
    denom = 0;
    const uint n = G1->getNumNodes();
    for (uint i = 0; i < n; i++) {
        for (uint j = i+1; j < n; j++) {
            if (totalDegrees[i] > 0) denom++;
        }
    }
    denom /= 2;
}

uint MultiS3::computeNumer(const Alignment& A) const {
#ifdef MULTI_PAIRWISE
    uint res = 0;
    const vector<array<uint, 2>> G1Edges = G1->getEdgeList();
    for (const auto& edge: *G1Edges) {
        uint node1 = edge[0], node2 = edge[1];
        auto weight = G2->edgeWeight(A[node1], A[node2]);
        if (MultiS3::_type==1) {
            if (weight >= 1) res += weight + 1; // +1 because G1 was pruned out of G2
        } else if (MultiS3::_type==0) {
            res += weight + 1; // +1 because G1 was pruned out of G2
        }
    }
    return res;
#else
    return 0;
#endif
}

double MultiS3::eval(const Alignment& A) {
#if MULTI_PAIRWISE
    if (!degreesInit) initDegrees(A, *G1, *G2);

    if (_type==1) denom = EdgeExposure::numExposedEdges(A);
    else if (_type==0) setDenom(A);

    uint newNumer = computeNumer();
    if(newNumer != numer) cerr << "inc eval MS3numer wrong: should be "<<newNumer<<" but is "<<numer << '\n';

    //eval shouldn't have side-effects! -Nil
    numer = newNumer;

    return ((double) newNumer) / denom / NUM_GRAPHS;
#else
    return 0.0;
#endif
}

void MultiS3::initDegrees(const Alignment& A, const Graph& G1, const Graph& G2) {
    totalDegrees = vector<uint>(G2.getNumNodes(), 0);
    for (uint i = 0; i < G1.getNumNodes(); ++i) totalDegrees[A[i]] += 1;
    const Matrix<EDGE_T>* G2Matrix = G2.getAdjMatrix();
    uint n2 = G2.getNumNodes();
    for (uint i = 0; i < n2; ++i) {
        for (uint j = 0; j < n2; ++j) {
            totalDegrees[i] += G2Matrix->get(i,j);
        }
    }
    degreesInit = true;
}
