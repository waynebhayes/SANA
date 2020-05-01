#ifndef GRAPHLOADER_H_
#define GRAPHLOADER_H_

#include <utility>
#include <vector>
#include <array>
#include <string>
#include "ArgumentParser.hpp"
#include "../Graph.hpp"
extern bool _graphsSwitched;

//static-method only class. It is a class instead of a namespace to differenciate
//between public/private methods. Do not add state to this class
class GraphLoader {
public:
    /* There are two ways to specify the input graphs: by name, or by file
        1. By name: this option requires that the network files exist in the file system
    as networks/g1name/g1name.gw and networks/g1name/g2name.gw (folder and file name must match)
    This options uses the -g1 and -g2 arguments like: -g1 g1name -g2 g2name
    This option only supports .gw format
        2. By file: this options allows the network files to be anywhere in the file system.
    It uses the -fg1 and -fg2 arguments, which should be followed by a file name.
    Then, the network names are deduced from the file names (by removing the path and the extension)
    This option supports .gw and edge list (.el) format.
    This option takes preference: if -fg1 and -fg2 are present, -g1 and -g2 are ignored.
    Furthermore, it creates new network folders and files (networks/g1name/g1name.gw
    and networks/g2name/g2name.gw) so in subsequent executions, one can use the "by name" option. */
    static pair<Graph,Graph> initGraphs(ArgumentParser& args);

    //output functions (Do not output functions directly to the Graph class)
    static void saveInGWFormat(const Graph& G, string outFile, bool saveWeights = false);

    //weightColumn: add 3rd column with edge weights
    //namedEdges: use node names instead of indices. indices are 0-based
    //headerLine: add a line at the top. leave empty to skip
    //sep: string between columns (usually just a space)
    static void saveInEdgeListFormat(const Graph& G, string outFile, bool weightColumn, bool namedEdges, 
                              const string& headerLine, const string& sep);

    //loads graph from filePath according to the file extension
    //if optionalColorFile == "", the graph is loaded as monochromatic
    static Graph loadGraphFromFile(
      const string& graphName, const string& filePath, const string& optionalColorFile, bool loadWeights);
private:
    //extract raw data from the file and process it as necessary to call the graph constructor
    static Graph loadGraphFromGWFile(
      const string& graphName, const string& filePath, const string& optionalColorFile, bool loadWeights);
    static Graph loadGraphFromEdgeListFile(
      const string& graphName, const string& filePath, const string& optionalColorFile, bool loadWeights);

    //for each supported file format, define a struct with the raw data that can be extracted from that file
    //(without any processing) and a constructor that extracts the raw data without doing any processing
    
    //GW format description: http://www.algorithmic-solutions.info/leda_guide/graphs/leda_native_graph_fileformat.html
    struct RawGWFileData {
        vector<string> nodeNames;
        vector<array<uint, 2>> edgeList;
        vector<int> edgeWeights; //may be left empty
        RawGWFileData(const string& filePath, bool containsEdgeWeights);
    };
    struct RawEdgeListFileData {
        vector<array<string, 2>> namedEdgeList;
        //both may be empty, but not both can be set at once
        vector<uint> intWeights;
        vector<float> floatWeights;
        //weight parameter type can be "", "int", "float"
        RawEdgeListFileData(const string& filePath, const string& weightType);
    };
    struct RawColorFileData {
        vector<array<string, 2>> nodeColorList;
        RawColorFileData(); //convenient to have a do-nothing constructor, since the color file is optional
        RawColorFileData(const string& filePath);
    };

    static pair<vector<array<uint, 2>>, vector<string>> namedEdgeListToEdgeListAndNodeNameList(
                                    const vector<array<string, 2>> namedEdgeList);
};

#endif /* GRAPHLOADER_H_ */
