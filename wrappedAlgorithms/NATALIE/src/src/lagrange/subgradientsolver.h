/* 
 * subgradientsolver.h
 *
 *  Created on: 9-feb-2011
 *      Author: M. El-Kebir
 */

#ifndef SUBGRADIENTSOLVER_H_
#define SUBGRADIENTSOLVER_H_

#include <vector>
#include <algorithm>
#include <limits>
#include <iostream>
#include <iomanip>
#include <lemon/time_measure.h>
#include "lagrange/lagrangeitfc.h"
#include "lagrange/lagrangesolver.h"
#include "verbose.h"

namespace nina {

/// Subgradient solver for solving a Lagrangian dual problem
///
/// Note that for all dual variables the same upper and lower bound is assumed
///
/// \tparam PVT is the type of the primal value
/// \tparam PT is the type of the primal variables
/// \tparam DT is the type of the dual variables
/// \tparam ST is the type of the subgradients
template<typename PVT, typename PT, typename DT, typename ST>
class SubgradientSolver : public virtual LagrangeSolver<PVT, PT, DT, ST>
{
public:
  /// Base class type
  typedef LagrangeSolver<PVT, PT, DT, ST> Parent;
  /// Primal value type
  typedef PVT PrimalValueType;
  /// Primal variable type
  typedef PT PrimalType;
  /// Dual variable type
  typedef DT DualType;
  /// Subgradient type
  typedef ST SubgradientType;
  /// Type of Lagrangian problem
  typedef typename Parent::LagrangianProblem LagrangianProblem;
  /// Type of dual variables vector
  typedef typename Parent::DualVector DualVector;
  /// Type of primal variables vector
  typedef typename Parent::PrimalVector PrimalVector;
  /// Type of subgradient indices (whose value is nonzero)
  typedef typename Parent::IndexList IndexList;
  /// Const iterator type of subgradient indices list
  typedef typename Parent::IndexListConstIt IndexListConstIt;
  /// Return type of the solver
  typedef typename Parent::SolverResult SolverResult;

  /// Type of value-primal vector pairs
  typedef typename Parent::ValuePrimalPair ValuePrimalPair;
  /// Type of list of value-primal vector pairs
  typedef typename Parent::ValuePrimalPairList ValuePrimalPairList;
  /// Const iterator type of value-primal vector pairs list
  typedef typename Parent::ValuePrimalPairListIt ValuePrimalPairListIt;

  using Parent::_pLR;
  using Parent::_nMaxIterations;
  using Parent::_timeLimit;
  using Parent::_dual;
  using Parent::_dualIndices;
  using Parent::_bestDual;
  using Parent::_nIterations;
  using Parent::init;
  using Parent::getBestPrimal;
  using Parent::getBestDual;
  using Parent::solve;
  using Parent::getLowerBound;
  using Parent::getUpperBound;
  using Parent::updateLBs;
  using Parent::updateUBs;
  using Parent::SOLVED;
  using Parent::CONTINUE;
  using Parent::UNSOLVED_FIXED_DUAL;
  using Parent::UNSOLVED_MAX_IT;
  using Parent::UNSOLVED_TIME_LIMIT;

private:
  /// Number of idle iterations after which _mu is updated
  const int _nMaxIdleIterations;
  /// Number of non idle iterations after which _mu is updated
  const int _nMaxNonIdleIterations;
  /// Parameter which is used in updating multipliers
  double _mu;
  /// Parameter which is used in updating multipliers
  double _alpha;

  /// Subgradient step size denominator
  SubgradientType _denom;

protected:
  /// Initialize member variables
  void init();
  /// Output header
  void outputHeader() const;
  /// Evaluate _dual
  void evaluate();
  /// Update _dual
  void update();

