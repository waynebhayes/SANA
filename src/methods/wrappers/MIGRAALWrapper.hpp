#ifndef MIGRAALWRAPPER_HPP
#define MIGRAALWRAPPER_HPP

#include <string>
#include <vector>
#include <iostream>
#include "../Method.hpp"
using namespace std;

class MIGRAALWrapper: public Method {
public:

	MIGRAALWrapper(Graph* G1, Graph* G2);
    Alignment run();
    void describeParameters(ostream& stream);
    string fileNameSuffix(const Alignment& A);

private:
    
    string g1Name, g2Name;
    string g1Folder, g2Folder;
    string g1GWFile, g2GWFile;
    string similarityFile;

    string outputName;
    string alignmetFile;

    Alignment loadAlignment(Graph* G1, Graph* G2, string fileName);

    void generateAlignment();
    void deleteAuxFiles();
};

#endif
