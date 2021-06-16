#ifndef MPI_HPP_
#define MPI_HPP_

#include "../../src/modes/Mode.hpp"
#include "../../src/Graph.hpp"
#include "../../src/arguments/ArgumentParser.hpp"
#include "../../src/measures/MeasureCombination.hpp"
#include "../../src/methods/Method.hpp"

using namespace std;

class MultiMode : public Mode {
public:
    void run(ArgumentParser& args);
    string getName();
};

#endif /* MPI_HPP_ */
