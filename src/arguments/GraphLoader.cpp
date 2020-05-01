#include <string>
#include <chrono>
#include <iostream>
#include <set>
#include <sstream>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <thread>
#include <future>
#include "GraphLoader.hpp"
#include "../utils/Timer.hpp"

using namespace std;
bool _graphsSwitched = false;

pair<Graph,Graph> GraphLoader::initGraphs(ArgumentParser& args) {
    cout << "Initializing graphs..." << endl;
    Timer T;
    T.start();
    createFolder("networks");
    createFolder(AUTOGENEREATED_FILES_FOLDER);
    string fg1 = args.strings["-fg1"], fg2 = args.strings["-fg2"];
    string g1Name, g2Name, g1File, g2File;
    if (fg1 != "") {
        g1Name = extractFileNameNoExtension(fg1);
        g1File = fg1;
    } else {
        g1Name = args.strings["-g1"];
        g1File = "networks/"+g1Name+"/"+g1Name+".gw";
    }
    if (fg2 != "") {
        g2Name = extractFileNameNoExtension(fg2);
        g2File = fg2;
    } else {
        g2Name = args.strings["-g2"];
        g2File = "networks/"+g2Name+"/"+g2Name+".gw";
    }
    string fcolors1 = args.strings["-fcolor1"], fcolors2 = args.strings["-fcolor2"];

    bool g1HasWeights, g2HasWeights;
#ifdef MULTI_PAIRWISE
 #ifdef FLOAT_WEIGHTS
  #error multipairwise currently does not support float weights
 #else
    g1HasWeights = false, g2HasWeights = true; //g1 is unweighted and g2 (the shadow) is weighted
 #endif
#elif FLOAT_WEIGHTS
    g1HasWeights = true, g2HasWeights = true;
#else 
    g1HasWeights = false, g2HasWeights = false; //unweighted
#endif

    Timer tLoad;
    tLoad.start();
    auto futureG1 = async(&loadGraphFromFile, g1Name, g1File, fcolors1, g1HasWeights);
    auto futureG2 = async(&loadGraphFromFile, g2Name, g2File, fcolors2, g2HasWeights);
    Graph G1 = futureG1.get();
    Graph G2 = futureG2.get();
    cout << "Graph loading completed in " << tLoad.elapsedString() << endl;

    if (G1.getNumNodes() > G2.getNumNodes()) {
        //note: this does not guarantee that a color-restricted alignment exists,
        //so the program might still crash when initializing SANA
        swap(G1, G2);
        cout << "Switching G1 and G2 because G1 has more nodes than G2" << endl;
        _graphsSwitched = true;
    }

    string path1 = args.strings["-pathmap1"], path2 = args.strings["-pathmap2"];
    if (path1 != "") {
        uint power1 = atoi(path1.c_str());
        if (power1 > 1) {
            cout << "Raising G1 to the " << power1 << " power" << endl;
            G1 = G1.graphPower(power1);
        }
    }
    if (path2 != "") {
        uint power2 = atoi(path2.c_str());
        if (power2 > 1) {
            cout << "Raising G2 to the " << power2 << " power" << endl;
            G2 = G2.graphPower(power2);
        }
    }

#ifdef MULTI_PAIRWISE
    //prune G1 from G2
    string nodeMapFile = args.strings["-startalignment"];
    cerr << "Starting to prune using " << nodeMapFile << endl;
    assert(nodeMapFile.size() > 6);
    string format = nodeMapFile.substr(nodeMapFile.size()-6);
    if (format != ".align") throw runtime_error("only edge list format is supported for start alignment");
    Alignment A = Alignment::loadEdgeList(G1, G2, nodeMapFile);
    const vector<uint>* G1ToG2Map = A.getVector();
    if (G1ToG2Map->size() != n1)
        throw runtime_error("G1ToG2Map size ("+to_string(G1ToG2Map->size())+
                            ") less than number of nodes ("+to_string(n1)+")");
    G2 = G2.subtractGraph(G1, *G1ToG2Map);
#endif

    double rewiredFraction1 = args.doubles["-rewire1"];
    if (rewiredFraction1 > 0) {
        if (rewiredFraction1 > 1)
            throw runtime_error("Cannot rewire more than 100% of G1 edges");
        G1 = G1.graphWithRewiredRandomEdges(rewiredFraction1);
    }
    double rewiredFraction2 = args.doubles["-rewire2"];
    if (rewiredFraction2 > 0) {
        if (rewiredFraction2 > 1)
            throw runtime_error("Cannot rewire more than 100% of G2 edges");
        G2 = G2.graphWithRewiredRandomEdges(rewiredFraction2);
    }

    if (G1.getNumEdges() == 0) throw runtime_error("G1 has 0 edges");
    if (G2.getNumEdges() == 0) throw runtime_error("G2 has 0 edges");
    cout << "Total time for loading graphs (" << T.elapsedString() << ")" << endl;
    return {G1, G2};
}

