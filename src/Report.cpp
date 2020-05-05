#include "Report.hpp"
#include "arguments/GraphLoader.hpp"
#include "measures/EdgeCorrectness.hpp"
#include "measures/InducedConservedStructure.hpp"
#include "measures/SymmetricSubstructureScore.hpp"
#include "measures/JaccardSimilarityScore.hpp"
#include "utils/utils.hpp"
#include "utils/randomSeed.hpp"

bool multiPairwiseIteration; //todo: refactor without this out here -Nil

void Report::makeReport(const Graph& G1, const Graph& G2, const Alignment& A,
                const MeasureCombination& M, Method* method,
                ofstream& stream, bool multiPairwiseIteration = false) {
    Timer T1;
    T1.start();
    const uint numCCsToPrint = 3;
    stream << endl << currentDateTime() << endl << endl;
    stream << "G1: " << G1.getName() << endl;
    if (!multiPairwiseIteration) {
        printStats(G1, numCCsToPrint, stream);
        stream << endl;
    }
    stream << "G2: " << G2.getName() << endl;
    if (!multiPairwiseIteration) {
        printStats(G2, numCCsToPrint, stream);
        stream << endl;
    }
    if (method != NULL) {
        stream << "Method: " << method->getName() << endl;
        method->describeParameters(stream);
        stream << endl << "execution time = " << method->getExecTime() << endl;
    }
    stream << endl << "Seed: " << getRandomSeed() << endl;

    if (!multiPairwiseIteration) {
        cout << "  printing stats done (" << T1.elapsedString() << ")" << endl;
        Timer T2;
        T2.start();

        stream << endl << "Scores:" << endl;
        M.printMeasures(A, stream);
        stream << endl;

        cout << "  printing scores done (" << T2.elapsedString() << ")" << endl;
        Timer T3;
        T3.start();

        Graph CS = G1.graphIntersection(G2, *(A.getVector()));
        stream << "Common subgraph:" << endl;
        printStats(CS, numCCsToPrint, stream);
        stream << endl;

        int tableRows = min((uint) 5, CS.getNumConnectedComponents())+2;
        vector<vector<string> > table(tableRows, vector<string> (9));
        table[0][0] = "Graph"; table[0][1] = "n"; table[0][2] = "m"; table[0][3] = "alig-edges";
        table[0][4] = "indu-edges"; table[0][5] = "EC";
        table[0][6] = "ICS"; table[0][7] = "S3"; table[0][8] = "JS";

        table[1][0] = "G1"; table[1][1] = to_string(G1.getNumNodes()); table[1][2] = to_string(G1.getNumEdges());
        table[1][3] = to_string(A.numAlignedEdges(G1, G2)); table[1][4] = to_string(G2.numEdgesInNodeInducedSubgraph(A.asVector()));
        table[1][5] = to_string(M.eval("ec",A));
        table[1][6] = to_string(M.eval("ics",A)); table[1][7] = to_string(M.eval("s3",A)); table[0][8] = to_string(M.eval("js", A));

        for (int i = 0; i < tableRows-2; i++) {
            const vector<uint>& nodes = (*(CS.getConnectedComponents()))[i];
            Graph H = CS.nodeInducedSubgraph(nodes);
            Alignment newA(nodes);
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

        stream << "Common connected subgraphs:" << endl;
        printTable(table, 2, stream);
        stream << endl;

        const bool PRINT_CCS = true;
        if (PRINT_CCS) {
            const uint EDGE_COUNT_DIST = 0;
            for (uint cc=0; cc < CS.getNumConnectedComponents(); cc++) {
                stream << "CCS_" << cc << " Alignment, local (distance 1 to " << EDGE_COUNT_DIST << ") edge counts and s3 score\n";
                const vector<uint>& nodes = (*(CS.getConnectedComponents()))[cc];
                if (nodes.size() < 2) break;
                for (uint i=0; i<nodes.size(); i++) {
                    stream << G1.getNodeName(nodes[i]) << '\t' << G2.getNodeName(A[nodes[i]]);
                    for (uint d=1; d<=EDGE_COUNT_DIST; d++){
                        uint fullCount=0; 
                        vector<uint> V1 = G1.numEdgesAroundByLayers(nodes[i], d); 
                        for (uint j=0;j<d;j++) fullCount+= V1[j];
                        stream << '\t' << fullCount;
                        fullCount=0; 
                        vector<uint> V2 = G2.numEdgesAroundByLayers(A[nodes[i]], d); 
                        for (uint j=0;j<d;j++) fullCount+= V2[j];
                        stream << '\t' << fullCount;
                        vector<uint> localNodes(G1.nodesAround(nodes[i], d));
                        Graph H = CS.nodeInducedSubgraph(localNodes);
                        Alignment localA(localNodes);
                        localA.compose(A);
                        SymmetricSubstructureScore s3(&H, &G2);
                        stream << '\t' << to_string(s3.eval(localA));
                    }
                    stream << endl;
                }
            }
            cout << "  printing tables done (" << T2.elapsedString() << ")" << endl;
        }
    }
}

void Report::saveReport(const Graph& G1, const Graph& G2, const Alignment& A,
        const MeasureCombination& M, Method* method, string reportFileName) {
    saveReport(G1, G2, A, M, method, reportFileName, false);
}

void Report::saveReport(const Graph& G1, const Graph& G2, const Alignment& A,
                        const MeasureCombination& M, Method* method,
                        string reportFileName, bool multiPairwiseIteration) {
    Timer T;
    T.start();
    ofstream outfile;
    reportFileName = ensureFileNameExistsAndOpenOutFile("report", reportFileName, outfile,
                                                    G1.getName(), G2.getName(), method, A);
    cout<<"Saving report as \""<<reportFileName<<"\""<<endl;

    for (uint i = 0; i < A.size(); i++) outfile<<A[i]<<" ";
    outfile<<endl;

    string elAligFile = reportFileName.substr(0,reportFileName.length()-4)+".align";
    ofstream ofs;
    ofs.open(elAligFile.c_str());
    for (uint i = 0; i < A.size(); i++) ofs<<G1.getNodeName(i)<<"\t"<<G2.getNodeName(A[i])<<endl;
    ofs.close();

    makeReport(G1, G2, A, M, method, outfile, multiPairwiseIteration);
    outfile.close();
    cout<<"Took "<<T.elapsed()<<" seconds to save the alignment and scores."<<endl;
}

void Report::saveLocalMeasures(const Graph& G1, const Graph& G2, const Alignment& A,
    const MeasureCombination& M, const Method* method, string& localMeasureFileName) {
    Timer T;
    T.start();
    if (M.getSumLocalWeight() <= 0) { //This is how needLocal is calculated in SANA.cpp
        cout << "No local measures provided, not writing local scores file." << endl;
        return;
    }
    ofstream outfile;
    ensureFileNameExistsAndOpenOutFile("local measure", localMeasureFileName, outfile, 
                                       G1.getName(), G2.getName(), method, A);
    cout << "Saving local measure as \"" << localMeasureFileName << "\"" << endl;
    M.writeLocalScores(outfile, G1, G2, A);
    outfile.close();
    cout << "Took " << T.elapsed() << " seconds to save the alignment and scores." << endl;
}

/*"Ensure" here means ensure that there is a valid file to output to.
NOTE: the && is a move semantic, which moves the internal pointers of one object
to another and then destructs the original, instead of destructing all of the
internal data of the original. */
string Report::ensureFileNameExistsAndOpenOutFile(const string& fileType, string outFileName, 
                    ofstream& outfile, const string& G1Name, const string& G2Name, 
                    const Method* method, Alignment const & A) {     
    string extension = fileType == "local measure" ? ".localscores" : ".out";
    if (outFileName == "") {
        outFileName = "alignments/" + G1Name + "_" + G2Name + "/"
        + G1Name + "_" + G2Name + "_" + method->getName() + method->fileNameSuffix(A);
        addUniquePostfixToFilename(outFileName, ".txt");
        outFileName += ".txt";
    } else {
        string location = outFileName.substr(0, outFileName.find_last_of("/"));
        if (location != outFileName) {
            uint lastPos = 0;
            while (not folderExists(location)) { //Making each of the folders, one by one.
                createFolder(location.substr(0, location.find("/", lastPos)));
                lastPos = location.find("/", location.find("/", lastPos)+1);
            }
        }
    }
    outFileName += extension;
    outfile.open(outFileName.c_str());
    if (not outfile.is_open()) {
        cout << "Problem saving " << fileType << " file to specified location. Saving to sana program file." << endl;
        outFileName = outFileName.substr(outFileName.find_last_of("/")+1);
        outfile.open(outFileName.c_str());
    }
    return outFileName;
}

void Report::printStats(const Graph& G, uint numCCsToPrint, ostream& stream) {
    stream << "n    = " << G.getNumNodes() << endl;
    stream << "m    = " << G.getNumEdges() << endl;
    uint numCCs = G.getNumConnectedComponents();
    stream << "#connectedComponents = " << numCCs << endl;
    stream << "Largest connectedComponents (nodes, edges) = ";
    for (uint i = 0; i < min(numCCsToPrint, numCCs); i++) {
        const vector<uint>& CC = (*(G.getConnectedComponents()))[i];
        stream << "(" << CC.size() << ", " << G.numEdgesInNodeInducedSubgraph(CC) << ") ";
    }
    stream << endl;
}