/*
 * makegraphs.cpp
 *
 *  Created on: 16-jan-2012
 *      Author: M. El-Kebir and M.E. van der Wees
 */

#include <string.h>
#include <sstream>
#include <lemon/list_graph.h>
#include <lemon/smart_graph.h>
#include <lemon/arg_parser.h>
#include "natalie.h"
#include "input/matchinggraph.h"
#include "verbose.h"
#include "config.h"

using namespace lemon;
using namespace nina::gna;
using namespace nina;

typedef SmartGraph Graph;
typedef SmartBpGraph BpGraph;
typedef Natalie<Graph, BpGraph> NatalieType;
typedef Parser<Graph> ParserType;
typedef BpParser<Graph, BpGraph> BpParserType;
typedef MatchingGraph<Graph, BpGraph> InputType;

int main(int argc, char** argv)
{
  ArgParser ap(argc, argv);

  // parameters input
  std::string g1, g2, gm;
  int inputFormatG1 = static_cast<int>(NatalieType::IN_STRING);
  int inputFormatG2 = static_cast<int>(NatalieType::IN_STRING);
  int inputFormatGm = static_cast<int>(NatalieType::BP_IN_BLAST);
  double eValCutOff = 10;
  double corrThreshold = 0.9;
  int verbosityLevel = static_cast<int>(VERBOSE_NON_ESSENTIAL);

  ap
    .boolOption("version", "Show version number")
    .refOption("v", "Specifies the verbosity level:\n"
                    "     0 - No output\n"
                    "     1 - Only necessary output\n"
                    "     2 - More verbose output (default)\n"
                    "     3 - Debug output", verbosityLevel, false)
    .synonym("-verbosity", "v")
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
    .refOption("e", "Specifies e value cutoff (only used in conjunction with -ifm 1 and -ifm 2, default: 10)",
        eValCutOff, false)
    .refOption("c", "Specifies the correlation threshold (only used in conjunction with -ifm 2, default: 0.9)", 
        corrThreshold, false);
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

  if (!(0 <= corrThreshold && corrThreshold <= 1))
  {
    std::cerr << "Correlation threshold (-c) should be between 0 and 1" << std::endl;
    return 1;
  }

  g_verbosity = static_cast<VerbosityLevel>(verbosityLevel);

  // Read input instance
  ParserType* pParserG1 =
      NatalieType::createParser(g1,
                                static_cast<NatalieType::InputFormatEnum>(inputFormatG1),
                                corrThreshold);

  ParserType* pParserG2 =
      NatalieType::createParser(g2,
                                static_cast<NatalieType::InputFormatEnum>(inputFormatG2),
                                corrThreshold);

  BpParserType* pParserGm =
      NatalieType::createBpParser(gm,
                                  static_cast<NatalieType::BpInputFormatEnum>(inputFormatGm),
                                  pParserG1,
                                  pParserG2,
                                  eValCutOff);
  
  InputType* pInput = new InputType();

  if (!pInput->init(pParserG1, pParserG2, pParserGm))
    return 1;

  // Transform correlation threshold into string to be saved in g1 and g2 filenames
  std::stringstream thresholdStream;
  thresholdStream << corrThreshold;
  std::string stringThreshold = thresholdStream.str();

  // Transform e-value cut-off into string to be saved in gm filename
  std::stringstream eValCutOffStream;
  eValCutOffStream << eValCutOff;
  std::string stringEValCutOff = eValCutOffStream.str();

  std::ofstream outFileG1((g1 + "-" + stringThreshold + ".lgf").c_str());
  if (!outFileG1.good())
  {
    std::cerr << "Error: could not open file '" << g1 + ".lgf'" << std::endl;
    return false;
  }
  pInput->saveG1(outFileG1);

  std::ofstream outFileG2((g2 + "-" + stringThreshold + ".lgf").c_str());
  if (!outFileG2.good())
  {
    std::cerr << "Error: could not open file '" << g2 + ".lgf'" << std::endl;
    return false;
  }
  pInput->saveG2(outFileG2);

  std::ofstream outFileGm((gm + "-" + stringEValCutOff + ".lgf").c_str());
  if (!outFileGm.good())
  {
    std::cerr << "Error: could not open file '" << gm + ".lgf'" << std::endl;
    return false;
  }
  pInput->saveGm(outFileGm);

  delete pInput;
  std::cout << "Succesfully created lgf files for e " << eValCutOff << " and c "
            << corrThreshold << std::endl;
  return 0;
}