  /// Number of currently consecutive iterations not changing the bounds
  int _nIdleIterations;
  /// Number of currently consecutive iterations changing the bounds
  int _nNonIdleIterations;
   /// Nonzero subgradient indices
  IndexList _subgradientIndices;

public:
  /// Constructor
  SubgradientSolver(LagrangianProblem* pLR,
                    int timeLimit,
                    int nMaxIterations,
                    int nSolutions,
                    int nMaxIdleIterations,
                    int nMaxNonIdleIterations,
                    double mu,
                    double alpha);
  /// Perform a step
  SolverResult performStep();
  /// Get parameter mu
  double getMu() const { return _mu; };
  /// Get parameter alpha
  double getAlpha() const { return _alpha; };
  /// Set parameter mu
  void setMu(double mu) { _mu = mu; };
  /// Set parameter alpha
  void setAlpha(double alpha) { _alpha = alpha; };
};

template<typename PVT, typename PT, typename DT, typename ST>
SubgradientSolver<PVT, PT, DT, ST>::SubgradientSolver(LagrangianProblem* pLR,
                                                      int timeLimit,
                                                      int nMaxIterations,
                                                      int nSolutions,
                                                      int nMaxIdleIterations,
                                                      int nMaxNonIdleIterations,
                                                      double mu,
                                                      double alpha)
  : Parent(pLR, timeLimit, nMaxIterations, nSolutions)
  , _nMaxIdleIterations(nMaxIdleIterations)
  , _nMaxNonIdleIterations(nMaxNonIdleIterations)
  , _mu(mu)
  , _alpha(alpha)
  , _denom()
  , _nIdleIterations()
  , _nNonIdleIterations()
  , _subgradientIndices()
{
}

template<typename PVT, typename PT, typename DT, typename ST>
void SubgradientSolver<PVT, PT, DT, ST>::init()
{
  Parent::init();
  _nIdleIterations = _nNonIdleIterations = 0;

  if (g_verbosity >= VERBOSE_ESSENTIAL)
  {
    std::cout << "Initialized subgradient solver with k = "
      << _nMaxIdleIterations 
      << " and l = " << _nMaxNonIdleIterations 
      << std::endl;
  }
}

template<typename PVT, typename PT, typename DT, typename ST>
void SubgradientSolver<PVT, PT, DT, ST>::outputHeader() const
{
  std::cout 
    << std::left << std::setw(7) << std::setprecision(6) << "i"
    << std::left << std::setw(12) << std::setprecision(4) << "mu"
    << std::left << std::setw(16) << std::setprecision(8) << "LB*" 
    << std::left << std::setw(16) << std::setprecision(8) << "UB*" 
    << std::left << std::setw(16) << std::setprecision(8) << " LB" 
    << std::left << std::setw(16) << std::setprecision(8) << " UB"
    << std::left << std::setw(9) << std::setprecision(3) << "time (s)"
    << std::left << std::setw(9) << std::setprecision(7) << "#subgr+"
    << std::left << std::setw(9) << std::setprecision(7) << "#subgr-"
    << std::left << std::setw(9) << std::setprecision(7) << "#dual"
    << std::endl;
}

template<typename PVT, typename PT, typename DT, typename ST>
void SubgradientSolver<PVT, PT, DT, ST>::evaluate()
{
  assert(_dual.size() == _pLR->getDualDim());

  lemon::Timer timer;
  _nIterations++;

  if (g_verbosity >= VERBOSE_NON_ESSENTIAL && g_verbosity != VERBOSE_DEBUG)
  {
    std::cout 
      << std::left << std::setw(7) << std::setprecision(6) << _nIterations
      << std::left << std::setw(12) << std::setprecision(3) << _mu
      << std::left << std::setw(16) << std::setprecision(8) << getLowerBound()
      << std::left << std::setw(16) << std::setprecision(8) << getUpperBound()
      << std::flush;
  }

  _denom = _pLR->evaluate(_dual, _dualIndices, _subgradientIndices);

  const PrimalValueType curLB = _pLR->getLowerBound();
  const PrimalValueType curUB = _pLR->getUpperBound();

  bool hasImprovedLB = updateLBs(curLB, _pLR->getPrimalVector());
  bool hasImprovedUB = updateUBs(curUB, _pLR->getPrimalVector());

  if (hasImprovedLB || hasImprovedUB)
  {
    _nIdleIterations = 0;
    _nNonIdleIterations++;

    // In case of a maximization problem: 
    //   a higher lower bound is a better feasible solution
    // In case of a minimization problem:
    //   a higher lower bound is a tighter solution of the dual problem

    // In case of a maximization problem: 
    //   a lower upper bound is a tighter solution of the dual problem
    // In case of a minimization problem:
    //   a lower upper bound is a better feasible solution
    if ((!_pLR->isMaximizationProblem() && hasImprovedLB) || 
        (_pLR->isMaximizationProblem() && hasImprovedUB))
    {
      _bestDual = _dual;
    }
  }  
  else
  {
    _nNonIdleIterations = 0;
    _nIdleIterations++;
  }

  if (_nIdleIterations == _nMaxIdleIterations)
  {
    _mu /= _alpha;
    _nIdleIterations = 0;
  }
  if (_nNonIdleIterations == _nMaxNonIdleIterations)
  {
    _mu *= _alpha;
    _nNonIdleIterations = 0;
  }

  if (g_verbosity >= VERBOSE_NON_ESSENTIAL && g_verbosity != VERBOSE_DEBUG)
  {
    std::cout
      << (hasImprovedLB ? "*" : " ")
      << std::left << std::setw(15) << std::setprecision(8) << curLB
      << (hasImprovedUB ? "*" : " ")
      << std::left << std::setw(14) << std::setprecision(8) << curUB
      << std::left << std::setw(9) << std::setprecision(3) << timer.realTime()
      << std::flush;
  }
}

template<typename PVT, typename PT, typename DT, typename ST>
void SubgradientSolver<PVT, PT, DT, ST>::update()
{
  assert(_dual.size() == _pLR->getDualDim());

  _dualIndices.clear();
  double ss = _mu * (getUpperBound() - getLowerBound()) / _denom;
  int pos = 0, neg = 0;
  for (IndexListConstIt sIdxIt = _subgradientIndices.begin();
    sIdxIt != _subgradientIndices.end(); sIdxIt++)
  {
    assert(_pLR->getSubgradient(*sIdxIt) != 0);

    const SubgradientType s = _pLR->getSubgradient(*sIdxIt);
    const DualType dualLB = _pLR->getDualVarLowerBound(*sIdxIt);
    const DualType dualUB = _pLR->getDualVarUpperBound(*sIdxIt);

    DualType d = std::max(std::min(_dual[*sIdxIt] - ss * s, dualUB), dualLB);

    if (d != _dual[*sIdxIt])
    {
      _dual[*sIdxIt] = d;
      _dualIndices.push_back(*sIdxIt);
    }

    if (s >= 0)
      pos++;
    else
      neg++;
  }

  if (g_verbosity >= VERBOSE_NON_ESSENTIAL && g_verbosity != VERBOSE_DEBUG)
  {
    std::cout
      << std::left << std::setw(9) << std::setprecision(7) << pos
      << std::left << std::setw(9) << std::setprecision(7) << neg
      << std::left << std::setw(9) << std::setprecision(7) << _dualIndices.size()
      << std::endl;
  }
}

template<typename PVT, typename PT, typename DT, typename ST>
typename SubgradientSolver<PVT, PT, DT, ST>::SolverResult 
  SubgradientSolver<PVT, PT, DT, ST>::performStep()
{
  evaluate();
  update();

  if (getLowerBound() == getUpperBound())
    return SOLVED;
  else if (_subgradientIndices.size() == 0 && _pLR->onlyEqualityConstraintsDualized())
    return SOLVED;
  else if (_dualIndices.size() == 0)
    return UNSOLVED_FIXED_DUAL;
  else
    return CONTINUE;
}

} // namespace nina

#endif /* SUBGRADIENTSOLVER_H_ */
