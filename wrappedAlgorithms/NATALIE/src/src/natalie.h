/*
 * natalie.h
 *
 *  Created on: 17-feb-2012
 *      Author: M. El-Kebir
 */

#ifndef NATALIE_H_
#define NATALIE_H_

#include "gna.h"
#include "input/parser.h"
#include "input/csvparser.h"
#include "input/gmlparser.h"
#include "input/graphmlparser.h"
#include "input/ledaparser.h"
#include "input/lgfparser.h"
#include "input/stringparser.h"
#include "input/edgelistparser.h"
#include "input/bpparser.h"
#include "input/bpblastparser.h"
#include "input/bpcandlistparser.h"
#include "input/bplgfparser.h"
#include "lagrange/lagrangegna.h"
#include "lagrange/lagrangegnaldcached.h"
#include "lagrange/lagrangegnasicached.h"
#include "lagrange/lagrangesolver.h"
#include "lagrange/subgradientsolver.h"
#include "lagrange/dualdescentsolver.h"
#include "lagrange/combinedsolver.h"
#include "output/output.h"
#include "output/outputanalyse.h"
#include "output/outputdot.h"
#include "output/outputlgf.h"
#include "output/outputsif.h"
#include "output/outputgml.h"
#include "output/outputjson.h"
#include "output/outputneato.h"
#include "output/outputcsv.h"
#include "output/outputeda.h"
#include "output/outputnoa.h"
#include "score/scoremodel.h"
#include "score/scoremodelcoexpressioncontinuous.h"
#include "score/scoremodelcoexpressioncontinuous2.h"
#include "score/scoremodelcoexpressiondiscrete.h"
#include "score/scoremodelcoexpressiondiscreteinverse.h"
#include "score/scoremodeledgecorrectness.h"
#include "score/scoremodelbitscore.h"
#include "input/matchinggraphshuffle.h"

namespace nina {
namespace gna {

template<typename GR, typename BGR>
class Natalie : public PairwiseGlobalNetworkAlignment<GR, BGR>
{
public:
  /// The graph type of the input graphs
  typedef GR Graph;
  /// The graph type of the bipartite matching graph
  typedef BGR BpGraph;
  /// Base class type
  typedef PairwiseGlobalNetworkAlignment<GR, BGR> Parent;
  /// Type of a map assigning a boolean to every matching edge
  typedef typename Parent::BpBoolMap BpBoolMap;
  /// Type of a matching map: maps a node to its matching edge
  typedef typename Parent::BpMatchingMap BpMatchingMap;
  /// Type of the matching graph
  typedef typename Parent::MatchingGraphType MatchingGraphType;
  /// Type of the analysis
  typedef Analyse<Graph, BpGraph> AnalyseType;

  /// Type of the output
  typedef typename Parent::OutputType OutputType;
  typedef OutputLgf<Graph, BpGraph> OutputLgfType;
  typedef OutputSif<Graph, BpGraph> OutputSifType;
  typedef OutputDot<Graph, BpGraph> OutputDotType;
  typedef OutputGml<Graph, BpGraph> OutputGmlType;
  typedef OutputJson<Graph, BpGraph> OutputJsonType;
  typedef OutputNeato<Graph, BpGraph> OutputNeatoType;
  typedef OutputCsv<Graph, BpGraph> OutputCsvType;
  typedef OutputEda<Graph, BpGraph> OutputEdaType;
  typedef OutputNoa<Graph, BpGraph> OutputNoaType;
  typedef OutputAnalyse<Graph, BpGraph> OutputAnalyseType;

  /// Type of a vector of output methods
  typedef typename Parent::OutputVector OutputVector;
  /// Iterator type of output vector
  typedef typename Parent::OutputVectorIt OutputVectorIt;

  /// Type of the score model
  typedef typename Parent::ScoreModelType ScoreModelType;
  typedef ScoreModelCoexpressionContinuous<Graph, BpGraph> ScoreModelCoexpressionContinuousType;
  typedef ScoreModelCoexpressionContinuous2<Graph, BpGraph> ScoreModelCoexpressionContinuous2Type;
  typedef ScoreModelCoexpressionDiscrete<Graph, BpGraph> ScoreModelCoexpressionDiscreteType;
  typedef ScoreModelCoexpressionDiscreteInverse<Graph, BpGraph> ScoreModelCoexpressionDiscreteInverseType;
  typedef ScoreModelEdgeCorrectness<Graph, BpGraph> ScoreModelEdgeCorrectnessType;
  typedef ScoreModelBitScore<Graph, BpGraph> ScoreModelBitScoreType;

