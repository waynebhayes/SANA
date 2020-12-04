#ifndef GREEDYLCCS_HPP
#define GREEDYLCCS_HPP

#include <string>
#include <vector>
#include <iostream>
#include "Method.hpp"

using namespace std;

class GreedyLCCS: public Method {
public:

    GreedyLCCS(Graph* G1, Graph* G2, string startAName);
    Alignment run();
    void describeParameters(ostream& stream);
    string fileNameSuffix(const Alignment& A);

private:

    Alignment* startA;
    string startAName;
};

#endif
