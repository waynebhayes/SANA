/* 
 * analysealignment.cpp
 *
 *  Created on: 18-jun-2011
 *      Author: M. El-Kebir
 */

#include <lemon/smart_graph.h>
#include <lemon/arg_parser.h>
#include <lemon/connectivity.h>
#include <string>
#include <sstream>

#include "natalie.h"
#include "input/matchinggraph.h"
#include "verbose.h"
#include "analyzealignment.h"
#include "score/scoremodel.h"
#include "score/scoremodelcoexpressiondiscrete.h"
#include <lemon/concepts/graph.h>

using namespace lemon;
using namespace nina;
using namespace nina::gna;

typedef SmartGraph Graph;
typedef SmartBpGraph BpGraph;
typedef MatchingGraph<Graph, BpGraph> InputBaseType;
typedef AnalyzeAlignment<Graph, BpGraph> AnalyzeAlignmentType;
typedef ScoreModel<Graph, BpGraph> ScoreModelType;
typedef ScoreModelCoexpressionDiscrete<Graph, BpGraph> ScoreModelCoexpressionType;
typedef Natalie<Graph, BpGraph> NatalieType;
typedef NatalieType::Options OptionsType;
typedef Parser<Graph> ParserType;
typedef BpParser<Graph, BpGraph> BpParserType;

template<typename GR, typename BGR>
AnalyzeAlignment<GR, BGR>::AnalyzeAlignment(const MatchingGraphType& matchingGraph,
                                            const ScoreModelType& scoreModel)
  : _matchingGraph(matchingGraph)
  , _scoreModel(scoreModel)
  , _alignmentG1(matchingGraph.getG1(), lemon::INVALID)
  , _alignmentG2(matchingGraph.getG2(), lemon::INVALID)
  , _g()
  , _isInG1(_g)
  , _fromG1toG(matchingGraph.getG1(), lemon::INVALID)
  , _fromG2toG(matchingGraph.getG2(), lemon::INVALID)
  , _fromGtoG12(_g, lemon::INVALID)
  , _mappedNodes(0)
  , _mappedEdges(0)
  , _EC(0)
  , _ICS(0)
  , _S3(0)
  , _sigmaScore(0)
  , _tauScore(0)
  , _componentMap(_g)
  , _edgeCountPerComponent1()
  , _edgeCountPerComponent2()
  , _nodeCountPerComponent1()
  , _nodeCountPerComponent2()
  , _largestComponentIdx(-1)
{
}

