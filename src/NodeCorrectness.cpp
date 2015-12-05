#include "NodeCorrectness.hpp"
#include <vector>
#include <iostream>
#include "LocalMeasure.hpp"
#include "Graph.hpp"
#include "Timer.hpp"
#include "utils.hpp"
using namespace std;

NodeCorrectness::NodeCorrectness(const Alignment& A): Measure(NULL, NULL, "nc"),
	trueA(A) {
}

NodeCorrectness::~NodeCorrectness() {
}

double NodeCorrectness::eval(const Alignment& A) {
	uint count = 0;
	for (uint i = 0; i < A.size(); i++) {
		if (A[i] == trueA[i]) count++;
	}
	return (double) count / A.size();
}