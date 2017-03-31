#ifndef PISWAPWRAPPER_HPP
#define PISWAPWRAPPER_HPP

#include "WrappedMethod.hpp"
using namespace std;

class PISwapWrapper: public WrappedMethod {
public:

    PISwapWrapper(Graph* G1, Graph* G2, double alpha, string startingAlignment, string args);

private:
    void loadDefaultParameters();
    string convertAndSaveGraph(Graph* graph, string name);
    string generateAlignment();
    Alignment loadAlignment(Graph* G1, Graph* G2, string fileName);
    void deleteAuxFiles();
    string startingAligName;
    double alpha;
};

#endif
