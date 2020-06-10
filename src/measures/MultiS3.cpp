#include "MultiS3.hpp"
#include "EdgeExposure.hpp"
#include <cmath>

uint NUM_GRAPHS;
uint MultiS3::denom = 1;
uint MultiS3::numer = 1;
double MultiS3::_type = 0;
bool MultiS3::degreesInit = false;
vector<uint> MultiS3::shadowDegree;

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
    const uint n1 = G1->getNumNodes();
    for (uint i = 0; i < n1; i++) {
        if (shadowDegree[i] > 0) denom += n1-1-i; // Nil correctly compressed this from SANA1.1 but I think SANA1.1 was wrong
    }
    denom /= 2;
}

uint MultiS3::computeNumer(const Alignment& A) const {
#ifdef MULTI_PAIRWISE
    uint res = 0;
    for (const auto& edge: *(G1->getEdgeList())) {
        uint node1 = edge[0], node2 = edge[1];
        auto weight = G2->getEdgeWeight(A[node1], A[node2]);
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
    if (not degreesInit) initDegrees(A, *G1, *G2);

    if (_type==1) denom = EdgeExposure::numExposedEdges(A, *G1, *G2);
    else if (_type==0) setDenom(A);

    uint correctNumer = computeNumer(A);
    if(correctNumer != numer) {
        cerr<<"inc eval MS3numer wrong: should be "<<correctNumer<<" but is "<<numer<<endl;
        numer = correctNumer;
    }
    return ((double) numer) / denom / NUM_GRAPHS;
#else
    return 0.0;
#endif
}

void MultiS3::initDegrees(const Alignment& A, const Graph& G1, const Graph& G2) {
    shadowDegree = vector<uint>(G2.getNumNodes(), 0);
    for (uint i = 0; i < G2.getNumNodes(); i++) shadowDegree[i] = G2.getNumNbrs(i);
    for (const auto& edge : *(G2.getEdgeList())) {
        auto w = G2.getEdgeWeight(edge[0],edge[1]);
        shadowDegree[edge[0]] += w; // doesn't this overcount by 1 since we already set it to getNumNbrs(i) above?
        if (edge[0] != edge[1]) shadowDegree[edge[1]] += w; //avoid double-counting for self-lopos
    }
    for (uint i = 0; i < G1.getNumNodes(); ++i) shadowDegree[A[i]] += 1;
    degreesInit = true;
}
