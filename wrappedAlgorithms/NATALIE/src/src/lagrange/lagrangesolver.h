/* 
 * lagrangesolver.h
 *
 *  Created on: 4-apr-2011
 *      Author: M. El-Kebir
 */

#ifndef LAGRANGESOLVER_H_
#define LAGRANGESOLVER_H_

#include <vector>
#include <algorithm>
#include <limits>
#include <iostream>
#include <iomanip>
#include <lemon/time_measure.h>
#include <assert.h>
#include "lagrange/lagrangeitfc.h"
#include "verbose.h"

namespace nina {

/// Base class for Lagrangian dual problem lagrangesolvers
///
/// Note that for all dual variables the same upper and lower bound is assumed
///
/// \tparam PVT is the type of the primal value
/// \tparam PT is the type of the primal variables
/// \tparam DT is the type of the dual variables
/// \tparam ST is the type of the subgradients
template<typename PVT, typename PT, typename DT, typename ST>
class LagrangeSolver
{
public:
  /// Primal value type
  typedef PVT PrimalValueType;
  /// Primal variable type
  typedef PT PrimalType;
  /// Dual variable type
  typedef DT DualType;
  /// Subgradient type
  typedef ST SubgradientType;
  /// Type of Lagrangian problem
  typedef LagrangeItfc<PrimalValueType, PrimalType, DualType, SubgradientType> LagrangianProblem;
  /// Type of dual variables vector
  typedef typename LagrangianProblem::DualVector DualVector;
  /// Type of primal variables vector
  typedef typename LagrangianProblem::PrimalVector PrimalVector;
  /// Const iterator type of primal variables vector type
  typedef typename LagrangianProblem::PrimalConstIt PrimalVectorIt;
  /// Type of subgradient indices (whose value is nonzero)
  typedef typename LagrangianProblem::IndexList IndexList;
  /// Const iterator type of subgradient indices list
  typedef typename IndexList::const_iterator IndexListConstIt;
  /// Return type of the lagrangesolver
  typedef enum {
    SOLVED,        // done, found optimal solution
    CONTINUE,      // not done, still solving
    UNSOLVED_FIXED_DUAL, // done, but not optimal (no change in duals)
    UNSOLVED_MAX_IT,     // done, but not optimal (max iterations reached)
    UNSOLVED_TIME_LIMIT, // done, but not optimal (time limit reached)
  } SolverResult;
  /// Type of value-primal vector pairs
  typedef std::pair<PrimalValueType, PrimalVector> ValuePrimalPair;
  /// Type of list of value-primal vector pairs
  typedef std::list<ValuePrimalPair> ValuePrimalPairList;
  /// Const iterator type of value-primal vector pairs list
  typedef typename ValuePrimalPairList::const_iterator ValuePrimalPairListIt;
  /// Const iterator type of value-primal vector pairs list
  typedef typename ValuePrimalPairList::iterator ValuePrimalPairListNonConstIt;

protected:
  /// Lagrangian primal problem
  LagrangianProblem* _pLR;
  /// Maximal number of iterations
  const int _nMaxIterations;

private:
  /// Number of solutions to generate
  const int _nSolutions;
  /// Number of lower bounds to store
  const size_t _nLBs;
  /// Number of upper bounds to store
  const size_t _nUBs;

protected:
  /// Time limit
  const int _timeLimit;
  /// Dual variables
  DualVector _dual;
  /// Indices of modified duals
  IndexList _dualIndices;
  /// List with best lower bounds and primal vectors
  ValuePrimalPairList _bestLBs;
  /// List with best (tightest) upper bound found so far
  ValuePrimalPairList _bestUBs;
  /// Dual variable vector corresponding to the best dual value
  DualVector _bestDual;
  /// Number of performed iterations
  int _nIterations;

  /// Initialize member variables
  virtual void init();
  /// Evaluate _dual
  virtual void evaluate() = 0;
  /// Update _dual
  virtual void update() = 0;

  bool arePrimalsEqual(const PrimalVector& primalA, const PrimalVector& primalB) const
  {
    if (primalA.size() != primalB.size())
    {
      return false;
    }
    
    for (PrimalVectorIt itA = primalA.begin(), itB = primalB.begin(); itA != primalA.end(); itA++, itB++)
    {
      if (*itA != *itB)
      {
        return false;
      }
    }

    return true;    
  }

