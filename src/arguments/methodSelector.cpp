#include <iostream>

#include "methodSelector.hpp"

#include "../modes/AlphaEstimation.hpp"

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

Method* initMethod(Graph& G1, Graph& G2, ArgumentParser& args, MeasureCombination& M) {
  Method* method;
  string name = args.strings["-method"];
  string startAName = args.strings["-startalignment"];
  string aligFile =  args.strings["-eval"];

  if (aligFile != "") {
  	method = new NoneMethod(&G1, &G2, aligFile);
  } else if (name == "greedylccs") {
	method = new GreedyLCCS(&G1, &G2, startAName);
  } else if (name == "wave") {
    LocalMeasure* waveNodeSim = (LocalMeasure*) M.getMeasure(args.strings["-wavenodesim"]);
    method = new WeightedAlignmentVoter(&G1, &G2, waveNodeSim);
  } else if (name == "lgraal") {
    double seconds = args.doubles["-t"]*60;
    double alpha = args.doubles["-alpha"];
    if (args.bools["-autoalpha"]) {
      string alphaFile = args.strings["-alphafile"];
      if (not fileExists(alphaFile)) error("Couldn't find file "+alphaFile);
      alpha = AlphaEstimation::getAlpha(alphaFile, "LGRAAL", G1.getName(), G2.getName());
      if (alpha == -1) alpha = args.doubles["-alpha"];
    }
    method = new LGraalWrapper(&G1, &G2, alpha, args.doubles["-lgraaliter"], seconds);
  } else if (name == "hubalign") {
    double alpha = args.doubles["-alpha"];
    if (args.bools["-autoalpha"]) {
      string alphaFile = args.strings["-alphafile"];
      if (not fileExists(alphaFile)) error("Couldn't find file "+alphaFile);
      alpha = AlphaEstimation::getAlpha(alphaFile, "HubAlign", G1.getName(), G2.getName());
      if (alpha == -1) alpha = args.doubles["-alpha"];
    }
    //in hubalign alpha is the fraction of topology
    method = new HubAlignWrapper(&G1, &G2, 1 - alpha);
  } else if (name == "tabu") {
    cerr << "=== Tabu -- optimize: ===" << endl;
    M.printWeights(cerr);
    cerr << endl;
    double minutes = args.doubles["-t"];
    uint ntabus = args.doubles["-ntabus"];
    uint nneighbors = args.doubles["-nneighbors"];
    method = new TabuSearch(&G1, &G2, minutes, &M, ntabus, nneighbors, args.bools["-nodetabus"]);
  } else if (name == "sana") {
    cerr << "=== SANA -- optimize: ===" << endl;
    M.printWeights(cerr);
    cerr << endl;

    double T_initial;
    if (args.strings["-T_initial"] == "auto") T_initial = 0;
    else T_initial = stod(args.strings["-T_initial"]);
    double T_decay;
    if (args.strings["-T_decay"] == "auto") T_decay = 0;
    else T_decay = stod(args.strings["-T_decay"]);

    double minutes = args.doubles["-t"];

    if (args.bools["-autoalpha"]) {
      cout << "Outdated; needs to be refactored" << endl;
      // string methodName = "SANA";
      // string alphaFile = args.strings["-alphafile"];
      // if (not fileExists(alphaFile)) error("Couldn't find file "+alphaFile);
      // alpha = AlphaEstimation::getAlpha(alphaFile, methodName, G1.getName(), G2.getName());
      // if (alpha == -1) alpha = args.doubles["-alpha"];
    }

    method = new SANA(&G1, &G2, T_initial, T_decay, minutes, &M);
    if (args.bools["-restart"]) {
      double tnew = args.doubles["-tnew"];
      uint iterperstep = args.doubles["-iterperstep"];
      uint numcand = args.doubles["-numcand"];
      double tcand = args.doubles["-tcand"];
      double tfin = args.doubles["-tfin"];
      ((SANA*) method)->enableRestartScheme(tnew, iterperstep, numcand, tcand, tfin);
    }
    if (args.strings["-T_initial"] == "auto") {
      ((SANA*) method)->setT_INITIALAutomatically();
    }
    if (args.strings["-T_decay"] == "auto") {
      ((SANA*) method)->setT_DECAYAutomatically();
    }
  } else if (name == "hc") {
	method = new HillClimbing(&G1, &G2, &M, startAName);
  } else if (name == "random") {
	method = new RandomAligner(&G1, &G2);
  } else   if (name == "none") {
	method = new NoneMethod(&G1, &G2, startAName);
  } else {
	  throw runtime_error("Error: unknown method: " + name);
  }

  return method;
}