  /// Type of input graph parser
  typedef Parser<Graph> ParserType;
  typedef CSVParser<Graph> ParserCsvType;
  typedef GMLParser<Graph> ParserGmlType;
  typedef GraphMLParser<Graph> ParserGraphMLType;
  typedef LedaParser<Graph> ParserLedaType;
  typedef LgfParser<Graph> ParserLgfType;
  typedef StringParser<Graph> ParserStringType;
  typedef EdgeListParser<Graph> ParserEdgeListType;

  /// Type of input matching graph parser
  typedef BpParser<Graph, BpGraph> BpParserType;
  typedef BpLgfParser<Graph, BpGraph> BpParserLgfType;
  typedef BpCandListParser<Graph, BpGraph> BpParserCandListType;
  typedef BpBlastParser<Graph, BpGraph> BpParserBlastType;

  /// Type of the Lagrangian relaxation
  typedef LagrangeGna<Graph, BpGraph> LagrangeGnaType;
  /// Type of the cached Lagrangian relaxation
  typedef LagrangeGnaCached<Graph, BpGraph> LagrangeGnaCachedType;
  /// Type of the cached Lagrangian Decompisition relaxation
  typedef LagrangeGnaLdCached<Graph, BpGraph> LagrangeGnaLdCachedType;
  /// Type of the cached Subproblem Isolation relaxation
  typedef LagrangeGnaSiCached<Graph, BpGraph> LagrangeGnaSiCachedType;
  /// Type of the solver
  typedef LagrangeSolver<double, bool, double, int> SolverType;
  /// Type of the subgradient optimization solver
  typedef SubgradientSolver<double, bool, double, int> SubgradientSolverType;
  /// Type of the dual descent solver
  typedef DualDescentSolver<Graph, BpGraph> DualDescentSolverType;
  /// Type of the combined solver
  typedef CombinedSolver<Graph, BpGraph> CombinedSolverType;
  /// Type of list of value-primal vector pairs
  typedef SolverType::ValuePrimalPairList ValuePrimalPairList;
  /// Const iterator type of value-primal vector pairs list
  typedef SolverType::ValuePrimalPairListIt ValuePrimalPairListIt;
  /// Global problem base type
  typedef typename LagrangeGnaType::GlobalProblemBaseType GlobalProblemBaseType;

  typedef MatchingGraphShuffle<Graph, BpGraph> MatchingGraphShuffleType;

  using Parent::_pMatchingGraph;
  using Parent::_pScoreModel;
  using Parent::_outputs;
  using Parent::addOutput;

  /// Output format type
  typedef enum {
                 BP_OUT_DOT,
                 BP_OUT_GML,
                 BP_OUT_LGF,
                 BP_OUT_SIF,
                 BP_OUT_JSON,
                 BP_OUT_NEATO,
                 BP_OUT_CSV_MATCHED,
                 BP_OUT_CSV_UNMATCHED_IN_G1,
                 BP_OUT_CSV_UNMATCHED_IN_G2,
                 BP_OUT_CSV_ALIGNMENT,
                 BP_OUT_SIF_EDGE_ATTR,
                 BP_OUT_SIF_NODE_ATTR,
               } OutputFormatEnum;
  /// Relaxation type
  typedef enum {
                 //LAGRANGIAN_DECOMPOSITION,
                 LAGRANGIAN_DECOMPOSITION_CACHED,
                 //SUBPROBLEM_ISOLATION,
                 SUBPROBLEM_ISOLATION_CACHED,
               } RelaxationEnum;
  /// Solver type
  typedef enum {
    SUBGRADIENT_OPTIMIZATION,
    DUAL_DESCENT,
    DUAL_DESCENT_ENHANCED,
    COMBINED,
  } SolverEnum;
  /// Matching graph input format type
  typedef enum {
                 BP_IN_CAND_LIST,
                 BP_IN_BLAST,
                 BP_IN_LGF,
               } BpInputFormatEnum;
  /// Input format type
  typedef enum {
                 IN_GML,
                 IN_GRAPHML,
                 IN_STRING,
                 IN_LGF,
                 IN_CSV,
                 IN_LEDA,
                 IN_EDGE_LIST,
               } InputFormatEnum;
  /// Score function
  typedef enum {
                 AVERAGE,
                 COEXPRESSION_CONTINUOUS,
                 COEXPRESSION_CONTINUOUS2,
                 COEXPRESSION_DISCRETE,
                 COEXPRESSION_DISCRETEINVERSE,
                 EDGE_CORRECTNESS,
                 BIT_SCORE,
               } ScoreFunctionEnum;