  bool updateLBs(const PrimalValueType curLB, const PrimalVector& primal)
  {
    // Only add when there is an empty slot or when curLB is better than lowest LB in _bestLBs
    if (_bestLBs.size() < _nLBs || curLB > _bestLBs.back().first)
    {
      // Find position for insertion by identifying element in _bestLBs
      // whose value is smaller than or equal to curLB
      ValuePrimalPairListNonConstIt it;
      for (it = _bestLBs.begin(); it != _bestLBs.end() && curLB < it->first; it++);

      // Insert if curLB > it->first or primal vectors are not equal 
      // (works because of lazy evaluation, latter condition is only evaluated when curLB == it->first)
      if (_bestLBs.empty() || curLB > it->first || arePrimalsEqual(primal, it->second))
      {
        // Check whether curLB is the currently best LB
        bool foundBestLB = (it == _bestLBs.begin());

        _bestLBs.insert(it, std::make_pair(curLB, primal));

        if (_bestLBs.size() == _nLBs + 1)
          _bestLBs.pop_back();
        
        return foundBestLB;
      }
    }

    return false;
  }

  bool updateUBs(const PrimalValueType curUB, const PrimalVector& primal)
  {
    // Only add when there is an empty slot or when curUB is better than highest UB in _bestUBs
    if (_bestUBs.size() < _nUBs || curUB < _bestUBs.back().first)
    {
      // Find position for insertion by identifying element in _bestUBs
      // whose value is larger than or equal to curUB
      ValuePrimalPairListNonConstIt it;
      for (it = _bestUBs.begin(); it != _bestUBs.end() && curUB > it->first; it++);
 
      // Insert if curUB < it->first or primal vectors are not equal 
      // (works because of lazy evaluation, latter condition is only evaluated when curUB == it->first)
      if (_bestUBs.empty() || curUB < it->first || !arePrimalsEqual(primal, it->second))
      {
        // Check whether curUB is the currently best UB
        bool foundBestUB = (it == _bestUBs.begin());

        _bestUBs.insert(it, std::make_pair(curUB, _pLR->getPrimalVector()));

        if (_bestUBs.size() == _nUBs + 1)
          _bestUBs.pop_back();
        
        return foundBestUB;
      }
    }

    return false;
  }

public:
  /// Constructor
  ///
  /// \param pLR specifies the Lagrangian problem to be solved
  /// \param timeLimit specifies a time limit in seconds, 
  /// when set to -1 no time limit is put in place.
  /// \param nMaxIterations specifies the maximal number of iterations
  /// the lagrangesolver is allowed to make
  /// \param nSolutions specifies the number of solutions the lagrangesolver has
  /// to generate
  LagrangeSolver(LagrangianProblem* pLR, int timeLimit, int nMaxIterations, int nSolutions);
  /// Virtual destructor
  virtual ~LagrangeSolver()
  {
  };
  /// Initialize the subgradient lagrangesolver
  ///
  /// \param dual is the dual vector to use initially
  ///
  /// \pre dual.size() == this->_pLR->getDualDim()
  void init(const DualVector& dual);
  /// Initialize the subgradient lagrangesolver
  ///
  /// \param dualVarValue is the value to which all variables in the dual vector are set
  ///
  /// \pre this->_dualLB <= dualVarValue <= this->_dualUB
  void init(DualType dualVarValue);
  /// Return primal variables corresponding to the best solution found so far
  const PrimalVector& getBestPrimal() const 
  {
    if (_pLR->isMaximizationProblem())
    {
      assert(!_bestLBs.empty());
      return _bestLBs.front().second;
    }
    else
    {
      assert(!_bestUBs.empty());
      return _bestUBs.front().second;
    }
  }
  /// Return primal objective value corresponding to the best solution found so far
  PrimalValueType getBestPrimalValue() const
  {
    if (_pLR->isMaximizationProblem())
    {
      assert(!_bestLBs.empty());
      return _bestLBs.front().first;
    }
    else
    {
      assert(!_bestUBs.empty());
      return _bestUBs.front().first;
    }
  }

  /***********************************************************************************************/
  /// Return list with nSolutions best LB primal pairs (if maximization problem) 
  /// or UB primal pairs (if minimization problem)
  const ValuePrimalPairList& getBestSolutions() const 
  {
    return _pLR->isMaximizationProblem() ? _bestLBs : _bestUBs;
  }

  /***********************************************************************************************/

