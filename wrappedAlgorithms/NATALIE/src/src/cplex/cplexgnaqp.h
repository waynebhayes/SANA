/* 
 * cplexgnaiqp.h
 *
 *  Created on: 21-mar-2011
 *      Author: M. El-Kebir
 */

#ifndef CPLEXGNAQP_H_
#define CPLEXGNAQP_H_

#include <ilcplex/ilocplex.h>
#include <lemon/core.h>
#include "verbose.h"
#include "input/matchinggraph.h"
#include "lagrange/lagrangegnaldcached.h"
#include "cplex/cplexgna.h"

namespace nina {
namespace gna {

template<typename GR, typename BGR>
class CplexGnaQp : public CplexGna<GR, BGR>
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
  typedef LagrangeGnaSiCached<Graph, BpGraph> LagrangeGnaQpType;
  /// Type of the matching graph
  typedef typename Parent::MatchingGraphType MatchingGraphType;
  /// Type of the score model
  typedef typename Parent::ScoreModelType ScoreModelType;
  /// Type of the relaxation
  typedef typename Parent::LagrangeGnaType LagrangeGnaType;

  using Parent::_matchingGraph;
  using Parent::_pLagrangeGna;
  using Parent::_pScoreModel;
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

public:
  CplexGnaQp(const MatchingGraphType& matchingGraph,
             const ScoreModelType* pScoreModel,
             LagrangeGnaQpType* pLagrangeGna,
             bool lp);
  ~CplexGnaQp() {}
};

template<typename GR, typename BGR>
inline CplexGnaQp<GR, BGR>::CplexGnaQp(const MatchingGraphType& matchingGraph,
                                       const ScoreModelType* pScoreModel,
                                       LagrangeGnaQpType* pLagrangeGna,
                                       bool lp)
  : Parent(matchingGraph, pScoreModel, pLagrangeGna, lp)
{
}

template<typename GR, typename BGR>
inline void CplexGnaQp<GR, BGR>::initConstraintsY()
{
}

template<typename GR, typename BGR>
inline void CplexGnaQp<GR, BGR>::initObjective()
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
    BpEdge ik = _pLagrangeGna->getOrgEdge(idxY, true);
    BpEdge jl = _pLagrangeGna->getOrgEdge(idxY, false);

    // TODO: put weights in here
    if (_lp)
      expr += _fracX[_pLagrangeGna->getIdxX(ik)] * _fracX[_pLagrangeGna->getIdxX(jl)];
    else
      expr += _intX[_pLagrangeGna->getIdxX(ik)] + _intX[_pLagrangeGna->getIdxX(jl)];
  }

  _obj = IloMaximize(_env, expr);
  expr.end();
}

} // namespace gna
} // namespace nina

#endif /* CPLEXGNAQP_H_ */
