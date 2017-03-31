#ifndef HILLCLIMBING_HPP
#define HILLCLIMBING_HPP
#include "../measures/MeasureCombination.hpp"
#include "Method.hpp"
#include <map>

class HillClimbing: public Method {
public:
    HillClimbing();
    HillClimbing(Graph* G1, Graph* G2, MeasureCombination* M, string startAName);
    virtual ~HillClimbing();

    Alignment run();
    void describeParameters(ostream& stream);
    string fileNameSuffix(const Alignment& A);
    double getExecutionTime() const;


private:

    MeasureCombination *M;
    double changeProbability;
    string startAName;
    Alignment startA;
    
    double executionTime;

    //random number generation
    mt19937 gen;
    uniform_int_distribution<> G1RandomNode;
    uniform_int_distribution<> G2RandomUnassignedNode;
    uniform_real_distribution<> randomReal;
};

#endif
