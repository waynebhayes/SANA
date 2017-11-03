#ifndef GHOSTWRAPPER_HPP
#define GHOSTWRAPPER_HPP

#include "WrappedMethod.hpp"
using namespace std;

class GHOSTWrapper: public WrappedMethod {
public:
    GHOSTWrapper(Graph* G1, Graph* G2, string args);

private:
    void createCfgFile(string cfgFileName);
    void loadDefaultParameters();
    string convertAndSaveGraph(Graph* graph, string name);
    string generateAlignment();
    Alignment loadAlignment(Graph* G1, Graph* G2, string fileName);
    void deleteAuxFiles();

    string matcher;
    string nneighbors;
    string beta;
    string ratio;
    string searchiter;
    string cfgFile;
    string alpha;
    string SeqScores;
};

#endif