template<typename GR, typename BGR>
bool AnalyzeAlignment<GR, BGR>::init(const std::string& resultFileName)
{
  std::ifstream is(resultFileName.c_str());
  if (!is.good())
  {
    std::cerr << "Could not open input file" << std::endl;
    return false;
  }

  std::string line, label1, label2, bs;
  int i = 0;
  _mappedNodes = 0;
  while (std::getline(is, line))
  {
    i++;
    std::stringstream ss(line);
    ss >> label1 >> bs >> label2;

    Node node1 = _matchingGraph.getNodeG1(label1);
    Node node2 = _matchingGraph.getNodeG2(label2);

    if (node1 == lemon::INVALID)
    {
      std::cerr << "Line " << i << " of '" << resultFileName << "' label '"
        << label1 << "' does not occur in G_1" << std::endl;
      return false;
    }
    if (node2 == lemon::INVALID)
    {
      std::cerr << "Line " << i << " of '" << resultFileName << "' label '"
        << label2 << "' does not occur in G_2" << std::endl;
      return false;
    }
    if (_alignmentG1[node1] != lemon::INVALID)
    {
      std::cerr << "Alignment is not a matching: node with label '" 
        << label1 << "' is already matched." << std::endl;
      return false;
    }
    if (_alignmentG2[node2] != lemon::INVALID)
    {
      std::cerr << "Alignment is not a matching: node with label '" 
        << label2 << "' is already matched." << std::endl;
      return false;
    }

    _alignmentG1[node1] = node2;
    _alignmentG2[node2] = node1;
    _mappedNodes++;
  }

  is.close();

  // now it's time to construct _g
  _g.clear();

  const Graph& g1 = _matchingGraph.getG1();
  const Graph& g2 = _matchingGraph.getG2();

  // first we incorporate g1
  for (NodeIt n(g1); n != lemon::INVALID; ++n)
  {
    Node node = _g.addNode();
    _isInG1[node] = true;
    _fromG1toG[n] = node;
    _fromGtoG12[node] = n;
  }

  for (EdgeIt e(g1); e != lemon::INVALID; ++e)
  {
    Node u = g1.u(e);
    Node v = g1.v(e);

    _g.addEdge(_fromG1toG[u], _fromG1toG[v]);
  }

  // then we incorporate g2
  for (NodeIt n(g2); n != lemon::INVALID; ++n)
  {
    Node node = _g.addNode();
    _isInG1[node] = false;
    _fromG2toG[n] = node;
    _fromGtoG12[node] = n;
  }

  for (EdgeIt e(g2); e != lemon::INVALID; ++e)
  {
    Node u = g2.u(e);
    Node v = g2.v(e);

    _g.addEdge(_fromG2toG[u], _fromG2toG[v]);
  }

  // finally the alignment itself
  for (NodeIt node1(g1); node1 != lemon::INVALID; ++node1)
  {
    Node node2 = _alignmentG1[node1];
    if (node2 != lemon::INVALID)
      _g.addEdge(_fromG1toG[node1], _fromG2toG[node2]);
  }

  return true;
}

template<typename GR, typename BGR>
void AnalyzeAlignment<GR, BGR>::analyze()
{
  computeScore(true);
  computeEC();
  computeICS();
  computeS3();
  computeComponents();

  std::cout << "Number of nodes in G_1: " << _matchingGraph.getNodeCountG1() << std::endl;
  std::cout << "Number of edges in G_1: " << _matchingGraph.getEdgeCountG1() << std::endl;
  std::cout << "Number of nodes in G_2: " << _matchingGraph.getNodeCountG2() << std::endl;
  std::cout << "Number of edges in G_2: " << _matchingGraph.getEdgeCountG2() << std::endl;

  std::cout << "Number of mapped nodes: " << _mappedNodes << std::endl;
  std::cout << "Number of mapped edges: " << _mappedEdges << std::endl;

  std::cout << "Edge correctness (EC): " << _EC << std::endl;
  std::cout << "Induced Conserved Structure (ICS): " << _ICS << std::endl;
  std::cout << "Symmetric Substructure Score (S3): " << _S3 << std::endl;
  std::cout << "Score: " << _sigmaScore << " + " 
    << _tauScore << " = " << _sigmaScore + _tauScore << std::endl;

  std::cout << "Number of connected components: " << _nComponents << std::endl;

  std::cout << "Node count per component in G_1: [";
  bool first = true;
  for (IntVectorIt it = _nodeCountPerComponent1.begin(); it != _nodeCountPerComponent1.end(); it++)
  {
    if (first)
      first = false;
    else
      std::cout << ", ";

    std::cout << *it;
  }
  std::cout << "]" << std::endl;

  std::cout << "Node count per component in G_2: [";
  first = true;
  for (IntVectorIt it = _nodeCountPerComponent2.begin(); it != _nodeCountPerComponent2.end(); it++)
  {
    if (first)
      first = false;
    else
      std::cout << ", ";

    std::cout << *it;
  }
  std::cout << "]" << std::endl;

  std::cout << "Edge count per component in G_1: [";
  first = true;
  for (IntVectorIt it = _edgeCountPerComponent1.begin(); it != _edgeCountPerComponent1.end(); it++)
  {
    if (first)
      first = false;
    else
      std::cout << ", ";

    std::cout << *it;
  }
  std::cout << "]" << std::endl;

  std::cout << "Edge count per component in G_2: [";
  first = true;
  for (IntVectorIt it = _edgeCountPerComponent2.begin(); it != _edgeCountPerComponent2.end(); it++)
  {
    if (first)
      first = false;
    else
      std::cout << ", ";

    std::cout << *it;
  }
  std::cout << "]" << std::endl;

  std::cout << "Largest component size: " << getLargestComponentSize() << std::endl;
  std::cout << "Cluster coefficient (G_1) of largest component: " << getClusterCoefficient(_largestComponentIdx, true) << std::endl;
  std::cout << "Cluster coefficient (G_2) of largest component: " << getClusterCoefficient(_largestComponentIdx, false) << std::endl;
}

