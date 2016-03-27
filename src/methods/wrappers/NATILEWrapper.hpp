#ifndef NATILEWRAPPER_HPP
#define NATILEWRAPPER_HPP

#include <string>
#include <vector>
#include <iostream>
#include "WrappedMethod.hpp"
using namespace std;

class NATILEWrapper: public WrappedMethod {
public:
	NATILEWrapper(Graph* G1, Graph* G2, string args);

private:
    void loadDefaultParameters();
    string convertAndSaveGraph(Graph* graph, string name);
    string generateAlignment();
    Alignment loadAlignment(Graph* G1, Graph* G2, string fileName);
    void deleteAuxFiles();
};

#endif