//dispatches to one of the format-specific functions based on the file extension
Graph GraphLoader::loadGraphFromFile(const string& graphName, const string& filePath,
                                     const string& optionalColorFile, bool loadWeights) {
    if (not fileExists(filePath)) throw runtime_error("File not found: " + filePath);
    if (optionalColorFile != "" and not fileExists(optionalColorFile))
        throw runtime_error("File not found: " + optionalColorFile);

    string format = filePath.substr(filePath.find_last_of('.')+1);
    string uncompressedFileExt = getUncompressedFileExtension(filePath);

    //dbg cerr<<graphName<<" "<<filePath<<" "<<optionalColorFile<<" "<<loadWeights<<endl<<format<<" "<<uncompressedFileExt;

    if (format == "gw" || uncompressedFileExt == "gw")
        return loadGraphFromGWFile(graphName, filePath, optionalColorFile, loadWeights);
    if (format == "el" || uncompressedFileExt == "el" || format == "elw" || uncompressedFileExt == "elw")
        return loadGraphFromEdgeListFile(graphName, filePath, optionalColorFile, loadWeights);
    if (format == "mpel")
        throw runtime_error("Multipartite edge list format (MPEL) no longer supported. Use coloring feature instead");
    if (format == "lgf" || format == "xml" || format == "csv" || format == "gml") {
        cerr<<"Sorry, this format has been deprecated and needs to be reimplemented following the new style."<<endl
            <<"To do so, follow the guidelines and already existing examples in GraphLoader.hpp."<<endl
            <<"*** You should not modify the Graph class, only GraphLoader ***"<<endl
            <<"If you want 'inspiration' for how to parse this file type, you can see the deprecated graph constructors here:"<<endl
            <<"https://github.com/waynebhayes/SANA/blob/b10802ed8683af7d3b238a2c4b729717b00e9e65/src/Graph.cpp"<<endl; 
        throw runtime_error("Unsuported graph format: " + format);
    }
    throw runtime_error("Unsupported graph format: " + format);
}

Graph GraphLoader::loadGraphFromGWFile(const string& graphName, const string& filePath, 
                                       const string& optionalColorFile, bool loadWeights) {
    RawGWFileData gwData(filePath, loadWeights);
    //reindex 1-based edges to 0-based edges
    for (uint i = 0; i < gwData.edgeList.size(); i++) {
        gwData.edgeList[i][0]--;
        gwData.edgeList[i][1]--;
    }

    RawColorFileData colorData;
    if (optionalColorFile != "") colorData = RawColorFileData(optionalColorFile);

    if (!loadWeights)
        return Graph(graphName, filePath, gwData.edgeList,
                     gwData.nodeNames, {}, colorData.nodeColorList);
#ifdef BOOL_EDGE_T
    throw runtime_error("cannot load weights for unweighted graph");
#elif FLOAT_WEIGHTS
    throw runtime_error("cannot load float weights from GW file");
#else
    vector<EDGE_T> edgeWeights;
    edgeWeights.reserve(edgeListData.intWeights.size());
    for (int w : edgeListData.intWeights) {
        assert(w > 0 and "graph cannot have negative weights");
        assert(w < (1L << 8*sizeof(EDGE_T)) -1 and "EDGE_T type is not wide enough for these weights");
        edgeWeights.push_back((EDGE_T) w);
    }
    return Graph(graphName, filePath, gwData.edgeList, gwData.nodeNames,
                    edgeWeights, colorData.nodeColorList);
#endif
}

