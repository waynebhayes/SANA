#ifndef SPINALWRAPPER_HPP
#define SPINALWRAPPER_HPP

#include "WrappedMethod.hpp"
using namespace std;

class SPINALWrapper: public WrappedMethod {
public:
    SPINALWrapper(Graph* G1, Graph* G2, double alpha, string args);

private:
    void loadDefaultParameters();
    string convertAndSaveGraph(Graph* graph, string name);
    double alpha;
    string generateAlignment();
    Alignment loadAlignment(Graph* G1, Graph* G2, string fileName);
    void deleteAuxFiles();
};

#endif
