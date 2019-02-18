#include <cassert>
#include <iostream>

#include "measureSelector.hpp"

#include "../measures/EdgeCorrectness.hpp"
#include "../measures/EdgeDifference.hpp"
#include "../measures/InducedConservedStructure.hpp"
#include "../measures/SymmetricSubstructureScore.hpp"
#include "../measures/SymmetricEdgeCoverage.hpp"
#include "../measures/LargestCommonConnectedSubgraph.hpp"
#include "../measures/GoAverage.hpp"
#include "../measures/NetGO.hpp"
#include "../measures/WeightedEdgeConservation.hpp"
#include "../measures/NodeCorrectness.hpp"
#include "../measures/ShortestPathConservation.hpp"
#include "../measures/MultiEdgeCorrectness.hpp"
#include "../measures/ExternalWeightedEdgeConservation.hpp"
#include "../measures/SquaredEdgeScore.hpp"
#include "../measures/TriangleCorrectness.hpp"
#include "../measures/EdgeExposure.hpp"

#include "../measures/localMeasures/NodeCount.hpp"
#include "../measures/localMeasures/NodeDensity.hpp"
#include "../measures/localMeasures/EdgeCount.hpp"
#include "../measures/localMeasures/EdgeDensity.hpp"
#include "../measures/localMeasures/ExternalSimMatrix.hpp"
#include "../measures/localMeasures/GoSimilarity.hpp"
#include "../measures/localMeasures/Importance.hpp"
#include "../measures/localMeasures/Sequence.hpp"
#include "../measures/localMeasures/Graphlet.hpp"
#include "../measures/localMeasures/GraphletLGraal.hpp"
#include "../measures/localMeasures/GraphletCosine.hpp"

using namespace std;

const string scoreFile = "topologySequenceScoreTable.cnf";

vector<vector<string> > getScoreTable() {
    if (not fileExists(scoreFile)) {
        throw runtime_error("Couldn't find file "+scoreFile);
    }
    return fileToStringsByLines(scoreFile);
}

vector<string> getScoreTuple(string methodName, string G1Name, string G2Name) {
    vector<vector<string> > scoreTable = getScoreTable();
    for (vector<string> line : scoreTable) {
        if (methodName == line[0] and G1Name == line[1] and G2Name == line[2]) {
            return line;
        }
    }
    throw runtime_error("Couldn't find entry in "+scoreFile+" for "+methodName+" "+G1Name+" "+G2Name);
}

double getTopScore(string methodName, string G1Name, string G2Name) {
    vector<string> tuple = getScoreTuple(methodName, G1Name, G2Name);
    return stod(tuple[3]);
}

double getSeqScore(string methodName, string G1Name, string G2Name) {
    vector<string> tuple = getScoreTuple(methodName, G1Name, G2Name);
    return stod(tuple[4]);
}

double betaDerivedAlpha(string methodName, string G1Name, string G2Name, double beta) {
    double topScore = getTopScore(methodName, G1Name, G2Name);
    double seqScore = getSeqScore(methodName, G1Name, G2Name);

    double topFactor = beta*topScore;
    double seqFactor = (1-beta)*seqScore;
    cout << "method: " << methodName << endl;
    cout << "G: " << G1Name << " " << G2Name << endl;
    cout << "beta: " << beta << endl;
    cout << "alpha: " << topFactor/(topFactor+seqFactor) << endl;
    return topFactor/(topFactor+seqFactor);
}

string getScoreTableMethodId(ArgumentParser& args) {
    string methodName = args.strings["-method"];
    if (methodName == "sana" or methodName == "tabu") {
        methodName += args.strings["-topomeasure"];
    }
    return methodName;
}