Graph GraphLoader::loadGraphFromEdgeListFile(const string& graphName, const string& filePath, 
                                const string& optionalColorFile, bool loadWeights) {
    string weightType;
    if (!loadWeights) weightType = "";
    else {
#ifdef BOOL_EDGE_T
        throw runtime_error("cannot load weights for unweighted graph");
#elif FLOAT_WEIGHTS
        weightType = "float";
#else
        weightType = "int";
#endif
    }

    RawEdgeListFileData edgeListData(filePath, weightType);
    pair<vector<array<uint, 2>>, vector<string>> edgeAndNodeNameLists =
        namedEdgeListToEdgeListAndNodeNameList(edgeListData.namedEdgeList);

    RawColorFileData colorData;
    if (optionalColorFile != "") colorData = RawColorFileData(optionalColorFile);

    if (not loadWeights)
        return Graph(graphName, filePath, edgeAndNodeNameLists.first,
                     edgeAndNodeNameLists.second, {}, colorData.nodeColorList);

#ifdef BOOL_EDGE_T
    throw runtime_error("cannot load weights for unweighted graph");
#elif FLOAT_WEIGHTS
    return Graph(graphName, filePath, edgeAndNodeNameLists.first, edgeAndNodeNameLists.second, 
                 edgeListData.floatWeights, colorData.nodeColorList);
#else
    vector<EDGE_T> edgeWeights;
    edgeWeights.reserve(edgeListData.intWeights.size());
    for (int w : edgeListData.intWeights) {
        assert(w > 0 and "graph cannot have negative weights");
        assert(w < (1L << 8*sizeof(EDGE_T)) -1 and "EDGE_T type is not wide enough for these weights");
        edgeWeights.push_back((EDGE_T) w);
    }
    return Graph(graphName, filePath, edgeAndNodeNameLists.first, edgeAndNodeNameLists.second, 
                 edgeWeights, colorData.nodeColorList);
#endif
}

GraphLoader::RawGWFileData::RawGWFileData(const string& filePath, bool containsEdgeWeights) {
    stdiobuf sbuf = readFileAsStreamBuffer(filePath);
    istream ifs(&sbuf);
    string line;
    //ignore header
    for (uint i = 0; i < 4; i++) getline(ifs, line);
    //read number of nodes
    int n;
    if (getline(ifs, line)) {
        n = stoi(line);
        if (n <= 0) throw runtime_error("Failed to read non-zero node number: "+line);
    } else throw runtime_error("Failed to read line with node number");
    //read nodes
    nodeNames.reserve(n);
    for (uint i = 0; i < (uint) n; i++) {
        if (getline(ifs, line)) {
            if (line.size() <= 4) throw runtime_error("Failed to read node name from line: "+line);
            string node = line.substr(2,line.size()-4); //strip |{ and }|
            nodeNames.emplace_back(node);
        } else throw runtime_error("Failed to read all nodes");
    }
    //read number of edges
    int m;
    if (getline(ifs, line)) {
        m = stoi(line);
        istringstream iss2(line);
        if (m <= 0) throw runtime_error("Failed to read non-zero edge number: "+line);        
    } else throw runtime_error("Failed to read line with edge number");
    //read edges
    edgeList.reserve(m);
    if (containsEdgeWeights) edgeWeights.reserve(m);
    for (int i = 0; i < m; ++i) {
        if (getline(ifs, line)) {
            istringstream iss(line);
            uint node1, node2;
            if (iss >> node1 >> node2)
                edgeList.push_back({node1, node2});
            else throw runtime_error("Failed to read edge from line: "+line);
            if (containsEdgeWeights) {
                int edgeWeight;
                char dumpChar;
                if (iss >> dumpChar >> dumpChar >> dumpChar >> edgeWeight)
                    edgeWeights.push_back(edgeWeight);
                else throw runtime_error("Failed to read edge weight from line: "+line);
            }
        } else throw runtime_error("Failed to read all edges");
    }
    if (getline(ifs, line) and line.size() != 0)
        cerr<<"Warning: ignoring leftover lines at the end of .gw file: "<<line<<endl;
}

GraphLoader::RawEdgeListFileData::RawEdgeListFileData(
            const string& filePath, const string& weightType) {

    //using reserve as an optimization to try to avoid reallocations. Would be
    //interesting to benchmark to see if it makes a difference
    uint estimatedUpperBound = 10000;
    namedEdgeList.reserve(estimatedUpperBound);                                       
    if (weightType == "int") intWeights.reserve(estimatedUpperBound);
    else if (weightType == "float") floatWeights.reserve(estimatedUpperBound);
    else if (weightType != "") throw runtime_error("weightType cannot be "+weightType);

    stdiobuf sbuf = readFileAsStreamBuffer(filePath);
    istream infile(&sbuf);
    for (string line; getline(infile, line); ) {
        string name1, name2;
        istringstream iss(line);
        if (iss >> name1 >> name2) namedEdgeList.push_back({name1, name2});
        else throw runtime_error("failed to read edge from line: "+line);
        if (weightType == "int") {
            int w;
            if (iss >> w) intWeights.push_back(w);
            else throw runtime_error("failed to read weight from line: "+line);
        } else if (weightType == "float") {
            float w;
            if (iss >> w) floatWeights.push_back(w);
            else throw runtime_error("failed to read weight from line: "+line);
        }
    }
    namedEdgeList.shrink_to_fit();
    if (weightType == "int") intWeights.shrink_to_fit();
    else if (weightType == "float") floatWeights.shrink_to_fit();
}

