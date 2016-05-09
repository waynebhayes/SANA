/*
 * cplexgnald.h
 *
 *  Created on: 17-mar-2011
 *      Author: M. El-Kebir
 */

#ifndef CPLEXGNALD_H_
#define CPLEXGNALD_H_

#include <ilcplex/ilocplex.h>
#include <lemon/core.h>
#include "verbose.h"
#include "input/matchinggraph.h"
#include "lagrange/lagrangegnaldcached.h"
#include "cplex/cplexgna.h"

namespace nina {
namespace gna {

template<typename GR, typename BGR>
class CplexGnaLd : public CplexGna<GR, BGR>
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
  /// Type of Lagrangian decomposition relaxation
  typedef LagrangeGnaLdCached<Graph, BpGraph> LagrangeGnaLdType;
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
  LagrangeGnaLdType* _pLagrangeGnaLd;

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

public:
  CplexGnaLd(const MatchingGraphType& matchingGraph,
             const ScoreModelType* pScoreModel,
             LagrangeGnaLdType* pLagrangeGna,
             bool lp);
  ~CplexGnaLd() {}
};

template<typename GR, typename BGR>
inline CplexGnaLd<GR, BGR>::CplexGnaLd(const MatchingGraphType& matchingGraph,
                                       const ScoreModelType* pScoreModel,
                                       LagrangeGnaLdType* pLagrangeGna,
                                       bool lp)
  : Parent(matchingGraph, pScoreModel, pLagrangeGna, lp)
  , _pLagrangeGnaLd(pLagrangeGna)
{
}

template<typename GR, typename BGR>
inline void CplexGnaLd<GR, BGR>::initConstraintsY()
{
  const BpGraph& gm = _matchingGraph.getGm();
  const size_t dimY = _pLagrangeGna->getPrimalYDim();

  if (g_verbosity >= VERBOSE_ESSENTIAL)
    std::cout << "Initializing y constraints..." << std::flush;

  IloExpr expr(_env);

  size_t constraintY1 = 0;
  std::sort(_defY.begin(), _defY.end(), Compare1(gm));

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

  assert(dimY % 2 == 0);
  size_t constraintY3 = 0;
  for (size_t idxY1 = 0; idxY1 < dimY; idxY1++)
  {
    size_t idxY2 = _pLagrangeGnaLd->getCounterpartY(idxY1);

    if (_pLagrangeGna->getOrgEdge(idxY1, true) < 
      _pLagrangeGna->getOrgEdge(idxY1, false))
    {
      if (_lp)
        _model.add(_fracY[idxY1] == _fracY[idxY2]);
      else
        _model.add(_intY[idxY1] == _intY[idxY2]);

      constraintY3++;
    }
  }

  if (g_verbosity >= VERBOSE_ESSENTIAL)
    std::cout << "Done! Added " 
      << constraintY1 << " + " 
      << constraintY2 << " + "
      << constraintY3 << " = "
      << constraintY1 + constraintY2 + constraintY3 
      << " constraints." << std::endl;
}

template<typename GR, typename BGR>
inline void CplexGnaLd<GR, BGR>::initObjective()
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
      //expr += _defY[idxY]._weight * _fracY[idxY];
      expr += _pLagrangeGna->getWeight(_defY[idxY]._idxY) * _fracY[idxY];
    else
      //expr += _defY[idxY]._weight * _intY[idxY];
      expr += _pLagrangeGna->getWeight(_defY[idxY]._idxY) * _intY[idxY];
  }

  _obj = IloMaximize(_env, expr);
  expr.end();
}

} // namespace gna
} // namespace nina

#endif /* CPLEXGNALD_H_ */
