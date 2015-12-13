#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <stdexcept>
#include <map>
#include <set>
#include <cassert>
#include <algorithm>
#include <iomanip>

#include "arguments/ArgumentParser.hpp"
#include "arguments/SupportedArguments.hpp"
#include "arguments/defaultArguments.hpp"

#include "modes/ParameterEstimation.hpp"
#include "modes/AlphaEstimation.hpp"
#include "modes/DebugMode.hpp"
#include "modes/NormalMode.hpp"
#include "modes/ClusterMode.hpp"
#include "modes/Experiment.hpp"

#include "Graph.hpp"
#include "ComplementaryProteins.hpp"
#include "Timer.hpp"
#include "utils.hpp"
#include "Alignment.hpp"
#include "RandomSeed.hpp"

using namespace std;

int main(int argc, char* argv[]) {
  ArgumentParser args(stringArgs, doubleArgs, boolArgs, vectorArgs);
  args.parse(getArgumentList(argc, argv, defaultArguments, true));

  if(args.doubles["-seed"] != 0) {
	  setSeed(args.doubles["-seed"]);
  }

  if (args.bools["-qsub"]) {
    for (int i = 0; i < args.doubles["-qcount"]; i++) {
      vector<string> argvs(argc);
      for (int i = 0; i < argc; i++) argvs[i] = argv[i];
      if (i > 0 and not strEq(args.strings["-o"], "")) {
        for (int j = 0; j < argc; j++) {
          if (strEq(argvs[j], "-o")) {
            argvs[j+1] += "_" + toString(i+1);
          }
        }
      }
      submitToCluster(argvs);
    }
    exit(0);
  }

  args.writeArguments();

  string exper = args.strings["-experiment"];
  if (not strEq(exper, "")) {
    string experFile = "experiments/"+exper+".exp";
    assert(fileExists(experFile));
    Experiment e(experFile);
    e.printData("experiments/"+exper+".txt");
    e.printDataCSV("experiments/"+exper+".csv");
    exit(0);
  }

  string paramEstimation = args.strings["-paramestimation"];
  if (not strEq(paramEstimation, "")) {
    string experFile = "experiments/"+paramEstimation+".exp";
    assert(fileExists(experFile));
    ParameterEstimation pe(experFile);
    if (args.bools["-submit"]) {
      pe.submitScriptsToCluster();
    }
    else {
      pe.collectData();
      pe.printData("experiments/"+paramEstimation+".out");
    }
    exit(0);
  }

  string alphaEstimation = args.strings["-alphaestimation"];
  if (not strEq(alphaEstimation, "")) {
    string experFile = "experiments/"+alphaEstimation;
    assert(fileExists(experFile));
    AlphaEstimation ae(experFile);
    ae.printData(experFile+".out");
    exit(0);
  }

  createFolders(args);
  Graph G1, G2;
  initGraphs(G1, G2, args);

  if (args.bools["-dbg"]) {
    dbgMode(G1, G2, args);
  }

  normalMode(G1, G2, args);
}
