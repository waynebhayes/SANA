#include "SANAPISWAPWrapper.hpp"

using namespace std;

SANAPISWAPWrapper::SANAPISWAPWrapper(Graph* G1, Graph* G2, ArgumentParser args, MeasureCombination M): WrappedMethod(G1, G2, "SANAPISWAP", args.strings["-wrappedArgs"]){
    wrappedDir = "wrappedAlgorithms/PISWAP";
    Graph1 = G1;
    Graph2 = G2;
    this->M = M;
    sanaMethod = (SANA*)(initSANA(*G1, *G2, args, this->M));
    
    string outfile = args.strings["-o"];
    int location = outfile.find_last_of("/");
    intermediateAlignment = outfile.substr(0, location+1) + "SANA_PrePISWAP_" + outfile.substr(location+1);

    cout << "Saving intermediate alignment file to " << intermediateAlignment << endl;
    double alpha = args.doubles["-alpha"];
    string wrappedArgs = args.strings["-wrappedArgs"];

    piswapMethod = new PISwapWrapper(Graph1, Graph2, alpha, intermediateAlignment + ".align", wrappedArgs);
}

Alignment SANAPISWAPWrapper::run(){
    Alignment A = sanaMethod->runAndPrintTime();
    A.printDefinitionErrors(*Graph1, *Graph2);
    assert(A.isCorrectlyDefined(*Graph1, *Graph2) and "Resulting alignment is not correctly defined");
    saveReport(*Graph1, *Graph2, A, M, sanaMethod, intermediateAlignment);

    Alignment B = piswapMethod->runAndPrintTime();
    return B;
}

void SANAPISWAPWrapper::loadDefaultParameters(){

}

string SANAPISWAPWrapper::convertAndSaveGraph(Graph* graph, string name){
    return "";
}

string SANAPISWAPWrapper::generateAlignment(){
    return "";
}

Alignment SANAPISWAPWrapper::loadAlignment(Graph* G1, Graph* G2, string fileName){
    return Alignment::empty();
}

void SANAPISWAPWrapper::deleteAuxFiles(){

}
