#include <iostream>

#include "methodSelector.hpp"

#include "measureSelector.hpp"

#include "../methods/NoneMethod.hpp"
#include "../methods/GreedyLCCS.hpp"
#include "../methods/WeightedAlignmentVoter.hpp"
#include "../methods/LGraalWrapper.hpp"
#include "../methods/HubAlignWrapper.hpp"
#include "../methods/TabuSearch.hpp"
#include "../methods/HillClimbing.hpp"
#include "../methods/SANA.hpp"
#include "../methods/RandomAligner.hpp"

using namespace std;

Method* initLgraal(Graph& G1, Graph& G2, ArgumentParser& args) {
    string objFunType = args.strings["-objfuntype"];

    double alpha;
    if (objFunType == "generic") {
        throw runtime_error("generic objective function not supported for L-GRAAL");
    } else if (objFunType == "alpha") {
        alpha = args.doubles["-alpha"];
    } else if (objFunType == "beta") {
        double beta = args.doubles["-beta"];
        alpha = betaDerivedAlpha("lgraal", G1.getName(), G2.getName(), beta);
    } else {
        throw runtime_error("unknown value of -objfuntype: "+objFunType);
    }

    double iters = args.doubles["-lgraaliter"];
    double seconds = args.doubles["-t"]*60;
    return new LGraalWrapper(&G1, &G2, alpha, iters, seconds);
}

Method* initHubAlign(Graph& G1, Graph& G2, ArgumentParser& args) {
    string objFunType = args.strings["-objfuntype"];

    double alpha;
    if (objFunType == "generic") {
        throw runtime_error("generic objective function not supported for HubAlign");
    } else if (objFunType == "alpha") {
        alpha = args.doubles["-alpha"];
    } else if (objFunType == "beta") {
        double beta = args.doubles["-beta"];
        alpha = betaDerivedAlpha("lgraal", G1.getName(), G2.getName(), beta);
    } else {
        throw runtime_error("unknown value of -objfuntype: "+objFunType);
    }

    return new HubAlignWrapper(&G1, &G2, alpha);
}

Method* initTabuSearch(Graph& G1, Graph& G2, ArgumentParser& args, MeasureCombination& M) {

    double minutes = args.doubles["-t"];
    uint ntabus = args.doubles["-ntabus"];
    uint nneighbors = args.doubles["-nneighbors"];
    bool nodeTabus = args.bools["-nodetabus"];
    return new TabuSearch(&G1, &G2, minutes, &M, ntabus, nneighbors, nodeTabus);
}

Method* initSANA(Graph& G1, Graph& G2, ArgumentParser& args, MeasureCombination& M) {

    double TInitial = 0;
    if (args.strings["-tinitial"] != "auto") {
        TInitial = stod(args.strings["-tinitial"]);
    }

    double TDecay = 0;
    if (args.strings["-tdecay"] != "auto") {
        TDecay = stod(args.strings["-tdecay"]);
    }

    double minutes = args.doubles["-t"];

    Method* sana = new SANA(&G1, &G2, TInitial, TDecay, minutes, &M);
    if (args.bools["-restart"]) {
        double tnew = args.doubles["-tnew"];
        uint iterperstep = args.doubles["-iterperstep"];
        uint numcand = args.doubles["-numcand"];
        double tcand = args.doubles["-tcand"];
        double tfin = args.doubles["-tfin"];
        ((SANA*) sana)->enableRestartScheme(tnew, iterperstep, numcand, tcand, tfin);
    }
    if (args.strings["-tinitial"] == "auto") {
        ((SANA*) sana)->setTInitialAutomatically();
    }
    if (args.strings["-tdecay"] == "auto") {
        ((SANA*) sana)->setTDecayAutomatically();
    }
    return sana;
}

Method* initMethod(Graph& G1, Graph& G2, ArgumentParser& args, MeasureCombination& M) {
 
    string aligFile = args.strings["-eval"];
    if (aligFile != "")
        return new NoneMethod(&G1, &G2, aligFile);

    string name = args.strings["-method"];
    string startAligName = args.strings["-startalignment"];

    if (name == "greedylccs")
        return new GreedyLCCS(&G1, &G2, startAligName);
    if (name == "wave") {
        LocalMeasure* waveNodeSim = 
            (LocalMeasure*) M.getMeasure(args.strings["-wavenodesim"]);
        return new WeightedAlignmentVoter(&G1, &G2, waveNodeSim);
    } 
    if (name == "lgraal")
        return initLgraal(G1, G2, args);
    if (name == "hubalign")
        return initHubAlign(G1, G2, args);
    if (name == "tabu")
        return initTabuSearch(G1, G2, args, M);
    if (name == "sana")
        return initSANA(G1, G2, args, M);
    if (name == "hc")
        return new HillClimbing(&G1, &G2, &M, startAligName);
    if (name == "random")
        return new RandomAligner(&G1, &G2);
    if (name == "none")
        return new NoneMethod(&G1, &G2, startAligName);

    throw runtime_error("Error: unknown method: " + name);
}
