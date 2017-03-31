#ifndef HUBALIGNWRAPPER_HPP
#define HUBALIGNWRAPPER_HPP

#include <string>
#include <vector>
#include <iostream>
#include "../Method.hpp"
using namespace std;

class HubAlignWrapper: public Method {
public:

    HubAlignWrapper(Graph* G1, Graph* G2, double alpha);
    Alignment run();
    void describeParameters(ostream& stream);
    string fileNameSuffix(const Alignment& A);

private:
    
    static const string hubalignProgram;

    double alpha;

    string g1Name, g2Name;
    string g1Folder, g2Folder;
    string g1EdgeListFile, g2EdgeListFile;
    string similarityFile;

    string alignmentFile;

    void generateEdgeListFile(int graphNum);
    void generateAlignment();
    void deleteAuxFiles();
};

#endif
