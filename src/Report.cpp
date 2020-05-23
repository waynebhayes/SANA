#include "Report.hpp"
#include "utils/utils.hpp"
#include "utils/randomSeed.hpp"
#include "utils/FileIO.hpp"
#include "arguments/GraphLoader.hpp"
#include "measures/EdgeCorrectness.hpp"
#include "measures/InducedConservedStructure.hpp"
#include "measures/SymmetricSubstructureScore.hpp"
#include "measures/JaccardSimilarityScore.hpp"
#include "measures/NodeCorrectness.hpp"

void Report::saveReport(const Graph& G1, const Graph& G2, const Alignment& A,
                        const MeasureCombination& M, const Method* method,
                        const string& reportFileName, bool longVersion) {
    Timer T;
    T.start();
    string fileName = formattedFileName(reportFileName, "out", G1.getName(), G2.getName(), method, A);
    ofstream ofs(fileName);
    cout<<"Saving report as \""<<fileName<<"\""<<endl;

    //first line is the alignment itself
    for (uint i = 0; i < A.size(); i++) ofs<<A[i]<<" ";
    ofs<<endl;

    Timer T1;
    T1.start();
    const uint numCCsToPrint = 3;
    ofs << endl << currentDateTime() << endl << endl;
    ofs << "G1: " << G1.getName() << endl;
    if (longVersion) printGraphStats(G1, numCCsToPrint, ofs);
    ofs << "G2: " << G2.getName() << endl;
    if (longVersion) printGraphStats(G2, numCCsToPrint, ofs);

    if (method != NULL) {
        ofs << "Method: " << method->getName() << endl;
        method->describeParameters(ofs);
        ofs << endl << "execution time = " << method->getExecTime() << endl;
    }
    ofs << endl << "Seed: " << getRandomSeed() << endl;

    if (longVersion) {
        cout << "  printing stats done (" << T1.elapsedString() << ")" << endl;
        Timer T2;
        T2.start();

        ofs << endl << "Scores:" << endl;
        M.printMeasures(A, ofs);
        ofs << endl;

        if (M.containsMeasure("nc")) {
            auto NC = M.getMeasure("nc");
            auto ncByColors = ((NodeCorrectness*) NC)->evalByColor(A, G1, G2);
            if (ncByColors.size() > 1) {
                ofs << "NC by color:" << endl;
                for (auto p : ncByColors) {
                    ofs << p.first << ": " << p.second << endl;
                }
                ofs << endl;
            }
        }

        cout << "  printing scores done (" << T2.elapsedString() << ")" << endl;

        Graph CS = G1.graphIntersection(G2, *(A.getVector()));
        ofs << "Common subgraph:" << endl;
        printGraphStats(CS, numCCsToPrint, ofs);

        auto CCs = CS.connectedComponents();
        uint numCCs = CCs.size();
        int tableRows = min((uint) 5, numCCs)+2;
        vector<vector<string>> table(tableRows, vector<string> (9));
        table[0][0] = "Graph"; table[0][1] = "n"; table[0][2] = "m"; table[0][3] = "alig-edges";
        table[0][4] = "indu-edges"; table[0][5] = "EC";
        table[0][6] = "ICS"; table[0][7] = "S3"; table[0][8] = "JS";

        table[1][0] = "G1"; table[1][1] = to_string(G1.getNumNodes()); table[1][2] = to_string(G1.getNumEdges());
        table[1][3] = to_string(A.numAlignedEdges(G1, G2)); table[1][4] = to_string(G2.numEdgesInNodeInducedSubgraph(A.asVector()));
        table[1][5] = to_string(M.eval("ec",A));
        table[1][6] = to_string(M.eval("ics",A)); table[1][7] = to_string(M.eval("s3",A)); table[0][8] = to_string(M.eval("js", A));

        for (int i = 0; i < tableRows-2; i++) {
            Graph H = CS.nodeInducedSubgraph(CCs[i]);
            Alignment newA(CCs[i]);
            newA.compose(A);
            table[i+2][0] = "CCS_"+to_string(i); table[i+2][1] = to_string(H.getNumNodes());
            table[i+2][2] = to_string(H.getNumEdges());
            table[i+2][3] = to_string(newA.numAlignedEdges(H, G2));
            table[i+2][4] = to_string(G2.numEdgesInNodeInducedSubgraph(newA.asVector()));
            EdgeCorrectness ec(&H, &G2);
            table[i+2][5] = to_string(ec.eval(newA));
            InducedConservedStructure ics(&H, &G2);
            table[i+2][6] = to_string(ics.eval(newA));
            SymmetricSubstructureScore s3(&H, &G2);
            table[i+2][7] = to_string(s3.eval(newA));
            JaccardSimilarityScore js(&H, &G2);
            table[i+2][8] = to_string(js.eval(newA));
        }

        ofs << "Common connected subgraphs:" << endl;
        printTable(table, 2, ofs);
        ofs << endl;

        const bool PRINT_CCS = true;
        if (PRINT_CCS) {
            const uint EDGE_COUNT_DIST = 0;
            for (uint j=0; j < numCCs; j++) {
                ofs<<"CCS_"<<j<<" Alignment, local (distance 1 to "<<EDGE_COUNT_DIST<<") edge counts and s3 score"<<endl;
                const vector<uint>& nodes = CCs[j];
                if (nodes.size() < 2) break;
                for (uint i=0; i<nodes.size(); i++) {
                    ofs << G1.getNodeName(nodes[i]) << '\t' << G2.getNodeName(A[nodes[i]]);
                    for (uint d=1; d<=EDGE_COUNT_DIST; d++){
                        uint fullCount=0; 
                        vector<uint> V1 = G1.numEdgesAroundByLayers(nodes[i], d); 
                        for (uint j=0;j<d;j++) fullCount+= V1[j];
                        ofs << '\t' << fullCount;
                        fullCount=0; 
                        vector<uint> V2 = G2.numEdgesAroundByLayers(A[nodes[i]], d); 
                        for (uint j=0;j<d;j++) fullCount+= V2[j];
                        ofs << '\t' << fullCount;
                        vector<uint> localNodes(G1.nodesAround(nodes[i], d));
                        Graph H = CS.nodeInducedSubgraph(localNodes);
                        Alignment localA(localNodes);
                        localA.compose(A);
                        SymmetricSubstructureScore s3(&H, &G2);
                        ofs << '\t' << to_string(s3.eval(localA));
                    }
                    ofs << endl;
                }
            }
            cout << "  printing tables done (" << T2.elapsedString() << ")" << endl;
        }
    }

    //print the alignment using node names in a separate file
    string edgeListFile = FileIO::fileNameWithoutExtension(fileName)+".align";
    ofstream elOfs(edgeListFile);
    for (uint i = 0; i < A.size(); i++) elOfs<<G1.getNodeName(i)<<"\t"<<G2.getNodeName(A[i])<<endl;

    cout<<"Took "<<T.elapsed()<<" seconds to save the alignment and scores."<<endl;
}