  /// Return dual variables corresponding to the best solution found so far
  const DualVector& getBestDual() const { return _bestDual; }
  /// Perform a step
  virtual SolverResult performStep() = 0;
  /// Solve the dual problem
  virtual SolverResult solve();
  /// Determine whether there exists a primal solution
  /// with at least (at most) specified objective value
  /// in case of maximization (minimization)
  virtual SolverResult exists(double value, bool& res);
  /// Return currently computed lower bound on the optimal solution
  PrimalValueType getLowerBound() const
  {
    if (!_bestLBs.empty())
      return _bestLBs.front().first;
    else
      // WATCH OUT: not true for integers, but true for reals
      return -1 * std::numeric_limits<PrimalValueType>::max();
  }
  /// Return currently computed upper bound on the optimal solution
  PrimalValueType getUpperBound() const 
  {
    if (!_bestUBs.empty())
      return _bestUBs.front().first;
    else
      return std::numeric_limits<PrimalValueType>::max();
  }
  /// Output header
  virtual void outputHeader() const = 0;
};

template<typename PVT, typename PT, typename DT, typename ST>
LagrangeSolver<PVT, PT, DT, ST>::LagrangeSolver(LagrangianProblem* pLR,
                                int timeLimit,
                                int nMaxIterations,
                                int nSolutions)
  : _pLR(pLR)
  , _nMaxIterations(nMaxIterations)
  , _nSolutions(nSolutions)
  , _nLBs(_pLR->isMaximizationProblem() ? nSolutions : 1)
  , _nUBs(_pLR->isMaximizationProblem() ? 1 : nSolutions)
  , _timeLimit(timeLimit)
  , _dual()
  , _dualIndices()
  , _bestLBs()
  , _bestUBs()
  , _bestDual()
  , _nIterations()
{
  assert(pLR);
}

template<typename PVT, typename PT, typename DT, typename ST>
void LagrangeSolver<PVT, PT, DT, ST>::init()
{
  _nIterations = 0;
}

template<typename PVT, typename PT, typename DT, typename ST>
void LagrangeSolver<PVT, PT, DT, ST>::init(const DualVector& dual)
{
  assert(dual.size() == _pLR->getDualDim());

  _bestDual = _dual = dual;

  const size_t n = _dual.size();
  for (size_t i = 0; i < n; i++)
    _dualIndices.push_back(i);

  init();
}

template<typename PVT, typename PT, typename DT, typename ST>
void LagrangeSolver<PVT, PT, DT, ST>::init(DualType dualVarValue)
{
  _bestDual = _dual = DualVector(_pLR->getDualDim(), dualVarValue);

  const size_t n = _dual.size();
  for (size_t i = 0; i < n; i++)
  {
    assert(_pLR->getDualVarLowerBound(i) <= dualVarValue
           && dualVarValue <= _pLR->getDualVarUpperBound(i));
    _dualIndices.push_back(i);
  }
  
  init();
}

template<typename PVT, typename PT, typename DT, typename ST>
typename LagrangeSolver<PVT, PT, DT, ST>::SolverResult
  LagrangeSolver<PVT, PT, DT, ST>::solve()
{
  assert(_dual.size() == _pLR->getDualDim());
  lemon::Timer timer;

  if (g_verbosity >= VERBOSE_NON_ESSENTIAL && g_verbosity != VERBOSE_DEBUG)
    outputHeader();

  for (int i = 0; i < _nMaxIterations || _nMaxIterations == -1; i++)
  {
    SolverResult res = performStep();
    if (res != CONTINUE)
      return res;

    if (_timeLimit != -1 && timer.realTime() > _timeLimit)
      return UNSOLVED_TIME_LIMIT;
  }

  return UNSOLVED_MAX_IT;
}

template<typename PVT, typename PT, typename DT, typename ST>
typename LagrangeSolver<PVT, PT, DT, ST>::SolverResult
  LagrangeSolver<PVT, PT, DT, ST>::exists(double value, bool& result)
{
  assert(_dual.size() == _pLR->getDualDim());
  lemon::Timer timer;

  if (g_verbosity >= VERBOSE_NON_ESSENTIAL && g_verbosity != VERBOSE_DEBUG)
    outputHeader();

  for (int i = 0; i < _nMaxIterations || _nMaxIterations == -1; i++)
  {
    SolverResult res = performStep();

    if (_pLR->isMaximizationProblem())
    {
      if (this->getLowerBound() >= value)
      {
        result = true;
        return SOLVED;
      }
      if (this->getUpperBound() < value)
      {
        result = false;
        return SOLVED;
      }
    }
    else
    {
      if (this->getUpperBound() <= value)
      {
        result = true;
        return SOLVED;
      }
      if (this->getLowerBound() > value)
      {
        result = false;
        return SOLVED;
      }
    }

    if (res != CONTINUE)
      return res;

    if (_timeLimit != -1 && timer.realTime() > _timeLimit)
      return UNSOLVED_TIME_LIMIT;
  }

  return UNSOLVED_MAX_IT;
}

} // namespace nina

#endif /* LAGRANGESOLVER_H_ */
