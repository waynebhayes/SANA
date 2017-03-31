#ifndef SIMIlARITYMODE_HPP_
#define SIMIlARITYMODE_HPP_

#include "Mode.hpp"
#include "../Graph.hpp"
#include "../arguments/ArgumentParser.hpp"
#include "../measures/MeasureCombination.hpp"
#include "../methods/Method.hpp"

class SimilarityMode : public Mode {
public:
    void run(ArgumentParser& args);
    std::string getName(void);
    void saveSimilarityMatrix(vector<vector <float> > sim, Graph &G1, Graph &G2, string file, int format);
};

#endif /* SIMIlARITYMODE_HPP_ */
