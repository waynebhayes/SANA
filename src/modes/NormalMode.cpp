#include <cassert>
#include "NormalMode.hpp"
#include "AlphaEstimation.hpp"

#include "../utils.hpp"

#include "../methods/NoneMethod.hpp"
#include "../methods/GreedyLCCS.hpp"
#include "../methods/WeightedAlignmentVoter.hpp"
#include "../methods/LGraalWrapper.hpp"
#include "../methods/HubAlignWrapper.hpp"
#include "../methods/TabuSearch.hpp"
#include "../methods/HillClimbing.hpp"
#include "../methods/SANA.hpp"
#include "../methods/RandomAligner.hpp"

#include "../arguments/MeasureSelector.hpp"

void NormalMode::run(ArgumentParser& args) {
	createFolders(args);
	Graph G1, G2;
	initGraphs(G1, G2, args);

	MeasureCombination M;
	initMeasures(M, G1, G2, args);

	Method* method;
	Alignment A = Alignment::empty();
	string aligFile = args.strings["-eval"];
	if (aligFile != "") {
		method = new NoneMethod(&G1, &G2, aligFile);
		A = Alignment::loadEdgeList(&G1, &G2, aligFile);
	}
	else {
		method = initMethod(G1, G2, args, M);
		A = method->runAndPrintTime();
	}

	A.printDefinitionErrors(G1,G2);
	assert(A.isCorrectlyDefined(G1, G2) and "Resulting alignment is not correctly defined");

	saveReport(G1, G2, A, M, method, args.strings["-o"]);
}


Method* initMethod(Graph& G1, Graph& G2, ArgumentParser& args, MeasureCombination& M) {
  string name = args.strings["-method"];
  string startAName = args.strings["-startalignment"];

  if (strEq(name, "greedylccs")) {
    return new GreedyLCCS(&G1, &G2, startAName);
  }
  if (strEq(name, "wave")) {
    LocalMeasure* waveNodeSim = (LocalMeasure*) M.getMeasure(args.strings["-wavenodesim"]);
    return new WeightedAlignmentVoter(&G1, &G2, waveNodeSim);
  }
  if (strEq(name, "lgraal")) {
    double seconds = args.doubles["-t"]*60;
    double alpha = args.doubles["-alpha"];
    if (args.bools["-autoalpha"]) {
      string alphaFile = args.strings["-alphafile"];
      if (not fileExists(alphaFile)) error("Couldn't find file "+alphaFile);
      alpha = AlphaEstimation::getAlpha(alphaFile, "LGRAAL", G1.getName(), G2.getName());
      if (alpha == -1) alpha = args.doubles["-alpha"];
    }
    return new LGraalWrapper(&G1, &G2, alpha, args.doubles["-lgraaliter"], seconds);
  }
  if (strEq(name, "hubalign")) {
    double alpha = args.doubles["-alpha"];
    if (args.bools["-autoalpha"]) {
      string alphaFile = args.strings["-alphafile"];
      if (not fileExists(alphaFile)) error("Couldn't find file "+alphaFile);
      alpha = AlphaEstimation::getAlpha(alphaFile, "HubAlign", G1.getName(), G2.getName());
      if (alpha == -1) alpha = args.doubles["-alpha"];
    }
    //in hubalign alpha is the fraction of topology
    return new HubAlignWrapper(&G1, &G2, 1 - alpha);
  }
  if (strEq(name, "tabu")) {
    cerr << "=== Tabu -- optimize: ===" << endl;
    M.printWeights(cerr);
    cerr << endl;
    double minutes = args.doubles["-t"];
    uint ntabus = args.doubles["-ntabus"];
    uint nneighbors = args.doubles["-nneighbors"];
    Method* method = new TabuSearch(&G1, &G2, minutes, &M, ntabus, nneighbors, args.bools["-nodetabus"]);
    return method;
  }
  if (strEq(name, "sana")) {
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

    Method* method = new SANA(&G1, &G2, T_initial, T_decay, minutes, &M);
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

    return method;
  }
  if (strEq(name, "hc")) {
    return new HillClimbing(&G1, &G2, &M, startAName);
  }
  if (strEq(name, "random")) {
    return new RandomAligner(&G1, &G2);
  }
  if (strEq(name, "none")) {
    return new NoneMethod(&G1, &G2, startAName);
  }
  throw runtime_error("Error: unknown method: " + name);
}