GraphLoader::RawColorFileData::RawColorFileData() {}
GraphLoader::RawColorFileData::RawColorFileData(const string& filePath) {
    uint estimatedUpperBound = 10000;
    nodeColorList.reserve(estimatedUpperBound);
    stdiobuf sbuf = readFileAsStreamBuffer(filePath);
    istream ifs(&sbuf);
    for (string line; getline(ifs, line); ) {
        if (line.size() == 0) continue;
        string nodeName, colorName;
        istringstream iss(line);
        if (iss >> nodeName >> colorName)
            nodeColorList.push_back({nodeName, colorName});
        else throw runtime_error("failed to read node color pair from line: "+line);
    }
    nodeColorList.shrink_to_fit();
}

pair<vector<array<uint, 2>>, vector<string>> GraphLoader::namedEdgeListToEdgeListAndNodeNameList(
        const vector<array<string, 2>> namedEdgeList) {
    vector<array<uint, 2>> edgeList;
    edgeList.reserve(namedEdgeList.size());
    vector<string> nodeNames;
    unordered_map<string, uint> nameToIndex;
    uint nextFreeIndex = 0;
    for (const auto& edge : namedEdgeList) {
        string name1 = edge[0], name2 = edge[1];
        uint index1, index2;
        if (!nameToIndex.count(name1)) {
            nameToIndex[name1] = nextFreeIndex;
            nodeNames.push_back(name1);
            index1 = nextFreeIndex;
            nextFreeIndex++;
        } else {
            index1 = nameToIndex[name1];
        }
        if (!nameToIndex.count(name2)) {
            nameToIndex[name2] = nextFreeIndex;
            nodeNames.push_back(name2);
            index2 = nextFreeIndex;
            nextFreeIndex++;
        } else {
            index2 = nameToIndex[name2];
        }
        edgeList.push_back({index1, index2});
    }
    return {edgeList, nodeNames};
}

void GraphLoader::saveInGWFormat(const Graph& G, string outFile,
    bool saveWeights) {
    ofstream outfile;
    string saveIn = "/tmp/saveInGWFormat", pid = to_string(getpid());
    string tempFile = saveIn+pid; //a comment explaining why tempFile is necessary would be nice  -Nil
    outfile.open(tempFile.c_str());
    outfile <<"LEDA.GRAPH"<<endl<<"string"<<endl<<"short"<<endl<<"-2"<<endl; //header
    outfile << G.getNumNodes() << endl;
    const vector<string>* names = G.getNodeNames();
    for (const auto& name : *names) {
        outfile << "|{" << name << "}|" << endl;
    }
    outfile << G.getNumEdges() << endl;
    const vector<array<uint, 2>>* edges = G.getEdgeList();

    for (const auto& edge : *edges) {
        outfile << edge[0]+1 << " " << edge[1]+1 << " 0 |{"; //re-indexing to 1-based
        if (saveWeights) outfile << G.edgeWeight(edge[0], edge[1]) << endl;
        outfile << "}|" << endl;
    }
    outfile.close();
    exec("mv "+tempFile+" "+outFile);
}

void GraphLoader::saveInEdgeListFormat(const Graph& G, string outFile, bool weightColumn, bool namedEdges, 
                                       const string& headerLine, const string& sep) {
    ofstream ofs;
    ofs.open(outFile.c_str());
    if (headerLine.size() != 0) ofs<<headerLine<<endl;
    const vector<array<uint, 2>>* edges = G.getEdgeList();
    for (const auto& edge : *edges) {
        if (namedEdges) ofs<<G.getNodeName(edge[0])<<sep<<G.getNodeName(edge[1]);
        else ofs<<edge[0]<<sep<<edge[1];
        if (weightColumn) ofs<<sep<<G.edgeWeight(edge[0], edge[1]);
        ofs<<endl;
    }
    ofs.close();
}