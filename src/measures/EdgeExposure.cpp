#include "EdgeExposure.hpp"
#include <cmath>

uint EdgeExposure::EDGE_SUM = 2;
uint EdgeExposure::MAX_EDGE = 1;
uint EdgeExposure::denom = 0;
uint EdgeExposure::numer = 0;

EdgeExposure::EdgeExposure(const Graph* G1, const Graph* G2): Measure(G1, G2, "ee") {
#ifdef MULTI_PAIRWISE
    extern char *getetv(char*);
    char *s = getenv((char*)"EDGE_SUM");
    if(s) assert(1==sscanf(s, "%u",&EDGE_SUM));
    else {
        cout << "Warning: EDGE_SUM should be an environment variable; setting to 2 for now\n";
        EDGE_SUM = 2;
    }
	
	char *m = getenv((char*)"MAX_EDGE");
    if(m) assert(1==sscanf(m, "%u",&MAX_EDGE));
    else {
        cout << "Warning: MAX_EDGE should be an environment variable; setting to 1 for now\n";
        MAX_EDGE = 1;
    }
	assert(EDGE_SUM >= MAX_EDGE);
	EdgeExposure::denom = EDGE_SUM - MAX_EDGE;
	cout << "EdgeExposure: MAX_EDGE is " << MAX_EDGE << ", EDGE_SUM is " << EDGE_SUM <<
		 ", EE DENOM is " << EdgeExposure::denom << endl;
#endif
}

EdgeExposure::~EdgeExposure() {}

uint EdgeExposure::getMaxEdge() { return MAX_EDGE; }

double EdgeExposure::eval(const Alignment& A) {
#if MULTI_PAIRWISE
    uint ne = numExposedEdges(A, *G1, *G2);
    if(ne != numer) cerr << "EdgeExposure::numer should be "<<ne<<" but is "<<ne <<'\n';
    numer = ne;
    assert(ne >= MAX_EDGE and ne <= EDGE_SUM);
    return 1 - (ne - MAX_EDGE)/(double) denom;
#else
    return 0.0;
#endif
}

int EdgeExposure::numExposedEdges(const Alignment& A, const Graph& G1, const Graph& G2) {
#if MULTI_PAIRWISE
    int res = G2.getNumEdges(); //every edge in G2 counts (edge lists don't store any 0-weight edges)
    //we also need to take into account the edges in G1 not mapped to any edge in G2:
    for (const auto& edge: *(G1.getEdgeList())) {
        uint g1Node1 = edge[0], g1Node2 = edge[1];
        uint g2Node1 = A[g1Node1], g2Node2 = A[g1Node2];
        if (!G2.hasEdge(g2Node1, g2Node2)) res++;
        assert(G1.edgeWeight(g1Node1, g1Node2) == 1); //sanity check
    }
    return res;
#else
    return -1; 
#endif
}

