#include "SquaredEdgeScore.hpp"
#include <cmath>

SquaredEdgeScore::SquaredEdgeScore(Graph* G1, Graph* G2) : Measure(G1, G2, "ses")
{
#if MULTI_PAIRWISE
    extern char *getetv(char*);
    char *s = getenv((char*)"SES_DENOM");
    if (s)
    {
        assert(1==sscanf(s, "%lf",&SES_DENOM));
    }
    else
    {
        cerr << "Warning: SES_DENOM should be an environment variable; setting to 1 for now\n";
        SES_DENOM = 1;
    }
#endif
}

SquaredEdgeScore::~SquaredEdgeScore() {
}

double SES_DENOM;

double SquaredEdgeScore::getDenom(void){return SES_DENOM;}

double SquaredEdgeScore::eval(const Alignment& A) {
#ifdef MULTI_PAIRWISE
    #ifdef SPARSE
        return -1; // A.numSquaredAlignedEdges has speed complexity of o(n^2)
                   // It will not finish for big networks.
    #else
        return (double) A.numSquaredAlignedEdges(*G1, *G2) / SES_DENOM;
    #endif
#else
    // ses not make much sense for non-weighted
    return -1;
#endif
}
