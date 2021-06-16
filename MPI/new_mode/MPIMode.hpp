#ifndef MPI_HPP_
#define MPI_HPP_

#include "Mode.hpp"
#include "../Graph.hpp"
#include "../arguments/ArgumentParser.hpp"
#include "../measures/MeasureCombination.hpp"
#include "../methods/Method.hpp"

using namespace std;

class MPIMode : public Mode {
public:
    void run(ArgumentParser& args);
    string getName();
};

#endif /* MPI_HPP_ */