template<typename GR, typename BGR>
void AnalyzeAlignment<GR, BGR>::computeComponents()
{
  _nComponents = lemon::connectedComponents(_g, _componentMap);

  _nodeCountPerComponent1 = _nodeCountPerComponent2 = IntVector(_nComponents, 0);
  for (NodeIt n(_g); n != lemon::INVALID; ++n)
  {
    int compIdx = _componentMap[n];
    if (_isInG1[n])
      _nodeCountPerComponent1[compIdx]++;
    else
      _nodeCountPerComponent2[compIdx]++;
  }

  _edgeCountPerComponent1 = _edgeCountPerComponent2 = IntVector(_nComponents, 0);
  for (EdgeIt e(_g); e != lemon::INVALID; ++e)
  {
    Node u = _g.u(e);
    Node v = _g.v(e);

    int compIdx = _componentMap[u];
    assert(_componentMap[u] == _componentMap[v]);

    if (_isInG1[u] && _isInG1[v])
      _edgeCountPerComponent1[compIdx]++;
    else if (!_isInG1[u] && !_isInG1[v])
      _edgeCountPerComponent2[compIdx]++;
  }

  int max = -1;
  for (int i = 0; i < _nComponents; i++)
  {
    if (_nodeCountPerComponent1[i] + _nodeCountPerComponent2[i] > max)
    {
      max = _nodeCountPerComponent1[i] + _nodeCountPerComponent2[i];
      _largestComponentIdx = i;
    }
  }
}

template<typename GR, typename BGR>
double AnalyzeAlignment<GR, BGR>::computeEC()
{
  const Graph& g1 = _matchingGraph.getG1();

  _mappedEdges = 0;

  for (EdgeIt e(g1); e != lemon::INVALID; ++e)
  {
    Node u1 = g1.u(e);
    Node v1 = g1.v(e);

    Node u2 = _alignmentG1[u1];
    Node v2 = _alignmentG1[v1];

    if (u2 != lemon::INVALID && v2 != lemon::INVALID 
        && _matchingGraph.getEdgeG2(u2, v2) != lemon::INVALID)
    {
      _mappedEdges++;
    }
  }

  _EC = static_cast<double>(_mappedEdges) / 
    std::min(_matchingGraph.getEdgeCountG1(), _matchingGraph.getEdgeCountG2());

  return _EC;
}

template<typename GR, typename BGR>
double AnalyzeAlignment<GR, BGR>::computeICS()
{
  const Graph& g1 = _matchingGraph.getG1();
  const Graph& g2 = _matchingGraph.getG2();
  
  _mappedEdges = 0;
  BoolNodeMap v_in_g2fV1(g2, false);
  BoolEdgeMap e_in_g2fV1(g2, true);

  for (EdgeIt e(g1); e != lemon::INVALID; ++e)
  {
    Node u1 = g1.u(e);
    Node v1 = g1.v(e);
    
    Node u2 = _alignmentG1[u1];
    Node v2 = _alignmentG1[v1];

    if (u2 != lemon::INVALID && v2 != lemon::INVALID 
        && _matchingGraph.getEdgeG2(u2, v2) != lemon::INVALID)
    {
      _mappedEdges++;
      v_in_g2fV1[u2] = v_in_g2fV1[v2] = true;
    }
  }

  SubGraph<const Graph> g2fV1(g2, v_in_g2fV1, e_in_g2fV1);

  _ICS = static_cast<double>(_mappedEdges) / countEdges(g2fV1);

  return _ICS;
}

