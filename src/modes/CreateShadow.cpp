#include "CreateShadow.hpp"
#include <cassert>
#include <utility>
#include <iostream>
#include <vector>
#include <string>
#include <array>
#include <algorithm>
#include "../utils/utils.hpp"
#include "../utils/Matrix.hpp"
#include "../Graph.hpp"
#include "../Alignment.hpp"
#include "../arguments/GraphLoader.hpp"

using namespace std;

void CreateShadow::run(ArgumentParser& args) {
#ifndef MULTI_PAIRWISE
    assert(false and "create shadow should be compiled in MULTI_PAIRWISE");
#else 
    //parse input
    stdiobuf sbuf = readFileAsStreamBuffer(args.strings["-fshadow"]);
    istream ifs(&sbuf);
    string outFile;
    ifs >> outFile;
    uint numDummy, k;
    ifs >> numDummy >> k;
    vector<string> graphFiles(k), aligFiles(k), colorFiles(k);
    bool hasAlignments, hasColors;
    for (uint i = 0; i < k; i++) ifs >> graphFiles[i]; 
    ifs >> hasAlignments;
    if (hasAlignments) for (uint i = 0; i < k; i++) ifs >> aligFiles[i];
    ifs >> hasColors;
    if (hasColors) for (uint i = 0; i < k; i++) ifs >> colorFiles[i];

    //figure out the number of nodes in the shadow graph
    uint numShadowNodes = 0;
    for (string file : graphFiles) {
        Graph G = GraphLoader::loadGraphFromFile("", file, false);
        if(G.getNumNodes() > numShadowNodes) numShadowNodes = G.getNumNodes();
    }
    numShadowNodes += numDummy;

    //generate the shadow names, which we assume to follow the format "shad_{i}" in the alignment files
    vector<string> shadNodeNames;
    shadNodeNames.reserve(numShadowNodes);
    for (uint i = 0; i < numShadowNodes; i++) shadNodeNames.push_back("shad_"+to_string(i));

    //shadNbrSets[i][j] is the weight of edge (i,j), assuming i<=j
    vector<unordered_map<uint, EDGE_T>> shadNbrSets(numShadowNodes);

    //add all the edges from all the graphs to the adj matrix
    for (uint i = 0; i < k; i++) {
        Graph G = GraphLoader::loadGraphFromFile("", graphFiles[i], false);

        //figure out the mapping from the graph to the shadow graph
        unordered_map<string, string> gNameToSName;
        gNameToSName.reserve(G.getNumNodes());
        if (hasAlignments) {
            //this expects that the first column of aligFiles containsevery node in G,
            //and the second column follows the format "shad_{i}" and contains unique nodes
            vector<array<string, 2>> namePairs = GraphLoader::rawTwoColumnFileData(aligFiles[i]);
            for (const auto& p : namePairs) gNameToSName[p[0]] = p[1];
        } else { //use a random mapping
            vector<string> shuffledShadNames = shadNodeNames;
            random_shuffle(shuffledShadNames.begin(), shuffledShadNames.end());
            for (uint i = 0; i < G.getNumNodes(); i++)
                gNameToSName[G.getNodeName(i)] = shuffledShadNames[i];
        }

        //add the edges
        for (const auto& edge : (*G.getEdgeList())) {
            string gName1 = G.getNodeName(edge[0]), gName2 = G.getNodeName(edge[1]);
            string sName1 = gNameToSName[gName1], sName2 = gNameToSName[gName2];
            uint shadNode1 = stoi(sName1.substr(5)), shadNode2 = stoi(sName2.substr(5));
            if (shadNode1 > shadNode2) swap(shadNode1, shadNode2); //to avoid double-counting
            shadNbrSets[shadNode1][shadNode2]++; //auto-inserts shadNode2 if missing
        }
    }

    //prepare the data for the graph constructor
    uint numShadEdges = 0;
    for (const auto& nbrSet : shadNbrSets) numShadEdges += nbrSet.size();
    vector<array<uint, 2>> shadEdgeList;
    vector<EDGE_T> shadEdgeWeights;
    shadEdgeList.reserve(numShadEdges);
    shadEdgeWeights.reserve(numShadEdges);
    for (uint node = 0; node < numShadowNodes; node++) {
        for (const auto& kv : shadNbrSets[node]) {
            uint nbr = kv.first;
            EDGE_T w = kv.second;
            shadEdgeList.push_back({node, nbr});
            shadEdgeWeights.push_back(w);
        }
    }
    Graph shadow("shadow", "", shadEdgeList, shadNodeNames, shadEdgeWeights, {});

    GraphLoader::saveInGWFormat(shadow, outFile, true);

    //todo: handle colors

#endif /* MULTI_PAIRWISE */
}

string CreateShadow::getName() { return "CreateShadow"; }

