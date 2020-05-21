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
#include "../utils/FileIO.hpp"
#include "../Alignment.hpp"

using namespace std;

pair<Graph,Graph> GraphLoader::initGraphs(ArgumentParser& args) {
    cout << "Initializing graphs..." << endl;
    Timer T;
    T.start();
    string fg1 = args.strings["-fg1"], fg2 = args.strings["-fg2"];
    string g1Name, g2Name, g1File, g2File;
    if (fg1 != "") {
        g1Name = FileIO::extractFileNameNoExtension(fg1);
        g1File = fg1;
    } else {
        g1Name = args.strings["-g1"];
        g1File = "networks/"+g1Name+"/"+g1Name+".gw";
    }
    if (fg2 != "") {
        g2Name = FileIO::extractFileNameNoExtension(fg2);
        g2File = fg2;
    } else {
        g2Name = args.strings["-g2"];
        g2File = "networks/"+g2Name+"/"+g2Name+".gw";
    }

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

    auto futureG1 = async(&loadGraphFromFile, g1Name, g1File, g1HasWeights);
    auto futureG2 = async(&loadGraphFromFile, g2Name, g2File, g2HasWeights);
    Graph G1 = futureG1.get();
    Graph G2 = futureG2.get();
    cout << "Graph loading completed in " << T.elapsedString() << endl;

    if (G1.getNumNodes() > G2.getNumNodes())
        throw runtime_error("G1 has more nodes than G2. Please swap the graphs");

    //colors or locking
    string fcolors1 = args.strings["-fcolor1"], fcolors2 = args.strings["-fcolor2"];
    string lockFile = args.strings["-lock"];
    bool g1HasColorFile = fcolors1 != "", g2HasColorFile = fcolors2 != "";
    bool lockPairs = lockFile != "";
    bool lockSameNames = args.bools["-lock-same-names"];
    //At most one option can be used:
    if (lockPairs and lockSameNames) throw runtime_error("\"-lock\" and \"-lock-same-names\" are incompatible. Use only one");
    if (lockPairs and (g1HasColorFile or g2HasColorFile)) throw runtime_error("\"-lock\" and color files are incompatible. Use only one");
    if (lockSameNames and (g1HasColorFile or g2HasColorFile)) throw runtime_error("\"-lock-same-names\" and color files are incompatible. Use only one");

    vector<array<string, 2>> g1Colors, g2Colors;
    if (lockPairs) {
        cout<<"Locking node pairs in file "<<lockFile<<endl;
        array<vector<array<string, 2>>, 2> nodeColorLists =
            nodeColorListsFromLockFile(lockFile);
        g1Colors = nodeColorLists[0], g2Colors = nodeColorLists[1];
    } else if (lockSameNames) {
        cout<<"Locking nodes with the same name"<<endl;
        array<vector<array<string, 2>>, 2> nodeColorLists =
            nodeColorListsFromCommonNames(G1.commonNodeNames(G2));
        g1Colors = nodeColorLists[0], g2Colors = nodeColorLists[1];
    } else {
        if (g1HasColorFile) g1Colors = RawColorFileData(fcolors1).nodeColorList;
        if (g2HasColorFile) g2Colors = RawColorFileData(fcolors2).nodeColorList;
    }
    if (g1Colors.empty() and g2Colors.empty() and G1.getNumNodes() > G2.getNumNodes()) {
        throw runtime_error("Please swap G1 and G2. G2 should have more nodes.");
    }
    G1.initColorDataStructs(g1Colors);
    G2.initColorDataStructs(g2Colors);

    const string COLOR_USAGE = 
        "For example, if you are aligning two virus-host networks, then the colors " \
        "should be 'virus' and 'host'; using species names won't work, because for example a node " \
        "of color 'mouse' cannot align to a node of color 'human'. Call both 'host'.";
    if (G1.numColors() > G2.numColors())
        throw runtime_error("some G1 nodes have a color non-existent in G2, so there is no valid alignment. "+COLOR_USAGE);
    else if (G1.numColors() < G2.numColors())
        throw runtime_error("some G2 nodes have a color non-existent in G1. "+COLOR_USAGE);

    //add dummy nodes to G2 to guarantee an alignment exists
    unordered_map<string,uint> colToNumDummies;
    for (const string& colName : *(G1.getColorNames())) {
        uint g1Count = G1.numNodesWithColor(G1.getColorId(colName));
        uint g2Count;
        if (not G2.hasColor(colName)) g2Count = 0;
        else g2Count = G2.numNodesWithColor(G2.getColorId(colName));
        if (g1Count > g2Count) colToNumDummies[colName] = g1Count - g2Count;
    }
    if (not colToNumDummies.empty()) {
        vector<string> g2NodeNames = *(G2.getNodeNames());
        g2NodeNames.reserve(g2NodeNames.size() + colToNumDummies.size());
        uint numDummiesWithDefCol = (colToNumDummies.count(Graph::DEFAULT_COLOR_NAME) ?
                                            colToNumDummies[Graph::DEFAULT_COLOR_NAME] : 0);
        g2Colors.reserve(g2Colors.size() + colToNumDummies.size() - numDummiesWithDefCol);
        for (const auto& kv : colToNumDummies) {
            string colName = kv.first;
            uint count = kv.second;
            cerr<<"adding "<<count<<" dummies colored "<<colName<<" to G2"<<endl;
            for (uint i = 0; i < count; i++) {
                string dummyName = "dummy_"+to_string(i);
                if (colName != Graph::DEFAULT_COLOR_NAME) dummyName += "_"+colName;
                g2NodeNames.push_back(dummyName);
                if (colName != Graph::DEFAULT_COLOR_NAME) {
                    g2Colors.push_back({dummyName, colName});
                }
            }
        }
        vector<EDGE_T> g2Weights;
        if (g2HasWeights) {
            g2Weights.reserve(G2.getNumEdges());
            for (const auto& edge : *(G2.getEdgeList()))
                g2Weights.push_back(G2.getEdgeWeight(edge[0], edge[1]));
        }
        G2 = Graph(G2.getName(), G2.getFilePath(), *(G2.getEdgeList()), g2NodeNames, g2Weights, g2Colors);
    }

    string path1 = args.strings["-pathmap1"], path2 = args.strings["-pathmap2"];
    if (path1 != "") {
        uint power1 = stoi(path1);
        if (power1 > 1) {
            cout << "Raising G1 to the " << power1 << " power" << endl;
            G1 = G1.graphPower(power1);
        }
    }
    if (path2 != "") {
        uint power2 = stoi(path2);
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
    vector<uint> G1ToG2Map = A.asVector();
    G2 = pruneG1FromG2(G1, G2, G1ToG2Map);
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

    assert(G1.isWellDefined());
    assert(G2.isWellDefined());
    G1.debugPrint();
    G2.debugPrint();

    cout << "Total time for loading graphs (" << T.elapsedString() << ")" << endl;
    return {G1, G2};
}

void GraphLoader::saveInGWFormat(const Graph& G, const string& outFile, bool saveWeights) {

    //a comment explaining why a temp file is necessary (and why only
    //in this function) would be nice -Nil
    string tempFile = "/tmp/saveInGWFormat"+to_string(getpid()); 

    ofstream ofs(tempFile);
    ofs << "LEDA.GRAPH" << endl << "string" << endl << "short" << endl << "-2" << endl;
    ofs << G.getNumNodes() << endl;
    for (const auto& name : *(G.getNodeNames())) ofs << "|{" << name << "}|" << endl;
    ofs << G.getNumEdges() << endl;
    for (const auto& edge : *(G.getEdgeList())) {
        ofs << edge[0]+1 << " " << edge[1]+1 << " 0 |{"; //re-indexing to 1-based
        if (saveWeights) ofs << +G.getEdgeWeight(edge[0], edge[1]); //the + makes it print as a number even if it has type char/bool
        ofs << "}|" << endl;
    }
    exec("mv "+tempFile+" "+outFile);
}

void GraphLoader::saveInEdgeListFormat(const Graph& G, const string& outFile, bool weightColumn, bool namedEdges, 
                                       const string& headerLine, const string& sep) {
    ofstream ofs(outFile);
    if (headerLine.size() != 0) ofs<<headerLine<<endl;
    const vector<array<uint, 2>>* edges = G.getEdgeList();
    for (const auto& edge : *edges) {
        if (namedEdges) ofs<<G.getNodeName(edge[0])<<sep<<G.getNodeName(edge[1]);
        else ofs<<edge[0]<<sep<<edge[1];
        if (weightColumn) ofs<<sep<<+G.getEdgeWeight(edge[0], edge[1]); //the + makes it print as a number even if it has type char/bool
        ofs<<endl;
    }
}

void GraphLoader::saveTwoColumnData(const vector<array<string, 2>>& rows, const string& outFile) {
    ofstream ofs(outFile);
    for (const auto& row : rows) ofs<<row[0]<<" "<<row[1]<<endl;
}

//dispatches to one of the format-specific functions based on the file extension
Graph GraphLoader::loadGraphFromFile(const string& graphName, const string& fileName,
                                     bool loadWeights) {
    string format = fileName.substr(fileName.find_last_of('.')+1);
    string uncompressedFileExt = FileIO::getUncompressedFileExtension(fileName);

    if (loadWeights and (format == "gml" or format == "lgf" or format == "xml" or format == "csv"))
        throw runtime_error("GraphLoader does not support weights for format '"+format+"'");
    //for dbg:
    //cerr<<graphName<<" "<<fileName<<" "<<loadWeights<<endl<<format<<" "<<uncompressedFileExt;
    if (format == "gw" || uncompressedFileExt == "gw")
        return loadGraphFromGWFile(graphName, fileName, loadWeights);
    if (format == "el" || uncompressedFileExt == "el" || format == "elw" || uncompressedFileExt == "elw")
        return loadGraphFromEdgeListFile(graphName, fileName, loadWeights);
    if (format == "gml") return loadGraphFromGmlFile(graphName, fileName);
    if (format == "lgf") return loadGraphFromLgfFile(graphName, fileName);
    if (format == "xml") return loadGraphFromXmlFile(graphName, fileName);
    if (format == "csv") return loadGraphFromCsvFile(graphName, fileName);
    if (format == "mpel") throw runtime_error(
        "Multipartite edge list format (MPEL) no longer supported. Use coloring feature instead");
    throw runtime_error("Unsupported graph format: " + format);
}

Graph GraphLoader::loadGraphFromGWFile(const string& graphName, const string& fileName, 
                                       bool loadWeights) {
    RawGWFileData gwData(fileName, loadWeights);
    //reindex 1-based edges to 0-based edges
    for (uint i = 0; i < gwData.edgeList.size(); i++) {
        gwData.edgeList[i][0]--;
        gwData.edgeList[i][1]--;
    }

    if (!loadWeights)
        return Graph(graphName, fileName, gwData.edgeList, gwData.nodeNames, {}, {});
#ifdef BOOL_EDGE_T
    throw runtime_error("cannot load weights for unweighted graph");
#elif FLOAT_WEIGHTS
    throw runtime_error("cannot load float weights from GW file");
#else
    vector<EDGE_T> edgeWeights;
    edgeWeights.reserve(gwData.edgeWeights.size());
    for (int w : gwData.edgeWeights) {
        assert(w > 0 and "graph cannot have negative weights");
        assert(w < (1L << 8*sizeof(EDGE_T)) -1 and "EDGE_T type is not wide enough for these weights");
        edgeWeights.push_back((EDGE_T) w);
    }
    return Graph(graphName, fileName, gwData.edgeList, gwData.nodeNames, edgeWeights, {});
#endif
}

Graph GraphLoader::loadGraphFromEdgeListFile(const string& graphName, const string& fileName, 
                                             bool loadWeights) {
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

    RawEdgeListFileData edgeListData(fileName, weightType);
    auto edgeAndNodeNameLists =
        namedEdgeListToEdgeListAndNodeNameList(edgeListData.namedEdgeList);

    if (not loadWeights)
        return Graph(graphName, fileName, edgeAndNodeNameLists.first,
                     edgeAndNodeNameLists.second, {}, {});

#ifdef BOOL_EDGE_T
    throw runtime_error("cannot load weights for unweighted graph");
#elif FLOAT_WEIGHTS
    return Graph(graphName, fileName, edgeAndNodeNameLists.first, edgeAndNodeNameLists.second, 
                 edgeListData.floatWeights, {});
#else
    vector<EDGE_T> edgeWeights;
    edgeWeights.reserve(edgeListData.intWeights.size());
    for (int w : edgeListData.intWeights) {
        assert(w > 0 and "graph cannot have negative weights");
        assert(w < (1L << 8*sizeof(EDGE_T)) -1 and "EDGE_T type is not wide enough for these weights");
        edgeWeights.push_back((EDGE_T) w);
    }
    return Graph(graphName, fileName, edgeAndNodeNameLists.first, edgeAndNodeNameLists.second, 
                 edgeWeights, {});
#endif
}

Graph GraphLoader::loadGraphFromGmlFile(const string& gName, const string& file) {
    RawGmlFileData gmlData(file);
    auto edgesAndNames = namedEdgeListToEdgeListAndNodeNameList(gmlData.namedEdgeList);
    return Graph(gName, file, edgesAndNames.first, edgesAndNames.second, {}, {});
}
Graph GraphLoader::loadGraphFromLgfFile(const string& gName, const string& file) {
    RawLgfFileData lgfData(file);
    auto edgesAndNames = namedEdgeListToEdgeListAndNodeNameList(lgfData.namedEdgeList);
    return Graph(gName, file, edgesAndNames.first, edgesAndNames.second, {}, {});
}
Graph GraphLoader::loadGraphFromXmlFile(const string& gName, const string& file) {
    RawXmlFileData xmlData(file);
    auto edgesAndNames = namedEdgeListToEdgeListAndNodeNameList(xmlData.namedEdgeList);
    return Graph(gName, file, edgesAndNames.first, edgesAndNames.second, {}, {});
}
Graph GraphLoader::loadGraphFromCsvFile(const string& gName, const string& file) {
    RawCsvFileData csvData(file);
    auto edgesAndNames = namedEdgeListToEdgeListAndNodeNameList(csvData.namedEdgeList);
    return Graph(gName, file, edgesAndNames.first, edgesAndNames.second, {}, {});
}

GraphLoader::RawGWFileData::RawGWFileData(const string& fileName, bool containsEdgeWeights) {
    FileIO::checkFileExists(fileName);
    stdiobuf sbuf = FileIO::readFileAsStreamBuffer(fileName);
    istream ifs(&sbuf);
    string line;
    //ignore header
    for (uint i = 0; i < 4; i++) FileIO::safeGetLine(ifs, line);
    //read number of nodes
    int n;
    if (FileIO::safeGetLine(ifs, line)) {
        n = stoi(line);
        if (n <= 0) throw runtime_error("Failed to read non-zero node number: "+line);
    } else throw runtime_error("Failed to read line with node number");
    //read nodes
    nodeNames.reserve(n);
    for (uint i = 0; i < (uint) n; i++) {
        if (FileIO::safeGetLine(ifs, line)) {
            if (line.size() <= 4) throw runtime_error("Failed to read node name from line: "+line);
            string node = line.substr(2,line.size()-4); //strip |{ and }|
            nodeNames.emplace_back(node);
        } else throw runtime_error("Failed to read all nodes");
    }
    //read number of edges
    int m;
    if (FileIO::safeGetLine(ifs, line)) {
        m = stoi(line);
        istringstream iss2(line);
        if (m <= 0) throw runtime_error("Failed to read non-zero edge number: "+line);        
    } else throw runtime_error("Failed to read line with edge number");
    //read edges
    edgeList.reserve(m);
    if (containsEdgeWeights) edgeWeights.reserve(m);
    for (int i = 0; i < m; ++i) {
        if (FileIO::safeGetLine(ifs, line)) {
            istringstream iss(line);
            uint node1, node2;
            if (iss >> node1 >> node2)
                edgeList.push_back({node1, node2});
            else throw runtime_error("Failed to read edge from line: "+line);
            if (containsEdgeWeights) {
                int w;
                char dumpChar;
                if (iss >> dumpChar >> dumpChar >> dumpChar >> w)
                    edgeWeights.push_back(w);
                else throw runtime_error("Failed to read edge weight from line: "+line);
            }
        } else throw runtime_error("Failed to read all edges");
    }
    if (FileIO::safeGetLine(ifs, line) and line.size() != 0)
        cerr<<"Warning: ignoring leftover lines at the end of .gw file: "<<line<<endl;
}

GraphLoader::RawEdgeListFileData::RawEdgeListFileData(
            const string& fileName, const string& weightType) {
    FileIO::checkFileExists(fileName);
    uint numLines = FileIO::numLinesInFile(fileName);
    namedEdgeList.reserve(numLines);                                       
    if (weightType == "int") intWeights.reserve(numLines);
    else if (weightType == "float") floatWeights.reserve(numLines);
    else if (weightType != "") throw runtime_error("weightType cannot be "+weightType);

    stdiobuf sbuf = FileIO::readFileAsStreamBuffer(fileName);
    istream ifs(&sbuf);
    for (string line; FileIO::safeGetLine(ifs, line); ) {
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

GraphLoader::RawGmlFileData::RawGmlFileData(const string& fileName) {
    stdiobuf sbuf = FileIO::readFileAsStreamBuffer(fileName);
    istream ifs(&sbuf);
    FileIO::skipWordInStream(ifs, "graph");
    FileIO::skipWordInStream(ifs, "[");
    while (FileIO::canSkipWordInStream(ifs, "edge")) {
        string name1, name2;
        FileIO::skipWordInStream(ifs, "[");
        FileIO::skipWordInStream(ifs, "source");
        ifs >> name1;
        FileIO::skipWordInStream(ifs, "target");
        ifs >> name2;
        FileIO::skipWordInStream(ifs, "]");
        namedEdgeList.push_back({name1, name2});
    }
    //does not check for closing bracket for "graph [ ... ]"
}

GraphLoader::RawLgfFileData::RawLgfFileData(const string& fileName) {
    stdiobuf sbuf = FileIO::readFileAsStreamBuffer(fileName);
    istream ifs(&sbuf);
    FileIO::skipWordInStream(ifs, "@edges");
    FileIO::skipWordInStream(ifs, "label");
    FileIO::skipWordInStream(ifs, "weight");
    string name1, name2;
    int waste1, waste2;
    while (ifs >> name1) {
        ifs >> name2 >> waste1 >> waste2;
        namedEdgeList.push_back({name1, name2});
    }
}

GraphLoader::RawXmlFileData::RawXmlFileData(const string& fileName) {
    stdiobuf sbuf = FileIO::readFileAsStreamBuffer(fileName);
    istream ifs(&sbuf);
    string line;
    string srcPrefix = "source=\"", trgPrefix = "target=\"";
    for (uint i = 0; i < 5; i++) FileIO::safeGetLine(ifs, line); //skip headers
    while (FileIO::canSkipWordInStream(ifs, "<edge")) {
        string waste, source, target;
        ifs >> waste >> source >> target;
        source = source.substr(srcPrefix.size()); //remove 'source="'
        target = target.substr(trgPrefix.size()); //remove 'target="'
        source.pop_back(); //remove closing '"'
        target.pop_back(); target.pop_back(); target.pop_back(); //remove closing '"/>'
        namedEdgeList.push_back({source, target});
    }
    //does not check for closing of the graph tag
}

GraphLoader::RawCsvFileData::RawCsvFileData(const string& fileName) {
    stdiobuf sbuf = FileIO::readFileAsStreamBuffer(fileName);
    istream ifs(&sbuf);
    string line;
    while(FileIO::safeGetLine(ifs, line)) {
        string source, target;
        size_t sep = line.find(";");
        if (sep == string::npos) continue;
        source = line.substr(0,sep);
        target = line.substr(sep+1);
        namedEdgeList.push_back({source, target});
    }
}

GraphLoader::RawColorFileData::RawColorFileData(const string& fileName) {
    nodeColorList = rawTwoColumnFileData(fileName);
}
GraphLoader::RawLockFileData::RawLockFileData(const string& fileName) {
    nodePairList = rawTwoColumnFileData(fileName);
}

vector<array<string, 2>> GraphLoader::rawTwoColumnFileData(const string& fileName) {
    FileIO::checkFileExists(fileName);
    stdiobuf sbuf = FileIO::readFileAsStreamBuffer(fileName);
    istream ifs(&sbuf);
    vector<array<string, 2>> res;
    res.reserve(FileIO::numLinesInFile(fileName));
    for (string line; FileIO::safeGetLine(ifs, line); ) {
        if (line.size() == 0) continue;
        string col1, col2;
        istringstream iss(line);
        if (iss >> col1 >> col2)
            res.push_back({col1, col2});
        else throw runtime_error("failed to read column pair from line: "+line+" in file: "+fileName);
    }
    return res;
}

pair<vector<array<uint, 2>>, vector<string>> GraphLoader::namedEdgeListToEdgeListAndNodeNameList(
        const vector<array<string, 2>>& namedEdgeList) {
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

array<vector<array<string, 2>>, 2> GraphLoader::nodeColorListsFromLockFile(
        const string& lockFile) {
    vector<array<string, 2>> lockedPairs = RawLockFileData(lockFile).nodePairList;
    array<vector<array<string, 2>>, 2> res;
    res[0].reserve(lockedPairs.size());
    res[1].reserve(lockedPairs.size());
    for (uint i = 0; i < lockedPairs.size(); i++) {
        //give color "i" to the i-th locked pair
        string g1Node = lockedPairs[i][0], g2Node = lockedPairs[i][1];
        string color_i = "lock_"+to_string(i);
        res[0].push_back({g1Node, color_i});
        res[1].push_back({g2Node, color_i});
    }
    return res;
}

array<vector<array<string, 2>>, 2> GraphLoader::nodeColorListsFromCommonNames(
        const vector<string>& commonNames) {
    array<vector<array<string, 2>>, 2> res;
    res[0].reserve(commonNames.size());
    res[1].reserve(commonNames.size());
    for (uint i = 0; i < commonNames.size(); i++) {
        //give color "i" to the i-th name
        string color_i = to_string(i);
        res[0].push_back({commonNames[i], color_i});
        res[1].push_back({commonNames[i], color_i});
    }
    return res;
}

Graph GraphLoader::pruneG1FromG2(const Graph& G1, const Graph& G2, const vector<uint>& G1ToG2NodeMap) {
#ifndef MULTI_PAIRWISE
    throw runtime_error("prunning only implemented for multi pairwise");
#else
    if (G1ToG2NodeMap.size() != G1.getNumNodes())
        throw runtime_error("G1ToG2NodeMap size ("+to_string(G1ToG2NodeMap.size())+
                            ") not same as G1's number of nodes ("+to_string(G1.getNumNodes())+")");
    //subtract the weights from the edges in 'G1' from a copy of G2's adjacency matrix
    auto adjMat = *(G2.getAdjMatrix());
    uint numEdgesDownTo0 = 0;
    for (const auto& g1Edge : *(G1.getEdgeList())) {
        assert(G1.getEdgeWeight(g1Edge[0], g1Edge[1]) == 1 and "G1 is not unweighted");
        uint g2Node1 = G1ToG2NodeMap.at(g1Edge[0]), g2Node2 = G1ToG2NodeMap.at(g1Edge[1]);
        assert(g2Node1 < G2.getNumNodes() and g2Node2 < G2.getNumNodes());
        assert(G2.getEdgeWeight(g2Node1, g2Node2) >= 1 and "edge cannot be pruned");            
        assert(G2.getEdgeWeight(g2Node1, g2Node2) == adjMat[g2Node1][g2Node2] and "edge has already been pruned");
        adjMat[g2Node1][g2Node2] -= 1;
        adjMat[g2Node2][g2Node1] -= 1;
        if (adjMat[g2Node1][g2Node2] == 0) numEdgesDownTo0++;        
    }
    //keep the edges in G2's edge list that still have positive weight
    vector<array<uint, 2>> newEdgeList;
    vector<EDGE_T> newEdgeWeights;
    uint newNumEdges = G2.getNumEdges() - numEdgesDownTo0;
    newEdgeList.reserve(newNumEdges);
    newEdgeWeights.reserve(newNumEdges);
    for (const auto& g2Edge : *(G2.getEdgeList())) {
        EDGE_T newWeight = adjMat.get(g2Edge[0], g2Edge[1]);
        assert(newWeight >= 0);
        if (newWeight > 0) {
            newEdgeList.push_back({g2Edge[0], g2Edge[1]});
            newEdgeWeights.push_back(newWeight);
        }
    }
    return Graph(G2.getName(), G2.getFilePath(), newEdgeList, *(G2.getNodeNames()), 
                 newEdgeWeights, G2.colorsAsNodeColorNamePairs());
#endif /* MULTI_PAIRWISE */
}

