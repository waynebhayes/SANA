#include "EdgeExposure.hpp"
#include <cmath>

unsigned EDGE_SUM;
unsigned MAX_EDGE;
unsigned EdgeExposure::denom = 0;
unsigned EdgeExposure::numer = 0;

EdgeExposure::EdgeExposure(Graph* G1, Graph* G2) : Measure(G1, G2, "ee") {
#if MULTI_PAIRWISE
    extern char *getetv(char*);
    char *s = getenv((char*)"EDGE_SUM");
    if(s)
        assert(1==sscanf(s, "%u",&EDGE_SUM));
    else
    {
        cout << "Warning: EDGE_SUM should be an environment variable; setting to 2 for now\n";
        EDGE_SUM = 2;
    }
	
	char *m = getenv((char*)"MAX_EDGE");
    if(m)
        assert(1==sscanf(m, "%u",&MAX_EDGE));
    else
    {
        cout << "Warning: MAX_EDGE should be an environment variable; setting to 1 for now\n";
        MAX_EDGE = 1;
    }
	assert(EDGE_SUM >= MAX_EDGE);
	EdgeExposure::denom = EDGE_SUM - MAX_EDGE;
	cout << "EdgeExposure: MAX_EDGE is " << MAX_EDGE << ", EDGE_SUM is " << EDGE_SUM <<
		 ", EE DENOM is " << EdgeExposure::denom << endl;
#endif
}

EdgeExposure::~EdgeExposure() {
}

unsigned EdgeExposure::getDenom()
{
    return EdgeExposure::denom;
}

unsigned EdgeExposure::getMaxEdge()
{
	return MAX_EDGE;
}

double EdgeExposure::eval(const Alignment& A) {
#if MULTI_PAIRWISE
    uint ne = A.numExposedEdges(*G1, *G2);
    if(int(ne) != int(numer)) cerr << "EdgeExposure::numer should be "<< numer <<" but is "<< ne <<'\n';
    numer = ne;
    assert(ne >= MAX_EDGE);
    assert(ne <= EDGE_SUM);
    return 1 - ((ne - MAX_EDGE) / (double)EdgeExposure::getDenom());
#else
    return 0.0;
#endif
}
