#ifndef MAGNAWRAPPER_HPP
#define MAGNAWRAPPER_HPP

#include <string>
#include <vector>
#include <iostream>
#include "Method.hpp"
using namespace std;

class MagnaWrapper: public WrappedMethod {
public:
    MagnaWrapper(Graph* G1, Graph* G2, string args);

private:
    void loadDefaultParameters();
    string convertAndSaveGraph(Graph* graph, string name);
    string generateAlignment();
    Alignment loadAlignment(Graph* G1, Graph* G2, string fileName);
    void deleteAuxFiles();
};

#endif
