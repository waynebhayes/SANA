#ifndef SANAPISWAPWRAPPER_HPP
#define SANAPISWAPWRAPPER_HPP

#include <cassert>

#include "WrappedMethod.hpp"
#include "../../arguments/ArgumentParser.hpp"
#include "../../measures/MeasureCombination.hpp"
#include "../../arguments/methodSelector.hpp"
#include "../../report.hpp"
#include "../SANA.hpp"
#include "PISwapWrapper.hpp"

using namespace std;

class SANAPISWAPWrapper: public WrappedMethod {
public:
    SANAPISWAPWrapper(Graph* G1, Graph* G2, ArgumentParser args, MeasureCombination M);
    Alignment run();

private:
    SANA* sanaMethod;
    PISwapWrapper* piswapMethod;

    Graph* Graph1;
    Graph* Graph2;
    MeasureCombination M;

    void loadDefaultParameters();
    string convertAndSaveGraph(Graph* graph, string name);
    string generateAlignment();
    Alignment loadAlignment(Graph* G1, Graph* G2, string fileName);
    void deleteAuxFiles();
    
    string intermediateAlignment;
};
#endif
