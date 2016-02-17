#ifndef NETALWRAPPER_HPP
#define NETALWRAPPER_HPP

#include <string>
#include <vector>
#include <iostream>
#include "../Method.hpp"
using namespace std;

class NETALWrapper: public Method {
public:

	NETALWrapper(Graph* G1, Graph* G2);
    Alignment run();
    void describeParameters(ostream& stream);
    string fileNameSuffix(const Alignment& A);

private:
    
    static const string NETALProgram;

    string g1Name, g2Name;
    string g1Folder, g2Folder;
    string g1EdgeListFile, g2EdgeListFile;
    string similarityFile;

    string alignmetFile;

    Alignment loadAlignment(Graph* G1, Graph* G2, string fileName);

    void generateAlignment();
    void deleteAuxFiles();
};

#endif
