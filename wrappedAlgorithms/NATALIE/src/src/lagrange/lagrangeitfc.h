/* 
 * lagrangeitfc.h
 *
 *  Created on: 13-feb-2011
 *      Author: M. El-Kebir
 */

#ifndef LAGRANGEITFC_H_
#define LAGRANGEITFC_H_

#include <vector>
#include <list>

namespace nina {

/// Abstract base class for all Lagrangian problems
///
/// \tparam PVT is the type of the primal value
/// \tparam PT is the type of the primal variables
/// \tparam DT is the type of the dual variables
/// \tparam ST is the type of the subgradients
template<typename PVT, typename PT, typename DT, typename ST>
class LagrangeItfc
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
  /// Type of dual variables vector
  typedef std::vector<DualType> DualVector;
  /// Type of primal variables vector
  typedef std::vector<PrimalType> PrimalVector;
  /// Type of vector of primal variables vector
  typedef std::vector<PrimalVector> PrimalMatrix;
  /// Type of subgradients vector
  typedef std::vector<SubgradientType> SubgradientVector;
  /// Const iterator type of primal variables vector type
  typedef typename PrimalVector::const_iterator PrimalConstIt;
  /// Iterator type of primal variables vector type
  typedef typename PrimalVector::iterator PrimalIt;
  /// Const iterator type of dual variables vector type
  typedef typename DualVector::const_iterator DualConstIt;
  /// Iterator type of dual variables vector type
  typedef typename DualVector::iterator DualIt;
  /// Const iterator type of subgradients vector type
  typedef typename SubgradientVector::const_iterator SubgradientConstIt;
  /// Iterator type of subgradients vector type
  typedef typename SubgradientVector::iterator SubgradientIt;
  /// Type of list of indices
  typedef typename std::vector<size_t> IndexList;
  /// Iterator type of indices list
  typedef typename IndexList::iterator IndexListIt;
  /// Const iterator type of indices list
  typedef typename IndexList::const_iterator IndexListConstIt;

  /// \brief Evaluate the primal problem
  ///
  /// Evaluate the primal problem given a vector of dual variables and
  /// return the sum of the squared subgradients.
  ///
  /// \param dual is the vector of dual variables
  /// \param dualIndices is contains the dual indices which
  /// have been updated by the solver
  /// \param subgradientIndices contains the subgradient indices 
  /// whose value is nonzero. If a subgradient is zero, its corresponding 
  /// multiplier is not changed.
  virtual SubgradientType evaluate(const DualVector& dual, 
                                   const IndexList& dualIndices,
                                   IndexList& subgradientIndices) = 0;
  /// \brief Evaluate the i-th subgradient
  ///
  /// \param i is the subgradient index
  virtual SubgradientType getSubgradient(size_t i) const = 0;
  /// \brief Return the number of primal variables
  virtual size_t getPrimalDim() const = 0;
  /// \brief Return the number of dual variables
  virtual size_t getDualDim() const = 0;
  /// \brief Return all primal variables
  virtual PrimalVector getPrimalVector() const = 0;
  /// \brief Set the primal variable vector
  ///
  /// \param primal is the new primal variable vector
  virtual void setPrimalVector(const PrimalVector& primal) = 0;
  /// \brief Return the value of the primal problem
  virtual PrimalValueType getPrimalValue() const = 0;
  /// \brief Return the value of the primal problem after making it feasible
  virtual PrimalValueType getFeasiblePrimalValue() const = 0;
  /// \brief Return lower bound on the optimal solution
  virtual PrimalValueType getLowerBound() const = 0;
  /// \brief Return upper bound on the optimal solution
  virtual PrimalValueType getUpperBound() const = 0;
  /// \brief Return whether the primal problem is a maximization problem
  virtual bool isMaximizationProblem() const = 0;
  /// \brief Return whether feasible solutions are integral
  virtual bool isIntegral() const = 0;
  /// \brief Return lower bound for multiplier i
  virtual DualType getDualVarLowerBound(size_t i) const = 0;
  /// \brief Return upper bound for multiplier i
  virtual DualType getDualVarUpperBound(size_t i) const = 0;
  /// \brief Reset the problem
  virtual void reset() = 0;
  /// \brief Initialize
  virtual void init() = 0;
  /// \brief Indicates whether only equality constraint are relaxed
  virtual bool onlyEqualityConstraintsDualized() const = 0;
  /// \brief Virtual destructor
  virtual ~LagrangeItfc() {}
};

} // namespace nina

#endif /* LAGRANGEITFC_H_ */