template<typename GR, typename BGR>
double AnalyzeAlignment<GR, BGR>::computeS3()
{
  const Graph& g1 = _matchingGraph.getG1();
  const Graph& g2 = _matchingGraph.getG2();
  
  _mappedEdges = 0;
  BoolNodeMap v_in_g2fV1(g2, false);
  BoolEdgeMap e_in_g2fV1(g2, true);

  for (EdgeIt e(g1); e != lemon::INVALID; ++e)
  {
    Node u1 = g1.u(e);
    Node v1 = g1.v(e);
    
    Node u2 = _alignmentG1[u1];
    Node v2 = _alignmentG1[v1];

    if (u2 != lemon::INVALID && v2 != lemon::INVALID 
        && _matchingGraph.getEdgeG2(u2, v2) != lemon::INVALID)
    {
      _mappedEdges++;
      v_in_g2fV1[u2] = v_in_g2fV1[v2] = true;
    }
  }

  SubGraph<const Graph> g2fV1(g2, v_in_g2fV1, e_in_g2fV1);

  _S3 = static_cast<double>(_mappedEdges) / (std::min(_matchingGraph.getEdgeCountG1(), _matchingGraph.getEdgeCountG2()) + countEdges(g2fV1) - _mappedEdges);

  return _S3;
}

template<typename GR, typename BGR>
double AnalyzeAlignment<GR, BGR>::computeScore(bool useSigma)
{
  const Graph& g1 = _matchingGraph.getG1();
  const BpGraph& gm = _matchingGraph.getGm();

  _sigmaScore = 0;
  if (useSigma)
  {
    lemon::ArcLookUp<BpGraph> arcLookUpGm(gm);

    for (NodeIt i(g1); i != lemon::INVALID; ++i)
    {
      Node k = _alignmentG1[i];
      if (k != lemon::INVALID)
      {
        BpNode bp_i = _matchingGraph.mapG1ToGm(i);
        BpNode bp_k = _matchingGraph.mapG2ToGm(k);

        BpEdge ik = arcLookUpGm(bp_i, bp_k);
        if (ik != lemon::INVALID)
        {
          _sigmaScore += _scoreModel.getWeightGm(ik);

          if (g_verbosity >= VERBOSE_NON_ESSENTIAL)
          {
            std::cout << _matchingGraph.getLabelG1(i) 
              << "\t" << _matchingGraph.getLabelG2(k)
              << "\t" << _scoreModel.getWeightGm(ik) << std::endl;
          }
        }
      }
    }
  }

  _tauScore = 0;
  for (EdgeIt ij(g1); ij != lemon::INVALID; ++ij)
  {
    Node i = g1.u(ij);
    Node j = g1.v(ij);

    Node k = _alignmentG1[i];
    Node l = _alignmentG1[j];

    if (k != lemon::INVALID && l != lemon::INVALID)
    {
      Edge kl = _matchingGraph.getEdgeG2(k, l);
      if (kl != lemon::INVALID)
      {
        _tauScore += _scoreModel.getWeightG1G2(ij, kl);
        if (g_verbosity >= VERBOSE_NON_ESSENTIAL)
        {
          std::cout << _matchingGraph.getLabelG1(i)
            << "\t" << _matchingGraph.getLabelG2(k)
            << "\t" << _matchingGraph.getLabelG1(j)
            << "\t" << _matchingGraph.getLabelG2(l)
            << "\t" << _scoreModel.getWeightG1G2(ij, kl) << std::endl;
        }
      }
    }
  }

  if (g_verbosity >= VERBOSE_ESSENTIAL)
  {
    std::cout << "Sigma: " << _sigmaScore << std::endl 
      << "Tau: " << _tauScore << std::endl;
  }

  return _sigmaScore + _tauScore;
}

