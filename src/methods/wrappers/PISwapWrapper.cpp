#include <vector>
#include <iostream>
#include <sstream>
#include "PISwapWrapper.hpp"

using namespace std;

const string CONVERTER = "";
const string PISwapSeqBinary = "python piswap.py";
const string alignmentconverter = "python convertOutput.py";
const string pyCreator = "gen-piswap-noseq.sh";
const string usingAlignmentPyName = "piswapNoSeq.py";
const string PISwapNoSeqBinary = "python " + usingAlignmentPyName;

PISwapWrapper::PISwapWrapper(Graph* G1, Graph* G2, double alpha, string startingAlignment, string args): WrappedMethod(G1, G2, "PISWAP", args) {
    startingAligName = startingAlignment;
    this->alpha = alpha;
    if(startingAligName != "" and args != ""){
        cerr << "ERROR: You have specified wrappedargs and a starting alignment for PISWAP. Please only do one or the other; the starting alignment for using topology or the wrappedargs for a sequence file." << endl;
        exit(-1);
    }
    wrappedDir = "wrappedAlgorithms/PISWAP";
}

void PISwapWrapper::loadDefaultParameters() {
    if(startingAligName == ""){
        cerr << "ERROR: PISWAP needs a similarity file (using sequence) or an input .align file (not using sequence). Use -wrappedArgs \"similarityFile\" to specify the similarity file or -startalignment \".align file\" to specify the starting alignment file." << endl;
        exit(-1);
        parameters = "";
    }
}

string PISwapWrapper::convertAndSaveGraph(Graph* graph, string name) {
    graph->writeGraphEdgeListFormatPISWAP(name + ".tab");
    return name + ".tab";
}

string PISwapWrapper::generateAlignment() {
    if(startingAligName == ""){
        execPrintOutput("cd " + wrappedDir + "; " + PISwapSeqBinary + " " +  g1File + " " + g2File + " ../../" + parameters);
    }else{
        execPrintOutput("cd " + wrappedDir + "; ./" + pyCreator + " ../../" + startingAligName + " > " + usingAlignmentPyName);
        execPrintOutput("cd " + wrappedDir + "; " + PISwapNoSeqBinary + " " + g1File + " " + g2File); 
    }
    execPrintOutput("cd " + wrappedDir + "; " + alignmentconverter + " match_output.txt " + alignmentTmpName);

    return wrappedDir + "/" + alignmentTmpName;
}

Alignment PISwapWrapper::loadAlignment(Graph* G1, Graph* G2, string fileName) {
    return Alignment::loadEdgeList(G1, G2, fileName);
}

void PISwapWrapper::deleteAuxFiles() {
    exec("cd " + wrappedDir + "; rm " + g1File + " " + g2File);
}//probably the plan is to create a new piswap.py based on inputs no matter what.
