/*
 * natalie.cpp
 *
 *  Created on: 17-feb-2012
 *      Author: M. El-Kebir
 */

#include <lemon/smart_graph.h>
#include <lemon/arg_parser.h>
#include "analysis/analyze.h"
#include "input/matchinggraphshuffle.h"
#include "output/outputanalyse.h"
#include "output/outputcsv.h"
#include "output/outputcsvamc.h"
#include "output/outputeda.h"
#include "output/outputjson.h"
#include "output/outputjsonamc.h"
#include "output/outputnoa.h"
#include "output/outputparameters.h"
#include "output/outputparametersamc.h"
#include "output/outputsif.h"
#include "output/outputstat.h"
#include "output/outputstatamc.h"
#include "config.h"
#include "natalie.h"
#include "verbose.h"

using namespace lemon;
using namespace nina;
using namespace nina::gna;

typedef SmartGraph Graph;
typedef SmartBpGraph BpGraph;
typedef Natalie<Graph, BpGraph> NatalieType;
typedef NatalieType::Options OptionsType;
typedef Output<Graph, BpGraph> OutputType;
typedef Parser<Graph> ParserType;
typedef BpParser<Graph, BpGraph> BpParserType;
typedef OutputAnalyse<Graph, BpGraph> OutputAnalyseType;
typedef OutputCsv<Graph, BpGraph> OutputCsvType;
typedef OutputCsvAmc<Graph, BpGraph> OutputCsvAmcType;
typedef OutputEda<Graph, BpGraph> OutputEdaType;
typedef OutputJson<Graph, BpGraph> OutputJsonType;
typedef OutputJsonAmc<Graph, BpGraph> OutputJsonAmcType;
typedef OutputNoa<Graph, BpGraph> OutputNoaType;
typedef OutputParameters<Graph, BpGraph> OutputParametersType;
typedef OutputParametersAmc<Graph, BpGraph> OutputParametersAmcType;
typedef OutputSif<Graph, BpGraph> OutputSifType;
typedef OutputStat<Graph, BpGraph> OutputStatType;
typedef OutputStatAmc<Graph, BpGraph> OutputStatAmcType;
typedef MatchingGraphShuffle<Graph, BpGraph> MatchingGraphShuffleType;

