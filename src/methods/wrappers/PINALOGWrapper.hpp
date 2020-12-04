#ifndef PINALOGWRAPPER_HPP_
#define PINALOGWRAPPER_HPP_

#include "WrappedMethod.hpp"
using namespace std;


class PINALOGWrapper: public WrappedMethod {

public:
	PINALOGWrapper(Graph* G1, Graph* G2, string args);

private:
    void loadDefaultParameters();
    string convertAndSaveGraph(Graph* graph, string name);
    string generateAlignment();
    Alignment loadAlignment(Graph* G1, Graph* G2, string fileName);
    void deleteAuxFiles();
};


#endif /* PINALOGWRAPPER_HPP_ */

