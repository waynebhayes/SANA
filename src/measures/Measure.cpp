#include "Measure.hpp"
#include <string>
#include "Measure.hpp"
#include <string>
#include <vector>

Measure::Measure(Graph* G1, Graph* G2, string name): G1(G1), G2(G2), name(name) {};

Measure::Measure(string name): name(name) {};//just for multimode

Measure::Measure(vector<Graph>* GV, string name) : Measure(name) {};

Measure::~Measure() {
}

string Measure::getName() {
    return name;
}

bool Measure::isLocal() {
    return false;
}