int main(int argc, char** argv)
{
  ArgParser ap(argc, argv);
  OptionsType options;

  std::string g1, g2, gm, outputFile;
  int inputFormatG1 = static_cast<int>(NatalieType::IN_STRING);
  int inputFormatG2 = static_cast<int>(NatalieType::IN_STRING);
  int inputFormatGm = static_cast<int>(NatalieType::BP_IN_BLAST);
  int verbosityLevel = static_cast<int>(VERBOSE_NON_ESSENTIAL);
  int webserver = static_cast<int>(WEBSERVER_NONE);
  int outputType = static_cast<int>(OutputType::ORIG_EDGES);
  int nSamples = 0;
  std::string outputFormat = "3";

  ap
    .boolOption("version", "Show version number")
    .refOption("p", "Compute p-value using specified number of samples (default: 0)",
        nSamples, false)
    .refOption("w", "Web server mode:\n"
                    "     0 - No web server (default)\n"
                    "     1 - Natalie web server\n"
                    "     2 - AMC web server", webserver, false)
    .refOption("v", "Specifies the verbosity level:\n"
                    "     0 - No output\n"
                    "     1 - Only necessary output\n"
                    "     2 - More verbose output (default)\n"
                    "     3 - Debug output", verbosityLevel, false)
    .synonym("-verbosity", "v")
    .refOption("t", "Time limit (wall) in seconds (default -1: no limit)",
        options._timeLimit, false)
    .refOption("n", "Number of iterations to compute (default -1: no limit)",
        options._nMaxIterations, false)
    .refOption("no", "Number of outputs to generate (default 1)",
        options._nOutputs, false)
    .refOption("nd", "Number of dual descent steps before switching method\n"
                     "     in the combined solver (default: 100)",
        options._maxDualDescentSteps, false)
    .refOption("ns", "Number of subgradient steps before switching method\n"
                     "     in the combined solver (default: -1;\n"
                     "     switch when subgradient method has converged)",
        options._maxSubgradientSteps, false)
    .refOption("k", "Number of nondecreasing iterations after which\n"
                    "     to update mu (default: 20)",
        options._nMaxNondecreasingIterations, false)
    .refOption("l", "Number of increasing iterations after which\n"
                    "     to update mu (default: 10)",
        options._nMaxIncreasingIterations, false)
    .refOption("alpha", "Parameter used in updating the multipliers",
        options._alpha, false)
    .refOption("mu", "Parameter used in updating the multipliers",
        options._mu, false)
    .refOption("d", "Force binary edge weights",
        options._discretizeWeight, false)
    .refOption("i", "Specifies that all feasible solutions are integral",
        options._integral, false)
    .refOption("g1", "File name of input graph G_1", g1, false)
    .refOption("g2", "File name of input graph G_2", g2, false)
    .refOption("gm", "File name in which matching edges of G_m are defined;\n"
                     "     if omitted, the complete graph is used", gm, false)
    .refOption("if1", "Specifies the input file format for G_1:\n"
                     "     0 - GML format\n"
                     "     1 - GraphML format\n"
                     "     2 - STRING format (default)\n"
                     "     3 - LGF format\n"
                     "     4 - CSV format\n"
                     "     5 - LEDA format\n"
                     "     6 - Edge list format", inputFormatG1, false)
    .refOption("if2", "Specifies the input file format for G_2:\n"
                     "     0 - GML format\n"
                     "     1 - GraphML format\n"
                     "     2 - STRING format (default)\n"
                     "     3 - LGF format\n"
                     "     4 - CSV format\n"
                     "     5 - LEDA format\n"
                     "     6 - Edge list format", inputFormatG2, false)
    .refOption("ifm", "Specifies the input file format for G_m:\n"
                     "     0 - Candidate list\n"
                     "     1 - BLAST (default)\n"
                     "     2 - LGF", inputFormatGm, false)
    .refOption("beta", "Trade-off parameter used in the objective function (in [0,1])\n"
                     "     0 - Sequence only\n"
                     "     1 - Network topology only (default)",
        options._beta, false)
    .refOption("e", "Specifies e value cutoff (only used in conjunction\n"
                    "     with -ifm 1 and -ifm 2, default: 10)",
        options._eValCutOff, false)
    .refOption("c", "Specifies the confidence/correlation threshold\n"
                    "     (only used in conjunction with -sf [1,2,3]\n"
                    "     or -if1 2 or -if2 2, default: 0.9)",
        options._corrThreshold, false)
    .refOption("sf", "Specifies the score function to be used:\n"
                     "     0 - Average weights (default)\n"
                     "     1 - Coexpression-based: continuous function\n"
                     "     2 - Coexpression-based: scaled continuous function\n"
                     "     3 - Coexpression-based: discrete values\n"
                     "     4 - Coexpression-based: discrete inverse values\n"
                     "     5 - Edge correctness\n"
                     "     6 - Bit score",
        options._scoreModel, false)
    .refOption("o", "Output file name",
        outputFile, false)
    .refOption("of", "Specifies the output file format:\n"
                     "     0 - DOT format\n"
                     "     1 - GML format\n"
                     "     2 - LGF format\n"
                     "     3 - SIF format (default)\n"
                     "     4 - JSON format\n"
                     "     5 - NEATO format\n"
                     "     6 - CSV (matched) format\n"
                     "     7 - CSV (unmatched in G_1) format\n"
                     "     8 - CSV (unmatched in G_2) format\n"
                     "     9 - CSV (alignment) format\n"
                     "    10 - EDA format\n"
                     "    11 - NOA format\n"
                     "    12 - ANALYSE (SIF, EDA and NOA) format", outputFormat, false)
    .refOption("op", "Specifies parts of the matching graph to output:\n"
                     "     0 - Nodes and matching edges present in the solution\n"
                     "     1 - Nodes, matching edges and original edges present\n"
                     "         in the solution (default)\n"
                     "     2 - Nodes and matching edges present in the solution\n"
                     "         as well as all original edges", outputType, false)
    .refOption("r", "Specifies which relaxation to use [0]:\n"
                     "     0 - Cached Lagrangian decomposition (#multipliers: O(n^4))\n"
                     "     1 - Cached subproblem isolation  (#multipliers: O(n^3))",
        options._relaxationType, false)
    .refOption("s", "Specifies the solver to use:\n"
                     "     0 - Subgradient optimization\n"
                     "     1 - Dual descent method\n"
                     "     2 - Enhanced dual descent method\n"
                     "     3 - Combined method (default)",
        options._solverType, false)
    .refOption("tau", "Parameter tau of dual descent procedure", options._tau, false)
    .refOption("phi", "Parameter phi of dual descent procedure", options._phi, false)
    .refOption("maxSwitches", "Maximum number of switches\n"
                              "     (only used in conjunction with -s 3, default: 3)",
        options._maxSwitches, false)
    .refOption("maxJsonNodes", "Maximum number of JSON nodes to be generated\n"
                               "    (default: 50)", options._maxJsonNodes, false);
  ap.parse();

  if (ap.given("version"))
  {
    std::cout << "Version number: " << NATALIE_VERSION << std::endl;
    return 0;
  }

  if (!ap.given("g1") || !ap.given("g2"))
  {
    std::cerr << "Both -g1 and -g2 need to be specified" << std::endl;
    return 1;
  }

  if (!(0 <= options._beta && options._beta <= 1))
  {
    std::cerr << "Parameter beta should be between 0 and 1" << std::endl;
    return 1;
  }

  if (!(0 <= options._corrThreshold && options._corrThreshold <= 1))
  {
    std::cerr << "Correlation threshold (-c) should be between 0 and 1" << std::endl;
    return 1;
  }

  if (options._alpha < 1)
  {
    std::cerr << "Value of alpha must be bigger than 1" << std::endl;
    return 1;
  }

  outputFormat += ",";

  g_verbosity = static_cast<VerbosityLevel>(verbosityLevel);
  g_webserver = static_cast<WebserverType>(webserver);

  NatalieType natalie(options);

  if (nSamples > 0)
  {
    natalie.setMatchingGraph(new MatchingGraphShuffleType());
  }

  ParserType* pParserG1 =
      NatalieType::createParser(g1,
                                static_cast<NatalieType::InputFormatEnum>(inputFormatG1),
                                options._corrThreshold);

  ParserType* pParserG2 =
      NatalieType::createParser(g2,
                                static_cast<NatalieType::InputFormatEnum>(inputFormatG2),
                                options._corrThreshold);

  BpParserType* pParserGm =
      NatalieType::createBpParser(gm,
                                  static_cast<NatalieType::BpInputFormatEnum>(inputFormatGm),
                                  pParserG1,
                                  pParserG2,
                                  options._eValCutOff);

  std::cout << "Version number: " << NATALIE_VERSION << std::endl;
  if (!natalie.init(pParserG1, pParserG2, pParserGm))
    return 1;

  int res = natalie.solve(options._nOutputs);

  natalie.parseOutputString(outputFormat);

  if (g_webserver == WEBSERVER_NATALIE)
  {
    // no need to delete, natalie will own it
    OutputParametersType* pParOut = new OutputParametersType(*natalie.getMatchingGraph(),
                                                             g2,
                                                             options._beta,
                                                             options._corrThreshold,
                                                             options._eValCutOff);
    natalie.addOutput(pParOut);

    OutputStatType* pStat = new OutputStatType(*natalie.getMatchingGraph(),
                                               *natalie.getScoreModel(),
                                               natalie.getElapsedTime(),
                                               natalie.getScore(),
                                               natalie.getUpperBound());
    natalie.addOutput(pStat);
  }
  else if (g_webserver == WEBSERVER_AMC)
  {
    // no need to delete, natalie will own it
    OutputParametersAmcType* pParOut = new OutputParametersAmcType(*natalie.getMatchingGraph(),
                                                                   g1,
                                                                   g2,
                                                                   natalie.getScoreModel()->getScoreFunction(),
                                                                   options._beta,
                                                                   options._corrThreshold,
                                                                   options._eValCutOff);
    natalie.addOutput(pParOut);

    OutputStatAmcType* pStat = new OutputStatAmcType(*natalie.getMatchingGraph(),
                                                     *natalie.getScoreModel(),
                                                     natalie.getElapsedTime(),
                                                     natalie.getScore(),
                                                     natalie.getUpperBound());
    natalie.addOutput(pStat);

    natalie.addOutput(new OutputCsvAmcType(*natalie.getMatchingGraph(),
                                           OutputCsvAmcType::CSV_MATCHED));
    natalie.addOutput(new OutputCsvAmcType(*natalie.getMatchingGraph(),
                                           OutputCsvAmcType::CSV_UNMATCHED_IN_G1));
    natalie.addOutput(new OutputCsvAmcType(*natalie.getMatchingGraph(),
                                           OutputCsvAmcType::CSV_UNMATCHED_IN_G2));
    natalie.addOutput(new OutputCsvAmcType(*natalie.getMatchingGraph(),
                                           OutputCsvAmcType::CSV_ALIGNMENT));

    OutputAnalyseType* pAnalyse = new OutputAnalyseType(*natalie.getMatchingGraph(),
                                                        natalie.getScoreModel(),
                                                        natalie.getElapsedTime(),
                                                        natalie.getUpperBound());
    natalie.addOutput(pAnalyse);

    pAnalyse->addOutput(new OutputJsonAmcType(*natalie.getMatchingGraph(), options._maxJsonNodes, *natalie.getScoreModel()));
    pAnalyse->addOutput(new OutputSifType(*natalie.getMatchingGraph()));
    pAnalyse->addOutput(new OutputEdaType(*natalie.getMatchingGraph()));
    pAnalyse->addOutput(new OutputNoaType(*natalie.getMatchingGraph()));
    pAnalyse->addOutput(new OutputCsvAmcType(*natalie.getMatchingGraph(),
                                             OutputCsvAmcType::CSV_MATCHED));
    pAnalyse->addOutput(new OutputCsvAmcType(*natalie.getMatchingGraph(),
                                             OutputCsvAmcType::CSV_UNMATCHED_IN_G1));
    pAnalyse->addOutput(new OutputCsvAmcType(*natalie.getMatchingGraph(),
                                             OutputCsvAmcType::CSV_UNMATCHED_IN_G2));
    pAnalyse->addOutput(new OutputCsvAmcType(*natalie.getMatchingGraph(),
                                             OutputCsvAmcType::CSV_ALIGNMENT));

    natalie.addOutput(new OutputStatAmcType(*natalie.getMatchingGraph(),
                                            *natalie.getScoreModel(),
                                            natalie.getElapsedTime(),
                                            natalie.getScore(),
                                            natalie.getUpperBound()));
  }
  natalie.generateOutput(static_cast<OutputType::OutputType>(outputType), outputFile);

  if (nSamples > 0)
    natalie.computePValue(nSamples);

  delete pParserG1;
  delete pParserG2;
  delete pParserGm;

  if (g_webserver)
    return 0;
  else
    return res;
}
