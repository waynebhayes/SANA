#ifndef OPTNETALIGNWRAPPER_HPP
#define OPTNETALIGNWRAPPER_HPP

#include "WrappedMethod.hpp"
using namespace std;

class OptNetAlignWrapper: public WrappedMethod {
public:
    OptNetAlignWrapper(Graph* G1, Graph* G2, string args);

private:
    void loadDefaultParameters();
    string convertAndSaveGraph(Graph* graph, string name);
    string generateAlignment();
    Alignment loadAlignment(Graph* G1, Graph* G2, string fileName);
    void deleteAuxFiles();
};

#endif
