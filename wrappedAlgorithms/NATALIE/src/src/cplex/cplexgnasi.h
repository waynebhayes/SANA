/* 
 * cplexgnasi.h
 *
 *  Created on: 17-mar-2011
 *      Author: M. El-Kebir
 */

#ifndef CPLEXGNASI_H_
#define CPLEXGNASI_H_

#include <ilcplex/ilocplex.h>
#include <lemon/core.h>
#include <algorithm>
#include <vector>
#include "verbose.h"
#include "input/matchinggraph.h"
#include "lagrange/lagrangegnasicached.h"
#include "cplex/cplexgna.h"

namespace nina {
namespace gna {

template<typename GR, typename BGR>
class CplexGnaSi : public CplexGna<GR, BGR>
{
public:
  /// The graph type of the input graphs
  typedef GR Graph;
  /// The graph type of the bipartite matching graph
  typedef BGR BpGraph;
  /// Base class type
  typedef CplexGna<GR, BGR> Parent;
  /// Type of a map assigning a boolean to every matching edge
  typedef typename Parent::BpMatchingMap BpMatchingMap;
  /// Type of subproblem isolation relaxation
  typedef LagrangeGnaSiCached<Graph, BpGraph> LagrangeGnaSiType;
  /// Type of the matching graph
  typedef typename Parent::MatchingGraphType MatchingGraphType;
  /// Type of the score model
  typedef typename Parent::ScoreModelType ScoreModelType;
  /// Type of the relaxation
  typedef typename Parent::LagrangeGnaType LagrangeGnaType;

  using Parent::_matchingGraph;
  using Parent::_pScoreModel;
  using Parent::_pLagrangeGna;
  using Parent::_lp;
  using Parent::_env;
  using Parent::_model;
  using Parent::_fracX;
  using Parent::_fracY;
  using Parent::_intX;
  using Parent::_intY;
  using Parent::_obj;
  using Parent::_defY;

protected:
  TEMPLATE_GRAPH_TYPEDEFS(Graph);
  typedef typename Parent::BpNode BpNode;
  typedef typename Parent::BpEdge BpEdge;
  typedef typename Parent::BpNodeIt BpNodeIt;
  typedef typename Parent::BpEdgeIt BpEdgeIt;
  typedef typename Parent::BpIncEdgeIt BpIncEdgeIt;
  typedef typename Parent::BpRedNode BpRedNode;
  typedef typename Parent::BpBlueNode BpBlueNode;
  typedef typename Parent::BpRedNodeIt BpRedNodeIt;
  typedef typename Parent::BpBlueNodeIt BpBlueNodeIt;
  typedef typename Parent::DefY DefY;
  typedef typename Parent::DefYVector DefYVector;

  void initConstraintsY();
  void initObjective();

private:

  struct Compare1
  {
  private:
    const BpGraph& _gm;

  public:
    Compare1(const BpGraph& gm)
      : _gm(gm)
    {
    }

    bool operator ()(const DefY& a, const DefY& b) const
    {
      return b._ik < a._ik ||
        (a._ik == b._ik &&
          (_gm.redNode(b._jl) < _gm.redNode(a._jl) ||
            (_gm.redNode(a._jl) == _gm.redNode(b._jl) &&
             _gm.blueNode(a._jl) < _gm.blueNode(b._jl))));
    };
  };

  struct Compare2
  {
  private:
    const BpGraph& _gm;

  public:
    Compare2(const BpGraph& gm)
      : _gm(gm)
    {
    }

    bool operator()(const DefY& a, const DefY& b)
    {
      return b._ik < a._ik ||
        (a._ik == b._ik &&
          (_gm.blueNode(b._jl) < _gm.blueNode(a._jl) ||
            (_gm.blueNode(a._jl) == _gm.blueNode(b._jl) &&
             _gm.redNode(a._jl) < _gm.redNode(b._jl))));
    };
  };

