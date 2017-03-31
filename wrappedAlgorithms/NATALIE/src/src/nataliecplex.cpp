/*
 * nataliecplex.cpp
 *
 *  Created on: 7-may-2012
 *      Author: M. El-Kebir
 */

#include <lemon/smart_graph.h>
#include <lemon/arg_parser.h>
#include "analysis/analyze.h"
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
#include "nataliecplex.h"
#include "verbose.h"
#include "config.h"
#include "webserver.h"

using namespace lemon;
using namespace nina;
using namespace nina::gna;

typedef SmartGraph Graph;
typedef SmartBpGraph BpGraph;
typedef NatalieCplex<Graph, BpGraph> NatalieCplexType;
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

int main(int argc, char** argv)
{
  ArgParser ap(argc, argv);
  OptionsType options;

  std::string g1, g2, gm, outputFile;
  int inputFormatG1 = static_cast<int>(NatalieType::IN_STRING);
  int inputFormatG2 = static_cast<int>(NatalieType::IN_STRING);
  int inputFormatGm = static_cast<int>(NatalieType::BP_IN_BLAST);
  int verbosityLevel = static_cast<int>(VERBOSE_NON_ESSENTIAL);
  int outputType = static_cast<int>(OutputType::ORIG_EDGES);
  std::string outputFormat = "3";
  bool lp = false;
  int formulationType =
    static_cast<int>(NatalieCplexType::LAGRANGIAN_DECOMPOSITION);

  ap
    .boolOption("version", "Show version number")
    .refOption("v", "Specifies the verbosity level:\n"
                    "     0 - No output\n"
                    "     1 - Only necessary output\n"
                    "     2 - More verbose output (default)\n"
                    "     3 - Debug output", verbosityLevel, false)
    .synonym("-verbosity", "v")
    .refOption("lp", "Specifies whether to solve the LP relaxation", lp, false)
    .refOption("t", "Time limit (wall) in seconds (default -1: no limit)",
        options._timeLimit, false)
    .refOption("d", "Force binary edge weights",
        options._discretizeWeight, false)
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
                     "     0 - sequence only\n"
                     "     1 - network topology only (default)",
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
                     "     3 - Coexpression-based: discrete values",
        options._scoreModel, false)
    .refOption("f", "Specifies which formulation to use:\n"
                     "      0 - Lagrangian decomposition\n"
                     "      1 - Subproblem isolation\n"
                     "      2 - Quadratic formulation", formulationType, false)
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
    .refOption("r", "Specifies which relaxation to use:\n"
                     "     0 - Cached Lagrangian decomposition (#multipliers: O(n^4))\n"
                     "     1 - Cached subproblem isolation  (#multipliers: O(n^3))",
        options._relaxationType, false);
  ap.parse();

  outputFormat += ",";

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

  NatalieCplexType natalie(options,
                           static_cast<NatalieCplexType::FormulationType>(formulationType),
                           lp);

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

  if (!natalie.init(pParserG1, pParserG2, pParserGm))
    return 1;

  int res = 0;

  try
  {
    res = natalie.solve(options._nOutputs);
    natalie.parseOutputString(outputFormat);
  }
  catch (std::exception& e)
  {
    std::cerr << e.what() << std::endl;
  }

  natalie.generateOutput(static_cast<OutputType::OutputType>(outputType), outputFile);

  delete pParserG1;
  delete pParserG2;
  delete pParserGm;

  return res;
}
