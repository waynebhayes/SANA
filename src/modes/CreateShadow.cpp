#include "CreateShadow.hpp"
#include <cassert>
#include <utility>
#include <iostream>
#include <vector>
#include <string>
#include <unordered_map>
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
#endif
    //parse input
    stdiobuf sbuf = readFileAsStreamBuffer(args.strings["-fshadow"]);
    istream ifs(&sbuf);
    string outFile, outColorFile;
    bool hasAligs, hasColors;
    uint numDummies, k;
    ifs >> outFile >> numDummies >> k;
    vector<string> graphFiles(k), aligFiles(k), colorFiles(k);
    for (uint i = 0; i < k; i++) ifs >> graphFiles[i]; 
    ifs >> hasAligs;
    if (hasAligs) for (uint i = 0; i < k; i++) ifs >> aligFiles[i];
    ifs >> hasColors;
    if (hasColors) {
        for (uint i = 0; i < k; i++) ifs >> colorFiles[i];
        ifs >> outColorFile;
    }
    createShadow(outFile, numDummies, graphFiles,
                 hasAligs, aligFiles, 
                 hasColors, colorFiles, outColorFile);
}

void CreateShadow::createShadow(const string& outFile, uint numDummies,
        const vector<string>& graphFiles, 
        bool hasAligs, const vector<string>& aligFiles,
        bool hasColors, const vector<string>& colorFiles, const string& outColorFile) {

    //figure out the colors and the number of nodes for each color
    //for each color in any of the graphs, the number of nodes in the shadow graph of that color
    //should be the maximum number of nodes of that color in any graph + numDummies
    //dummies are added at the end
    unordered_map<string, uint> colToNodeCount;
    for (uint i = 0; i < graphFiles.size(); i++) {
        Graph G = GraphLoader::loadGraphFromFile("", graphFiles[i], false);
        //if hasColors is false, every node gets the default color, which is a valid color
        if (hasColors) {
            auto nodeColorPairs = GraphLoader::rawTwoColumnFileData(colorFiles[i]);
            G.initColorDataStructs(nodeColorPairs);
        }
        for (const string& gColor : *(G.getColorNames())) {
            uint gColorCount = G.numNodesWithColor(G.getColorId(gColor));
            if (colToNodeCount[gColor] < gColorCount) colToNodeCount[gColor] = gColorCount;
        }
    }
    for (const auto& kv : colToNodeCount) colToNodeCount[kv.first] += numDummies;

    //generate the node names, which are called "shad_{i}"
    vector<string> shadNodeNames;
    uint numShadNodes = 0;
    for (const auto& kv : colToNodeCount) numShadNodes += colToNodeCount[kv.first];
    shadNodeNames.reserve(numShadNodes);
    for (uint i = 0; i < numShadNodes; i++) shadNodeNames.push_back("shad_"+to_string(i));

    //assign colors to the names in alphabetical order of the colors
    //to get always the same mapping from node names to color names
    vector<string> shadNodeColors;
    shadNodeColors.reserve(numShadNodes);
    vector<string> orderedColNames;
    for (const auto& kv : colToNodeCount) orderedColNames.push_back(kv.first);
    sort(orderedColNames.begin(), orderedColNames.end());
    uint nodeColInd = 0;
    for (const string& col : orderedColNames) {
        for (uint i = 0; i < colToNodeCount[col]; i++) {
            shadNodeColors[nodeColInd] = col;
            nodeColInd++;
        }
    }
    assert(nodeColInd == shadNodeColors.size());

    //put the color names in the appropriate format for the graph constructor
    vector<array<string, 2>> nodeColorPairs;        
    nodeColorPairs.reserve(numShadNodes);
    for (uint i = 0; i < numShadNodes; i++) {
        if (shadNodeColors[i] != Graph::DEFAULT_COLOR_NAME) 
            nodeColorPairs.push_back({shadNodeNames[i], shadNodeColors[i]});
    }

    //data structure to "accumulate" the edges from the graphs
    //shadNbrSets[i][j] stores the weight of edge (i,j), assuming i<=j
    vector<unordered_map<uint, EDGE_T>> shadNbrSets(numShadNodes);

    //add all the edges from all the graphs to shadNbrSets

    //we don't have the edges yet, this is just to create random alignments if needed
    Graph emptyShadow("", "", {}, shadNodeNames, {}, nodeColorPairs);

    for (uint i = 0; i < graphFiles.size(); i++) {
        Graph G = GraphLoader::loadGraphFromFile("", graphFiles[i], false);
        if (hasColors) {
            auto nodeColorPairs = GraphLoader::rawTwoColumnFileData(colorFiles[i]);
            G.initColorDataStructs(nodeColorPairs);
        }

        //figure out the mapping from G to the shadow graph
        unordered_map<string, string> gNameToSName;
        gNameToSName.reserve(G.getNumNodes());
        if (hasAligs) {
            //this expects that the first column of aligFiles containsevery node in G,
            //and the second column follows the format "shad_{i}" and contains unique nodes
            vector<array<string, 2>> namePairs = GraphLoader::rawTwoColumnFileData(aligFiles[i]);
            for (const auto& p : namePairs) gNameToSName[p[0]] = p[1];
        } else { //use a random mapping
            Alignment A = Alignment::randomColorRestrictedAlignment(G, emptyShadow);
            for (uint i = 0; i < G.getNumNodes(); i++)
                gNameToSName[G.getNodeName(i)] = emptyShadow.getNodeName(A[i]);
        }

        //sanity check - verify that the alignment maps colors correctly
        for (uint gNode = 0; gNode < G.getNumNodes(); gNode++) {
            string gNodeName = G.getNodeName(gNode);
            string gNodeColor = G.getColorName(G.getNodeColor(gNode));
            string sNodeName = gNameToSName[gNodeName];
            uint sNode = emptyShadow.getNameIndex(sNodeName);
            string sNodeColor = emptyShadow.getColorName(emptyShadow.getNodeColor(sNode));
            assert (gNodeColor == sNodeColor);
        }

        //add the edges
        for (const auto& edge : (*G.getEdgeList())) {
            string gName1 = G.getNodeName(edge[0]), gName2 = G.getNodeName(edge[1]);
            string sName1 = gNameToSName[gName1], sName2 = gNameToSName[gName2];
            uint shadNode1 = stoi(sName1.substr(5)), shadNode2 = stoi(sName2.substr(5)); //"shad_{i} -> i"
            if (shadNode1 > shadNode2) swap(shadNode1, shadNode2); //to avoid double-counting
            shadNbrSets[shadNode1][shadNode2]++; //auto-inserts shadNode2 if missing
        }
    }

    //put the edges and weights in the appropriate format for the graph constructor
    vector<array<uint, 2>> shadEdgeList;
    vector<EDGE_T> shadEdgeWeights;
    uint numShadEdges = 0;
    for (const auto& nbrSet : shadNbrSets) numShadEdges += nbrSet.size();
    shadEdgeList.reserve(numShadEdges);
    shadEdgeWeights.reserve(numShadEdges);
    for (uint node = 0; node < numShadNodes; node++) {
        for (const auto& kv : shadNbrSets[node]) {
            uint nbr = kv.first;
            auto weight = kv.second;
            shadEdgeList.push_back({node, nbr});
            shadEdgeWeights.push_back(weight);
        }
    }

    //finally, build and save the shadow
    Graph shadow("", "", shadEdgeList, shadNodeNames, shadEdgeWeights, nodeColorPairs);
    GraphLoader::saveInGWFormat(shadow, outFile, true);
    if (hasColors) {
        //the colors never change after the first iteration of multi pairwise, so we could
        //store them only when hasAligs is false. this optimizationi is turned off:
        // if (hasAligs) continue;

        GraphLoader::saveTwoColumnData(nodeColorPairs, outColorFile);
    }
}

string CreateShadow::getName() { return "CreateShadow"; }

