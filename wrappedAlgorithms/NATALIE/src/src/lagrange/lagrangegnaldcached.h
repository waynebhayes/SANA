/*
 * lagrangegnaldcached.h
 *
 *  Created on: 8-mar-2011
 *      Author: M. El-Kebir
 */

#ifndef LAGRANGEGNALDCACHED_H_
#define LAGRANGEGNALDCACHED_H_

#include <lemon/core.h>
#include <lemon/bp_matching.h>
#include <lemon/adaptors.h>
#include <limits>
#include "lagrange/lagrangegna.h"
#include "lagrange/lagrangegnald.h"
#include "lagrange/lagrangegnacached.h"
#include "verbose.h"

namespace nina {
namespace gna {

/// The number of dual variables is:
/// \f$\frac{1}{2} |V_1|(|V_1|+1)|V_2|^2 = O(|V_1|^2 |V_2|^2)\f$.
///
/// The number of primal variables is:
/// \f$|V_1||V_2| + |V_1|^2|V_2^2 \f$.
///
/// Note that only variables whose weight != 0 are considered.
template<typename GR, typename BGR>
class LagrangeGnaLdCached
  : public LagrangeGnaCached<GR, BGR>, public LagrangeGnaLd<GR, BGR>
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
  typedef LagrangeGnaLd<GR, BGR> Parent2;
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
  using Parent1::getRedNodeCount;
  using Parent1::getBlueNodeCount;

  using Parent2::_defX;
  using Parent2::_defExtX;
  using Parent2::_defExtY;
  using Parent2::_defY;
  using Parent2::_defLambda;
  using Parent2::_defMu;
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
  using Parent2::getOrgEdge;
  using Parent2::getDualSlackX;
  using Parent2::getIdxX;
  using Parent2::getIdxY;

protected:
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
  typedef typename Parent1::PrimalDefY PrimalDefY;
  typedef typename Parent1::PrimalDefYIt PrimalDefYIt;
  typedef typename Parent1::LocalProblemVector LocalProblemVector;
  typedef typename Parent1::LocalProblemVectorIt LocalProblemVectorIt;
  typedef typename Parent1::LocalProblemVectorConstIt LocalProblemVectorConstIt;

  typedef typename Parent2::DefExtY DefExtY;
  typedef typename Parent2::DefLambda DefLambda;
  typedef typename Parent2::PrimalDefExtY PrimalDefExtY;
  typedef typename Parent2::PrimalDefExtYNonConstIt PrimalDefExtYNonConstIt;
  typedef typename Parent2::PrimalDefExtYIt PrimalDefExtYIt;
  typedef typename Parent2::DualDefLambda DualDefLambda;
  typedef typename Parent2::DualDefLambdaIt DualDefLambdaIt;

  typedef std::vector<bool> BoolVector;  
  typedef std::vector<double> DoubleVector;
  typedef typename DoubleVector::const_iterator DoubleVectorIt;

protected:
  /// Indicates whether a dual lambda variable corresponding to an x var has been updated
  BoolVector _changedLambdaX;
  /// Indicates whether a mu lambda variable corresponding to an x var has been updated
  BoolVector _changedMuX;
  /// Indicates whether the corresponding dual variable has been updated
  BoolVector _changedDual;
  /// Cache containing c_{ik} + Sum_{j,l} (mu_{ikjl} + mu_{jlik}) for all i,k
  DoubleVector _initProfitX;