int main(int argc, char** argv)
{
  ArgParser ap(argc, argv);

  // parameters input
  std::string g1, g2, gm;
  int inputFormatG1 = static_cast<int>(NatalieType::IN_STRING);
  int inputFormatG2 = static_cast<int>(NatalieType::IN_STRING);
  int inputFormatGm = static_cast<int>(NatalieType::BP_IN_BLAST);
  int verbosityLevel = static_cast<int>(VERBOSE_NONE);
  double beta = 1;
  bool discretizeWeight = false;
  double eValCutOff = 10;
  double corrThreshold = 0.9;
  int scoreModel = static_cast<int>(NatalieType::AVERAGE);

  ap.refOption("v", "Specifies the verbosity level:\n"
                    "     0 - No output\n"
                    "     1 - Only necessary output\n"
                    "     2 - More verbose output (default)\n"
                    "     3 - Debug output", verbosityLevel, false)
    .synonym("-verbosity", "v")
    .refOption("beta", "Parameter used in the objective function (default: 1)",
        beta, false)
    .refOption("d", "Force binary edge weights", discretizeWeight, false)
    .refOption("g1", "File name of input graph G_1", g1, true)
    .refOption("g2", "File name of input graph G_2", g2, true)
    .refOption("gm", "File name in which matching edges of G_m are defined;\n"
                     "     if omitted, no sigma component is used", gm, false)
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
    .refOption("e", "Specifies e value cutoff (only used in conjunction\n"
                    "     with -ifm 1 and -ifm 2, default: 10)", eValCutOff, false)
    .refOption("c", "Specifies the confidence/correlation threshold\n"
                      "     (only used in conjunction with -sf [1,2,3]\n"
                      "     or -if1 2 or -if2 2, default: 0.9)", corrThreshold, false)
    .refOption("sf", "Specifies the score function to be used:\n"
                     "     0 - Average weights (default)\n"
                     "     1 - Coexpression-based: continuous function\n"
                     "     2 - Coexpression-based: scaled continuous function\n"
                     "     3 - Coexpression-based: discrete values\n"
                     "     4 - Coexpression-based: discrete inverse values\n"
                     "     5 - Edge correctness\n"
                     "     6 - Bit score", scoreModel, false);
  ap.parse();

  if (ap.files().size() == 0)
  {
    std::cerr << "Missing input file" << std::endl;
    return 1;
  }

  if (!ap.given("g1") || !ap.given("g2"))
  {
    std::cerr << "Both -g1 and -g2 need to be specified" << std::endl;
    return 1;
  }

  if (!(0 <= beta && beta <= 1))
  {
    std::cerr << "Parameter beta should be between 0 and 1" << std::endl;
    return 1;
  }

  if (!(0 <= corrThreshold && corrThreshold <= 1))
  {
    std::cerr << "Correlation threshold (-c) should be between 0 and 1" << std::endl;
    return 1;
  }

  g_verbosity = static_cast<VerbosityLevel>(verbosityLevel);
  const std::string& inputFileName = ap.files()[0];

  // Read input instance
  InputBaseType input;


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

  if (!input.init(pParserG1, pParserG2, pParserGm))
    return 1;

  // Initialize score model
  NatalieType::ScoreFunctionEnum score =
    static_cast<NatalieType::ScoreFunctionEnum>(scoreModel);

  ScoreModelType* pScoreModel = NatalieType::createScoreModel(input, score, corrThreshold, beta, discretizeWeight, true);
  AnalyzeAlignmentType analyze(input, *pScoreModel);
  if (!analyze.init(inputFileName))
    return 1;

  analyze.analyze();

  delete pScoreModel;
  return 0;
}