void Report::saveLocalMeasures(const Graph& G1, const Graph& G2, const Alignment& A,
                    const MeasureCombination& M, const Method* method, const string& localMeasureFileName) {
    Timer T;
    T.start();
    if (M.getSumLocalWeight() <= 0) { //This is how needLocal is calculated in SANA.cpp
        cout << "No local measures provided, not writing local scores file." << endl;
        return;
    }
    string fileName = formattedFileName(localMeasureFileName, "localscores", 
                                       G1.getName(), G2.getName(), method, A);
    ofstream ofs(fileName);
    cout << "Saving local measure as \"" << localMeasureFileName << "\"" << endl;
    M.writeLocalScores(ofs, G1, G2, A);
    cout << "Took " << T.elapsed() << " seconds to save the alignment and scores." << endl;
}

string Report::formattedFileName(const string& outFileName, const string& extension, 
                    const string& G1Name, const string& G2Name, 
                    const Method* method, const Alignment& A) {
    string res;
    if (outFileName == "") {
        string gNames = G1Name+"_"+G2Name;
        res = "alignments/"+gNames+"/"+gNames+"_"+method->getName()+method->fileNameSuffix(A)+".txt";
        res = FileIO::addVersionNumIfFileAlreadyExists(res);
    } else {
        res = outFileName;
        string folder = FileIO::getFilePath(res);
        FileIO::createFolder(folder);
    }
    res += "."+extension; //is the double extension after .txt intended? -Nil
    ofstream ofs(res);
    if (not ofs.is_open()) {
        cout << "Problem saving " << res << " to specified folder. Saving to base project folder." << endl;
        res = res.substr(res.find_last_of("/")+1);
    }
    return res;
}

void Report::printGraphStats(const Graph& G, uint numCCsToPrint, ofstream& ofs) {
    ofs << "n    = " << G.getNumNodes() << endl;
    ofs << "m    = " << G.getNumEdges() << endl;
    auto CCs = G.connectedComponents();
    uint numCCs = CCs.size();
    ofs << "#connectedComponents = " << numCCs << endl;
    ofs << "Largest connectedComponents (nodes, edges) = ";
    for (uint i = 0; i < min(numCCsToPrint, numCCs); i++) {
        ofs << "(" << CCs[i].size() << ", " << G.numEdgesInNodeInducedSubgraph(CCs[i]) << ") ";
    }
    ofs << endl << endl;
}