double getAlpha(Graph& G1, Graph& G2, ArgumentParser& args) {
    string objFunType = args.strings["-objfuntype"];
    if (objFunType == "alpha") {
        return args.doubles["-alpha"];
    } else if (objFunType == "beta") {
        string methodName = getScoreTableMethodId(args);
        double beta = args.doubles["-beta"];
        return betaDerivedAlpha(methodName, G1.getName(), G2.getName(), beta);
    } else {
        throw runtime_error("alpha is not defined if -objfuntype is "+objFunType);
    }
}

double totalGenericWeight(ArgumentParser& args) {
    vector<string> optimizableDoubleMeasures = {
        "ec","ed", "s3","ics","tc","sec","wec","nodec","noded","edgec","edged", "go","importance",
        "sequence","graphlet","graphletlgraal", "graphletcosine", "spc", "nc","mec", "ewec", "ses", "ee"
    };
    double total = 0;
    for (uint i = 0; i < optimizableDoubleMeasures.size(); i++) 
        total += args.doubles["-"+optimizableDoubleMeasures[i]];

    vector<string> optimizableDoubleVectorsMeasures = {
        "esim"
    };

    for (uint i = 0; i < optimizableDoubleVectorsMeasures.size(); i++){
        for (auto j : args.doubleVectors["-" + optimizableDoubleVectorsMeasures[i] ])
            total += j;
    }
    return total;
}

double getWeight(string measureName, Graph& G1, Graph& G2, ArgumentParser& args, int index=0) {
    string method = args.strings["-method"];
    if (method == "random") return 0;

    string objFunType = args.strings["-objfuntype"];
    if (objFunType == "generic") {
        double weight;
        if (args.doubles.count("-" + measureName)){
            weight = args.doubles["-"+measureName];
        }
        else
            weight = args.doubleVectors["-" + measureName][index];
        return weight/totalGenericWeight(args);
    } else if (objFunType == "alpha" or objFunType == "beta") {
        double alpha = getAlpha(G1, G2, args);
        
        string topMeasure;
        if (method == "sana" or method == "tabu") {
            topMeasure = args.strings["-topomeasure"];
        } else if (method == "lgraal") topMeasure = "wec";
        else if (method == "hubalign") topMeasure = "importance";
        else {
            throw runtime_error("alpha is not defined if -objfuntype is "+objFunType);
        }

        if (measureName == "sequence") {
            return alpha;
        } else if (measureName == topMeasure) {
            return 1-alpha;
        } else {
            return 0;
        }
    } else {
        throw runtime_error("Invalid value of -objfuntype: "+objFunType);
    }
}

bool shouldInit(string measureName, Graph& G1, Graph& G2, ArgumentParser& args) {

    bool detailedReport = args.bools["-detailedreport"];
    if (detailedReport) return true;

    if (args.doubleVectors.count("-" + measureName))
        return args.doubleVectors["-" + measureName].size() ? true : false;
    else if (args.stringVectors.count("-" + measureName))
        return args.stringVectors["-" + measureName].size() ? true : false;
    else if (getWeight(measureName, G1, G2, args) > 0)
        return true;

    double wecWeight = getWeight("wec", G1, G2, args);
    string wecNodeSim = args.strings["-wecnodesim"];
    return wecWeight > 0 and wecNodeSim == measureName;
}

