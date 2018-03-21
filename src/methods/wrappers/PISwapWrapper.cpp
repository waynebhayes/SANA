#include "PISwapWrapper.hpp"

using namespace std;

const string CONVERTER = "";
const string PISwapSeqBinary = "python piswap.py";
const string alignmentconverter = "python convertOutput.py";
const string pyCreator = "gen-piswap-nohungarian.sh";
const string usingAlignmentPyName = "piswap-nohungarian";
const string PISwapNoHungarianBinary = "python " + usingAlignmentPyName;

PISwapWrapper::PISwapWrapper(Graph* G1, Graph* G2, double alpha, string startingAlignment, string args): WrappedMethod(G1, G2, "PISWAP", args) {
    startingAligName = startingAlignment;
    this->alpha = 1-alpha;
    wrappedDir = "wrappedAlgorithms/PISWAP";
}

void PISwapWrapper::loadDefaultParameters() {
    cout << "ERROR: PISWAP needs a similarity file (using sequence) whether or not you are optimizing using sequence or using the Hungarian algorithm. Use -wrappedArgs \"similarityFile\" to specify the similarity file. An additional starting alignment file can be specified with -startalignment \".align file\" to simply improve an alignment and not use the Hungarian algorithm. Also make sure to specify alpha, 1 is all topology and 0 is all sequence." << endl;
    exit(-1);
    parameters = "";
    
}

string PISwapWrapper::convertAndSaveGraph(Graph* graph, string name) {
    graph->writeGraphEdgeListFormatPISWAP(name + ".tab");
    return name + ".tab";
}

string PISwapWrapper::generateAlignment() {
    if(startingAligName == ""){
        execPrintOutput("cd " + wrappedDir + "; " + PISwapSeqBinary + " " +  g1File + " " + g2File + " ../../" + parameters + " " + to_string(alpha));
    }else{
        execPrintOutput("cd " + wrappedDir + "; ./" + pyCreator + " ../../" + startingAligName + " > " + usingAlignmentPyName + TMP + ".py");
        execPrintOutput("cd " + wrappedDir + "; " + PISwapNoHungarianBinary + TMP + ".py " + g1File + " " + g2File + " ../../" + parameters + " " + to_string(alpha)); 
    }
    execPrintOutput("cd " + wrappedDir + "; " + alignmentconverter + " match_output.txt " + alignmentTmpName);

    return wrappedDir + "/" + alignmentTmpName;
}

Alignment PISwapWrapper::loadAlignment(Graph* G1, Graph* G2, string fileName) {
    return Alignment::loadEdgeListUnordered(G1, G2, fileName);
}

void PISwapWrapper::deleteAuxFiles() {
    exec("cd " + wrappedDir + "; rm -f " + g1File + " " + g2File + " " + usingAlignmentPyName + TMP + ".py " + usingAlignmentPyName + TMP + ".pyc");
}//probably the plan is to create a new piswap.py based on inputs no matter what.
