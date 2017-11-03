#include "GHOSTWrapper.hpp"

using namespace std;

const string CONVERTER = "./bio-graph";
const string GHOSTBinary = "./GHOST";
const string GLOBAL_PARAMETERS = "linear all 1.0 8.0 10"; // do not modify these, they are hard-coded parsed below

GHOSTWrapper::GHOSTWrapper(Graph* G1, Graph* G2, string args): WrappedMethod(G1, G2, "GHOST", args) {
    wrappedDir = "wrappedAlgorithms/GHOST";
    parameters = args;
}

// -wrappedArgs "matcher nneighbors beta ratio searchiter"
void GHOSTWrapper::loadDefaultParameters() {
}

string GHOSTWrapper::convertAndSaveGraph(Graph* graph, string name) {
    string gwFile   = name + ".gw";
    string gexfFile = name + ".gexf";

    graph->saveInGWFormatWithNames(gwFile);

    exec("mv " + gwFile + " " + wrappedDir);
    exec("cd " + wrappedDir + " && chmod +x " + CONVERTER);
    exec("bash -c \"cd " + wrappedDir + " && " + CONVERTER + " --convert " + gwFile + " --output " + gexfFile + "\" 2>/dev/null");
    exec("mv " + wrappedDir + "/" + gexfFile + " " + gexfFile);
    exec("cd " + wrappedDir + " && ln -s SIGS/" + G1->getName() + ".sig.gz " + g1TmpName + ".sig.gz 2>/dev/null");
    exec("cd " + wrappedDir + " && ln -s SIGS/" + G2->getName() + ".sig.gz " + g2TmpName + ".sig.gz 2>/dev/null");

    return gexfFile;
}

void GHOSTWrapper::createCfgFile(string cfgFileName) {
    vector<string> G_PARAMS = split(GLOBAL_PARAMETERS, ' ');
    vector<string> params = split(parameters, ' ');

    matcher    = G_PARAMS[0];
    nneighbors = G_PARAMS[1];
    beta       = G_PARAMS[2];
    ratio      = G_PARAMS[3];
    searchiter = G_PARAMS[4];

    ofstream outfile;
    outfile.open(cfgFileName.c_str());

    outfile << "[main]" << endl;
    outfile << "network1: " << g1File << endl;
    outfile << "network2: " << g2File << endl;
    outfile << "sigs1: " << g1TmpName << ".sig.gz" << endl;
    outfile << "sigs2: " << g2TmpName << ".sig.gz" << endl;
    outfile << "matcher: "  << matcher << endl;
    outfile << "nneighbors: "  << nneighbors << endl;
    outfile << "beta: " << beta << endl;
    outfile << "ratio: " << ratio << endl;
    outfile << "searchiter: " << searchiter << endl;
    // outfile << "dumpDistances: true " << endl; // to dump huge 3-column nxm matrix of all-by-all node similarities
    for(uint i=0;i<params.size();i+=2) {
        if(params[i][0] == '-')
        params[i].erase(0,1);
        outfile << params[i] << ": " << params[i+1] << endl;
    }

    outfile.close();
}

string GHOSTWrapper::generateAlignment() {
    cfgFile = g1TmpName + ".cfg";
    createCfgFile(wrappedDir + "/" + cfgFile);
    string af = g1TmpName+"_vs_"+g2TmpName;
    string name1 = G1->getName();
    string name2 = G2->getName();

    execPrintOutput("cd " + wrappedDir + "; " + GHOSTBinary + " -c " + cfgFile);
    execPrintOutput("cd " + wrappedDir + "; ./int2string.sh 0 2 ../../networks/"+name1+"/"+name1+".gw ../../networks/"+name2+"/"+name2+".gw "+ af + ".af > " + af + ".align");
    return wrappedDir + "/" + af + ".align";
}

Alignment GHOSTWrapper::loadAlignment(Graph* G1, Graph* G2, string fileName) {
    return Alignment::loadPartialEdgeList(G1,G2,fileName, true); // true = byName
}

void GHOSTWrapper::deleteAuxFiles() {
//    exec("cd " + wrappedDir + "; rm " + g1File + " " + g2File);
}
