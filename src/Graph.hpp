#ifndef GRAPH_H
#define GRAPH_H

#include <set>
#include <string>
#include <vector>
#include <utility>
#include <iostream>
#include <fstream>
#include <sstream>
#include <cstdlib>
#include <stdexcept>
#include <unordered_set>
#include <algorithm>
#include <random>
#include <queue>
#include <set>
#include <cmath>
#include <fstream>
#include <time.h>
#include <cassert>
#include <iterator>
#include "utils/utils.hpp"
#include "utils/Timer.hpp"
#include "computeGraphlets.hpp"
#include "utils/Matrix.hpp"


using namespace std;

class Graph {

public:

    static const int NODE_TYPE_GENE;  //= 1;
    static const int NODE_TYPE_MIRNA; // = 2;
    
    static const string AUTOGENEREATED_FILES_FOLDER;

    void setMaxGraphletSize(double number);
    static Graph& loadGraph(string name, Graph& g);
    static Graph& loadGraphFromPath(string path, string name, Graph& g, int multipartite = 1);
    static Graph& multGraph(string name, uint path, Graph& g);

    static void loadFromMultipartiteEdgeList(string fin, string graphName, Graph& g, int multipartite);
    static void loadFromEdgeListFile(string fin, string graphName, Graph& g, bool bipartite = false);
    static void loadFromLedaFile(string fin, string graphName, Graph& g, bool bipartite = false);
    static void loadFromLgfFile(string fin, string graphName, Graph& g, bool bipartite = false);
    static void loadFromGmlFile(string fin, string graphName, Graph& g, bool bipartite = false);
    static void loadFromGraphmlFile(string fin, string graphName, Graph& g, bool bipartite = false);
    static void loadFromCsvFile(string fin, string graphName, Graph& g, bool bipartite = false);


    static void loadGraphFromBinary(Graph& g, string graphName, string lockFile, bool bipartite, bool lockedSameName);
    static void serializeGraph(Graph& G, string outputName, bool bipartite, bool locked);
    void serializeShadow(Graph& G);

    static void saveInGWFormat(string outputFile, const vector<string>& nodeNames,
        const vector<vector<uint>>& edgeList);
    static void saveInGWFormatShuffled(string outputFile, const vector<string>& nodeNames,
        const vector<vector<uint>>& edgeList);

    static void edgeList2gw(string fin, string fout);

    static void GeoGeneDuplicationModel(uint numNodes, uint numEdges, string outputFile);

    //void returnmaxSize(double max);
    //creates the null graph
    Graph();
    Graph(const Graph& G);
    //double maxsize;

    Graph(uint n, const vector<vector<uint> > edgeList);

    string getName() const;

    uint getNumNodes() const;
#ifdef MULTI_PAIRWISE
    uint getWeightedNumEdges();
#endif
    uint getNumEdges() const;
    const vector<vector<uint> >& getConnectedComponents() const;
    uint getNumConnectedComponents() const;

#ifndef NO_ADJ_MATRIX
    void getMatrix(Matrix<MATRIX_UNIT>& matrix) const;
    void setMatrix(Matrix<MATRIX_UNIT>& matrix);
    // TODO: make const
    Matrix<MATRIX_UNIT>& getMatrix();
#endif

    const vector<vector<uint>>& getAdjLists() const;
    const vector<vector<uint>>& getEdgeList() const;

    void getAdjLists(vector<vector<uint> >& adjListsCopy) const;
    void getEdgeList(vector<vector<uint> > & edgeListCopy) const;

    void setAdjLists(vector<vector<uint> >& adjListsCopy);
    void setEdgeList(vector<vector<uint> >& edgeListCopy);

    vector<string> getNodeNames() const;

    //loads graph from file in GraphWin (.gw) format:
    //http://www.algorithmic-solutions.info/leda_guide/graphs/leda_native_graph_fileformat.html
    //note: it does not parse correctly files with comments or blank lines.
    void loadGwFile(const string& fileName);
    void multGwFile(const string& fileName, uint path);

    //nodes are relabeled so that the new i-th node is the node nodes[i]-th in this
    Graph nodeInducedSubgraph(const vector<uint>& nodes) const;

    uint numNodeInducedSubgraphEdges(const vector<uint>& subgraphNodes) const;

    vector<uint> numEdgesAround(uint node, uint maxDist) const;
    vector<uint> getAllNodesAround(uint node, uint maxDist) const;
    vector<uint> numNodesAround(uint node, uint maxDist) const;

    void printStats(int numConnectedComponentsToPrint, ostream& stream) const;

