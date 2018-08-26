#include "HubAlignWrapper.hpp"
#include "../../measures/localMeasures/Sequence.hpp"

using namespace std;

const string wrappedDir = "wrappedAlgorithms/HubAlign/";
const string HubAlignWrapper::hubalignProgram = "./HubAlign";

HubAlignWrapper::HubAlignWrapper(Graph* G1, Graph* G2, double alpha): Method(G1, G2, "HubAlign"),
    alpha(alpha), g1Name(G1->getName()), g2Name(G2->getName()) {

    createFolder("sequence");
    createFolder("sequence/bitscores");
    similarityFile = "sequence/bitscores/" + g1Name + "_" + g2Name + ".bitscores";
#if 1 // TEMPORRAY CODE UNTIL WE INHERIT FROM WrappedMethod
    string TMP = "_tmp" + intToString(randInt(0, 2100000000)) + "_";
    string g1TmpName = "HubAlign" + TMP + g1Name;
    string g2TmpName = "HubAlign" + TMP + g2Name;
    string alignmentTmpName = g1TmpName+"-"+g2TmpName;
#endif
    //rand int used to avoid collision between parallel runs
    //these files cannot be moved to the tmp/ folder
    g1EdgeListFile = g1TmpName;
    g2EdgeListFile = g2TmpName;
    //this file cannot be moved to the tmp/ folder
    alignmentFile = alignmentTmpName + ".alignment";
}

void HubAlignWrapper::generateEdgeListFile(int graphNum) {
    Graph* G;
    if (graphNum == 1) G = G1;
    else G = G2;

    vector<vector<uint> > edgeList;
    G->getEdgeList(edgeList);
    unordered_map<uint,string> names = G->getIndexToNodeNameMap();
    uint m = G->getNumEdges();
    vector<vector<string> > edgeListNames(m, vector<string> (2));
    for (uint i = 0; i < m; i++) {
        edgeListNames[i][0] = names[edgeList[i][0]];
        edgeListNames[i][1] = names[edgeList[i][1]];
    }

    //to avoid the case where aligning a network to itself
    //the true mapping is 0,1,2,3,4,...
    randomShuffle(edgeListNames);

    string file = graphNum == 1 ? g1EdgeListFile : g2EdgeListFile;
    writeDataToFile(edgeListNames, wrappedDir+file, true);
}

//Examples of executions of HubAlign (if alpha==1 sim file is not needed)
//./HubAlign Test1.tab Test2.tab –l 0.1 –a 0.7 –d 10 –b similarityFile.txt
//./HubAlign Test1.tab Test2.tab –l 0.1 –a 1
void HubAlignWrapper::generateAlignment() {
    //in hubalign alpha is the fraction of topology
    double reversedAlpha = 1 - alpha;

    //hubalign has a bug where it doesn't work if it
    //tries to optimize only sequence, hence this
    //ad hoc fix
    if (reversedAlpha == 0) reversedAlpha = 0.0001;

    string cmd = "cd "+wrappedDir+";"+hubalignProgram + " " + g1EdgeListFile + " " + g2EdgeListFile;
    cmd += " -l 0.1 -a " + to_string(reversedAlpha);
    cmd += " -d 10 ";
    if (reversedAlpha < 1) {
        cmd += " -b " + similarityFile;
    }
    cout << "Executing " << cmd << endl;
    execPrintOutput(cmd);
    cout << "Done" << endl;
}

void HubAlignWrapper::deleteAuxFiles() {
    string evalFile = g1EdgeListFile + "-" + g2EdgeListFile + ".eval";
    //exec("cd "+wrappedDir+";rm " + g1EdgeListFile + " " + g2EdgeListFile + " " + evalFile + " " + alignmentFile);
}

Alignment HubAlignWrapper::run() {
    if (alpha > 0 and not fileExists(similarityFile)) {
        Sequence sequence(G1, G2);

        sequence.generateBitscoresFile(similarityFile);
    }
    generateEdgeListFile(1);
    generateEdgeListFile(2);

    generateAlignment();
    Alignment A = Alignment::loadEdgeList(G1, G2, wrappedDir+alignmentFile);
    deleteAuxFiles();
    return A;
}

void HubAlignWrapper::describeParameters(ostream& stream) {
    stream << "alpha: " << alpha << endl;
    if (alpha > 0) {
        stream << "Similarity file: " << similarityFile << endl;
    }
}

string HubAlignWrapper::fileNameSuffix(const Alignment& A) {
    if (alpha == 1) return "_alpha_1";
    else return "_alpha_0" + extractDecimals(alpha,1);
}
