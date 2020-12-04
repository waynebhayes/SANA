#include <vector>
#include <iostream>
#include "InvalidMeasure.hpp"

using namespace std;

InvalidMeasure::InvalidMeasure(): Measure(NULL, NULL, "invalid") {
}

InvalidMeasure::~InvalidMeasure() {
}

double InvalidMeasure::eval(const Alignment& A) {
    return -1;
}

double InvalidMeasure::eval(const MultiAlignment& MA){return 0;} //dummy declare
