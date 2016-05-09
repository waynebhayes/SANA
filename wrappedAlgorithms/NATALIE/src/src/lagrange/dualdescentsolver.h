/* 
 * dualdescentsolver.h
 *
 *  Created on: 9-feb-2011
 *      Author: M. El-Kebir
 */

#ifndef DUALDESCENTSOLVER_H_
#define DUALDESCENTSOLVER_H_

#include <vector>
#include <algorithm>
#include <limits>
#include <iostream>
#include <iomanip>
#include <lemon/time_measure.h>
#include "lagrange/lagrangegnacached.h"
#include "lagrange/lagrangesolver.h"
#include "verbose.h"

namespace nina {
namespace gna {

/// Dual descent solver for solving a Lagrangian dual problem
template<typename GR, typename BGR>
class DualDescentSolver : public virtual LagrangeSolver<double, bool, double, int>
{
public:
  /// Base class type
  typedef LagrangeSolver<double, bool, double, int> Parent;
  /// Lagrange GNA problem
  typedef LagrangeGnaCached<GR, BGR> LagrangeGnaCachedType;

protected:
  /// Lagrangian cached GNA primal problem
  LagrangeGnaCachedType* _pGNA;
  /// Parameter which is used in updating multipliers
  double _tau;
  /// Parameter which is used in updating multipliers
  double _phi;
  /// Enhanced
  bool _enhanched;

protected:
  /// Initialize member variables
  void init();
  /// Output header
  void outputHeader() const;
  /// Evaluate _dual
  void evaluate();
  /// Update _dual
  void update();

public:
  /// Constructor
  DualDescentSolver(LagrangeGnaCachedType* pCachedGna,
                    int timeLimit,
                    int nMaxIterations,
                    int nSolutions,
                    double tau,
                    double phi,
                    bool enhanched);
  /// Perform a step
  SolverResult performStep();
};

template<typename GR, typename BGR>
DualDescentSolver<GR, BGR>::DualDescentSolver(LagrangeGnaCachedType* pCachedGna,
                                              int timeLimit,
                                              int nMaxIterations,
                                              int nSolutions,
                                              double tau,
                                              double phi,
                                              bool enhanched)
  : Parent(pCachedGna, timeLimit, nMaxIterations, nSolutions)
  , _pGNA(pCachedGna)
  , _tau(tau)
  , _phi(phi)
  , _enhanched(enhanched)
{
}

template<typename GR, typename BGR>
void DualDescentSolver<GR, BGR>::init()
{
  Parent::init();

  if (g_verbosity >= VERBOSE_ESSENTIAL)
  {
    std::cout << "Initialized dual ascent solver with tau = "
      << _tau
      << ", phi = " << _phi
      << " and enhanced = "
      << (_enhanched ? "1" : "0")
      << std::endl;
  }
}

template<typename GR, typename BGR>
void DualDescentSolver<GR, BGR>::outputHeader() const
{
  std::cout 
    << std::left << std::setw(7) << std::setprecision(6) << "i"
    << std::left << std::setw(16) << std::setprecision(8) << "LB*" 
    << std::left << std::setw(16) << std::setprecision(8) << "UB*" 
    << std::left << std::setw(16) << std::setprecision(8) << " LB" 
    << std::left << std::setw(16) << std::setprecision(8) << " UB"
    << std::left << std::setw(9) << std::setprecision(3) << "time (s)"
    << std::left << std::setw(9) << std::setprecision(7) << "#dual"
    << std::endl;
}

template<typename GR, typename BGR>
void DualDescentSolver<GR, BGR>::evaluate()
{
  assert(_dual.size() == _pLR->getDualDim());

  lemon::Timer timer;
  _nIterations++;

  if (g_verbosity >= VERBOSE_NON_ESSENTIAL && g_verbosity != VERBOSE_DEBUG)
  {
    std::cout 
      << std::left << std::setw(7) << std::setprecision(6) << _nIterations
      << std::left << std::setw(16) << std::setprecision(8) << getLowerBound()
      << std::left << std::setw(16) << std::setprecision(8) << getUpperBound()
      << std::flush;
  }

  if (_enhanched)
    _pGNA->evaluateEnhanced(_dual, _dualIndices);
  else
    _pGNA->evaluate(_dual, _dualIndices);

  const double curLB = _pGNA->getLowerBound();
  const double curUB = _pGNA->getUpperBound();

  bool hasImprovedLB = updateLBs(curLB, _pLR->getPrimalVector());
  bool hasImprovedUB = updateUBs(curUB, _pLR->getPrimalVector());

  //assert(curUB <= getUpperBound()); // only true if dual descent is used
  
  if ((!_pLR->isMaximizationProblem() && hasImprovedLB) || 
      (_pLR->isMaximizationProblem() && hasImprovedUB))
  {
    _bestDual = _dual;
  }

  if (g_verbosity >= VERBOSE_NON_ESSENTIAL && g_verbosity != VERBOSE_DEBUG)
  {
    std::cout
      << (hasImprovedLB ? "*" : " ")
      << std::left << std::setw(15) << std::setprecision(8) << curLB
      << (hasImprovedUB ? "*" : " ")
      << std::left << std::setw(15) << std::setprecision(8) << curUB
      << std::left << std::setw(9) << std::setprecision(3) << timer.realTime()
      << std::flush;
  }
}

template<typename GR, typename BGR>
void DualDescentSolver<GR, BGR>::update()
{
  assert(_dual.size() == _pLR->getDualDim());

  _pGNA->updateDual(_dual, _dualIndices, _tau, _phi);

  if (g_verbosity >= VERBOSE_NON_ESSENTIAL && g_verbosity != VERBOSE_DEBUG)
  {
    std::cout
      << std::left << std::setw(9) << std::setprecision(7) << _dualIndices.size()
      << std::endl;
  }
}

template<typename GR, typename BGR>
typename DualDescentSolver<GR, BGR>::SolverResult 
  DualDescentSolver<GR, BGR>::performStep()
{
  evaluate();
  update();

  if (getLowerBound() == getUpperBound())
    return SOLVED;
  else if (_dualIndices.size() == 0)
    return UNSOLVED_FIXED_DUAL;
  else
    return CONTINUE;
}

} // namespace gna
} // namespace nina

#endif /* DUALDESCENTSOLVER_H_ */
