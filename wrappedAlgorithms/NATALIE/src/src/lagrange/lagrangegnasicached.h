/*
 * lagrangegnaldcached.h
 *
 *  Created on: 12-mar-2011
 *      Author: M. El-Kebir
 */

#ifndef LAGRANGEGNASICACHED_H_
#define LAGRANGEGNASICACHED_H_

#include <lemon/core.h>
#include <lemon/bp_matching.h>
#include <lemon/adaptors.h>
#include <limits>
#include "input/matchinggraph.h"
#include "lagrange/lagrangegna.h"
#include "lagrange/lagrangegnasi.h"
#include "lagrange/lagrangegnacached.h"
#include "verbose.h"

namespace nina {
namespace gna {

template<typename GR, typename BGR>
class LagrangeGnaSiCached
  : public LagrangeGnaCached<GR, BGR>, public LagrangeGnaSi<GR, BGR>
{
public:
  /// The graph type of the input graphs
  typedef GR Graph;
  /// The graph type of the bipartite matching graph
  typedef BGR BpGraph;
  /// Base class type
  typedef LagrangeGna<GR, BGR> GrandParent;
  /// Base class type
  typedef LagrangeGnaCached<GR, BGR> Parent1;
  /// Base class type
  typedef LagrangeGnaSi<GR, BGR> Parent2;
  /// Type of a map assigning a boolean to every matching edge
  typedef typename Parent1::BpMatchingMap BpMatchingMap;
  /// Type of a matching map: maps a node to its matching edge
  typedef typename Parent1::BpBoolMap BpBoolMap;
  /// Type of the matching graph
  typedef typename Parent1::MatchingGraphType MatchingGraphType;
  /// Type of the score model
  typedef typename Parent1::ScoreModelType ScoreModelType;

  using Parent1::_matchingGraph;
  using Parent1::_scoreModel;
  using Parent1::_gm;
  using Parent1::_x;
  using Parent1::_y;
  using Parent1::_idxMapX;
  using Parent1::_yIdx2xIdx;
  using Parent1::_primalValue;
  using Parent1::_feasiblePrimalValue;
  using Parent1::_pGlobal;
  using Parent1::_localProblemVector;
  using Parent1::_xIdx2lpIdx;
  using Parent1::_integral;
  using Parent1::getDualSlackY;
  using Parent1::getIdxX;
  using Parent1::getRedNodeCount;
  using Parent1::getBlueNodeCount;

  using Parent2::_defExtX;
  using Parent2::_defX;
  using Parent2::_defExtY;
  using Parent2::_defY;
  using Parent2::_defLambdaH;
  using Parent2::_defLambdaV;
  using Parent2::_subgradientH;
  using Parent2::_subgradientV;
  using Parent2::_sumH;
  using Parent2::_sumV;
  using Parent2::initPrimal;
  using Parent2::initDual;
  using Parent2::computeFeasiblePrimalValue;
  using Parent2::getSubgradient;
  using Parent2::getDualDim;
  using Parent2::getDualVarLowerBound;
  using Parent2::getDualVarUpperBound;
  using Parent2::getMatching;
  using Parent2::printPrimalDef;
  using Parent2::printDualDef;
  using Parent2::evaluate;
  using Parent2::getRedEdge;
  using Parent2::resetLocal;
  using Parent2::initLocal;
  using Parent2::createLocal;
  using Parent2::getDualSlackX;
  using Parent2::getDualH;
  using Parent2::getDualV;
  using Parent2::computeSumH;
  using Parent2::computeSumV;

private:
  TEMPLATE_GRAPH_TYPEDEFS(Graph);
  typedef typename Parent1::BpNode BpNode;
  typedef typename Parent1::BpEdge BpEdge;
  typedef typename Parent1::BpNodeIt BpNodeIt;
  typedef typename Parent1::BpEdgeIt BpEdgeIt;
  typedef typename Parent1::BpIncEdgeIt BpIncEdgeIt;
  typedef typename Parent1::BpRedNode BpRedNode;
  typedef typename Parent1::BpBlueNode BpBlueNode;
  typedef typename Parent1::BpRedNodeIt BpRedNodeIt;
  typedef typename Parent1::BpBlueNodeIt BpBlueNodeIt;
  typedef typename Parent1::BpWeightMap BpWeightMap;
  typedef typename Parent1::BpIdxMap BpIdxMap;
  typedef typename Parent1::BpNodeMap BpNodeMap;
  typedef typename Parent1::BpEdgeMap BpEdgeMap;
  typedef typename Parent1::MWBM MWBM;
  typedef typename Parent1::GlobalProblemType GlobalProblemType;
  typedef typename Parent1::LocalProblem LocalProblem;
  typedef typename Parent1::DualVector DualVector;
  typedef typename Parent1::PrimalVector PrimalVector;
  typedef typename Parent1::SubgradientVector SubgradientVector;
  typedef typename Parent1::PrimalConstIt PrimalConstIt;
  typedef typename Parent1::PrimalIt PrimalIt;
  typedef typename Parent1::DualConstIt DualConstIt;
  typedef typename Parent1::DualIt DualIt;
  typedef typename Parent1::SubgradientConstIt SubgradientConstIt;
  typedef typename Parent1::SubgradientIt SubgradientIt;
  typedef typename Parent1::IndexList IndexList;
  typedef typename Parent1::IndexListIt IndexListIt;
  typedef typename Parent1::IndexListConstIt IndexListConstIt;
  typedef typename Parent1::IndexVector IndexVector;
  typedef typename Parent1::IndexVectorIt IndexVectorIt;
  typedef typename Parent1::IndexVectorConstIt IndexVectorConstIt;
  typedef typename Parent1::PrimalDefX PrimalDefX;
  typedef typename Parent1::PrimalDefXIt PrimalDefXIt;
  typedef typename Parent1::DefY DefY;
  typedef typename Parent1::PrimalDefY PrimalDefY;
  typedef typename Parent1::PrimalDefYIt PrimalDefYIt;
  typedef typename Parent1::LocalProblemVector LocalProblemVector;
  typedef typename Parent1::LocalProblemVectorIt LocalProblemVectorIt;
  typedef typename Parent1::LocalProblemVectorConstIt LocalProblemVectorConstIt;

  typedef typename Parent2::DefExtX DefExtX;
  typedef typename Parent2::DefExtY DefExtY;
  typedef typename Parent2::DefLambdaH DefLambdaH;
  typedef typename Parent2::DefLambdaV DefLambdaV;
  typedef typename Parent2::PrimalDefExtX PrimalDefExtX;
  typedef typename Parent2::PrimalDefExtXIt PrimalDefExtXIt;
  typedef typename Parent2::PrimalDefExtY PrimalDefExtY;
  typedef typename Parent2::PrimalDefExtYNonConstIt PrimalDefExtYNonConstIt;
  typedef typename Parent2::PrimalDefExtYIt PrimalDefExtYIt;
  typedef typename Parent2::DualDefLambdaH DualDefLambdaH;
  typedef typename Parent2::DualDefLambdaHIt DualDefLambdaHIt;
  typedef typename Parent2::DualDefLambdaV DualDefLambdaV;
  typedef typename Parent2::DualDefLambdaVIt DualDefLambdaVIt;
  typedef typename Parent2::DoubleVector DoubleVector;

  typedef std::vector<bool> BoolVector;

protected:
  /// Indicates whether a dual variable corresponding to an x var has been updated
  BoolVector _changedDualX;
  /// Indicates whether a dual variable has been updated
  BoolVector _changedDualH;
  /// Indicates whether a dual variable has been updated
  BoolVector _changedDualV;
  /// Indicates whether the sum^h corresponding to (i,k) has been updated
  BpBoolMap _updatedSumH;
  /// Indicates whether the sum^v corresponding to (i,k) has been updated
  BpBoolMap _updatedSumV;

  /// Solve the local problem for matching edge ik
  virtual void solveLocal(const DualVector& dual, 
                          const size_t idxX, 
                          const BpEdge ik, 
                          PrimalDefYIt& itY, 
                          size_t& idxYFwd);
  /// Indicates whether it is necessary to solve idxX;
  /// this is a single shot only function
  bool hasChangedX(size_t idxX);
  /// Indicates whether it is necessary to profit of idxY;
  /// this is a single shot only function
  bool hasChangedY(size_t idxY);
  /// Check whether a dual variable is flagged as changed,
  /// if not flag it and add it to dualIndices
  void checkOrFlag(const DualVector& dual, 
                   IndexList& dualIndices,
                   size_t idxD);
  /// Flag a dual variable as changed
  void flag(const DualVector& dual, size_t idxD);
  /// Unflag a dual variable as changed
  void unflag(size_t idxD);
  /// Update multipliers (enhanced dual descent method)
  void updateDualEnhanced(DualVector& dual, 
                          IndexList& dualIndices,
                          const size_t idxX,
                          PrimalDefYIt& itY,
                          size_t& idxY);

public:
  /// Constructor
  LagrangeGnaSiCached(const MatchingGraphType& matchingGraph,
                      const ScoreModelType& scoreModel,
                      bool integral);
  /// Destructor
  virtual ~LagrangeGnaSiCached() {}
  /// Initialize the primal and dual variables
  void init();
  /// Update multipliers according to the dual descent scheme
  void updateDualOld(DualVector& dual,
                     IndexList& dualIndices,
                     const double tau,
                     const double phi) const;
  /// Update multipliers according to the dual descent scheme
  void updateDual(DualVector& dual,
                  IndexList& dualIndices,
                  const double tau,
                  const double phi) const;
  /// Indicates whether only equality constraint are relaxed
  bool onlyEqualityConstraintsDualized() const { return false; }
};

template<typename GR, typename BGR>
inline LagrangeGnaSiCached<GR, BGR>::LagrangeGnaSiCached(const MatchingGraphType& matchingGraph,
                                                         const ScoreModelType& scoreModel,
                                                         bool integral)
  : GrandParent(matchingGraph, scoreModel, integral)
  , Parent1(matchingGraph, scoreModel, integral)
  , Parent2(matchingGraph, scoreModel, integral)
  , _changedDualX()
  , _changedDualH()
  , _changedDualV()
  , _updatedSumH(_gm, false)
  , _updatedSumV(_gm, false)
{
}

template<typename GR, typename BGR>
inline void LagrangeGnaSiCached<GR, BGR>::init()
{
  createLocal();
  initPrimal();
  initDual();
  initLocal();

  _changedDualX = BoolVector(_x.size(), false);
  _changedDualH = BoolVector(_defLambdaH.size(), false);
  _changedDualV = BoolVector(_defLambdaV.size(), false);

  if (g_verbosity >= VERBOSE_ESSENTIAL)
  {
    std::cout << "Number of primal x variables: " << _x.size() << std::endl;
    std::cout << "Number of primal y variables: " << _y.size() << std::endl;
    std::cout << "Number of subgradients: " << getDualDim() << std::endl;
  }

  if (g_verbosity >= VERBOSE_DEBUG)
  {
    std::cout << std::endl;
    printPrimalDef(std::cout);
    printDualDef(std::cout);
    std::cout << std::endl;
  }
}

template<typename GR, typename BGR>
inline bool LagrangeGnaSiCached<GR, BGR>::hasChangedX(size_t idxX)
{
  return _changedDualX[idxX];
}

template<typename GR, typename BGR>
inline bool LagrangeGnaSiCached<GR, BGR>::hasChangedY(size_t idxY)
{
  DefExtY& defext_y = _defExtY[idxY];
  return _changedDualH[defext_y._idxLambdaH] || _changedDualV[defext_y._idxLambdaV];
}

template<typename GR, typename BGR>
inline void LagrangeGnaSiCached<GR, BGR>::checkOrFlag(const DualVector& dual,
                                                      IndexList& dualIndices,
                                                      size_t idxD)
{
  assert(0 <= idxD && idxD < _defLambdaH.size() + _defLambdaV.size());

  bool changed = idxD < _defLambdaH.size() ? 
    _changedDualH[idxD] : _changedDualV[idxD - _defLambdaH.size()];

  if (!changed)
  {
    flag(dual, idxD);
    dualIndices.push_back(idxD);
  }
}

template<typename GR, typename BGR>
inline void LagrangeGnaSiCached<GR, BGR>::flag(const DualVector& dual, 
                                               size_t idxD)
{
  if (idxD < _defLambdaH.size())
  {
    const DefLambdaH& def_h = _defLambdaH[idxD];
    _changedDualX[_idxMapX[def_h._jl]] = true;

    for (BpIncEdgeIt ik(_gm, def_h._i); ik != lemon::INVALID; ++ik)
    {
      _changedDualX[_idxMapX[ik]] = true;
    }

    _changedDualH[idxD] = true;
    _subgradientH[idxD] = 0;

    if (!_updatedSumH[def_h._jl])
    {
      computeSumH(dual, def_h._jl);
      _updatedSumH[def_h._jl] = true;
    }
  }
  else
  {
    idxD -= _defLambdaH.size();
    assert(0 <= idxD && idxD < _defLambdaV.size());

    const DefLambdaV& def_v = _defLambdaV[idxD];
    _changedDualX[_idxMapX[def_v._jl]] = true;

    for (BpIncEdgeIt ik(_gm, def_v._k); ik != lemon::INVALID; ++ik)
    {
      _changedDualX[_idxMapX[ik]] = true;
    }

    _changedDualV[idxD] = true;
    _subgradientV[idxD] = 0;

    if (!_updatedSumV[def_v._jl])
    {
      computeSumV(dual, def_v._jl);
      _updatedSumV[def_v._jl] = true;
    }
  }
}

template<typename GR, typename BGR>
inline void LagrangeGnaSiCached<GR, BGR>::unflag(size_t idxD)
{
  if (idxD < _defLambdaH.size())
  {
    const DefLambdaH& def_h = _defLambdaH[idxD];
    _changedDualX[_idxMapX[def_h._jl]] = false;

    for (BpIncEdgeIt ik(_gm, def_h._i); ik != lemon::INVALID; ++ik)
    {
      _changedDualX[_idxMapX[ik]] = false;
    }

    _changedDualH[idxD] = false;
    _updatedSumH[def_h._jl] = false;
  }
  else
  {
    idxD -= _defLambdaH.size();
    assert(0 <= idxD && idxD < _defLambdaV.size());

    const DefLambdaV& def_v = _defLambdaV[idxD];
    _changedDualX[_idxMapX[def_v._jl]] = false;

    for (BpIncEdgeIt ik(_gm, def_v._k); ik != lemon::INVALID; ++ik)
    {
      _changedDualX[_idxMapX[ik]] = false;
    }

    _changedDualV[idxD] = false;
    _updatedSumV[def_v._jl] = false;
  }
}

template<typename GR, typename BGR>
inline void LagrangeGnaSiCached<GR, BGR>::updateDualEnhanced(DualVector& dual, 
                                                             IndexList& dualIndices,
                                                             const size_t idxX,
                                                             PrimalDefYIt& itY,
                                                             size_t& idxY)
{
  // TODO
}

template<typename GR, typename BGR>
inline void LagrangeGnaSiCached<GR, BGR>::solveLocal(const DualVector& dual, 
                                                     const size_t idxX,
                                                     const BpEdge ik,
                                                     PrimalDefYIt& itY,
                                                     size_t& idxY)
{
  LocalProblem* pLocal = NULL;
  if (itY != _defY.end() && _yIdx2xIdx[idxY] == idxX)
    pLocal = _localProblemVector[_xIdx2lpIdx[idxX]];

  PrimalDefExtYIt itExtY = _defExtY.begin() + idxY;
  if (g_verbosity >= VERBOSE_DEBUG)
  {
    std::cout << "sum^h = " << _sumH[idxX] 
      << ", sum^v = " << _sumV[idxX] << ". "
      << "Solving local problem " << idxX << "..." << std::flush;
  }

  double profit = 0;
  IndexList& y_list = _pGlobal->getRealizedY(idxX);
  if (pLocal)
  {
    // update the weights ...
    for (; itY != _defY.end() && _yIdx2xIdx[idxY] == idxX; itY++, itExtY++, idxY++)
    {
      assert(itExtY != _defExtY.end());   
      //assert(_subgradientH[_defExtY[idxY]._idxLambdaH] == 0);
      //assert(_subgradientV[_defExtY[idxY]._idxLambdaV] == 0);

      _subgradientH[_defExtY[idxY]._idxLambdaH] = 0;
      _subgradientV[_defExtY[idxY]._idxLambdaV] = 0;
      _y[idxY] = false;

      if (hasChangedY(idxY))
      {
        double w = itY->_weight 
          - getDualH(dual, itExtY->_idxLambdaH) - getDualV(dual, itExtY->_idxLambdaV);
        pLocal->setProfit(itY->_jl, w);

        if (g_verbosity >= VERBOSE_DEBUG)
        {
          std::cout << "(" << idxY << ": " << w << "), ";
        }
      }
    }

    pLocal->scale();
    pLocal->solve();

    // update primal y variables
    y_list.clear();
    for (BpRedNodeIt j(pLocal->getRedGm()); j != lemon::INVALID; ++j)
    {
      BpEdge jl = pLocal->matching(j);
      if (jl != lemon::INVALID)
      {
        y_list.push_back(pLocal->getIdxY(jl));    
        profit += pLocal->getProfit(jl);

        // delete matching edge
        pLocal->clearMatching(jl);
      }
    }
    _pGlobal->incrProfit(ik, profit);
  }

  if (g_verbosity >= VERBOSE_DEBUG)
  {
    std::cout << "Done! Profit: " << _pGlobal->getProfit(ik) << ". Realizing y-s: ";
    for (IndexListConstIt it = y_list.begin(); it != y_list.end(); it++)
    {
      std::cout << *it << " ";
    }
    std::cout << std::endl;
  }
}

template<typename GR, typename BGR>
inline void LagrangeGnaSiCached<GR, BGR>::updateDualOld(DualVector& dual, 
                                                        IndexList& dualIndices,
                                                        const double tau,
                                                        const double phi) const
{
  assert(dual.size() == getDualDim());
  dualIndices.clear();

  size_t idxX = 0;
  for (PrimalDefExtXIt itExtX = _defExtX.begin(); 
    itExtX != _defExtX.end(); itExtX++, idxX++)
  {
    const IndexList& lambdaH = itExtX->_lambdaHIndices;
    const IndexList& lambdaV = itExtX->_lambdaVIndices;

    double deltaH = getDualSlackX(idxX) / (2. * lambdaH.size());
    if (deltaH != 0)
    {
      for (IndexListConstIt itH = lambdaH.begin(); itH != lambdaH.end(); itH++)
      {
        dualIndices.push_back(*itH);
        dual[*itH] += deltaH;
      }
    }

    double deltaV = getDualSlackX(idxX) / (2. * lambdaV.size());
    if (deltaV != 0)
    {
      for (IndexListConstIt itV = lambdaV.begin(); itV != lambdaV.end(); itV++)
      {
        dualIndices.push_back(_defLambdaH.size() + *itV);
        dual[_defLambdaH.size() + *itV] += deltaV;
      }
    }
  }
}

template<typename GR, typename BGR>
inline void LagrangeGnaSiCached<GR, BGR>::updateDual(DualVector& dual, 
                                                     IndexList& dualIndices,
                                                     const double tau,
                                                     const double phi) const
{
  updateDualOld(dual, dualIndices, tau, phi);
  return;

  assert(dual.size() == getDualDim());
  dualIndices.clear();

  DoubleVector maxDelta(dual.size(), 0);// std::numeric_limits<double>::max());

  size_t idxY = 0;
  for (PrimalDefExtYIt itExtY = _defExtY.begin(); 
    itExtY != _defExtY.end(); itExtY++, idxY++)
  {
    size_t idxX = getIdxX(idxY);
    int n1 = getRedNodeCount(idxY);
    int n2 = getBlueNodeCount(idxY);

    double delta = phi * (getDualSlackY(idxY) / 2. 
      + tau * (1./(4*n1) + 1./(4*n2)) * getDualSlackX(idxX));

    if (maxDelta[itExtY->_idxLambdaH] < delta)
      maxDelta[itExtY->_idxLambdaH] = delta;
    if (maxDelta[itExtY->_idxLambdaV + _defLambdaH.size()] < delta)
      maxDelta[itExtY->_idxLambdaV + _defLambdaH.size()] = delta;
  }

  for (size_t idxD = 0; idxD < dual.size(); idxD++)
  {
    dual[idxD] = dual[idxD] + maxDelta[idxD];
    if (maxDelta[idxD] != 0)
      dualIndices.push_back(idxD);
  }
}

} // namespace gna
} // namespace nina

#endif /* LAGRANGEGNASICACHED_H_ */