  /// Solve the local problem for matching edge ik
  virtual void solveLocal(const DualVector& dual, 
                          const size_t idxX, 
                          const BpEdge ik, 
                          PrimalDefYIt& itY, 
                          size_t& idxYFwd);
  /// Indicates whether it is necessary to solve idxX
  bool hasChangedX(size_t idxX);
  /// Indicates whether it is necessary to profit of idxY
  bool hasChangedY(size_t idxY);
  /// Initialize dual variable definitions
  void initDual();
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
  LagrangeGnaLdCached(const MatchingGraphType& matchingGraph,
                      const ScoreModelType& scoreModel,
                      bool integral);
  /// Destructor
  virtual ~LagrangeGnaLdCached() {}
  /// Initialize the primal and dual variables
  void init();
  /// Update multipliers according to the dual descent scheme
  void updateDual(DualVector& dual,
                  IndexList& dualIndices,
                  const double tau,
                  const double phi) const;
  /// Initialize global profit
  virtual void initGlobalProfit(const BpEdge ik,
                                const size_t idxX,
                                const DualVector& dual);
  /// Indicates whether only equality constraint are relaxed
  bool onlyEqualityConstraintsDualized() const { return _defMu.size() == 0; }
};

template<typename GR, typename BGR>
inline LagrangeGnaLdCached<GR, BGR>::LagrangeGnaLdCached(const MatchingGraphType& matchingGraph,
                                                         const ScoreModelType& scoreModel,
                                                         bool integral)
  : GrandParent(matchingGraph, scoreModel, integral)
  , Parent1(matchingGraph, scoreModel, integral)
  , Parent2(matchingGraph, scoreModel, integral)
  , _changedLambdaX()
  , _changedMuX()
  , _changedDual()
  , _initProfitX()
{
}

template<typename GR, typename BGR>
inline void LagrangeGnaLdCached<GR, BGR>::init()
{
  createLocal();
  initPrimal();
  _initProfitX = DoubleVector(_defX.size(), 0);
  initDual();
  initLocal();

  _changedLambdaX = _changedMuX = BoolVector(_x.size(), false);
  _changedDual = BoolVector(getDualDim(), false);

  DoubleVector dual(getDualDim(), 0);
  size_t idxX = 0;
  for (BpEdgeIt ik(_gm); ik != lemon::INVALID; ++ik, idxX++)
  {
    _initProfitX[idxX] = _scoreModel.getWeightGm(ik);
    initGlobalProfit(ik, idxX, dual);
  }

  if (g_verbosity >= VERBOSE_ESSENTIAL)
  {
    std::cout << "Number of primal x variables: " << _x.size() << std::endl;
    std::cout << "Number of primal y variables: " << _y.size() << std::endl;
    std::cout << "Number of dual lambda variables: " << _defLambda.size() << std::endl;
    std::cout << "Number of dual mu variables: " << _defMu.size() << std::endl;
    std::cout << "Number of subgradients: " << getDualDim() << std::endl;
  }

  if (g_verbosity >= VERBOSE_DEBUG)
  {
    std::cout << std::endl;
    printPrimalDef(std::cout);
    printDualDef(std::cout);
    std::cout << std::endl;
  }

  // init global profits, caching screws everything up (TODO)
  for (PrimalDefXIt itX = _defX.begin(); itX != _defX.end(); itX++)
  {
    const BpEdge ik = *itX;
    _pGlobal->setProfit(ik, _scoreModel.getWeightGm(ik));
  }
}

template<typename GR, typename BGR>
inline void LagrangeGnaLdCached<GR, BGR>::initDual()
{
  _defLambda.clear();
  _defMu.clear();

  size_t idxY = 0;
  PrimalDefExtYNonConstIt itExtY = _defExtY.begin();
  PrimalDefYIt itY = _defY.begin();
  for (; itY != _defY.end(); itY++, itExtY++, idxY++)
  {
    assert(itExtY != _defExtY.end());

    // init mu
    if (_defY[idxY]._weight < 0)
    {
      _defExtY[idxY]._idxMu = _defMu.size();
      _defMu.push_back(idxY);
    }

    if (itExtY->_fwd)
    {
      // find y_{jlik}
      BpEdge ik = getOrgEdge(idxY, true);
      BpEdge jl = getOrgEdge(idxY, false);
      LocalProblem& local = *_localProblemVector[_xIdx2lpIdx[_idxMapX[jl]]];
      bool found = false;
      for (BpEdgeIt red_jl(local.getRedGm()); red_jl != lemon::INVALID; ++red_jl)
      {
        if (local.getOrgEdge(red_jl) == ik)
        {
          size_t idxY1 = local.getIdxY(red_jl);
          itExtY->_idxLambda = _defExtY[idxY1]._idxLambda = _defLambda.size();

          DefLambda def_lambda = {idxY, idxY1};
          _defLambda.push_back(def_lambda);
          found = true;
          break;
        }
      }
      assert(found);
    }
  }
}

template<typename GR, typename BGR>
inline void LagrangeGnaLdCached<GR, BGR>::checkOrFlag(const DualVector& dual,
                                                      IndexList& dualIndices, 
                                                      size_t idxD)
{
  if (!_changedDual[idxD])
  {
    flag(dual, idxD);
    dualIndices.push_back(idxD);
  }
}

template<typename GR, typename BGR>
inline void LagrangeGnaLdCached<GR, BGR>::flag(const DualVector&, 
                                               size_t idxD)
{
  assert(0 <= idxD && idxD < _defLambda.size() + _defMu.size());

  if (idxD < _defLambda.size())
  {
    const DefLambda& def_lambda = _defLambda[idxD];
    _changedLambdaX[_yIdx2xIdx[def_lambda._idxYBwd]] = true;
    _changedLambdaX[_yIdx2xIdx[def_lambda._idxYFwd]] = true;
  }
  else
  {
    _changedMuX[_yIdx2xIdx[_defMu[idxD - _defLambda.size()]]] = true;
  }

  _changedDual[idxD] = true;
}

template<typename GR, typename BGR>
inline void LagrangeGnaLdCached<GR, BGR>::unflag(size_t idxD)
{
  assert(0 <= idxD && idxD < _defLambda.size() + _defMu.size());

  if (idxD < _defLambda.size())
  {
    const DefLambda& def_lambda = _defLambda[idxD];
    _changedLambdaX[_yIdx2xIdx[def_lambda._idxYBwd]] = false;
    _changedLambdaX[_yIdx2xIdx[def_lambda._idxYFwd]] = false;
  }
  else
  {
    _changedMuX[_yIdx2xIdx[_defMu[idxD - _defLambda.size()]]] = false;
  }

  _changedDual[idxD] = false;
}

template<typename GR, typename BGR>
inline bool LagrangeGnaLdCached<GR, BGR>::hasChangedX(size_t idxX)
{
  return _changedLambdaX[idxX] || _changedMuX[idxX];
}

template<typename GR, typename BGR>
inline bool LagrangeGnaLdCached<GR, BGR>::hasChangedY(size_t idxY)
{
  return _changedDual[_defExtY[idxY]._idxLambda] ||
      _changedDual[_defExtY[idxY]._idxMu + _defLambda.size()];
}

template<typename GR, typename BGR>
inline void LagrangeGnaLdCached<GR, BGR>::updateDualEnhanced(DualVector& dual, 
                                                             IndexList& dualIndices,
                                                             const size_t idxX,
                                                             PrimalDefYIt& itY,
                                                             size_t& idxY)
{
  LocalProblem& local = *_localProblemVector[_xIdx2lpIdx[idxX]];
  PrimalDefExtYIt itExtY = _defExtY.begin() + idxY;
  
  for (; itY != _defY.end() && _yIdx2xIdx[idxY] == idxX; itY++, itExtY++, idxY++)
  {
    if (itExtY->_fwd && getDualSlackY(idxY) != 0)
    {
      dual[itExtY->_idxLambda] += getDualSlackY(idxY);
      checkOrFlag(dual, dualIndices, itExtY->_idxLambda);
      local.reduceScaledProfit(itY->_jl);
    }
  }
}

template<typename GR, typename BGR>
inline void LagrangeGnaLdCached<GR, BGR>::solveLocal(const DualVector& dual, 
                                                     const size_t idxX,
                                                     const BpEdge ik,
                                                     PrimalDefYIt& itY,
                                                     size_t& idxY)
{
  LocalProblem& local = *_localProblemVector[_xIdx2lpIdx[idxX]];
  const BpGraph& redGm = local.getRedGm();

  PrimalDefExtYIt itExtY = _defExtY.begin() + idxY;
  if (g_verbosity >= VERBOSE_DEBUG)
  {
    std::cout << "Solving local problem " << idxX << "..." << std::flush;
  }

  // update the weights ...
  for (; itY != _defY.end() && _yIdx2xIdx[idxY] == idxX; itY++, itExtY++, idxY++)
  {
    assert(itExtY != _defExtY.end());
    _y[idxY] = false;

    if (hasChangedY(idxY))
    {
      double w = itY->_weight
          + (itExtY->_fwd ? dual[itExtY->_idxLambda] : - dual[itExtY->_idxLambda])
          + (itY->_weight < 0 ? dual[itExtY->_idxMu] : 0);
      local.setProfit(itY->_jl, w);
    }
  }

  local.scale();
  local.solve();

  // update primal y variables
  IndexList& y_list = _pGlobal->getRealizedY(idxX);
  y_list.clear();
  double profit = 0;
  for (BpRedNodeIt j(redGm); j != lemon::INVALID; ++j)
  {
    BpEdge jl = local.matching(j);
    if (jl != lemon::INVALID)
    {
      y_list.push_back(local.getIdxY(jl));
      profit += local.getProfit(jl);

      // delete matching edge
      local.clearMatching(jl);
    }
  }

  _pGlobal->incrProfit(ik, profit);
  if (g_verbosity >= VERBOSE_DEBUG)
  {
    std::cout << "Done! Profit: " << profit << std::endl;
  }
}

template<typename GR, typename BGR>
inline void LagrangeGnaLdCached<GR, BGR>::updateDual(DualVector& dual, 
                                                     IndexList& dualIndices,
                                                     const double tau,
                                                     const double phi) const
{
  assert(dual.size() == getDualDim());
  dualIndices.clear();
  for (size_t idxD = 0; idxD < _defLambda.size(); idxD++)
  {
    size_t idxY_ik = getIdxY(idxD, true);
    size_t idxY_jl = getIdxY(idxD, false);

    size_t idxX_ik = getIdxX(idxY_ik);
    size_t idxX_jl = getIdxX(idxY_jl);

    double slack_x_ik = getDualSlackX(idxX_ik);
    double slack_x_jl = getDualSlackX(idxX_jl);
    double slack_y_ik = getDualSlackY(idxY_ik);
    double slack_y_jl = getDualSlackY(idxY_jl);

    int n1_ik = getRedNodeCount(idxY_ik);
    int n2_ik = getBlueNodeCount(idxY_ik);
    int n1_jl = getRedNodeCount(idxY_jl);
    int n2_jl = getBlueNodeCount(idxY_jl);

    double tau_ik = tau * (1. / (2*n1_ik) + 1. / (2*n2_ik));
    double tau_jl = tau * (1. / (2*n1_jl) + 1. / (2*n2_jl));

    double delta = 
      phi * (slack_y_ik + tau_ik * slack_x_ik)
      - phi * (slack_y_jl + tau_jl * slack_x_jl);

    if (delta != 0)
    {
      dualIndices.push_back(idxD);
      dual[idxD] += delta;
    }
  }
}

template<typename GR, typename BGR>
inline void LagrangeGnaLdCached<GR, BGR>::initGlobalProfit(const BpEdge ik,
                                                           const size_t idxX,
                                                           const DualVector& dual)
{
  if (_changedMuX[idxX])
  {
    double profit = _scoreModel.getWeightGm(ik);

    const IndexVector& relevantDuals = _defExtX[idxX];
    for (IndexVectorConstIt itIdxD = relevantDuals.begin();
         itIdxD != relevantDuals.end(); itIdxD++)
    {
      profit -= dual[*itIdxD + _defLambda.size()];
    }

    _initProfitX[idxX] = profit;
  }

  _pGlobal->setProfit(ik, _initProfitX[idxX]);
}

} // namespace gna
} // namespace nina

#endif /* LAGRANGEGNALDCACHED_H_ */