#include "../measures/EdgeCorrectness.hpp"
#include "../measures/InducedConservedStructure.hpp"
#include "../measures/SymmetricSubstructureScore.hpp"

void NormalMode::makeReport(const Graph& G1, Graph& G2, const Alignment& A,
  const MeasureCombination& M, Method* method, ofstream& stream) {
  int numCCsToPrint = 3;

  stream << endl << currentDateTime() << endl << endl;

  stream << "G1: " << G1.getName() << endl;
  G1.printStats(numCCsToPrint, stream);
  stream << endl;

  stream << "G2: " << G2.getName() << endl;
  G2.printStats(numCCsToPrint, stream);
  stream << endl;

  stream << "Method: " << method->getName() << endl;
  method->describeParameters(stream);

  stream << endl << "execution time = " << method->getExecTime() << endl;

  stream << endl << "Scores:" << endl;
  M.printMeasures(A, stream);
  stream << endl;

  // string compareAFile = args.strings["-compare"];
  // if (not strEq(compareAFile, "")) {
  //   stream << "Alignment comparison:" << endl;
  //   vector<ushort> compareA = loadAlignment(compareAFile);
  //   stream << compareAFile << " = " << alignmentSimilarity(A, compareA) << endl;
  // }

  Graph CS = A.commonSubgraph(G1, G2);
  stream << "Common subgraph:" << endl;
  CS.printStats(numCCsToPrint, stream);
  stream << endl;

  int tableRows = min(5, CS.getConnectedComponents().size())+2;
  vector<vector<string> > table(tableRows, vector<string> (8));

  table[0][0] = "Graph"; table[0][1] = "n"; table[0][2] = "m"; table[0][3] = "alig-edges";
  table[0][4] = "indu-edges"; table[0][5] = "EC";
  table[0][6] = "ICS"; table[0][7] = "S3";

  table[1][0] = "G1"; table[1][1] = to_string(G1.getNumNodes()); table[1][2] = to_string(G1.getNumEdges());
  table[1][3] = to_string(A.numAlignedEdges(G1, G2)); table[1][4] = to_string(G2.numNodeInducedSubgraphEdges(A.getMapping()));
  table[1][5] = to_string(M.eval("ec",A));
  table[1][6] = to_string(M.eval("ics",A)); table[1][7] = to_string(M.eval("s3",A));

  for (int i = 0; i < tableRows-2; i++) {
    const vector<ushort>& nodes = CS.getConnectedComponents()[i];
    Graph H = CS.nodeInducedSubgraph(nodes);
    Alignment newA(nodes);
    newA.compose(A);
    table[i+2][0] = "CCS_"+to_string(i); table[i+2][1] = to_string(H.getNumNodes());
    table[i+2][2] = to_string(H.getNumEdges());
    table[i+2][3] = to_string(newA.numAlignedEdges(H, G2));
    table[i+2][4] = to_string(G2.numNodeInducedSubgraphEdges(newA.getMapping()));
    EdgeCorrectness ec(&H, &G2);
    table[i+2][5] = to_string(ec.eval(newA));
    InducedConservedStructure ics(&H, &G2);
    table[i+2][6] = to_string(ics.eval(newA));
    SymmetricSubstructureScore s3(&H, &G2);
    table[i+2][7] = to_string(s3.eval(newA));
  }

  stream << "Common connected subgraphs:" << endl;
  printTable(table, 2, stream);
  stream << endl;
}

void NormalMode::saveReport(const Graph& G1, Graph& G2, const Alignment& A,
  const MeasureCombination& M, Method* method, string reportFile) {
  string G1Name = G1.getName();
  string G2Name = G2.getName();
  if (strEq(reportFile, "")) {
    reportFile = "alignments/" + G1Name + "_" + G2Name + "/"
    + G1Name + "_" + G2Name + "_" + method->getName() + method->fileNameSuffix(A);
    addUniquePostfixToFilename(reportFile, ".txt");
    reportFile += ".txt";
  }

  ofstream outfile;
  outfile.open(reportFile.c_str());
  cerr << "Saving report as \"" << reportFile << "\"" << endl;
  A.write(outfile);
  makeReport(G1, G2, A, M, method, outfile);
  outfile.close();
}