    void writeGraphEdgeListFormat(const string& fileName);
    void writeGraphEdgeListFormatNETAL(const string& fileName);
    void writeGraphEdgeListFormatPISWAP(const string& fileName);
    void writeGraphEdgeListFormatPINALOG(const string& fileName);

    void addRandomEdges(double addedEdgesProportion);
    void removeRandomEdges(double removedEdgesProportion);
    void rewireRandomEdges(double rewiredEdgesProportion);

    uint randomNode();

    vector<vector<uint> > loadGraphletDegreeVectors();

    unordered_map<string,uint> getNodeNameToIndexMap() const;
    unordered_map<uint,string> getIndexToNodeNameMap() const;

    bool hasSelfLoop(uint source) const;

    void getDistanceMatrix(vector<vector<short> >& dist) const;

    vector<uint> degreeDistribution() const;

    double getAverageDistance() const;

    void saveInGWFormat(string outputFile);
    void saveInGWFormatWithNames(string outputFile);
    void saveInShuffledOrder(string outputFile);
    void saveGraphletsAsSigs(string outputFile);

    Graph randomNodeInducedSubgraph(uint numNodes);
    Graph randomNodeShuffle(vector<uint> &shuffle);

    bool isWellDefined();

    bool sameNodeNames(const Graph& other) const;

    // For locking
    void setLockedList(vector<string>& nodes, vector<string>& pairs);
    vector<bool>& getLockedList();
    bool isLocked(uint index) const;
    string getLockedTo(uint index);
    int getLockedCount();

    unordered_map<uint, uint> getLocking_ReIndexMap() const;
    unordered_map<uint, uint> getBipartiteNodeTypes_ReIndexMap() const;

    void reIndexGraph(unordered_map<uint, uint> reIndexMap);  // Changes the node indexes according to the map

    //string getBipartiteNodeType(uint i);
    int getBipartiteNodeType(uint i);
    bool isBipartite();
    int getNumMultipartite();

    //vector<string> nodeTypes;
    vector<int> nodeTypes;
    vector<int> nodeTypesCount;
    bool bipartiteEnabled = false;
    uint geneCount = 0;
    uint miRNACount = 0;
    int unlockedGeneCount = -1;
    int unlockedmiRNACount = -1;
    vector<uint> geneIndexList;
    vector<uint> miRNAIndexList;
    vector<vector<uint>> typedNodesIndexList;
    void removeEdge(uint node1, uint node2);
    
    int multipartite = 1;

    // TODO: make const
    Matrix<float>& getFloatWeights() ;
    bool hasFloatWeight() const;
    
    static string autogenFilesFolder();
    static string serializedFilesFolder();
private:
    bool parseFloatWeight = false;
    Matrix<float> floatWeights;
    double maxGraphletSize = 4; //default is 4, 5 is too big
    string name;
    string path;
    //double maxsize;
    vector<vector<uint> > edgeList; //edges in no particular order
#ifndef NO_ADJ_MATRIX
    Matrix<MATRIX_UNIT> matrix;
#endif
    vector<vector<uint> > adjLists; //neighbors in no particular order

    //list of the nodes of each connected component, sorted from larger to smaller
    vector<vector<uint> > connectedComponents;
    //int maxsize;

    // NOTE: these don't change after reIndexing G1 (method #3 of locking),
    // and they are used to reIndex the graph to normal at the end.
    vector<bool> lockedList;  // shows which nodes are locked
    vector<string> lockedTo;  // name of node we lock to in other graph
    int weightedNumEdges = 0;
    int lockedCount = 0;


    unordered_map<string,uint> nodeNameToIndexMap;

    void updateUnlockedGeneCount();

    void initConnectedComponents();

    void addEdge(uint node1, uint node2);
    void addRandomEdge();
    void removeRandomEdge();
    
    vector<vector<uint> > computeGraphletDegreeVectors();

    //places in dist a matrix with the distance between every pair of nodes (a -1 indicates infinity)
    void computeDistanceMatrix(vector<vector<short> >& dist) const;

    //serialization
    friend class cereal::access;
    template<class Archive>
    void serialize(Archive& archive)
    {
        archive(CEREAL_NVP(adjLists), CEREAL_NVP(matrix), CEREAL_NVP(edgeList), CEREAL_NVP(lockedList),
                CEREAL_NVP(lockedTo), CEREAL_NVP(nodeTypes), CEREAL_NVP(miRNACount), CEREAL_NVP(geneCount),
                CEREAL_NVP(connectedComponents), CEREAL_NVP(unlockedGeneCount), CEREAL_NVP(unlockedmiRNACount),
                CEREAL_NVP(lockedCount), CEREAL_NVP(geneIndexList), CEREAL_NVP(miRNAIndexList), CEREAL_NVP(nodeNameToIndexMap));
    }
};
#endif