#include "InvalidMeasure.hpp"
#include <vector>
#include <iostream>
#include "LocalMeasure.hpp"
#include "Graph.hpp"
#include "Timer.hpp"
#include "utils.hpp"
using namespace std;

InvalidMeasure::InvalidMeasure(): Measure(NULL, NULL, "invalid") {
}

InvalidMeasure::~InvalidMeasure() {
}

double InvalidMeasure::eval(const Alignment& A) {
	return -1;
}