  struct Compare3
  {
  private:
    const BpGraph& _gm;

  public:
    Compare3(const BpGraph& gm)
      : _gm(gm)
    {
    }

    bool operator()(const DefY& a, const DefY& b)
    {
      return b._jl < a._jl ||
        (a._jl == b._jl &&
          (_gm.redNode(b._ik) < _gm.redNode(a._ik) ||
            (_gm.redNode(a._ik) == _gm.redNode(b._ik) &&
             _gm.blueNode(a._ik) < _gm.blueNode(b._ik))));
    };
  };

  struct Compare4
  {
  private:
    const BpGraph& _gm;

  public:
    Compare4(const BpGraph& gm)
      : _gm(gm)
    {
    }

    bool operator()(const DefY& a, const DefY& b)
    {
      return b._jl < a._jl ||
        (a._jl == b._jl &&
          (_gm.blueNode(b._ik) < _gm.blueNode(a._ik) ||
            (_gm.blueNode(a._ik) == _gm.blueNode(b._ik) &&
             _gm.redNode(a._ik) < _gm.redNode(b._ik))));
    };
  };

public:
  CplexGnaSi(const MatchingGraphType& matchingGraph,
             const ScoreModelType* pScoreModel,
             LagrangeGnaSiType* pLagrangeGna,
             bool lp);
  ~CplexGnaSi() {}
};

template<typename GR, typename BGR>
inline CplexGnaSi<GR, BGR>::CplexGnaSi(const MatchingGraphType& matchingGraph,
                                       const ScoreModelType* pScoreModel,
                                       LagrangeGnaSiType* pLagrangeGna,
                                       bool lp)
  : Parent(matchingGraph, pScoreModel, pLagrangeGna, lp)
{
}

template<typename GR, typename BGR>
inline void CplexGnaSi<GR, BGR>::initConstraintsY()
{
  const BpGraph& gm = _matchingGraph.getGm();
  const size_t dimY = _pLagrangeGna->getPrimalYDim();

  if (g_verbosity >= VERBOSE_NON_ESSENTIAL)
    std::cout << "Initializing y constraints..." << std::flush;

  IloExpr expr(_env);

  // constraint 1
  size_t constraintY1 = 0;
  std::sort(_defY.begin(), _defY.end(), Compare1(gm));

  //for (size_t i = 0; i < dimY; i++)
  //{
  //  std::cout << "{" << gm.id(_defY[i]._ik) << "," 
  //    << gm.id(_defY[i]._jl) << "}" << std::endl;
  //}

  size_t idxY = 0;
  for (BpEdgeIt ik(gm); ik != lemon::INVALID; ++ik)
  {
    for (BpRedNodeIt j(gm); j != lemon::INVALID; ++j)
    {
      bool exists = false;
      while (idxY < dimY && 
        _defY[idxY]._ik == ik && gm.redNode(_defY[idxY]._jl) == j)
      {
        if (_lp)
          expr += _fracY[_defY[idxY]._idxY];
        else
          expr += _intY[_defY[idxY]._idxY];

        exists = true;
        idxY++;
      }

      if (exists)
      {
        if (_lp)
          _model.add(expr <= _fracX[_pLagrangeGna->getIdxX(ik)]);
        else
          _model.add(expr <= _intX[_pLagrangeGna->getIdxX(ik)]);

        expr.clear();
        constraintY1++;
      }
    }
  }

  // constraint 2 
  size_t constraintY2 = 0;
  std::sort(_defY.begin(), _defY.end(), Compare2(gm));

  idxY = 0;
  for (BpEdgeIt ik(gm); ik != lemon::INVALID; ++ik)
  {
    for (BpBlueNodeIt l(gm); l != lemon::INVALID; ++l)
    {
      bool exists = false;
      while (idxY < dimY && 
        _defY[idxY]._ik == ik && gm.blueNode(_defY[idxY]._jl) == l)
      {
        if (_lp)
          expr += _fracY[_defY[idxY]._idxY];
        else
          expr += _intY[_defY[idxY]._idxY];

        exists = true;
        idxY++;
      }

      if (exists)
      {
        if (_lp)
          _model.add(expr <= _fracX[_pLagrangeGna->getIdxX(ik)]);
        else
          _model.add(expr <= _intX[_pLagrangeGna->getIdxX(ik)]);

        expr.clear();
        constraintY2++;
      }
    }
  }

  // constraint 3
  size_t constraintY3 = 0;
  std::sort(_defY.begin(), _defY.end(), Compare3(gm));

  idxY = 0;
  for (BpEdgeIt jl(gm); jl != lemon::INVALID; ++jl)
  {
    for (BpRedNodeIt i(gm); i != lemon::INVALID; ++i)
    {
      bool exists = false;
      while (idxY < dimY && 
        _defY[idxY]._jl == jl && gm.redNode(_defY[idxY]._ik) == i)
      {
        if (_lp)
          expr += _fracY[_defY[idxY]._idxY];
        else
          expr += _intY[_defY[idxY]._idxY];

        exists = true;
        idxY++;
      }

      if (exists)
      {
        if (_lp)
          _model.add(expr <= _fracX[_pLagrangeGna->getIdxX(jl)]);
        else
          _model.add(expr <= _intX[_pLagrangeGna->getIdxX(jl)]);

        expr.clear();
        constraintY3++;
      }
    }
  }

  // constraint 4
  size_t constraintY4 = 0;
  std::sort(_defY.begin(), _defY.end(), Compare4(gm));

  idxY = 0;
  for (BpEdgeIt jl(gm); jl != lemon::INVALID; ++jl)
  {
    for (BpBlueNodeIt k(gm); k != lemon::INVALID; ++k)
    {
      bool exists = false;
      while (idxY < dimY && 
        _defY[idxY]._jl == jl && gm.blueNode(_defY[idxY]._ik) == k)
      {
        if (_lp)
          expr += _fracY[_defY[idxY]._idxY];
        else
          expr += _intY[_defY[idxY]._idxY];

        exists = true;
        idxY++;
      }

      if (exists)
      {
        if (_lp)
          _model.add(expr <= _fracX[_pLagrangeGna->getIdxX(jl)]);
        else
          _model.add(expr <= _intX[_pLagrangeGna->getIdxX(jl)]);

        expr.clear();
        constraintY4++;
      }
    }
  }

  expr.end();
  
  if (g_verbosity >= VERBOSE_NON_ESSENTIAL)
    std::cout << "Done! Added "
      << constraintY1 << " + "
      << constraintY2 << " + "
      << constraintY3 << " + "
      << constraintY4 << " = "
      << constraintY1 + constraintY2 + constraintY3 + constraintY4
      << " constraints." << std::endl;
}

template<typename GR, typename BGR>
inline void CplexGnaSi<GR, BGR>::initObjective()
{
  const BpGraph& gm = _matchingGraph.getGm();

  IloExpr expr(_env);
  // x component
  for (BpEdgeIt ik(gm); ik != lemon::INVALID; ++ik)
  {
    const size_t idxX = _pLagrangeGna->getIdxX(ik);
    if (_lp)
      expr += _pScoreModel->getWeightGm(ik) * _fracX[idxX];
    else
      expr += _pScoreModel->getWeightGm(ik) * _intX[idxX];
  }

  // y component
  const size_t dimY = _pLagrangeGna->getPrimalYDim();
  for (size_t idxY = 0; idxY < dimY; idxY++)
  {
     if (_lp)
      expr += _defY[idxY]._weight * _fracY[idxY];
    else
      expr += _defY[idxY]._weight * _intY[idxY];
  }

  _obj = IloMaximize(_env, expr);
  expr.end();
}

} // namespace gna
} // namespace nina

#endif /* CPLEXGNALD_H_ */