/*
The program requires that there exist the network files in GW format
in networks/g1name/g1name.gw and networks/g1name/g2name.gw.

The -g1 and -g2 arguments allow you to specify g1name and g2name directly.
These arguments assume that the files already exist.

The -fg1 and -fg2 arguments allow you to specify external files containing
the graph definitions (in either GW or edge list format). If these
arguments are used, -g1 and -g2 are ignored. g1Name and g2Name are deduced
from the file names (by removing the path and the extension). Then,
the network definitions are parsed and the necessary network files are created.

*/
void initGraphs(Graph& G1, Graph& G2, ArgumentParser& args) {
  string fg1 = args.strings["-fg1"], fg2 = args.strings["-fg2"];
  createFolder("networks");
  string g1Name, g2Name;
  createFolder("alignments/"+g1Name+"_"+g2Name);
  if (fg1 != "") {
    g1Name = extractFileNameNoExtension(fg1);
  } else {
    g1Name = args.strings["-g1"];
  }
  if (fg2 != "") {
    g2Name = extractFileNameNoExtension(fg2);
  } else {
    g2Name = args.strings["-g2"];
  }

  string g1Folder, g2Folder;
  g1Folder = "networks/"+g1Name;
  g2Folder = "networks/"+g2Name;
  createFolder(g1Folder);
  createFolder(g2Folder);
  createFolder(g1Folder+"/autogenerated");
  createFolder(g2Folder+"/autogenerated");

  string g1GWFile, g2GWFile;
  g1GWFile = g1Folder+"/"+g1Name+".gw";
  if (fileExists(g1GWFile) and fg1 != "") {
    cerr << "Warning: argument of -fg1 (" << fg1 <<
      ") ignored because there already exists a network named " << g1Name << endl;
  }
  g2GWFile = g2Folder+"/"+g2Name+".gw";
  if (fileExists(g2GWFile) and fg2 != "") {
    cerr << "Warning: argument of -fg2 (" << fg2 <<
      ") ignored because there already exists a network named " << g2Name << endl;
  }

  if (not fileExists(g1GWFile)) {
    if (fg1 != "") {
      if (fileExists(fg1)) {
        if (fg1.size() > 3 and fg1.substr(fg1.size()-3) == ".gw") {
          exec("cp "+fg1+" "+g1GWFile);
        } else {
          Graph::edgeList2gw(fg1, g1GWFile);
        }
      } else {
        error("File not found: "+fg1);
      }
    } else {
      error("File not found: "+g1GWFile);
    }
  }
  if (not fileExists(g2GWFile)) {
    if (fg2 != "") {
      if (fileExists(fg2)) {
        if (fg2.size() > 3 and fg2.substr(fg2.size()-3) == ".gw") {
          exec("cp "+fg2+" "+g2GWFile);
        } else {
          Graph::edgeList2gw(fg2, g2GWFile);
        }
      } else {
        error("File not found: "+fg2);
      }
    } else {
      error("File not found: "+g2GWFile);
    }
  }

  cerr << "Initializing graphs... ";
  Timer T;
  T.start();
  G1 = Graph::loadGraph(g1Name);
  G2 = Graph::loadGraph(g2Name);

  double rewiredFraction = args.doubles["-rewire"];
  if (rewiredFraction > 0) {
    if (rewiredFraction > 1) error("Cannot rewire more than 100% of the edges");
    G2.rewireRandomEdges(rewiredFraction);
  }

  if (G1.getNumNodes() > G2.getNumNodes()) error("G2 has less nodes than G1");
  if (G1.getNumEdges() == 0 or G2.getNumEdges() == 0) error ("One of the networks has 0 edges");
  cerr << "done (" << T.elapsedString() << ")" << endl;
}

void createFolders(ArgumentParser& args) {
  createFolder("matrices");
  createFolder("matrices/autogenerated");
  createFolder("tmp");
  createFolder("alignments");
  createFolder("go");
  createFolder("go/autogenerated");
}