  /// Options type
  struct Options
  {
    double _eValCutOff;
    double _corrThreshold;
    double _beta;
    bool _discretizeWeight;
    bool _normalize;
    int _scoreModel;

    // parameters solver
    int _relaxationType;
    int _nMaxIterations;
    int _timeLimit;
    int _solverType;
    bool _integral;
    int _nOutputs;

    // parameters subgradient solver
    double _mu;
    double _alpha;
    int _nMaxNondecreasingIterations;
    int _nMaxIncreasingIterations;

    // parameters dual descent solver
    double _tau;
    double _phi;

    // parameters combined solver
    int _maxSubgradientSteps;
    int _maxDualDescentSteps;
    int _maxSwitches;

    int _maxJsonNodes;

    Options()
      : _eValCutOff(10)
      , _corrThreshold(0.9)
      , _beta(1)
      , _discretizeWeight(false)
      , _normalize(true)
      , _scoreModel(static_cast<int>(AVERAGE))
      , _relaxationType(static_cast<int>(LAGRANGIAN_DECOMPOSITION_CACHED))
      , _nMaxIterations(-1)
      //, _relaxationType(0)
      , _timeLimit(-1)
      , _solverType(static_cast<int>(COMBINED))
      , _integral(false)
      , _nOutputs(1)
      , _mu(1)
      , _alpha(2)
      , _nMaxNondecreasingIterations(20)
      , _nMaxIncreasingIterations(10)
      , _tau(1)
      , _phi(.5)
      , _maxSubgradientSteps(-1)
      , _maxDualDescentSteps(100)
      , _maxSwitches(3)
      , _maxJsonNodes(50)
    {
    }
  };

protected:
  Options _options;
  mutable LagrangeGnaType* _pLagrangeGna;
  SolverType* _pSolver;
  double _elapsedTime;
  double _score;
  double _upperBound;

public:
  /// Constructor
  Natalie();
  /// Constructor
  Natalie(const Options& options);
  /// Destructor
  virtual ~Natalie();
  /// Initializes the graphs using the specified parsers
  virtual bool init(ParserType* pParserG1,
                    ParserType* pParserG2,
                    BpParserType* pParserGm);
  /// Solves the problem and generates at most nSolutions
  ///
  /// \pre init must have been called successfully
  virtual int solve(int nSolutions = 1);
  /// Determines whether there exists a solution with score at least
  /// the specified value
  virtual int exists(double score, bool& result);
  /// Stores solution number i in map m
  ///
  /// \pre 0 <= i < getNumberOfSolutions()
  virtual void getSolution(BpBoolMap& m, int i = 0) const;
  /// Stores solution number i in map m
  ///
  /// \pre 0 <= i < getNumberOfSolutions()
  virtual void getSolution(BpMatchingMap& m, int i = 0) const;
  /// Returns the number of solutions generated
  virtual int getNumberOfSolutions() const;

  virtual double computePValue(int n);

  bool initMatchingGraph(ParserType* pParserG1,
                         ParserType* pParserG2,
                         BpParserType* pParserGm)
  {
    bool res = false;
    if (pParserG1 && pParserG2 && pParserGm)
      res = _pMatchingGraph->init(pParserG1, pParserG2, pParserGm);

    if (res)
    {
      if (!_pScoreModel)
        setScoreModel(static_cast<ScoreFunctionEnum>(_options._scoreModel), pParserG1, pParserG2);

      _pScoreModel->init();

      if (!_pLagrangeGna)
        setRelaxation(static_cast<RelaxationEnum>(_options._relaxationType));
    }

    return res;
  }