void initMeasures(MeasureCombination& M, Graph& G1, Graph& G2, ArgumentParser& args) {
    cout << "Initializing measures... ";
    Timer T;
    T.start();
    Measure *m;

    //detailedReport; if false, init only basic measures and any measure necessary to run SANA
    bool detRep = args.bools["-detailedreport"];
    
    bool pareto = (args.strings["-mode"] == "pareto") ? true : false;
    if(pareto) {
        M.setParetoInitial(args.doubles["-paretoInitial"]);
        M.setParetoCapacity(args.doubles["-paretoCapacity"]);
	M.setParetoIterations(args.doubles["-paretoIterations"]);
        M.setParetoThreads(args.strings["-paretoThreads"]);
    }

    m = new EdgeCorrectness(&G1, &G2);
    M.addMeasure(m, getWeight("ec", G1, G2, args));
 
    m = new EdgeDifference(&G1, &G2);
    M.addMeasure(m, getWeight("ed", G1, G2, args));

    m = new InducedConservedStructure(&G1, &G2);
    M.addMeasure(m, getWeight("ics", G1, G2, args));

    m = new MultiEdgeCorrectness(&G1, &G2);
    M.addMeasure(m, getWeight("mec", G1, G2, args));

    m = new SquaredEdgeScore(&G1, &G2);
    M.addMeasure(m, getWeight("ses", G1, G2, args));
	
	m = new EdgeExposure(&G1, &G2);
    M.addMeasure(m, getWeight("ee", G1, G2, args));

    m = new InducedConservedStructure(&G1, &G2);
    M.addMeasure(m);

    m = new SymmetricSubstructureScore(&G1, &G2);
    M.addMeasure(m, getWeight("s3", G1, G2, args));

    m = new LargestCommonConnectedSubgraph(&G1, &G2);
    M.addMeasure(m);

    m = new SymmetricEdgeCoverage(&G1, &G2);
    M.addMeasure(m, getWeight("sec", G1, G2, args));

    if (shouldInit("tc", G1, G2, args)){
        m = new TriangleCorrectness(&G1, &G2);
        M.addMeasure(m, getWeight("tc", G1, G2, args));
    }
    
    //local measures must be initialized before wec,
    //as wec uses one of the local measures

    if (shouldInit("nodec", G1, G2, args)) {
        m = new NodeCount(&G1, &G2, args.doubleVectors["-nodecweights"]);
        double nodecWeight = getWeight("nodec", G1, G2, args);
        M.addMeasure(m, nodecWeight);
    }

    if (shouldInit("noded", G1, G2, args)) {
        m = new NodeDensity(&G1, &G2, args.doubles["-maxDist"]);
        double nodedWeight = getWeight("noded", G1, G2, args);
        M.addMeasure(m, nodedWeight);
    }

    if (shouldInit("edgec", G1, G2, args)) {
        m = new EdgeCount(&G1, &G2, args.doubleVectors["-edgecweights"]);
        double edgecWeight = getWeight("edgec", G1, G2, args);
        M.addMeasure(m, edgecWeight);
    }

    if (shouldInit("edged", G1, G2, args)) {
        m = new EdgeDensity(&G1, &G2, args.doubles["-maxDist"]);
        double edgedWeight = getWeight("edged", G1, G2, args);
        M.addMeasure(m, edgedWeight);
    }

    if (shouldInit("esim", G1, G2, args)) {
        int n_esim = args.doubleVectors["-esim"].size();
        int n_simFile = args.stringVectors["-simFile"].size();
        int n_simFormat = args.doubleVectors["-simFormat"].size();

        if (n_esim != n_simFile || n_simFile != n_simFormat)
            throw runtime_error("Numbers of arguments passed into -esim does not match -simFile and/oor -simFormat");

        cout << n_esim << '\n';
        for (int i = 0; i < n_esim; i++){
            cout << args.stringVectors["-simFile"][i] << ' ' << args.doubleVectors["-simFormat"][i] << endl << flush;
            m = new ExternalSimMatrix(&G1, &G2, args.stringVectors["-simFile"][i], args.doubleVectors["-simFormat"][i]);
            double esimWeight = getWeight("esim", G1, G2, args, i);
            M.addMeasure(m, esimWeight);
        }
    }

    if (shouldInit("ewec", G1, G2, args)) {
        m = new ExternalWeightedEdgeConservation(&G1, &G2, args.strings["-ewecFile"]);
        double ewecWeight = getWeight("ewec", G1, G2, args);
        M.addMeasure(m, ewecWeight);
    }

    if (GoSimilarity::fulfillsPrereqs(&G1, &G2)) {
        if (shouldInit("go", G1, G2, args)) {
            m = new GoSimilarity(&G1, &G2,
                args.doubleVectors["-goweights"], args.doubles["-gofrac"]);
            double goWeight = getWeight("go", G1, G2, args);
            M.addMeasure(m, goWeight);
        }
        if (detRep || args.doubles["-netgo"] > 0) {
            m = new NetGO(&G1, &G2);
            M.addMeasure(m);
        }
        //commented because it takes really long to compute,
        //and only works with yeast and human networks
        // m = new GoAverage(&G1, &G2);
        // M.addMeasure(m);
    }

    if (Importance::fulfillsPrereqs(&G1, &G2)) {
        if (shouldInit("importance", G1, G2, args)) {
            m = new Importance(&G1, &G2);
            double impWeight = getWeight("importance", G1, G2, args);
	    assert(impWeight > 0.0);
            M.addMeasure(m, impWeight);
        }
    }

    if (Sequence::fulfillsPrereqs(&G1, &G2)) {
        if (shouldInit("sequence", G1, G2, args)) {
            m = new Sequence(&G1, &G2);
            double seqWeight = getWeight("sequence", G1, G2, args);
            M.addMeasure(m, seqWeight);
        }
    }

    if (shouldInit("graphlet", G1, G2, args)) {
        m = new Graphlet(&G1, &G2);
        double graphletWeight = getWeight("graphlet", G1, G2, args);
        M.addMeasure(m, graphletWeight);
    }

    if (shouldInit("graphletlgraal", G1, G2, args)) {
        m = new GraphletLGraal(&G1, &G2);
        double graphletLgraalWeight = getWeight("graphletlgraal", G1, G2, args);
        M.addMeasure(m, graphletLgraalWeight);
    }

    if (shouldInit("graphletcosine", G1, G2, args)) {
        m = new GraphletCosine(&G1, &G2);
        double graphletWeight = getWeight("graphletcosine", G1, G2, args);
        M.addMeasure(m, graphletWeight);
    }

    double wecWeight = getWeight("wec", G1, G2, args);    
    if (detRep or wecWeight > 0) {
        LocalMeasure* nodeSim = (LocalMeasure*) M.getMeasure(args.strings["-wecnodesim"]);
        m = new WeightedEdgeConservation(&G1, &G2, nodeSim);
        M.addMeasure(m, wecWeight);
    }

    if (args.strings["-truealignment"] != "") {
        vector<string> edges = fileToStrings(args.strings["-truealignment"]);
        double ncWeight = 0;
        try{
            ncWeight = getWeight("nc", G1, G2, args);    
        }
        catch(...){
        }
        m = new NodeCorrectness(NodeCorrectness::convertAlign(G1, G2, edges));
        M.addMeasure(m, ncWeight);
    } 
    else if (G1.sameNodeNames(G2)) {
        Alignment a(Alignment::correctMapping(G1,G2));
        vector<uint> mapping = a.getMapping();
        mapping.push_back(G1.getNumNodes());
        double ncWeight = 0;
        try{
            ncWeight = getWeight("nc", G1, G2, args);
        }
        catch(...){
        }    
        m = new NodeCorrectness(mapping);
        M.addMeasure(m, ncWeight);
    }

    if (shouldInit("spc", G1, G2, args)) {
        m = new ShortestPathConservation(&G1, &G2);
        double spcWeight = getWeight("spc", G1, G2, args);
        M.addMeasure(m, spcWeight);
    }
    
    if (args.strings["-balance"] != ""){
        M.rebalanceWeight(args.strings["-balance"]);
    }else if (args.bools["-balance-all"]){
        M.rebalanceWeight();
    }

    //not necessary, as getWeight returns normalized weight:
    // M.normalize();
    
    cout << "done (" << T.elapsedString() << ")" << endl;

    cout << "=== optimize: ===" << endl;
    M.printWeights(cerr);
    cout << endl;
}
