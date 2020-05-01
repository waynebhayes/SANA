#ifndef GEDEVOWRAPPER_HPP
#define GEDEVOWRAPPER_HPP

#include "WrappedMethod.hpp"
using namespace std;

class GEDEVOWrapper: public WrappedMethod {
public:
    GEDEVOWrapper(Graph* G1, Graph* G2, string args, uint maxGraphletSize);

private:
	uint maxGraphletSize;
    void loadDefaultParameters();
    string convertAndSaveGraph(const Graph* graph, string name);
    string generateAlignment();
    Alignment loadAlignment(Graph* G1, Graph* G2, string fileName);
    void deleteAuxFiles();
};

#endif
