#include "Measure.hpp"
#include <string>

Measure::Measure(Graph* G1, Graph* G2, string name): G1(G1), G2(G2), name(name) {};

Measure::~Measure() {
}

string Measure::getName() {
    return name;
}

bool Measure::isLocal() {
    return false;
}
