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
#include "methods/SANA.hpp"

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
    string edgeListFileName = FileIO::fileNameWithoutExtension(fileName)+".align";
    saveAlignmentAsEdgeList(A, G1, G2, edgeListFileName);

    cout<<"Took "<<T.elapsed()<<" seconds to save the alignment and scores."<<endl;
}

void Report::saveAlignmentAsEdgeList(const Alignment& A, const Graph& G1, const Graph& G2, const string& fileName) {
    //sort the color names from least common to most common
    vector<uint> g1ColorIds;
    g1ColorIds.reserve(G1.numColors());
    for (uint colId = 0; colId < G1.numColors(); colId++) g1ColorIds.push_back(colId);
    sort(g1ColorIds.begin(), g1ColorIds.end(), [&G1](const uint& id1, const uint& id2) {
        return G1.numNodesWithColor(id1) < G1.numNodesWithColor(id2);
    });

    ofstream ofs(fileName);
    for (uint colId : g1ColorIds) {
        for (uint node : *(G1.getNodesWithColor(colId))) {
            ofs<<G1.getNodeName(node)<<"\t"<<G2.getNodeName(A[node])<<endl;
        }
    }
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

void Report::saveCoreScores(const Graph& G1, const Graph& G2, const Alignment& A, const Method* method,
        Matrix<unsigned long>& pegHoleFreq, vector<unsigned long>& numPegSamples,
        Matrix<double>& weightedPegHoleFreq_pBad, vector<double>& totalWeightedPegWeight_pBad,
        Matrix<double>& weightedPegHoleFreq_1mpBad, vector<double>& totalWeightedPegWeight_1mpBad,
        Matrix<double>& weightedPegHoleFreq_pwPBad, vector<double>& totalWeightedPegWeight_pwPBad,
        Matrix<double>& weightedPegHoleFreq_1mpwPBad, vector<double>& totalWeightedPegWeight_1mpwPBad,
        const string& outputFileName)
{
    Timer T;
    T.start(); 
    string fileName = formattedFileName(outputFileName, "naf", G1.getName(), G2.getName(), method, A);
    ofstream ofs(fileName); 
    cout << "Saving core scores as \""<< fileName << "\'" << endl;
#ifdef UNWEIGHTED_CORES
    double SminUnW = SANA::TrimCoreScores(pegHoleFreq,numPegSamples);
    ofs << "# Smin_UnW "<< SminUnW << endl;
#endif
    double Smin_pBad = SANA::TrimCoreScores(weightedPegHoleFreq_pBad, totalWeightedPegWeight_pBad);
    double Smin_1mpBad = SANA::TrimCoreScores(weightedPegHoleFreq_1mpBad, totalWeightedPegWeight_1mpBad);
    double Smin_pwPBad = SANA::TrimCoreScores(weightedPegHoleFreq_pwPBad, totalWeightedPegWeight_pwPBad);
    double Smin_1mpwPBad = SANA::TrimCoreScores(weightedPegHoleFreq_1mpwPBad, totalWeightedPegWeight_1mpwPBad);
    ofs << "# Smin_pBad "<< Smin_pBad << endl <<  "# Smin_(1-pBad) " << Smin_1mpBad << endl;
    ofs << "# Smin_pwPBad "<< Smin_pwPBad << endl <<  "# Smin_(1-pwPBad) " << Smin_1mpwPBad << endl;
    ofs << "# p1\tp2";
#ifdef UNWEIGHTED_CORES
    ofs << "\tunwgtd";
#endif
    ofs << "\twpB\tw1-pB\twpwPB\tw1-pwPB"<< endl;
    for (uint i=0; i<G1.getNumNodes(); i++) for (uint j=0; j<G2.getNumNodes(); j++) {
#ifdef UNWEIGHTED_CORES
        double unweightedScore = pegHoleFreq[i][j]/(double)numPegSamples[i];
#endif
        double weightedScore_pBad = weightedPegHoleFreq_pBad[i][j]/totalWeightedPegWeight_pBad[i];
        double weightedScore_1mpBad = weightedPegHoleFreq_1mpBad[i][j]/totalWeightedPegWeight_1mpBad[i];
        double weightedScore_pwPBad = weightedPegHoleFreq_pwPBad[i][j]/totalWeightedPegWeight_pwPBad[i];
        double weightedScore_1mpwPBad = weightedPegHoleFreq_1mpwPBad[i][j]/totalWeightedPegWeight_1mpwPBad[i];
        const double MIN_CORE_SCORE = 1e-4;
        if (
#ifdef UNWEIGHTED_CORES
        unweightedScore  >= max(MIN_CORE_SCORE,SminUnW) ||
#endif
        weightedScore_pBad >= max(MIN_CORE_SCORE,Smin_pBad) ||
        weightedScore_1mpBad >= max(MIN_CORE_SCORE,Smin_1mpBad) ||
        weightedScore_pwPBad >= max(MIN_CORE_SCORE,Smin_pwPBad) ||
        weightedScore_1mpwPBad >= max(MIN_CORE_SCORE,Smin_1mpwPBad)
        ) {
            ofs << (G1.getNodeName(i)).c_str() << "\t" << (G2.getNodeName(j)).c_str() << "\t" << setprecision(6) <<
#ifdef UNWEIGHTED_CORES
                unweightedScore << "\t" <<
#endif
                weightedScore_pBad << "\t" << weightedScore_1mpBad << "\t" << weightedScore_pwPBad << "\t" << weightedScore_1mpwPBad <<endl;
        }
    }
    cout << "Took " << T.elapsed() << " seconds to save the core scores." << endl;
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
