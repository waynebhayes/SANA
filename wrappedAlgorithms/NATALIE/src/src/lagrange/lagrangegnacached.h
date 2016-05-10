/*
 * lagrangegnacached.h
 *
 *  Created on: 9-mar-2011
 *      Author: M. El-Kebir
 */

#ifndef LAGRANGEGNACACHED_H_
#define LAGRANGEGNACACHED_H_

#include <lemon/core.h>
#include <lemon/bp_matching.h>
#include <lemon/adaptors.h>
#include <limits>
#include <vector>
#include "lagrange/lagrangegna.h"
#include "verbose.h"

namespace nina {
namespace gna {

template<typename GR, typename BGR>
class LagrangeGnaCached : public virtual LagrangeGna<GR, BGR>
{
public:
  /// The graph type of the input graphs
  typedef GR Graph;
  /// The graph type of the bipartite matching graph
  typedef BGR BpGraph;
  /// Base class type
  typedef LagrangeGna<GR, BGR> Parent;
  /// Type of a map assigning a boolean to every matching edge
  typedef typename Parent::BpMatchingMap BpMatchingMap;
  /// Type of a matching map: maps a node to its matching edge
  typedef typename Parent::BpBoolMap BpBoolMap;
  /// Type of the matching graph
  typedef typename Parent::MatchingGraphType MatchingGraphType;
  /// Type of the score model
  typedef typename Parent::ScoreModelType ScoreModelType;

  using Parent::_matchingGraph;
  using Parent::_scoreModel;
  using Parent::_gm;
  using Parent::_defX;
  using Parent::_defY;
  using Parent::_x;
  using Parent::_y;
  using Parent::_idxMapX;
  using Parent::_yIdx2xIdx;
  using Parent::_primalValue;
  using Parent::_feasiblePrimalValue;
  using Parent::_pGlobal;
  using Parent::_integral;
  using Parent::hasChangedX;
  using Parent::computeFeasiblePrimalValue;
  using Parent::identifyLocalProblems;
  using Parent::initGlobalProfit;
  using Parent::solveLocal;
  using Parent::realize;
  using Parent::clearLocalProblems;

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
  typedef typename Parent::BpWeightMap BpWeightMap;
  typedef typename Parent::BpIdxMap BpIdxMap;
  typedef typename Parent::BpNodeMap BpNodeMap;
  typedef typename Parent::BpEdgeMap BpEdgeMap;
  typedef typename Parent::MWBM MWBM;
  typedef typename Parent::GlobalProblemType GlobalProblemType;
  typedef typename Parent::LocalProblem LocalProblem;
  typedef typename Parent::DualVector DualVector;
  typedef typename Parent::PrimalVector PrimalVector;
  typedef typename Parent::SubgradientVector SubgradientVector;
  typedef typename Parent::PrimalConstIt PrimalConstIt;
  typedef typename Parent::PrimalIt PrimalIt;
  typedef typename Parent::DualConstIt DualConstIt;
  typedef typename Parent::DualIt DualIt;
  typedef typename Parent::SubgradientConstIt SubgradientConstIt;
  typedef typename Parent::SubgradientIt SubgradientIt;
  typedef typename Parent::IndexList IndexList;
  typedef typename Parent::IndexListIt IndexListIt;
  typedef typename Parent::IndexListConstIt IndexListConstIt;
  typedef typename Parent::IndexVector IndexVector;
  typedef typename Parent::IndexVectorIt IndexVectorIt;
  typedef typename Parent::IndexVectorConstIt IndexVectorConstIt;
  typedef typename Parent::PrimalDefX PrimalDefX;
  typedef typename Parent::PrimalDefXIt PrimalDefXIt;
  typedef typename Parent::DefY DefY;
  typedef typename Parent::PrimalDefY PrimalDefY;
  typedef typename Parent::PrimalDefYIt PrimalDefYIt;

  typedef std::vector<LocalProblem*> LocalProblemVector;
  typedef typename LocalProblemVector::iterator LocalProblemVectorIt;
  typedef typename LocalProblemVector::const_iterator LocalProblemVectorConstIt;

private:
  struct RedNodePair
  {
    size_t _idxX;
    BpNode _n;

    RedNodePair()
      : _idxX(std::numeric_limits<size_t>::max())
      , _n(lemon::INVALID)
    {
    }

    bool valid(size_t idxX)
    {
      return _idxX == idxX && _n != lemon::INVALID;
    }
  };

  typedef typename BpGraph::template NodeMap<RedNodePair> BpRedNodeMap;

private:
  /// Maps nodes from the original matching graph to nodes in the reduced one
  BpRedNodeMap _org2redNode;

protected:
  /// Local problem stuff
  LocalProblemVector _localProblemVector;
  /// Maps an x index to a local problem index
  IndexVector _xIdx2lpIdx;

