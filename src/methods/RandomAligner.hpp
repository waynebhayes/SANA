#ifndef RANDOMALIGNER_HPP
#define RANDOMALIGNER_HPP

#include <string>
#include <vector>
#include <iostream>
#include "Method.hpp"

using namespace std;

class RandomAligner: public Method {
public:

    RandomAligner(Graph* G1, Graph* G2);
    Alignment run();
    void describeParameters(ostream& stream);
    string fileNameSuffix(const Alignment& A);

private:

};

#endif
