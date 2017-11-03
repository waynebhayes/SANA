#ifndef GREATWRAPPER_HPP
#define GREATWRAPPER_HPP

#include "WrappedMethod.hpp"
using namespace std;

class GREATWrapper: public WrappedMethod {
public:
    GREATWrapper(Graph* G1, Graph* G2, string args);

private:
    void loadDefaultParameters();
    string convertAndSaveGraph(Graph* graph, string name);
    string generateAlignment();
    Alignment loadAlignment(Graph* G1, Graph* G2, string fileName);
    void deleteAuxFiles();
};

#endif
