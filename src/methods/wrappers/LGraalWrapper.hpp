#ifndef LGRAALWRAPPER_HPP
#define LGRAALWRAPPER_HPP

#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <sstream>

#include "../Method.hpp"
using namespace std;

class LGraalWrapper: public Method {
public:

    LGraalWrapper(Graph* G1, Graph* G2, double alpha, uint iterlimit, uint timelimit);
    Alignment run();
    void describeParameters(ostream& stream);
    string fileNameSuffix(const Alignment& A);
    
private:
    static const string GDVCounterProgram;
    static const string lgraalProgram;
    
    string lgraalOutputFile;

    string g1Name, g2Name;
    double alpha;
    uint iterlimit;
    uint timelimit;

    string similarityFile;
    string g1Folder, g2Folder;
    string g1NetworkFile, g2NetworkFile;
    string g1GDVFile, g2GDVFile;

    void generateGDVFile(int graphNum);
    void generateAlignment();
    string generateDummySimilarityFile();

    void deleteAuxFiles();

};

#endif
