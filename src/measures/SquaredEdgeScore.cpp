#include "SquaredEdgeScore.hpp"
#include <cmath>

SquaredEdgeScore::SquaredEdgeScore(Graph* G1, Graph* G2) : Measure(G1, G2, "ses") {
}

SquaredEdgeScore::~SquaredEdgeScore() {
}

double SquaredEdgeScore::eval(const Alignment& A) {
#ifdef WEIGHTED
    static double SES_DENOM = -1;
    if(SES_DENOM == -1){
        extern char *getetv(char*);
        char *s = getenv((char*)"SES_DENOM");
        if(s) assert(1==sscanf(s, "%lf",&SES_DENOM));
	else
	{
	    cout << "Warning: SES_DENOM should be an environment variables; setting to 1 for now\n";
	    SES_DENOM = 1;
	}
    }
    return (double) A.numSquaredAlignedEdges(*G1, *G2) / SES_DENOM;
#else
    // ses not make much sense for non-weighted
    return -1;
#endif
}
