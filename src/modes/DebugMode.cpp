#include "DebugMode.hpp"

#include <algorithm>

#include "../utils/utils.hpp"

#include "../arguments/ArgumentParser.hpp"
#include "../arguments/measureSelector.hpp"
#include "../arguments/methodSelector.hpp"
#include "../arguments/graphLoader.hpp"

#include "../measures/MeasureCombination.hpp"
#include "../measures/localMeasures/GoSimilarity.hpp"
#include "../methods/Method.hpp"
#include "../methods/SANA.hpp"
#include "../Graph.hpp"
#include "../Alignment.hpp"
#include "../complementaryProteins.hpp"
#include "NormalMode.hpp"

void DebugMode::run(ArgumentParser& args) {

  Graph G1, G2;
  initGraphs(G1, G2, args);

  GoSimilarity::leastFrequentGoTerms(G1, 0.5);
  GoSimilarity::leastFrequentGoTerms(G2, 0.5);

  exit(0);

  G1.saveInShuffledOrder("testfile.x");
  exit(0);


  G1.isWellDefined();
  //G2.isWellDefined();
  exit(0);
  // printLocalTopologicalSimilarities(G1, G2, 1);
  // exit(0);

  // LGraalObjectiveFunction m(&G1, &G2, 0);
  // GenericLocalMeasure m2(&G1, &G2, "arst", m.getSims());
  // m2.writeSimsWithNames("x");

  printProteinPairCountInNetworks(false);
  Alignment A = Alignment::loadEdgeList(&G1, &G2, "ignore/dijkstra_alignment.txt");
  printComplementaryProteinCounts(A, false);
  exit(0);

  // printProteinPairCountInNetworks(true);
  // string alig;
  // while (cin >> alig) {
  //   cout << alig << endl;
  //   Alignment A = Alignment::loadMapping(alig);
  //   printComplementaryProteinCounts(A, true);
  //   cout << endl;
  // }
  // exit(0);


  // Graph::GeoGeneDuplicationModel(5, 5, "geo5.gw");
  // Graph::GeoGeneDuplicationModel(10, 25, "geo10.gw");
  // Graph::GeoGeneDuplicationModel(20, 20*5, "geo20.gw");
  // Graph::GeoGeneDuplicationModel(40, 40*5, "geo40.gw");
  // Graph::GeoGeneDuplicationModel(80, 80*5, "geo80.gw");
  // Graph::GeoGeneDuplicationModel(160, 160*5, "geo160.gw");
  // Graph::GeoGeneDuplicationModel(320, 320*5, "geo320.gw");
  // Graph::GeoGeneDuplicationModel(640, 640*5, "geo640.gw");
  // Graph::GeoGeneDuplicationModel(1280, 1280*5, "geo1280.gw");
  // Graph::GeoGeneDuplicationModel(2560, 2560*5, "geo2560.gw");
  // Graph::GeoGeneDuplicationModel(5120, 5120*5, "geo5120.gw");
  // Graph::GeoGeneDuplicationModel(10240, 10240*5, "geo10240.gw");
  // Graph::GeoGeneDuplicationModel(20480, 20480*5, "geo20480.gw");

  // uint n = G1.getNumNodes();
  // while (n >= 5){
  //   n /= 2;
  //   cerr << n;
  //   G1 = G1.randomNodeInducedSubgraph(n);
  //   G1.saveInGWFormat("subgeo"+intToString(n)+".gw");
  // }
  // exit(0);

  // vector<vector<string> > networks = fileToStringsByLines("experiments/allgeo.exp");
  // cout << "G1\tG2\tLogSearchSpace\tHC_iters\n";
  // for (uint i = 0; i < networks.size(); i++) {
  //   G1 = Graph::loadGraph(networks[i][0]);
  //   G2 = Graph::loadGraph(networks[i][1]);
  //   MeasureCombination M;
  //   initMeasures(M, G1, G2, args);
  //   Method* method = initMethod(G1, G2, args, M);
  //   double x = ((SANA*) method)->searchSpaceSizeLog();
  //   double y = ((SANA*) method)->hillClimbingIterations();
  //   cout << networks[i][0] << "\t" << networks[i][1] << "\t" << to_string(x) << "\t" << to_string(y) << endl;
  // }
  MeasureCombination M;
  initMeasures(M, G1, G2, args);
  Method* method = initMethod(G1, G2, args, M);
  ((SANA*) method)->setTemperatureScheduleAutomatically();
}

std::string DebugMode::getName(void) {
    return "DebugMode";
}