  /// Return reduced edge corresponding to (i,k)
  BpEdge getRedEdge(size_t idxX, BpEdge org_jl);
  /// Create local problem data structures(s)
  void createLocal();
  /// Init local problem(s)
  ///
  /// \pre initPrimal() has been called
  void initLocal();
  /// Reset local problem(s)
  void resetLocal();
  /// Check whether a dual variable is flagged as changed,
  /// if not flag it and add it to dualIndices
  virtual void checkOrFlag(const DualVector& dual, 
                           IndexList& dualIndices,
                           size_t idxD) = 0;
  /// Update multipliers (enhanced dual descent method)
  virtual void updateDualEnhanced(DualVector& dual, 
                                  IndexList& dualIndices,
                                  const size_t idxX,
                                  PrimalDefYIt& itY,
                                  size_t& idxY) = 0;

public:
  /// Constructor
  LagrangeGnaCached(const MatchingGraphType& matchingGraph,
                    const ScoreModelType& scoreModelType,
                    bool integral);
  /// Destructor
  virtual ~LagrangeGnaCached();
  /// Get original edge corresponding to y_{ikjl} with index idxY
  BpEdge getOrgEdge(size_t idxY, bool ik) const;
  /// Get dual slack corresponding to idxY
  double getDualSlackY(size_t idxY) const;
  /// Get number of red nodes corresponding to local problem with idxY
  int getRedNodeCount(size_t idxY) const;
  /// Get number of blue nodes corresponding to local problem with idxY
  int getBlueNodeCount(size_t idxY) const;
  /// Get number of edges corersponding to local problem with idxY
  int getEdgeCount(size_t idxY) const;
  /// Solve the primal problem given a vector of dual multipliers
  /// without compiling a list of nonzero subgradients
  /// 
  /// Used for the enhanced dual descent solver
  void evaluateEnhanced(DualVector& dual, 
                        IndexList& dualIndices);
  /// Update multipliers according to the dual descent scheme
  virtual void updateDual(DualVector& dual,
                          IndexList& dualIndices,
                          const double tau,
                          const double phi) const = 0;
};

template<typename GR, typename BGR>
inline LagrangeGnaCached<GR, BGR>::LagrangeGnaCached(const MatchingGraphType& matchingGraph,
                                                     const ScoreModelType& scoreModel,
                                                     bool integral)
  : Parent(matchingGraph, scoreModel, integral)
  , _org2redNode(_gm, RedNodePair())
  , _localProblemVector()
  , _xIdx2lpIdx()
{
}

template<typename GR, typename BGR>
inline LagrangeGnaCached<GR, BGR>::~LagrangeGnaCached()
{
  for (LocalProblemVectorIt it = _localProblemVector.begin();
    it != _localProblemVector.end(); it++)
  {
    delete *it;
  }
}

template<typename GR, typename BGR>
inline typename LagrangeGnaCached<GR, BGR>::BpEdge 
  LagrangeGnaCached<GR, BGR>::getRedEdge(size_t idxX, BpEdge org_ik)
{
  if (_idxMapX[org_ik] == idxX)
  {
    // edge (i,k) is not part of its corresponding local problem,
    // i.e. there is no y_{ikik}
    return lemon::INVALID;
  }

  LocalProblem* pLocal;
  if (_xIdx2lpIdx[idxX] == std::numeric_limits<size_t>::max())
  {
    pLocal = new LocalProblem();
    pLocal->createIdxMapY();
    _xIdx2lpIdx[idxX] = _localProblemVector.size();
    _localProblemVector.push_back(pLocal);
  }
  else
  {
    pLocal = _localProblemVector[_xIdx2lpIdx[idxX]];
  }

  const BpNode org_i = _gm.redNode(org_ik);
  const BpNode org_k = _gm.blueNode(org_ik);
  
  RedNodePair& redPair_i = _org2redNode[org_i];
  RedNodePair& redPair_k = _org2redNode[org_k];

  if (!redPair_i.valid(idxX))
  {
    redPair_i._idxX = idxX;
    redPair_i._n = pLocal->addRedNode(org_i);
  }

  if (!redPair_k.valid(idxX))
  {
    redPair_k._idxX = idxX;
    redPair_k._n = pLocal->addBlueNode(org_k);
  }

  BpEdge red_ik = pLocal->addEdge(org_ik, redPair_i._n, redPair_k._n);
  pLocal->setIdxY(red_ik, _defY.size());

  return red_ik;
}

template<typename GR, typename BGR>
inline typename LagrangeGnaCached<GR, BGR>::BpEdge 
  LagrangeGnaCached<GR, BGR>::getOrgEdge(size_t idxY, bool ik) const
{
  assert(0 <= idxY && idxY < _defY.size());

  const size_t idxX = _yIdx2xIdx[idxY];
  if (ik)
    return _defX[idxX];
  else
    return _localProblemVector[_xIdx2lpIdx[idxX]]->getOrgEdge(_defY[idxY]._jl);
}

template<typename GR, typename BGR>
inline void LagrangeGnaCached<GR, BGR>::createLocal()
{
  for (LocalProblemVectorIt it = _localProblemVector.begin();
    it != _localProblemVector.end(); it++)
  {
    delete *it;
  }
  _localProblemVector.clear();

  _xIdx2lpIdx = IndexVector(_matchingGraph.getEdgeCountGm(),
    std::numeric_limits<size_t>::max());

  lemon::mapFill(_gm, _org2redNode, RedNodePair());
}

template<typename GR, typename BGR>
inline void LagrangeGnaCached<GR, BGR>::initLocal()
{
  for (LocalProblemVectorIt it = _localProblemVector.begin();
    it != _localProblemVector.end(); it++)
  {
    (*it)->init();
  }
}

template<typename GR, typename BGR>
inline void LagrangeGnaCached<GR, BGR>::resetLocal()
{
  for (LocalProblemVectorIt it = _localProblemVector.begin();
    it != _localProblemVector.end(); it++)
  {
    (*it)->reset();
  }
}

template<typename GR, typename BGR>
inline double LagrangeGnaCached<GR, BGR>::getDualSlackY(size_t idxY) const
{
  assert(0 <= idxY && idxY < _defY.size());
  const BpEdge jl = _defY[idxY]._jl;
  const size_t lpIdx = _xIdx2lpIdx[_yIdx2xIdx[idxY]];
  const LocalProblem* pLocal = _localProblemVector[lpIdx];

  const BpNode j = pLocal->getRedGm().redNode(jl);
  const BpNode l = pLocal->getRedGm().blueNode(jl);

  //double w = pLocal->getScaledProfit(jl);
  //double pot_j = pLocal->getPot(j);
  //double pot_l = pLocal->getPot(l);

  return pLocal->getPot(j) + pLocal->getPot(l) - pLocal->getScaledProfit(jl);
}

template<typename GR, typename BGR>
inline int LagrangeGnaCached<GR, BGR>::getRedNodeCount(size_t idxY) const
{
  assert(0 <= idxY && idxY < _defY.size());
  return _localProblemVector[_xIdx2lpIdx[_yIdx2xIdx[idxY]]]->getRedNodeCount();
}

template<typename GR, typename BGR>
inline int LagrangeGnaCached<GR, BGR>::getBlueNodeCount(size_t idxY) const
{
  assert(0 <= idxY && idxY < _defY.size());
  return _localProblemVector[_xIdx2lpIdx[_yIdx2xIdx[idxY]]]->getBlueNodeCount();
}

template<typename GR, typename BGR>
inline int LagrangeGnaCached<GR, BGR>::getEdgeCount(size_t idxY) const
{
  assert(0 <= idxY && idxY < _defY.size());
  return _localProblemVector[_xIdx2lpIdx[_yIdx2xIdx[idxY]]]->getEdgeCount();  
}

template<typename GR, typename BGR>
inline void LagrangeGnaCached<GR, BGR>::evaluateEnhanced(DualVector& dual,
                                                         IndexList& dualIndices)
{
  assert(_pGlobal);
  identifyLocalProblems(dual, dualIndices);

  // solve the local problems
  size_t idxY = 0, idxX = 0;
  PrimalDefYIt itY = _defY.begin();
  PrimalDefYIt cpyItY = itY;
  size_t cpyIdxY = idxY;
  double max_v = 0;
  for (PrimalDefXIt itX = _defX.begin(); itX != _defX.end(); itX++, idxX++)
  {
    const BpEdge ik = *itX;

    _x[idxX] = false;
    if (hasChangedX(idxX))
    {
      initGlobalProfit(ik, idxX, dual);
      solveLocal(dual, idxX, ik, itY, idxY);
      updateDualEnhanced(dual, dualIndices, idxX, cpyItY, cpyIdxY);
    }
    else
    {
      for (; itY != _defY.end() && _yIdx2xIdx[idxY] == idxX; itY++, idxY++, cpyItY++, cpyIdxY++)
      {
        _y[idxY] = false;
      }
    }

    double abs_v = fabs(_pGlobal->getProfit(ik));
    if (abs_v > max_v)
      max_v = abs_v;
  }

  assert(itY == _defY.end() && idxY == _defY.size() 
    && cpyItY == _defY.end() && cpyIdxY == _defY.size());

  if (g_verbosity >= VERBOSE_DEBUG)
  {
    std::cout << "Solving global problem..." << std::flush;
  }

  // solve the global problem
  _pGlobal->scale();
  _pGlobal->solve();

  // update primal variables
  _primalValue = 0;
  for (BpRedNodeIt r(_gm); r != lemon::INVALID; ++r)
  {
    const BpEdge ik = _pGlobal->matching(r);
    if (ik != lemon::INVALID)
    {
      realize(ik);
      _primalValue += _pGlobal->getProfit(ik);

      // delete matching edge
      _pGlobal->clearMatching(ik);
    }
  }

  if (g_verbosity >= VERBOSE_DEBUG)
  {
    std::cout << "Done! Value: " << _primalValue << ". x-s: ";
    for (size_t i = 0; i < _x.size(); i++)
    {
      if (_x[i])
        std::cout << i << " ";
    }
    std::cout << std::endl;
  }

  computeFeasiblePrimalValue();
  clearLocalProblems(dualIndices);
}

} // namespace gna
} // namespace nina

#endif /* LAGRANGEGNACACHED_H_ */
