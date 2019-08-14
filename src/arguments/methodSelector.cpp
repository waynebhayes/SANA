#include <iostream>

#include "methodSelector.hpp"

#include "measureSelector.hpp"

#include "../utils/Timer.hpp"
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

#include "../schedulemethods/ScheduleMethod.hpp"
#include "../schedulemethods/scheduleUtils.hpp"
#include "../schedulemethods/LinearRegressionVintage.hpp"

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

#ifdef MULTI_PAIRWISE
Method* initSANA(Graph& G1, Graph& G2, ArgumentParser& args, MeasureCombination& M, string startAligName) {
#else
Method* initSANA(Graph& G1, Graph& G2, ArgumentParser& args, MeasureCombination& M) {
#endif

    string TIniArg = args.strings["-tinitial"];
    string TDecayArg = args.strings["-tdecay"];
    string scheduleMethodName = args.strings["-schedulemethod"];

    //this is a special argument value that does a comparison between all temperature schedule methods
    //made for the purpose of running the experiments for the paper on the tempertaure schedule
    if (scheduleMethodName == "comparison") {

#ifdef MULTI_PAIRWISE
        SANA sana(&G1, &G2, 0, 0, 0, args.bools["-usingIterations"], 0, &M, args.strings["-combinedScoreAs"], startAligName);
#else
        SANA sana(&G1, &G2, 0, 0, 0, args.bools["-usingIterations"], 0, &M, args.strings["-combinedScoreAs"]);
#endif
        scheduleMethodComparison(&sana);
        exit(0);
    }

    double TInitial = 0, TDecay = 0;
    bool useMethodForTIni = (TIniArg == "auto");
    bool useMethodForTDecay = (TDecayArg == "auto");

    //read explicit values for TInitial or TDecay if not using an automatic method
    if (not useMethodForTIni) {
        try {
            TInitial = stod(TIniArg);
        } catch (const invalid_argument& ia) {
            cerr << "invalid -tinitial argument: " << TIniArg << endl;
            throw ia;
        }
    } 
    if (not useMethodForTDecay) {
        try {
            TDecay = stod(TDecayArg);
        } catch (const invalid_argument& ia) {
            cerr << "invalid -tdecay argument: " << TDecayArg << endl;
            throw ia;
        }
    }

    SANA* sana;
    double time = args.doubles["-t"];
#ifdef MULTI_PAIRWISE
    sana = new SANA(&G1, &G2, TInitial, TDecay, time, args.bools["-usingIterations"], args.bools["-add-hill-climbing"], &M, args.strings["-combinedScoreAs"], startAligName);
#else
    sana = new SANA(&G1, &G2, TInitial, TDecay, time, args.bools["-usingIterations"], args.bools["-add-hill-climbing"], &M, args.strings["-combinedScoreAs"]);
#endif
    sana->setOutputFilenames(args.strings["-o"], args.strings["-localScoresFile"]);

    if (useMethodForTIni or useMethodForTDecay) {
        if (scheduleMethodName == "auto" ) {
            //if user uses 'auto', choose for them
            scheduleMethodName = LinearRegressionVintage::NAME;
        }

        ScheduleMethod::setSana(sana);
        auto scheduleMethod = getScheduleMethod(scheduleMethodName);
        scheduleMethod->setSampleTime(2);
        ScheduleMethod::Resources maxRes(60, 200.0); //#samples, seconds
        if (useMethodForTIni) {
            sana->setTInitial(scheduleMethod->computeTInitial(maxRes));
        }
        if (useMethodForTDecay) {
            sana->setTFinal(scheduleMethod->computeTFinal(maxRes));
            sana->setTDecayFromTempRange();
        }
        scheduleMethod->printScheduleStatistics();
    }

    if (args.bools["-restart"]) {
        double tnew = args.doubles["-tnew"];
        uint iterperstep = args.doubles["-iterperstep"];
        uint numcand = args.doubles["-numcand"];
        double tcand = args.doubles["-tcand"];
        double tfin = args.doubles["-tfin"];
        sana->enableRestartScheme(tnew, iterperstep, numcand, tcand, tfin);
    }

    if (args.bools["-dynamictdecay"]) {
       sana->setDynamicTDecay();
    }
    if (args.strings["-lock"] != ""){
      sana->setLockFile(args.strings["-lock"] );
    }
    if(args.bools["-lock-same-names"] && args.strings["-lock"].size()== 0){
        sana->setLockFile("/dev/null");
    }
    return static_cast<Method*>(sana);
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
#ifdef MULTI_PAIRWISE
        return initSANA(G1, G2, args, M, startAligName);
#else
        return initSANA(G1, G2, args, M);
#endif
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
