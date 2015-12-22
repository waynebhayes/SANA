#include <cassert>
#include <iostream>

#include "measureSelector.hpp"

#include "../measures/EdgeCorrectness.hpp"
#include "../measures/InducedConservedStructure.hpp"
#include "../measures/SymmetricSubstructureScore.hpp"
#include "../measures/LargestCommonConnectedSubgraph.hpp"
#include "../measures/GoAverage.hpp"
#include "../measures/GoCoverage.hpp"
#include "../measures/WeightedEdgeConservation.hpp"
#include "../measures/NodeCorrectness.hpp"

#include "../measures/localMeasures/NodeDensity.hpp"
#include "../measures/localMeasures/EdgeDensity.hpp"
#include "../measures/localMeasures/GoSimilarity.hpp"
#include "../measures/localMeasures/Importance.hpp"
#include "../measures/localMeasures/Sequence.hpp"
#include "../measures/localMeasures/Graphlet.hpp"
#include "../measures/localMeasures/GraphletLGraal.hpp"

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
    return topFactor/(topFactor+seqFactor);
}

string getScoreTableMethodId(ArgumentParser& args) {
    string methodName = args.strings["-method"];
    if (methodName == "sana" or methodName == "tabu") {
        methodName += args.strings["-topmeasure"];
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
    vector<string> optimizableMeasures = {
        "ec","s3","wec","noded","edged","go","importance",
        "sequence","graphlet","graphletlgraal"
    };
    double total = 0;
    for (uint i = 0; i < optimizableMeasures.size(); i++) {
        total += args.doubles["-"+optimizableMeasures[i]];
    }
    return total;
}

double getWeight(string measureName, Graph& G1, Graph& G2, ArgumentParser& args) {
    string objFunType = args.strings["-objfuntype"];
    if (objFunType == "generic") {
        double weight = args.doubles["-"+measureName];
        return weight/totalGenericWeight(args);
    } else if (objFunType == "alpha" or objFunType == "beta") {
        double alpha = getAlpha(G1, G2, args);
        
        string topMeasure;
        string method = args.strings["-method"];
        if (method == "sana" or method == "tabu") {
            topMeasure = args.strings["-topmeasure"];
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

    double weight = getWeight(measureName, G1, G2, args);
    if (weight > 0) return true;

    double wecWeight = getWeight("wec", G1, G2, args);
    string wecNodeSim = args.strings["-wecnodesim"];
    return wecWeight > 0 and wecNodeSim == measureName;
}

void initMeasures(MeasureCombination& M, Graph& G1, Graph& G2, ArgumentParser& args) {
    cerr << "Initializing measures... " << endl;
    Timer T;
    T.start();
    Measure *m;

    //detailedReport; if false, init only basic measures and any measure necessary to run SANA
    bool detRep = args.bools["-detailedreport"];

    m = new EdgeCorrectness(&G1, &G2);
    M.addMeasure(m, getWeight("ec", G1, G2, args));

    m = new InducedConservedStructure(&G1, &G2);
    M.addMeasure(m);

    m = new SymmetricSubstructureScore(&G1, &G2);
    M.addMeasure(m, getWeight("s3", G1, G2, args));

    m = new LargestCommonConnectedSubgraph(&G1, &G2);
    M.addMeasure(m);

    //local measures must be initialized before wec,
    //as wec uses one of the local measures

    if (shouldInit("noded", G1, G2, args)) {
        m = new NodeDensity(&G1, &G2, args.vectors["-nodedweights"]);
        double nodedWeight = getWeight("noded", G1, G2, args);
        M.addMeasure(m, nodedWeight);
    }

    if (shouldInit("edged", G1, G2, args)) {
        m = new EdgeDensity(&G1, &G2, args.vectors["-edgedweights"]);
        double edgedWeight = getWeight("edged", G1, G2, args);
        M.addMeasure(m, edgedWeight);
    }

    if (GoSimilarity::fulfillsPrereqs(&G1, &G2)) {
        if (shouldInit("go", G1, G2, args)) {
            m = new GoSimilarity(&G1, &G2, args.vectors["-goweights"]);
            double goWeight = getWeight("go", G1, G2, args);
            M.addMeasure(m, goWeight);
        }
        if (detRep) {
            m = new GoCoverage(&G1, &G2);
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

    double wecWeight = getWeight("wec", G1, G2, args);    
    if (detRep or wecWeight > 0) {
        LocalMeasure* nodeSim = (LocalMeasure*) M.getMeasure(args.strings["-wecnodesim"]);
        m = new WeightedEdgeConservation(&G1, &G2, nodeSim);
        M.addMeasure(m, wecWeight);
    }

    //NC is evaluated iff G1 and G2 have the same size
    //(for the networks we have, only happens in the syeast dataset)
    //alternatively, an argument '-evalnc' could be used
    if (G1.getNumNodes() == G2.getNumNodes()) {
        if (args.strings["-truealignment"] == "") {
            m = new NodeCorrectness(Alignment::identity(G1.getNumNodes()));
        }
        else {
            m = new NodeCorrectness(Alignment::loadMapping(args.strings["-truealignment"]));
        }
        M.addMeasure(m);
    }

    //commented because it requires distance matrices,
    //which can take hours to compute:
    // m = new ShortestPathConservation(&G1, &G2);
    // M.addMeasure(m);

    //not necessary, as getWeight returns normalized weight:
    // M.normalize();
    
    cerr << "done (" << T.elapsedString() << ")" << endl;

    cerr << "=== optimize: ===" << endl;
    M.printWeights(cerr);
    cerr << endl;
}