  void init()
  {
    _pLagrangeGna->init();

    if (!_pSolver)
      setSolver(static_cast<SolverEnum>(_options._solverType));

    _pSolver->init(0);
  }

  double getElapsedTime() const
  {
    return _elapsedTime;
  }

  double getScore() const
  {
    return _score;
  }

  double getUpperBound() const
  {
    return _upperBound;
  }

  const Options& getOptions() const
  {
    return _options;
  }

  const LagrangeGnaType* getRelaxation() const { return _pLagrangeGna; }

  void setScoreModel(ScoreFunctionEnum type,
                     ParserType* pParserG1,
                     ParserType* pParserG2);

  void setRelaxation(LagrangeGnaType* pLagrangeGna);
  void setRelaxation(RelaxationEnum type);

  void setSolver(SolverType* pSolver);
  void setSolver(SolverEnum type);

  void addOutput(OutputFormatEnum fmt);
  void parseOutputString(const std::string& str);

  void setGlobalProblem(GlobalProblemBaseType* pGlobal)
  {
    _pLagrangeGna->setGlobalProblem(pGlobal);
  }

  static ScoreModelType* createScoreModel(const MatchingGraphType& matchingGraph,
                                          ScoreFunctionEnum type,
                                          double corrThreshold,
                                          double beta,
                                          bool discretizeWeight,
                                          bool normalize);


  static ParserType* createParser(const std::string& filename,
                                  InputFormatEnum fmt,
                                  double corrThreshold);

