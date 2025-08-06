#ifndef SANAPISWAPWRAPPER_HPP
#define SANAPISWAPWRAPPER_HPP

#include <cassert>
#include "PISwapWrapper.hpp"
#include "SanaWrapper.hpp"
#include "WrappedMethod.hpp"
#include "../SANA.hpp"
#include "../../arguments/ArgumentParser.hpp"
#include "../../measures/MeasureCombination.hpp"

using namespace std;

class SANAPISWAPWrapper: public WrappedMethod {
public:
    SANAPISWAPWrapper(const Graph* G1, const Graph* G2, ArgumentParser args, MeasureCombination M);
    Alignment run();

private:
    SanaWrapper* sanaMethod;
    PISwapWrapper* piswapMethod;
    MeasureCombination M;

    void loadDefaultParameters();
    string convertAndSaveGraph(const Graph* graph, string name);
    string generateAlignment();
    Alignment loadAlignment(const Graph* G1, const Graph* G2, string fileName);
    void deleteAuxFiles();
    
    string intermediateAlignment;
};

#endif /* SANAPISWAPWRAPPER_HPP */
