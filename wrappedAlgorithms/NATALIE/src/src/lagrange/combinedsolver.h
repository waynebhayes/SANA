/* 
 * combinedsolver.h
 *
 *  Created on: 14-apr-2011
 *      Author: M. El-Kebir
 */

#ifndef COMBINEDSOLVER_H_
#define COMBINEDSOLVER_H_

#include "lagrange/dualdescentsolver.h"
#include "lagrange/subgradientsolver.h"

namespace nina {
namespace gna {

/// Combined subgradient and dual descent solver
/// for solving a Lagrangian dual problem
template<typename GR, typename BGR>
class CombinedSolver : 
  public DualDescentSolver<GR, BGR>,
  public SubgradientSolver<double, bool, double, int>
{
public:
  /// Base class type
  typedef LagrangeSolver<double, bool, double, int> GrandParent;
  /// Base class type
  typedef DualDescentSolver<GR, BGR> Parent1;
  /// Base class type
  typedef SubgradientSolver<double, bool, double, int> Parent2;
  /// Lagrange GNA problem
  typedef typename Parent1::LagrangeGnaCachedType LagrangeGnaCachedType;
  /// Solver result type
  typedef typename Parent::SolverResult SolverResult;

  using Parent1::SOLVED;
  using Parent1::CONTINUE;
  using Parent1::UNSOLVED_FIXED_DUAL;
  using Parent1::UNSOLVED_MAX_IT;
  using Parent1::UNSOLVED_TIME_LIMIT;
  using Parent1::_pGNA;

protected:
  /// Initialize member variables
  void init();
  /// Output header
  void outputHeader() const;
  /// Evaluate _dual
  void evaluate();
  /// Update _dual
  void update();

private:
  bool _dualDescentMode;
  IndexList _allDualIndices;
  int _descentSteps;
  int _subgradientSteps;
  const double _initMu;
  const double _initAlpha;
  int _maxDescentSteps;
  int _maxSubgradientSteps;
  int _maxSwitches;
  int _switches;

public:
  /// Constructor
  CombinedSolver(LagrangeGnaCachedType* pCachedGna,
                 int timeLimit,
                 int nMaxIterations,
                 int nSolutions,
                 int nMaxIdleIterations,
                 int nMaxNonIdleIterations,
                 double mu,
                 double alpha,
                 double tau,
                 double phi,
                 bool enhanched,
                 int maxDescentSteps,
                 int maxSubgradientSteps,
                 int nPhases);
  /// Perform a step
  SolverResult performStep();
  /// Solve the dual problem
  SolverResult solve();
};

template<typename GR, typename BGR>
CombinedSolver<GR, BGR>::CombinedSolver(LagrangeGnaCachedType* pCachedGna,
                                        int timeLimit,
                                        int nMaxIterations,
                                        int nSolutions,
                                        int nMaxIdleIterations,
                                        int nMaxNonIdleIterations,
                                        double mu,
                                        double alpha,
                                        double tau,
                                        double phi,
                                        bool enhanched,
                                        int maxDescentSteps,
                                        int maxSubgradientSteps,
                                        int maxSwitches)
  : GrandParent(pCachedGna, timeLimit, nMaxIterations, nSolutions)
  , Parent1(pCachedGna, timeLimit, nMaxIterations, nSolutions,
            tau, phi, enhanched)
  , Parent2(pCachedGna, timeLimit, nMaxIterations, nSolutions,
            nMaxIdleIterations, nMaxNonIdleIterations, mu, alpha)
  , _dualDescentMode(false)
  , _allDualIndices()
  , _descentSteps(0)
  , _subgradientSteps(0)
  , _initMu(mu)
  , _initAlpha(alpha)
  , _maxDescentSteps(maxDescentSteps)
  , _maxSubgradientSteps(maxSubgradientSteps)
  , _maxSwitches(maxSwitches)
  , _switches(0)
{
}

template<typename GR, typename BGR>
void CombinedSolver<GR, BGR>::outputHeader() const
{
  if (_dualDescentMode)
    Parent1::outputHeader();
  else
    Parent2::outputHeader();
}

template<typename GR, typename BGR>
void CombinedSolver<GR, BGR>::init()
{
  Parent1::init();
  Parent2::init();
  _switches = 0;
  _dualDescentMode = false;

  if (g_verbosity >= VERBOSE_ESSENTIAL)
  {
    std::cout << "Initialized combined solver with maxSwitches = "
      << _maxSwitches << std::endl;
  }

  _allDualIndices = _dualIndices;
}

template<typename GR, typename BGR>
void CombinedSolver<GR, BGR>::evaluate()
{
  if (_dualDescentMode)
    Parent1::evaluate();
  else
    Parent2::evaluate();
}

template<typename GR, typename BGR>
void CombinedSolver<GR, BGR>::update()
{
  if (_dualDescentMode)
  {
    Parent1::update();
    _descentSteps++;
  }
  else
  {
    Parent2::update();
    _subgradientSteps++;
  }
}

template<typename GR, typename BGR>
typename CombinedSolver<GR, BGR>::SolverResult CombinedSolver<GR, BGR>::performStep()
{
  evaluate();

  // check whether we should switch mode
  if (_dualDescentMode && _descentSteps == _maxDescentSteps)
  {
    _switches++;
    
    // switch to subgradient if dual descent has converged
    _dualDescentMode = false;
    _descentSteps = 0;
    _dualIndices = _allDualIndices;

    // reset subgradient parameters
    _nIdleIterations = _nNonIdleIterations = 0;
    setMu(_initMu);

    if (g_verbosity >= VERBOSE_NON_ESSENTIAL &&
        g_verbosity != VERBOSE_DEBUG &&
        _switches != _maxSubgradientSteps)
    {
      std::cout << std::endl;
      Parent2::outputHeader();
    }
  }
  else if (!_dualDescentMode && 
    ((_maxSubgradientSteps == -1 && _dualIndices.size() == 0) || 
     (_subgradientSteps == _maxSubgradientSteps || _dualIndices.size() == 0)))
  {
    // switch to dual descent method if subgradient 
    // method has found something better
    _dualDescentMode = true;
    _dualIndices = _allDualIndices;
    _subgradientSteps = 0;
    _dual = _bestDual;

    if (g_verbosity >= VERBOSE_NON_ESSENTIAL && g_verbosity != VERBOSE_DEBUG)
    {
      std::cout << std::endl;
      Parent1::outputHeader();
    }
  }
  else
  {
    update();
  }

  if (getLowerBound() == getUpperBound())
    return SOLVED;
  else if (_subgradientIndices.size() == 0 && _pLR->onlyEqualityConstraintsDualized())
    return SOLVED;
  else if (_switches == _maxSwitches)
    return UNSOLVED_MAX_IT;
  else
    return CONTINUE;
}

template<typename GR, typename BGR>
typename CombinedSolver<GR, BGR>::SolverResult 
  CombinedSolver<GR, BGR>::solve()
{
  assert(_dual.size() == _pLR->getDualDim());
  lemon::Timer timer;
  bool timeLimitHit = false;

  if (g_verbosity >= VERBOSE_NON_ESSENTIAL && g_verbosity != VERBOSE_DEBUG)
    outputHeader();

  for (int i = 0; i < _nMaxIterations || _nMaxIterations == -1; i++)
  {
    SolverResult res = performStep();
    if (res != CONTINUE)
      return res;

    if (_timeLimit != -1 && timer.realTime() > _timeLimit)
    {
      timeLimitHit = true;
      break;
    }
  }

  if (timeLimitHit)
  {
    // do/finish final descent steps, no we don't, time's up = time's up
    //_dualDescentMode = true;
    //_switches = 0;
    //for (; _descentSteps < _maxDescentSteps; _descentSteps++)
    //{
    //  if (performStep() == SOLVED)
    //    return SOLVED;
    //}
    return UNSOLVED_TIME_LIMIT;
  }
  else
    return UNSOLVED_MAX_IT;
}

} // namespace gna
} // namespace nina

#endif /* COMBINEDSOLVER_H_ */
