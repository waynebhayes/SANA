#ifndef HUBALIGNWRAPPER_HPP
#define HUBALIGNWRAPPER_HPP

#include <string>
#include <vector>
#include <iostream>
#include "Method.hpp"
using namespace std;

class HubAlignWrapper: public Method {
public:

    HubAlignWrapper(Graph* G1, Graph* G2, double alpha);
    Alignment run();
    void describeParameters(ostream& stream);
    string fileNameSuffix(const Alignment& A);

private:
    
    static const string hubalignProgram;
    string g1TmpFile;
    string g2TmpFile;

    double alpha;

    string g1Name, g2Name;
    string g1Folder, g2Folder;
    string g1EdgeListFile, g2EdgeListFile;
    string similarityFile;

    string alignmetFile;

    void generateEdgeListFile(int graphNum);
    void generateAlignment();
    void copyEdgeListsToTmpFiles();
    void deleteAuxFiles();
};

#endif