  static BpParserType* createBpParser(const std::string& filename,
                                      BpInputFormatEnum fmt,
                                      ParserType* pParserG1,
                                      ParserType* pParserG2,
                                      double eValCutOff);
};

template<typename GR, typename BGR>
inline int Natalie<GR, BGR>::getNumberOfSolutions() const
{
  return static_cast<int>(_pSolver->getBestSolutions().size());
}

template<typename GR, typename BGR>
inline void Natalie<GR, BGR>::getSolution(BpBoolMap& m, int i) const
{
  assert(0 <= i && i < getNumberOfSolutions());

  const ValuePrimalPairList& bestSolutions = _pSolver->getBestSolutions();

  ValuePrimalPairListIt it = bestSolutions.begin();
  for (; it != bestSolutions.end() && i > 0; it++, i--);

  _pLagrangeGna->setPrimalVector(it->second);
  _pLagrangeGna->getMatching(m);
}

template<typename GR, typename BGR>
inline void Natalie<GR, BGR>::getSolution(BpMatchingMap& m, int i) const
{
  assert(0 <= i && i < getNumberOfSolutions());

  const ValuePrimalPairList& bestSolutions = _pSolver->getBestSolutions();

  ValuePrimalPairListIt it = bestSolutions.begin();
  for (; it != bestSolutions.end() && i > 0; it++, i--);

  _pLagrangeGna->setPrimalVector(it->second);
  _pLagrangeGna->getMatching(m);
}

template<typename GR, typename BGR>
inline void Natalie<GR, BGR>::parseOutputString(const std::string& str)
{
  size_t idx = 0, new_idx = 0;
  while ((new_idx = str.find(",", idx)) != std::string::npos)
  {
    std::string substr = str.substr(idx, new_idx - idx);
    if (substr == "0")
      addOutput(static_cast<OutputFormatEnum>(0));
    else if (substr == "1")
      addOutput(static_cast<OutputFormatEnum>(1));
    else if (substr == "2")
      addOutput(static_cast<OutputFormatEnum>(2));
    else if (substr == "3")
      addOutput(static_cast<OutputFormatEnum>(3));
    else if (substr == "4")
      addOutput(static_cast<OutputFormatEnum>(4));
    else if (substr == "5")
      addOutput(static_cast<OutputFormatEnum>(5));
    else if (substr == "6")
      addOutput(static_cast<OutputFormatEnum>(6));
    else if (substr == "7")
      addOutput(static_cast<OutputFormatEnum>(7));
    else if (substr == "8")
      addOutput(static_cast<OutputFormatEnum>(8));
    else if (substr == "9")
      addOutput(static_cast<OutputFormatEnum>(9));
    else if (substr == "10")
      addOutput(static_cast<OutputFormatEnum>(10));
    else if (substr == "11")
      addOutput(static_cast<OutputFormatEnum>(11));

    idx = new_idx + 1;
  }
}

template<typename GR, typename BGR>
inline void Natalie<GR, BGR>::setScoreModel(ScoreFunctionEnum type,
                                            ParserType* pParserG1,
                                            ParserType* pParserG2)
{
  delete _pScoreModel;
  _pScoreModel = createScoreModel(*_pMatchingGraph, type,
                                  _options._corrThreshold,
                                  _options._beta,
                                  _options._discretizeWeight,
                                  _options._normalize);
}

template<typename GR, typename BGR>
inline void Natalie<GR, BGR>::setSolver(SolverType* pSolver)
{
  delete _pSolver;
  _pSolver = pSolver;
}

template<typename GR, typename BGR>
inline void Natalie<GR, BGR>::setSolver(SolverEnum type)
{
  delete _pSolver;
  _pSolver = NULL;

  switch (type)
  {
    case SUBGRADIENT_OPTIMIZATION:
      _pSolver = new SubgradientSolverType(_pLagrangeGna,
                                           _options._timeLimit,
                                           _options._nMaxIterations,
                                           _options._nOutputs,
                                           _options._nMaxNondecreasingIterations,
                                           _options._nMaxIncreasingIterations,
                                           _options._mu,
                                           _options._alpha);
      break;
    case DUAL_DESCENT:
    case DUAL_DESCENT_ENHANCED:
      {
        LagrangeGnaCachedType* pCachedGna =
          dynamic_cast<LagrangeGnaCachedType*>(_pLagrangeGna);
        if (pCachedGna)
        {
          _pSolver = new DualDescentSolverType(pCachedGna,
                                               _options._timeLimit,
                                               _options._nMaxIterations,
                                               _options._nOutputs,
                                               _options._tau,
                                               _options._phi,
                                               type == DUAL_DESCENT_ENHANCED);
        }
        else
        {
          std::cerr << "The dual descent procedure requires a cached relaxation variant."
                    << std::endl;
        }
      }
      break;
    case COMBINED:
      {
        LagrangeGnaCachedType* pCachedGna =
          dynamic_cast<LagrangeGnaCachedType*>(_pLagrangeGna);
        if (pCachedGna)
        {
          _pSolver = new CombinedSolverType(pCachedGna,
                                            _options._timeLimit,
                                            _options._nMaxIterations,
                                            _options._nOutputs,
                                            _options._nMaxNondecreasingIterations,
                                            _options._nMaxIncreasingIterations,
                                            _options._mu,
                                            _options._alpha,
                                            _options._tau,
                                            _options._phi,
                                            false,
                                            _options._maxDualDescentSteps,
                                            _options._maxSubgradientSteps,
                                            _options._maxSwitches);
        }
        else
        {
          std::cerr << "The combined procedure requires a cached relaxation variant."
                    << std::endl;
        }
      }
      break;
  }
}

template<typename GR, typename BGR>
inline void Natalie<GR, BGR>::addOutput(OutputFormatEnum fmt)
{
  OutputType* pOutput = NULL;

  switch (fmt)
  {
    case BP_OUT_DOT:
      pOutput = new OutputDotType(*_pMatchingGraph);
      break;
    case BP_OUT_GML:
      pOutput = new OutputGmlType(*_pMatchingGraph);
      break;
    case BP_OUT_SIF:
      pOutput = new OutputSifType(*_pMatchingGraph);
      break;
    case BP_OUT_LGF:
      pOutput = new OutputLgfType(*_pMatchingGraph);
      break;
    case BP_OUT_JSON:
      pOutput = new OutputJsonType(*_pMatchingGraph, _options._maxJsonNodes);
      break;
    case BP_OUT_NEATO:
      pOutput = new OutputNeatoType(*_pMatchingGraph);
      break;
    case BP_OUT_CSV_MATCHED:
      pOutput = new OutputCsvType(*_pMatchingGraph, OutputCsvType::CSV_MATCHED);
      break;
    case BP_OUT_CSV_UNMATCHED_IN_G1:
      pOutput = new OutputCsvType(*_pMatchingGraph, OutputCsvType::CSV_UNMATCHED_IN_G1);
      break;
    case BP_OUT_CSV_UNMATCHED_IN_G2:
      pOutput = new OutputCsvType(*_pMatchingGraph, OutputCsvType::CSV_UNMATCHED_IN_G2);
      break;
    case BP_OUT_CSV_ALIGNMENT:
      pOutput = new OutputCsvType(*_pMatchingGraph, OutputCsvType::CSV_ALIGNMENT);
      break;
    case BP_OUT_SIF_EDGE_ATTR:
      pOutput = new OutputEdaType(*_pMatchingGraph);
      break;
    case BP_OUT_SIF_NODE_ATTR:
      pOutput = new OutputNoaType(*_pMatchingGraph);
      break;
  }

  addOutput(pOutput);
}

template<typename GR, typename BGR>
inline void Natalie<GR, BGR>::setRelaxation(LagrangeGnaType* pLagrangeGna)
{
  delete _pLagrangeGna;
  _pLagrangeGna = pLagrangeGna;
}

template<typename GR, typename BGR>
inline void Natalie<GR, BGR>::setRelaxation(RelaxationEnum type)
{
  delete _pLagrangeGna;
  _pLagrangeGna = NULL;

  switch (type)
  {
    case LAGRANGIAN_DECOMPOSITION_CACHED:
      _pLagrangeGna = new LagrangeGnaLdCachedType(*_pMatchingGraph,
                                                  *_pScoreModel,
                                                  _options._integral);
      break;
    case SUBPROBLEM_ISOLATION_CACHED:
      _pLagrangeGna = new LagrangeGnaSiCachedType(*_pMatchingGraph,
                                                  *_pScoreModel,
                                                  _options._integral);
      break;
  }
}

template<typename GR, typename BGR>
inline typename Natalie<GR, BGR>::ScoreModelType*
    Natalie<GR, BGR>::createScoreModel(const MatchingGraphType& matchingGraph,
                                       ScoreFunctionEnum type,
                                       double corrThreshold,
                                       double beta,
                                       bool discretizeWeight,
                                       bool normalize)
{
  ScoreModelType* pScoreModel = NULL;

  switch (type)
  {
  case AVERAGE:
    pScoreModel = new ScoreModelType(matchingGraph,
                                     beta,
                                     discretizeWeight,
                                     normalize);
    break;
  case COEXPRESSION_CONTINUOUS:
    pScoreModel = new ScoreModelCoexpressionContinuousType(matchingGraph,
                                                           beta,
                                                           corrThreshold);
    break;
  case COEXPRESSION_CONTINUOUS2:
    pScoreModel = new ScoreModelCoexpressionContinuous2Type(matchingGraph,
                                                            beta,
                                                            corrThreshold);
    break;
  case COEXPRESSION_DISCRETE:
    pScoreModel = new ScoreModelCoexpressionDiscreteType(matchingGraph,
                                                         beta,
                                                         corrThreshold);
    break;
  case COEXPRESSION_DISCRETEINVERSE:
    pScoreModel = new ScoreModelCoexpressionDiscreteInverseType(matchingGraph,
                                                                beta,
                                                                corrThreshold);
    break;
  case EDGE_CORRECTNESS:
    pScoreModel = new ScoreModelEdgeCorrectnessType(matchingGraph);
    break;
  case BIT_SCORE:
    pScoreModel = new ScoreModelBitScoreType(matchingGraph);
    break;
  }

  return pScoreModel;
}

template<typename GR, typename BGR>
inline typename Natalie<GR, BGR>::ParserType*
    Natalie<GR, BGR>::createParser(const std::string& filename,
                                   InputFormatEnum fmt,
                                   double corrThreshold)
{
  ParserType* pParser = NULL;
  switch (fmt)
  {
    case IN_GML:
      pParser = new ParserGmlType(filename);
      break;
    case IN_GRAPHML:
      pParser = new ParserGraphMLType(filename);
      break;
    case IN_STRING:
      pParser = new ParserStringType(filename, corrThreshold);
      break;
    case IN_LGF:
      pParser = new ParserLgfType(filename);
      break;
    case IN_CSV:
      pParser = new ParserCsvType(filename, corrThreshold);
      break;
    case IN_LEDA:
      pParser = new ParserLedaType(filename);
      break;
    case IN_EDGE_LIST:
      pParser = new ParserEdgeListType(filename);
      break;
  }

  return pParser;
}

template<typename GR, typename BGR>
inline typename Natalie<GR, BGR>::BpParserType*
    Natalie<GR, BGR>::createBpParser(const std::string& filename,
                                     BpInputFormatEnum fmt,
                                     ParserType* pParserG1,
                                     ParserType* pParserG2,
                                     double eValCutOff)
{
  BpParserType* pBpParser = NULL;
  switch (fmt)
  {
    case BP_IN_CAND_LIST:
      pBpParser = new BpParserCandListType(filename, pParserG1, pParserG2);
      break;
    case BP_IN_BLAST:
      pBpParser = new BpParserBlastType(filename, pParserG1, pParserG2, eValCutOff);
      break;
    case BP_IN_LGF:
      pBpParser = new BpParserLgfType(filename, pParserG1, pParserG2);
      break;
  }

  return pBpParser;
}

template<typename GR, typename BGR>
inline Natalie<GR, BGR>::Natalie()
  : Parent(NULL)
  , _options()
  , _pLagrangeGna(NULL)
  , _pSolver(NULL)
{
}

template<typename GR, typename BGR>
inline Natalie<GR, BGR>::Natalie(const Options& options)
  : Parent(NULL)
  , _options(options)
  , _pLagrangeGna(NULL)
  , _pSolver(NULL)
  , _elapsedTime(0)
  , _score(0)
  , _upperBound(0)
{
}

template<typename GR, typename BGR>
inline Natalie<GR, BGR>::~Natalie()
{
  delete _pLagrangeGna;
  delete _pSolver;
}

template<typename GR, typename BGR>
inline bool Natalie<GR, BGR>::init(ParserType* pParserG1,
                                   ParserType* pParserG2,
                                   BpParserType* pParserGm)
{
  bool res = false;
  if (pParserG1 && pParserG2 && pParserGm)
    res = _pMatchingGraph->init(pParserG1, pParserG2, pParserGm);

  if (res)
  {
    if (!_pScoreModel)
      setScoreModel(static_cast<ScoreFunctionEnum>(_options._scoreModel), pParserG1, pParserG2);
    
    _pScoreModel->init();
    
    if (!_pLagrangeGna)
      setRelaxation(static_cast<RelaxationEnum>(_options._relaxationType));

    _pLagrangeGna->init();

    if (!_pSolver)
      setSolver(static_cast<SolverEnum>(_options._solverType));

    _pSolver->init(0);
  }

  return res;
}

template<typename GR, typename BGR>
inline int Natalie<GR, BGR>::exists(double score, bool& result)
{
  lemon::Timer t;

  SubgradientSolverType::SolverResult res = _pSolver->exists(score, result);
  _score = _pSolver->getLowerBound();
  _upperBound = _pSolver->getUpperBound();
  _elapsedTime = t.realTime();

  if (g_verbosity >= VERBOSE_ESSENTIAL)
  {
    switch (res)
    {
      case SubgradientSolverType::SOLVED:
        std::cout << "Solved decision problem to optimality." << std::endl;
        break;
      case SubgradientSolverType::UNSOLVED_MAX_IT:
        std::cout << "Not solved to optimality, maximal number of iterations exceeded." << std::endl;
        break;
      case SubgradientSolverType::UNSOLVED_FIXED_DUAL:
        std::cout << "Not solved to optimality, dual variables have converged." << std::endl;
        break;
      case SubgradientSolverType::UNSOLVED_TIME_LIMIT:
        std::cout << "Not solved to optimality, time limit exceeded." << std::endl;
        break;
      default:
        assert(false);
    }

    std::cerr << "Found bounds: ["
      << std::setprecision(10) << _score
      << ", " << std::setprecision(10) << _upperBound
      << "]" << std::endl;

    if (res == SubgradientSolverType::SOLVED)
    {
      std::cerr << "A solution with objective value " << score
                << (result ? " does " : " does not ") << "exists" << std::endl;
    }
    else
    {
      std::cerr << "Unable to determine whether a solution with objective value " << score << " exists" << std::endl;
    }
    std::cerr << "Elapsed time: " << _elapsedTime << "s" << std::endl;
  }

  return static_cast<int>(res);
}

template<typename GR, typename BGR>
inline int Natalie<GR, BGR>::solve(int nSolutions)
{
  lemon::Timer t;

  SubgradientSolverType::SolverResult res = _pSolver->solve();
  _score = _pSolver->getLowerBound();
  _upperBound = _pSolver->getUpperBound();
  _elapsedTime = t.realTime();

  if (g_verbosity >= VERBOSE_ESSENTIAL)
  {
    switch (res)
    {
      case SubgradientSolverType::SOLVED:
        std::cout << "Solved to optimality." << std::endl;
        break;
      case SubgradientSolverType::UNSOLVED_MAX_IT:
        std::cout << "Not solved to optimality, maximal number of iterations exceeded." << std::endl;
        break;
      case SubgradientSolverType::UNSOLVED_FIXED_DUAL:
        std::cout << "Not solved to optimality, dual variables have converged." << std::endl;
        break;
      case SubgradientSolverType::UNSOLVED_TIME_LIMIT:
        std::cout << "Not solved to optimality, time limit exceeded." << std::endl;
        break;
      default:
        assert(false);
    }

    std::cerr << "Found bounds: ["
      << std::setprecision(10) << _score
      << ", " << std::setprecision(10) << _upperBound
      << "]" << std::endl;

    const ValuePrimalPairList& nBestSolutions = _pSolver->getBestSolutions();
    int i = 1;
    for (ValuePrimalPairListIt it = nBestSolutions.begin();
         it != nBestSolutions.end(); it++, i++)
    {
      std::cerr << "Solution " << i << ": " << it->first << std::endl;
    }

    std::cerr << "Elapsed time: " << _elapsedTime << "s" << std::endl;
  }

  return static_cast<int>(res);
}

template<typename GR, typename BGR>
inline double Natalie<GR, BGR>::computePValue(int n)
{
  MatchingGraphShuffleType* pMatchingGraphShuffle = dynamic_cast<MatchingGraphShuffleType*>(_pMatchingGraph);
  if (pMatchingGraphShuffle == NULL) return 1;

  double LB = _score;
  std::vector<double> UB;
  double meanUB = 0;
  int nBetterUB = 0;

  VerbosityLevel orgVerbosity = g_verbosity;
  g_verbosity = VERBOSE_NONE;

  for (int i = 0; i < n; i++)
  {
    if (orgVerbosity >= VERBOSE_DEBUG)
    {
      std::cout << "Computing p-val " << i << "..." << std::flush;
    }

    pMatchingGraphShuffle->shuffle();
    _pLagrangeGna->init();

    SubgradientSolverType solver(_pLagrangeGna,
                                 _options._timeLimit,
                                 10,
                                 1,
                                 _options._nMaxNondecreasingIterations,
                                 _options._nMaxIncreasingIterations,
                                 _options._mu,
                                 _options._alpha);

    solver.init(0);
    solver.solve();
    UB.push_back(solver.getUpperBound());

    meanUB += solver.getUpperBound();

    if (solver.getUpperBound() >= LB) nBetterUB++;

    if (orgVerbosity >= VERBOSE_DEBUG)
    {
      std::cout << "Done! UB is " << solver.getUpperBound() << std::endl;
    }
  }

  // let's compute the mean and stddev
  meanUB /= n;
  double pval = static_cast<double>(nBetterUB) / static_cast<double>(n);

  if (orgVerbosity >= VERBOSE_NON_ESSENTIAL)
  {
    std::cerr << "Mean UB: " << meanUB << std::endl;
    std::cerr << "p-value: " << pval << std::endl;
  }

  g_verbosity = orgVerbosity;

  return pval;
}

} // namespace gna
} // namespace nina

#endif // NATALIE_H_
