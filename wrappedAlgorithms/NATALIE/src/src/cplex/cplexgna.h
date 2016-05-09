/* 
 * cplexgna.h
 *
 *  Created on: 16-mar-2011
 *      Author: M. El-Kebir
 */

#ifndef CPLEXGNA_H_
#define CPLEXGNA_H_

#include <ilcplex/ilocplex.h>
#include <lemon/core.h>
#include "verbose.h"
#include "score/scoremodel.h"
#include "input/matchinggraph.h"
#include "lagrange/lagrangegna.h"

namespace nina {
namespace gna {

template<typename GR, typename BGR>
class CplexGna
{
public:
  /// The graph type of the input graphs
  typedef GR Graph;
  /// The graph type of the bipartite matching graph
  typedef BGR BpGraph;
  /// Type of the matching graph
  typedef MatchingGraph<Graph, BpGraph> MatchingGraphType;
  /// Type of the score model
  typedef ScoreModel<Graph, BpGraph> ScoreModelType;
  /// Type of the relaxation
  typedef LagrangeGna<Graph, BpGraph> LagrangeGnaType;

protected:
  TEMPLATE_GRAPH_TYPEDEFS(Graph);
  typedef typename BpGraph::Node BpNode;
  typedef typename BpGraph::Edge BpEdge;
  typedef typename BpGraph::NodeIt BpNodeIt;
  typedef typename BpGraph::EdgeIt BpEdgeIt;
  typedef typename BpGraph::IncEdgeIt BpIncEdgeIt;
  typedef typename BpGraph::RedNode BpRedNode;
  typedef typename BpGraph::BlueNode BpBlueNode;
  typedef typename BpGraph::RedNodeIt BpRedNodeIt;
  typedef typename BpGraph::BlueNodeIt BpBlueNodeIt;

  struct DefY
  {
    BpEdge _ik;
    BpEdge _jl;
    size_t _idxY;
    double _weight;
  };

  typedef std::vector<DefY> DefYVector;

public:
  /// Type of a matching map: maps a node to its matching edge
  typedef typename BpGraph::template NodeMap<BpEdge> BpMatchingMap;
  /// Type of a map assigning a boolean to every matching edge
  typedef typename BpGraph::template EdgeMap<bool> BpBoolMap;

protected:
  const MatchingGraphType& _matchingGraph;
  const ScoreModelType* _pScoreModel;
  const LagrangeGnaType* _pLagrangeGna;
  const bool _lp;
  IloEnv _env;
  IloModel _model;
  IloNumVarArray _fracX;
  IloNumVarArray _fracY;
  IloBoolVarArray _intX;
  IloBoolVarArray _intY;
  IloObjective _obj;
  IloCplex* _pCplex;
  DefYVector _defY;

