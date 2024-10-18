#include "FMeasure.hpp"
using namespace std;

FMeasure::FMeasure(const Graph* G1,const Graph* G2): Measure(NULL, NULL, "invalid"){}
    //emptyc

FMeasure::~FMeasure(){}

double FMeasure::eval(const Alignment& A) {
    //Currently no math added
    return -1;
}