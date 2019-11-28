#ifndef WRAPPEDMETHOD_HPP_
#define WRAPPEDMETHOD_HPP_

#include <string>
#include <vector>
#include <iostream>
#include <sstream>

#include "../Method.hpp"

class WrappedMethod: public Method {
public:

    WrappedMethod(Graph* G1, Graph* G2, std::string name, std::string args);
    Alignment run();
    void describeParameters(ostream& stream);
    std::string fileNameSuffix(const Alignment& A);

protected:
    std::string parameters;

    std::string g1TmpName, g2TmpName;
    std::string g1Name, g2Name;
    std::string g1File, g2File;
    std::string alignmentFile;
    std::string alignmentTmpName;
    std::string TMP;

    std::string wrappedDir;

    // When no arguments are given default parameters are loaded
    virtual void loadDefaultParameters() = 0;

    // Convert the graph to the correct format and save it to a file
    // The file name is returned.  this differers from the name passed in because
    // it will have the correct file extension.
    virtual std::string convertAndSaveGraph(Graph* graph, std::string name) = 0;

    // Runs the program
    // Returns the alignment file
    virtual std::string generateAlignment() = 0;

    // Loads the alignment file into an Alignment Class
    virtual Alignment loadAlignment(Graph* G1, Graph* G2, std::string fileName) = 0;

    // Deletes all files generated when running
    virtual void deleteAuxFiles() = 0;

private:
    // Many of the wrapped algorithms like having the files at the same level
    // as the executable this function moves the files to there.
    void moveFilesToWrappedDir();
};

#endif
