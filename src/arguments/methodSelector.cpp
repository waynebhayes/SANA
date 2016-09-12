#include <iostream>

#include "methodSelector.hpp"

#include "measureSelector.hpp"

#include "../methods/NoneMethod.hpp"
#include "../methods/GreedyLCCS.hpp"
#include "../methods/WeightedAlignmentVoter.hpp"
#include "../methods/TabuSearch.hpp"
#include "../methods/HillClimbing.hpp"
#include "../methods/SANA.hpp"
#include "../methods/RandomAligner.hpp"
#include "../methods/wrappers/LGraalWrapper.hpp"
#include "../methods/wrappers/HubAlignWrapper.hpp"
#include "../methods/wrappers/NETALWrapper.hpp"
#include "../methods/wrappers/MIGRAALWrapper.hpp"
#include "../methods/wrappers/GHOSTWrapper.hpp"
#include "../methods/wrappers/PISwapWrapper.hpp"
#include "../methods/wrappers/OptNetAlignWrapper.hpp"
#include "../methods/wrappers/SPINALWrapper.hpp"
#include "../methods/wrappers/GREATWrapper.hpp"
#include "../methods/wrappers/NATALIEWrapper.hpp"
#include "../methods/wrappers/GEDEVOWrapper.hpp"
#include "../methods/wrappers/WAVEWrapper.hpp"
#include "../methods/wrappers/MagnaWrapper.hpp"
#include "../methods/wrappers/PINALOGWrapper.hpp"
#include "../methods/wrappers/SANAPISWAPWrapper.hpp"
#include "../methods/wrappers/CytoGEDEVOWrapper.hpp"
#include "../methods/Dijkstra.hpp"

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
        alpha = betaDerivedAlpha("hubalign", G1.getName(), G2.getName(), beta);
    } else {
        throw runtime_error("unknown value of -objfuntype: "+objFunType);
    }

    return new HubAlignWrapper(&G1, &G2, alpha);
}

Method* initDijkstra(Graph& G1, Graph& G2, ArgumentParser& args, MeasureCombination& M) {
    double delta = args.doubles["-dijkstradelta"];
    if(delta < 0.0 || delta > 1.0){
        throw runtime_error("Dijkstra:delta not in valid range [0.0,1.0)");
    }
    return new Dijkstra(&G1, &G2, &M, delta); 
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

    Method* sana;

    if(args.doubles["-i"] > 0){
        uint iterations = (uint)(args.doubles["-i"]);
        sana = new SANA(&G1, &G2, TInitial, TDecay, iterations, &M, args.strings["-score"]);
    }else{
        double minutes = args.doubles["-t"];
        sana = new SANA(&G1, &G2, TInitial, TDecay, minutes, &M, args.strings["-score"]);
    }
    
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
    if (args.bools["-dynamictdecay"]) {
	((SANA*) sana)->setDynamicTDecay();
    } 
    if (args.strings["-lock"] != ""){
    	sana->setLockFile(args.strings["-lock"] );
    }
    return sana;
}

Method* initMethod(Graph& G1, Graph& G2, ArgumentParser& args, MeasureCombination& M) {
 
    string aligFile = args.strings["-eval"];
    if (aligFile != "")
        return new NoneMethod(&G1, &G2, aligFile);	
    string name = toLowerCase(args.strings["-method"]);
    string startAligName = args.strings["-startalignment"];
    double alpha = args.doubles["-alpha"];
    string wrappedArgs = args.strings["-wrappedArgs"];

    if (name == "greedylccs")
        return new GreedyLCCS(&G1, &G2, startAligName);
    if (name == "waveSim") {
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
    if (name == "dijkstra")
        return initDijkstra(G1, G2, args, M);
      //return new Dijkstra(&G1, &G2, &M);
    if (name == "netal")
        return new NETALWrapper(&G1, &G2, wrappedArgs);
    if (name == "mi-graal" || name == "migraal")
        return new MIGRAALWrapper(&G1, &G2, wrappedArgs);
    if (name == "ghost")
    	return new GHOSTWrapper(&G1, &G2, wrappedArgs);
    if (name == "piswap")
    	return new PISwapWrapper(&G1, &G2, alpha, startAligName, wrappedArgs);
    if (name == "optnetalign")
		return new OptNetAlignWrapper(&G1, &G2, wrappedArgs);
    if (name == "spinal")
		return new SPINALWrapper(&G1, &G2, alpha, wrappedArgs);
    if (name == "great")
    	return new GREATWrapper(&G1, &G2, wrappedArgs);
    if (name == "natalie")
    	return new NATALIEWrapper(&G1, &G2, wrappedArgs);
    if (name == "gedevo")
		return new GEDEVOWrapper(&G1, &G2, wrappedArgs);
    if (name == "wave")
		return new WAVEWrapper(&G1, &G2, wrappedArgs);
    if (name == "sana")
        return initSANA(G1, G2, args, M);
    if (name == "hc")
        return new HillClimbing(&G1, &G2, &M, startAligName);
    if (name == "random")
        return new RandomAligner(&G1, &G2);
    if (name == "none")
        return new NoneMethod(&G1, &G2, startAligName);
    if (name == "magna")
    	return new MagnaWrapper(&G1, &G2, wrappedArgs);
    if (name == "pinalog")
        return new PINALOGWrapper(&G1, &G2, wrappedArgs);
    if (name == "sana+piswap")
        return new SANAPISWAPWrapper(&G1, &G2, args, M);
    if (name == "cytogedevo")
        return new CytoGEDEVOWrapper(&G1, &G2, wrappedArgs);

    throw runtime_error("Error: unknown method: " + name);
}