  virtual void initConstraintsY() = 0;
  virtual void initObjective() = 0;

public:
  CplexGna(const MatchingGraphType& matchingGraph,
           const ScoreModelType* pScoreModel,
           LagrangeGnaType* pLagrangeGna,
           bool lp);
  virtual ~CplexGna();
  void init();
  int solve(int timeLimit);
  void getMatching(BpMatchingMap& m) const;
  void getMatching(BpBoolMap& m) const;
};

template<typename GR, typename BGR>
inline CplexGna<GR, BGR>::CplexGna(const MatchingGraphType& matchingGraph,
                                   const ScoreModelType* pScoreModel,
                                   LagrangeGnaType* pLagrangeGna,
                                   bool lp)
  : _matchingGraph(matchingGraph)
  , _pScoreModel(pScoreModel)
  , _pLagrangeGna(pLagrangeGna)
  , _lp(lp)
  , _env()
  , _model(_env)
  , _fracX(_env)
  , _fracY(_env)
  , _intX(_env)
  , _intY(_env)
  , _obj(_env)
  , _pCplex(NULL)
  , _defY()
{
}

template<typename GR, typename BGR>
inline CplexGna<GR, BGR>::~CplexGna()
{
  delete _pCplex;
  _env.end();
}

template<typename GR, typename BGR>
inline void CplexGna<GR, BGR>::init()
{
  IloExpr expr(_env);

  // initialize primal x and y variables
  const size_t dimX = _pLagrangeGna->getPrimalXDim();
  const size_t dimY = _pLagrangeGna->getPrimalYDim();
  if (_lp)
  {
    _fracX = IloNumVarArray(_env, dimX, 0, 1);
    _fracY = IloNumVarArray(_env, dimY, 0, 1);
  }
  else
  {
    _intX = IloBoolVarArray(_env, dimX);
    _intY = IloBoolVarArray(_env, dimY);
  }

  // initialize x-constraints
  if (g_verbosity >= VERBOSE_ESSENTIAL)
  {
    std::cout << "Initializing x constraints..." << std::flush;
  }

  size_t constraintX1 = 0;
  const BpGraph& gm = _matchingGraph.getGm();
  for (BpRedNodeIt i(gm); i != lemon::INVALID; ++i)
  {
    bool exists = false;
    for (BpIncEdgeIt ik(gm, i); ik != lemon::INVALID; ++ik)
    {
      const size_t idxX = _pLagrangeGna->getIdxX(ik);
      if (_lp)
        expr += _fracX[idxX];
      else
        expr += _intX[idxX];

      exists = true;
    }

    if (exists)
    {
      _model.add(expr <= 1);
      expr.clear();
      constraintX1++;
    }
  }

  size_t constraintX2 = 0;
  for (BpBlueNodeIt k(gm); k != lemon::INVALID; ++k)
  {
    bool exists = false;
    for (BpIncEdgeIt ik(gm, k); ik != lemon::INVALID; ++ik)
    {
      const size_t idxX = _pLagrangeGna->getIdxX(ik);
      if (_lp)
        expr += _fracX[idxX];
      else
        expr += _intX[idxX];

      exists = true;
    }

    if (exists)
    {
      _model.add(expr <= 1);
      expr.clear();
      constraintX2++;
    }
  }

  if (g_verbosity >= VERBOSE_ESSENTIAL)
    std::cout << "Done! Added " 
      << constraintX1 << " + " << constraintX2 << " = "
      << constraintX1 + constraintX2 << " constraints." << std::endl;

  // initialize y-constraints
  _defY = DefYVector(dimY);
  for (size_t idxY = 0; idxY < dimY; idxY++)
  {
    _defY[idxY]._ik = _pLagrangeGna->getOrgEdge(idxY, true);
    _defY[idxY]._jl = _pLagrangeGna->getOrgEdge(idxY, false);
    _defY[idxY]._idxY = idxY;
    _defY[idxY]._weight = _pLagrangeGna->getWeight(idxY);

    if (!_lp && _defY[idxY]._weight < 0)
      _model.add(_intX[_pLagrangeGna->getIdxX(_defY[idxY]._ik)]
                 + _intX[_pLagrangeGna->getIdxX(_defY[idxY]._jl)]
                 - _intY[idxY] <= 1);
  }

  // initialize objective
  if (g_verbosity >= VERBOSE_ESSENTIAL)
    std::cout << "Initializing objective..." << std::flush;

  initObjective();

  if (g_verbosity >= VERBOSE_ESSENTIAL)
    std::cout << "Done!" << std::endl;

  initConstraintsY();

  // attach objective to model
  _model.add(_obj);

  expr.end();
}

template<typename GR, typename BGR>
inline int CplexGna<GR, BGR>::solve(int timeLimit)
{
  // returns 0 when solved to optimality
  // returns 1 when solved but no to optimality
  // returns 2 when there is no solution due to time limit
  // returns 3 when there is no solution not due to time limit

  lemon::Timer t;

  //if (g_verbosity >= VERBOSE_NON_ESSENTIAL)
  //  std::cout << "Solving..." << std::flush;

  delete _pCplex;
  _pCplex = new IloCplex(_model);

  _pCplex->setParam(IloCplex::RootAlg, IloCplex::Barrier);
  if (timeLimit > 0)
    _pCplex->setParam(IloCplex::TiLim, timeLimit);

  int res;
  if (!_pCplex->solve())
  {
    std::cerr << "Solution status = " << _pCplex->getStatus() << std::endl;
    if (_pCplex->getCplexStatus() == CPX_STAT_ABORT_TIME_LIM)
    {
      res = 2;
      std::cout << "No feasibile solution, time limit exceeded." << std::endl;
    }
    else
    {
      res = 3;
      std::cout << "No feasible solution." << std::endl;
    }
  }
  else
  {
    std::cerr << "Solution status = " << _pCplex->getStatus() << std::endl;
    if (_pCplex->getCplexStatus() == CPX_STAT_ABORT_TIME_LIM)
    {
      res = 1;
      std::cout << "Not solved to optimality, time limit exceeded." << std::endl;
    }
    else
    {
      res = 0;
      std::cout << "Solved to optimality." << std::endl;
    }

    std::cerr << "Found bounds: ["
      << std::setprecision(10) << _pCplex->getObjValue()
      << ", " << std::setprecision(10) << _pCplex->getBestObjValue()
      << "]" << std::endl;

  }

  if (g_verbosity >= VERBOSE_NON_ESSENTIAL)
    std::cout << "Done!" << std::endl;

  std::cerr << "Solution : " << _pCplex->getObjValue() << std::endl;
  std::cerr << "Elapsed time: " << t.realTime() << "s" << std::endl;

  //IloNumArray vals(_env);
  //if (_lp)
  //  cplex.getValues(vals, _fracX);
  //else
  //  cplex.getValues(vals, _intX);
  //_env.out() << "x             = " << vals << std::endl;

  //if (_lp)
  //  cplex.getValues(vals, _fracY);
  //else
  //  cplex.getValues(vals, _intY);
  //_env.out() << "y             = " << vals << std::endl;

  //cplex.exportModel("lpex1.lp");
  return res;
}

template<typename GR, typename BGR>
inline void CplexGna<GR, BGR>::getMatching(BpMatchingMap& m) const
{
  assert(_pCplex);

  const BpGraph& gm = _matchingGraph.getGm();
  lemon::mapFill(gm, m, lemon::INVALID);

  if (_lp)
    return;

  for (BpEdgeIt ik(gm); ik != lemon::INVALID; ++ik)
  {
    const size_t idxX = _pLagrangeGna->getIdxX(ik);
    if (_pCplex->getValue(_intX[idxX]))
    {
      m.set(gm.redNode(ik), ik);
      m.set(gm.blueNode(ik), ik);
    }
  }
}

template<typename GR, typename BGR>
inline void CplexGna<GR, BGR>::getMatching(BpBoolMap& m) const
{
  assert(_pCplex);

  const BpGraph& gm = _matchingGraph.getGm();
  if (_lp)
    return;

  for (BpEdgeIt ik(gm); ik != lemon::INVALID; ++ik)
  {
    const size_t idxX = _pLagrangeGna->getIdxX(ik);
    m[ik] = _pCplex->getValue(_intX[idxX]);
  }
}

} // namespace gna
} // namespace nina

#endif /* CPLEXGNA_H_ */

