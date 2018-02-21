#include "SquaredEdgeScore.hpp"
#include <cmath>

SquaredEdgeScore::SquaredEdgeScore(Graph* G1, Graph* G2) : Measure(G1, G2, "ses") {
}

SquaredEdgeScore::~SquaredEdgeScore() {
}

double SquaredEdgeScore::eval(const Alignment& A) {
    static double SES_DENOM;
    if(SES_DENOM == 0.0){
	extern char *getetv(char*);
	char *s = getenv((char*)"SES_DENOM");
	assert(1==sscanf(s, "%lf",&SES_DENOM));
    }
    return (double) A.numSquaredAlignedEdges(*G1, *G2) / SES_DENOM;
}
