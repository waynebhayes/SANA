#ifndef MIGRAALWRAPPER_HPP
#define MIGRAALWRAPPER_HPP

#include "WrappedMethod.hpp"
using namespace std;

class MIGRAALWrapper: public WrappedMethod {
public:
    MIGRAALWrapper(Graph* G1, Graph* G2, string args);

private:
    string outputName;

    void loadDefaultParameters();
    string convertAndSaveGraph(Graph* graph, string name);
    string generateAlignment();
    Alignment loadAlignment(Graph* G1, Graph* G2, string fileName);
    void deleteAuxFiles();
};

#endif
