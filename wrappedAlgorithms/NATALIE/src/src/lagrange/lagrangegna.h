/*
 * lagrangegna.h
 *
 *  Created on: 9-feb-2011
 *      Author: M. El-Kebir
 */

#ifndef LAGRANGEGNA_H_
#define LAGRANGEGNA_H_

#include <lemon/core.h>
#include <lemon/bp_matching.h>
#include <lemon/adaptors.h>
#include <limits>
#include <ostream>
#include "input/matchinggraph.h"
#include "score/scoremodel.h"
#include "lagrange/lagrangeitfc.h"
#include "verbose.h"
#include "lagrange/globalproblem.h"
#include "lagrange/globalproblemconstrained.h"

namespace nina {
namespace gna {

template<typename GR, typename BGR>
class LagrangeGna : public LagrangeItfc<double, bool, double, int>
{
public:
  /// The graph type of the input graphs
  typedef GR Graph;
  /// The graph type of the bipartite matching graph
  typedef BGR BpGraph;

protected:
  TEMPLATE_GRAPH_TYPEDEFS(Graph);

  typedef typename BpGraph::Node BpNode;
  typedef typename BpGraph::Edge BpEdge;
  typedef typename BpGraph::NodeIt BpNodeIt;
  typedef typename BpGraph::EdgeIt BpEdgeIt;
  typedef typename BpGraph::IncEdgeIt BpIncEdgeIt;
  typedef typename BpGraph::RedNode BpRedNode;
  typedef typename BpGraph::BlueNode BpBlueNode;
  typedef typename BpGraph::RedNodeIt BpRedNodeIt;
  typedef typename BpGraph::BlueNodeIt BpBlueNodeIt;
  typedef typename BpGraph::template EdgeMap<double> BpWeightMap;
  typedef typename BpGraph::template EdgeMap<size_t> BpIdxMap;
  typedef typename BpGraph::template NodeMap<BpNode> BpNodeMap;
  typedef typename BpGraph::template EdgeMap<BpEdge> BpEdgeMap;

public:
  /// Type of a map assigning a boolean to every matching edge
  typedef typename BpGraph::template EdgeMap<bool> BpBoolMap;
  /// Type of a matching map: maps a node to its matching edge
  typedef typename BpGraph::template NodeMap<BpEdge> BpMatchingMap;
  /// Type of the matching graph
  typedef MatchingGraph<Graph, BpGraph> MatchingGraphType;
  /// Type of the score model
  typedef ScoreModel<Graph, BpGraph> ScoreModelType;
  /// Type of the global problem
  typedef GlobalProblem<Graph, BpGraph> GlobalProblemType;
  /// Type of the base global problem
  typedef GlobalProblemBase<Graph, BpGraph> GlobalProblemBaseType;

protected:
  typedef lemon::MaxWeightedBipartiteMatching<BpGraph, BpWeightMap> MWBM;

  typedef std::vector<size_t> IndexVector;
  typedef typename IndexVector::iterator IndexVectorIt;
  typedef typename IndexVector::const_iterator IndexVectorConstIt;
  typedef std::vector<BpEdge> PrimalDefX; // in _gm
  typedef typename PrimalDefX::const_iterator PrimalDefXIt;

  struct DefY
  {
    BpEdge _jl; // in _redGm
    double _weight;
  };
  typedef std::vector<DefY> PrimalDefY;
  typedef typename PrimalDefY::const_iterator PrimalDefYIt;

  struct LocalProblem
  {
  private:
    /// The relevant matching graph
    BpGraph _redGm;
    /// Number of red nodes in reduced matching graph
    int _nRedNodes;
    /// Number of blue nodes in reduced matching graph
    int _nBlueNodes;
    /// Number of edges in reduced matching graph
    int _nEdges;
    /// Maps nodes from the reduced matching graph to nodes in the original one
    BpNodeMap _red2orgNode;
    /// Maps edges from the reduced matching graph to nodes in the original one
    BpEdgeMap _red2orgEdge;
    /// Potential node map used for local MWBM
    typename MWBM::PotMap* _pPotMap;
    /// Matching map used for local MWBM
    typename MWBM::MatchingMap* _pMatchingMap;
    /// Distance map used for local MWBM
    typename MWBM::DistMap* _pDistMap;
    /// Predecessor map used for local MWBM
    typename MWBM::PredMap* _pPredMap;
    /// Heap cross reference used for local MWBM
    typename MWBM::HeapCrossRef* _pHeapCrossRef;
    /// Heap used for local MWBM
    typename MWBM::Heap* _pHeap;
    /// Local profits
    BpWeightMap* _pProfit;
    /// Local scaled profits
    BpWeightMap* _pScaledProfit;
    /// Local index map: maps a reduced matching edge to its corresponding y_{ikjl} variable
    BpIdxMap* _pIdxMapY;
    /// Local MWBM algorithm
    MWBM* _pMWBM;

  public:
    /// Number of red nodes in the reduced graph
    int getRedNodeCount() const
    {
      return _nRedNodes;
    }
    /// Number of blue nodes in the reduced graph
    int getBlueNodeCount() const
    {
      return _nBlueNodes;
    }
    /// Number of nodes in the reduced graph
    int getNodeCount() const 
    {
      return _nRedNodes + _nBlueNodes;
    }
    /// Number of edges in the reduced graph
    int getEdgeCount() const
    {
      return _nEdges;
    }
    /// Add a red node
    BpNode addRedNode(BpNode org_i) 
    {
      _nRedNodes++;
      BpNode red_i = _redGm.addRedNode();
      _red2orgNode[red_i] = org_i;
      return red_i;
    }
    /// Add a blue node
    BpNode addBlueNode(BpNode org_k)
    {
      _nBlueNodes++;
      BpNode red_k = _redGm.addBlueNode();
      _red2orgNode[red_k] = org_k;
      return red_k;
    }
    /// Add an edge
    BpEdge addEdge(BpEdge org_ik, BpNode red_i, BpNode red_k)
    {
      _nEdges++;
      BpEdge red_ik = _redGm.addEdge(_redGm.asRedNodeUnsafe(red_i), _redGm.asBlueNodeUnsafe(red_k));
      _red2orgEdge[red_ik] = org_ik;
      return red_ik;
    }
    /// Get original node
    BpNode getOrgNode(BpNode u) const
    {
      return _red2orgNode[u];
    }
    /// Get original node
    BpNode getOrgNode(BpEdge e, bool red) const
    {
      return red ? 
        _red2orgNode[_redGm.redNode(e)] : _red2orgNode[_redGm.blueNode(e)];
    }
    /// Get original edge
    BpEdge getOrgEdge(BpEdge e) const
    {
      return _red2orgEdge[e];
    }
    /// Get reduced matching graph
    const BpGraph& getRedGm() const
    {
      return _redGm;
    }
    /// Get matching edge
    BpEdge matching(BpNode u) const
    {
      return (*_pMatchingMap)[u];
    }
    /// Clear matching edge
    void clearMatching(BpEdge e)
    {
      _pMatchingMap->set(_redGm.u(e), lemon::INVALID);
      _pMatchingMap->set(_redGm.v(e), lemon::INVALID);
    }
    /// Clear edge profit
    void clearProfit(BpEdge e)
    {
      _pProfit->set(e, 0);
      _pScaledProfit->set(e, 0);
    }
    /// Get edge profit
    double getProfit(BpEdge e) const
    {
      return (*_pProfit)[e];
    }
    /// Set edge profit
    void setProfit(BpEdge e, double p)
    {
      _pProfit->set(e, p);
    }
    double getScaledProfit(BpEdge e) const
    {
      return (*_pScaledProfit)[e];
    }
    /// Set scaled edge profit
    void setScaledProfit(BpEdge e, double S, double one_over_S)
    {
      _pScaledProfit->set(e, MWBM::scale(getProfit(e), S, one_over_S));
    }
    /// Reduced edge profit such that slack becomes 0
    void reduceScaledProfit(BpEdge e)
    {
      _pScaledProfit->set(e, getPot(_redGm.u(e)) + getPot((_redGm.v(e))));
    }
    /// Get y_{ikjl} variable index corresponding to reduced matching edge
    size_t getIdxY(BpEdge e) const
    {
      return (*_pIdxMapY)[e];
    }
    /// Get y_{ikjl} variable index corresponding to reduced matching edge
    void setIdxY(BpEdge e, size_t idxY)
    {
      _pIdxMapY->set(e, idxY);
    }
    /// Solve
    void solve()
    {
      _pMWBM->run();
    }
    /// Scale profits
    void scale()
    {
      _pMWBM->scale(_redGm, *_pProfit, *_pScaledProfit);
    }
    /// Potential
    double getPot(BpNode n) const
    {
      return (*_pPotMap)[n];
    }

    LocalProblem()
      : _redGm()
      , _nRedNodes(0)
      , _nBlueNodes(0)
      , _nEdges(0)
      , _red2orgNode(_redGm, lemon::INVALID)
      , _red2orgEdge(_redGm, lemon::INVALID)
      , _pPotMap(NULL)
      , _pMatchingMap(NULL)
      , _pDistMap(NULL)
      , _pPredMap(NULL)
      , _pHeapCrossRef(NULL)
      , _pHeap(NULL)
      , _pProfit(NULL)
      , _pScaledProfit(NULL)
      , _pIdxMapY(NULL)
      , _pMWBM(NULL)
    {
    }

    ~LocalProblem()
    {
      delete _pPotMap;
      delete _pMatchingMap;
      delete _pDistMap;
      delete _pPredMap;
      delete _pHeapCrossRef;
      delete _pHeap;
      delete _pProfit;
      delete _pScaledProfit;
      delete _pMWBM;
      delete _pIdxMapY;
    }

    void reset()
    {
      _redGm.clear();
      _nRedNodes = _nBlueNodes = _nEdges = 0;

      delete _pPotMap;
      delete _pMatchingMap;
      delete _pDistMap;
      delete _pPredMap;
      delete _pHeapCrossRef;
      delete _pHeap;
      delete _pProfit;
      delete _pScaledProfit;
      delete _pMWBM;
      delete _pIdxMapY;
    }

    void createIdxMapY()
    {
      _pIdxMapY = new BpIdxMap(_redGm, std::numeric_limits<size_t>::max());
    }

    void init()
    {
      assert(!_pPotMap && !_pMatchingMap && !_pDistMap && !_pPredMap
        && !_pHeapCrossRef && !_pHeap && !_pProfit && !_pScaledProfit && !_pMWBM);

      _pPotMap = new typename MWBM::PotMap(_redGm, 0);
      _pMatchingMap = new typename MWBM::MatchingMap(_redGm, lemon::INVALID);
      _pDistMap = new typename MWBM::DistMap(_redGm, std::numeric_limits<double>::max());
      _pPredMap = new typename MWBM::PredMap(_redGm, lemon::INVALID);
      _pHeapCrossRef = new typename MWBM::HeapCrossRef(_redGm, MWBM::Heap::PRE_HEAP);
      _pHeap = new typename MWBM::Heap(*_pHeapCrossRef);
      _pProfit = new BpWeightMap(_redGm, 0);
      _pScaledProfit = new BpWeightMap(_redGm, 0);
      if (!_pIdxMapY)
        createIdxMapY();

      _pMWBM = new MWBM(_redGm, *_pScaledProfit);
      (*_pMWBM)
        .potMap(*_pPotMap)
        .matchingMap(*_pMatchingMap)
        .distMap(*_pDistMap)
        .predMap(*_pPredMap)
        .heap(*_pHeap, *_pHeapCrossRef);
    }
  };

  /// Matching graph
  const MatchingGraphType& _matchingGraph;
  /// Score model
  const ScoreModelType& _scoreModel;
  /// The actual matching graph
  const BpGraph& _gm;
  /// x_{ik} variable definitions
  PrimalDefX _defX;
  /// y_{ikjl} variable definitions
  PrimalDefY _defY;
  /// x_{ik} variables
  PrimalVector _x;
  /// y_{ikjl} variables
  PrimalVector _y;
  /// Maps an original matching edge to its corresponding x_{ik} variable
  BpIdxMap _idxMapX;
  /// Maps yIdx to its corresponding xIdx (y_{ikjl} -> x_{ik})
  IndexVector _yIdx2xIdx;
  /// Primal value
  double _primalValue;
  /// Feasible primal value
  double _feasiblePrimalValue;
  /// Global problem stuff
  GlobalProblemBaseType* _pGlobal;
  /// Indicates whether feasible solutions are integral
  bool _integral;

protected:
  /// Initialize primal variables
  virtual void initPrimal() = 0;
  /// Initialize dual variable definitions
  virtual void initDual() = 0;
  /// Get reduced edge corresponding to y_{ikjl} with index idxX
  virtual BpEdge getRedEdge(size_t idxX, BpEdge jl) = 0;
  /// Indicates whether it is necessary to solve idxX
  virtual bool hasChangedX(size_t idxX) = 0;
  /// Solve local problem
  virtual void solveLocal(const DualVector& dual, 
                          const size_t idxX,
                          const BpEdge ik,
                          PrimalDefYIt& itY,
                          size_t& idxY) = 0;
  /// Initialize global profit
  virtual void initGlobalProfit(const BpEdge ik,
                                const size_t idxX,
                                const DualVector& dual) = 0;
  /// Transform computed solution into a feasible one
  virtual void computeFeasiblePrimalValue() = 0;
  /// Create local problem data structures(s)
  virtual void createLocal() = 0;
  /// Init local problem(s)
  virtual void initLocal() = 0;
  /// Reset local problem(s)
  virtual void resetLocal() = 0;
  /// Realize y-vars corresponding to given x-var
  virtual void realize(const BpEdge ik) = 0;
  /// Flag a dual variable as changed
  virtual void flag(const DualVector& dual, size_t idxD) = 0;
  /// Unflag a dual variable as changed
  virtual void unflag(size_t idxD) = 0;
  /// Deduce which local problems need solving
  void identifyLocalProblems(const DualVector& dual, const IndexList& dualIndices);
  /// Clean up local problems
  void clearLocalProblems(const IndexList& dualIndices);

public:
  /// Constructor
  LagrangeGna(const MatchingGraphType& matchingGraph,
              const ScoreModelType& scoreModel,
              bool integral);
  /// Destructor
  virtual ~LagrangeGna();
  /// Return the number of primal variables corresponding to the problem instance
  size_t getPrimalDim() const { return _y.size() + _x.size(); }
  /// Return the number of primal x variables
  size_t getPrimalXDim() const { return _x.size(); }
  /// Return the number of primal y variables
  size_t getPrimalYDim() const { return _y.size(); }
  /// Return primal vector
  PrimalVector getPrimalVector() const;
  /// Set primal vector
  ///
  /// \pre primal.size() == this->getPrimalDim()
  void setPrimalVector(const PrimalVector& primal);
  /// Return primal value
  double getPrimalValue() const { return _primalValue; }
  /// Return feasible primal value
  double getFeasiblePrimalValue() const { return _feasiblePrimalValue; }
  /// Return true, as the problem is a maximization problem
  bool isMaximizationProblem() const { return true; }
  /// \brief Return lower bound on the optimal solution
  double getLowerBound() const { return _feasiblePrimalValue; }
  /// \brief Return upper bound on the optimal solution
  double getUpperBound() const
  {
    if (_integral)
    {
      if (ceil(_primalValue) - _primalValue < 1e-4) return ceil(_primalValue);
      else return floor(_primalValue);
    }
    else
    {
      return _primalValue;
    }
  }
  /// \brief Return whether feasible solutions are integral
  bool isIntegral() const { return _integral; }
  /// Reset the problem
  void reset();
  /// Get matching
  void getMatching(BpBoolMap& m) const;
  /// Get matching
  void getMatching(BpMatchingMap& m) const;
  /// Output primal variable definitions
  virtual void printPrimalDef(std::ostream& out) const = 0;
  /// Output dual variable definitions
  virtual void printDualDef(std::ostream& out) const = 0;
  /// Solve the primal problem given a vector of dual multipliers
  /// without compiling a list of nonzero subgradients
  /// 
  /// Used for the non-enhanced dual descent solver
  void evaluate(const DualVector& dual, 
                const IndexList& dualIndices);
  /// Solve the primal problem given a vector of dual multipliers
  ///
  /// Return the sum of squared subgradients 
  int evaluate(const DualVector& dual, 
               const IndexList& dualIndices, 
               IndexList& subgradientIndices);
  /// Get original edge corresponding to x_{ik} with index idxX
  virtual BpEdge getOrgEdge(size_t idxX) const;
  /// Get original edge corresponding to y_{ikjl} with index idxY
  virtual BpEdge getOrgEdge(size_t idxY, bool ik) const = 0;
  /// Get original node corresponding to y_{ikjl} with index idxY
  virtual BpNode getOrgNode(size_t idxY, bool ik, bool red) const;
  /// Get corresponding x variable index of edge (i,k)
  size_t getIdxX(const BpEdge ik) const { return _idxMapX[ik]; }
  /// Return idxX corresponding to idxY
  size_t getIdxX(size_t idxY) const;
  /// Get dual slack corresponding to idxX
  double getDualSlackX(size_t idxX) const;
  /// Return input graphs
  const MatchingGraphType& getMatchingGraph() const
  {
    return _matchingGraph;
  }
  /// Return score model
  const ScoreModelType& getScoreModel() const
  {
    return _scoreModel;
  }
  /// Return w_{ikjl}
  double getWeight(size_t idxY) const
  {
    assert(0 <= idxY && idxY < _defY.size());
    return _defY[idxY]._weight;
  }
  /// Get global problem
  const GlobalProblemBaseType* getGlobalProblem() const
  {
    return _pGlobal;
  }
  /// Set global problem
  void setGlobalProblem(GlobalProblemBaseType* pGlobal)
  {
    delete _pGlobal;
    _pGlobal = pGlobal;
  }
};

template<typename GR, typename BGR>
inline LagrangeGna<GR, BGR>::LagrangeGna(const MatchingGraphType& matchingGraph,
                                         const ScoreModelType& scoreModel,
                                         bool integral)
  : _matchingGraph(matchingGraph)
  , _scoreModel(scoreModel)
  , _gm(matchingGraph.getGm())
  , _defX()
  , _defY()
  , _x()
  , _y()
  , _idxMapX(_gm, std::numeric_limits<size_t>::max())
  , _yIdx2xIdx()
  , _primalValue(0)
  , _feasiblePrimalValue(0)
  , _pGlobal(new GlobalProblemType(matchingGraph, scoreModel))
  , _integral(integral)
{
}

template<typename GR, typename BGR>
inline LagrangeGna<GR, BGR>::~LagrangeGna()
{
  delete _pGlobal;
}

template<typename GR, typename BGR>
inline void LagrangeGna<GR, BGR>::getMatching(BpBoolMap& m) const
{
  size_t idxX = 0;
  for (PrimalConstIt itX = _x.begin(); itX != _x.end(); itX++, idxX++)
    m[_defX[idxX]] = *itX;
}

template<typename GR, typename BGR>
inline void LagrangeGna<GR, BGR>::getMatching(BpMatchingMap& m) const
{
  lemon::mapFill(_gm, m, lemon::INVALID);

  size_t idxX = 0;
  for (PrimalConstIt itX = _x.begin(); itX != _x.end(); itX++, idxX++)
  {
    if (*itX)
    {
      BpEdge e = _defX[idxX];
      m.set(_gm.redNode(e), e);
      m.set(_gm.blueNode(e), e);
    }
  }
}

template<typename GR, typename BGR>
inline void LagrangeGna<GR, BGR>::reset()
{
  assert(_pGlobal);

  _x = PrimalVector(_x.size(), false);
  _y = PrimalVector(_y.size(), false);
  _pGlobal->reset();

  _primalValue = 0;
  _feasiblePrimalValue = 0;
}

template<typename GR, typename BGR>
inline typename LagrangeGna<GR, BGR>::PrimalVector LagrangeGna<GR, BGR>::getPrimalVector() const
{
  PrimalVector res = _x;
  res.insert(res.end(), _y.begin(), _y.end());
  return res;
}

template<typename GR, typename BGR>
inline void LagrangeGna<GR, BGR>::setPrimalVector(const PrimalVector& primal)
{
  assert(_x.size() + _y.size() == primal.size());

  _x = PrimalVector(primal.begin(), primal.begin() + _x.size());
  _y = PrimalVector(primal.begin() + _x.size(), primal.end());
}

template<typename GR, typename BGR>
inline typename LagrangeGna<GR, BGR>::BpEdge
LagrangeGna<GR, BGR>::getOrgEdge(size_t idxX) const
{
  assert(0 <= idxX && idxX < _defX.size());
  return _defX[idxX];
}

template<typename GR, typename BGR>
inline typename LagrangeGna<GR, BGR>::BpNode
  LagrangeGna<GR, BGR>::getOrgNode(size_t idxY, bool ik, bool red) const
{
  BpEdge e = getOrgEdge(idxY, ik);
  if (red)
  {
    return _gm.redNode(e);
  }
  else
  {
    return _gm.blueNode(e);
  }
}

template<typename GR, typename BGR>
inline double LagrangeGna<GR, BGR>::getDualSlackX(size_t idxX) const
{
  assert(0 <= idxX && idxX < _defX.size());
  assert(_pGlobal);

  const BpEdge ik = _defX[idxX];
  const BpNode i = _gm.redNode(ik);
  const BpNode k = _gm.blueNode(ik);

  //double w = _global._scaledProfit[ik];
  //double w_org = _global._profit[ik];
  //if (w != w_org)
  //  w=w;
  //double pot_i = _global._gMWBM.pot(i);
  //double pot_k = _global._gMWBM.pot(k);
  
  return _pGlobal->getPot(i) + _pGlobal->getPot(k) - _pGlobal->getScaledProfit(ik);
}

template<typename GR, typename BGR>
inline size_t LagrangeGna<GR, BGR>::getIdxX(size_t idxY) const
{
  assert(0 <= idxY && idxY < _defY.size());
  return _yIdx2xIdx[idxY];
}

template<typename GR, typename BGR>
inline void LagrangeGna<GR, BGR>::identifyLocalProblems(const DualVector& dual,
                                                        const IndexList& dualIndices)
{
  for (IndexListConstIt itDualIdx = dualIndices.begin(); 
    itDualIdx != dualIndices.end(); itDualIdx++)
  {
    flag(dual, *itDualIdx);
  }
}

template<typename GR, typename BGR>
inline void LagrangeGna<GR, BGR>::clearLocalProblems(const IndexList& dualIndices)
{
  for (IndexListConstIt itDualIdx = dualIndices.begin(); 
    itDualIdx != dualIndices.end(); itDualIdx++)
  {
    unflag(*itDualIdx);
  }
}

template<typename GR, typename BGR>
inline void LagrangeGna<GR, BGR>::evaluate(const DualVector& dual,
                                           const IndexList& dualIndices)
{
  assert(_pGlobal);

  //if (g_verbosity >= VERBOSE_DEBUG)
  //{
  //  std::cout << IDX << "." << std::endl << "Changed duals: ";
  //  for (IndexListConstIt itD = dualIndices.begin(); itD != dualIndices.end(); itD++)
  //  {
  //    std::cout << *itD << ", ";
  //  }
  //  std::cout << std::endl << "Duals: ";
  //  
  //  size_t idxD = 0;
  //  for (DualConstIt itD = dual.begin(); itD != dual.end(); itD++, idxD++)
  //  {
  //    std::cout << "(" << idxD << ": " << *itD << "), ";
  //  }
  //  std::cout << std::endl;
  //}

  identifyLocalProblems(dual, dualIndices);

  // solve the local problems
  size_t idxY = 0, idxX = 0;
  PrimalDefYIt itY = _defY.begin();
  double max_v = 0;
  for (PrimalDefXIt itX = _defX.begin(); itX != _defX.end(); itX++, idxX++)
  {
    const BpEdge ik = *itX;

    _x[idxX] = false;
    if (hasChangedX(idxX))
    {
      initGlobalProfit(ik, idxX, dual);
      solveLocal(dual, idxX, ik, itY, idxY);
    }
    else
    {
      for (; itY != _defY.end() && _yIdx2xIdx[idxY] == idxX; itY++, idxY++)
      {
        _y[idxY] = false;
      }
    }

    double abs_v = fabs(_pGlobal->getProfit(ik));
    if (abs_v > max_v)
      max_v = abs_v;
  }

  assert(itY == _defY.end() && idxY == _defY.size());

  if (g_verbosity >= VERBOSE_DEBUG)
  {
    std::cout << "Solving global problem..." << std::flush;
  }

  // solve the global problem
  _pGlobal->scale();
  _pGlobal->solve();

  // update primal variables
  _primalValue = 0;
  int count = 0;
  for (BpRedNodeIt r(_gm); r != lemon::INVALID; ++r, count++)
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

template<typename GR, typename BGR>
inline int LagrangeGna<GR, BGR>::evaluate(const DualVector& dual,
                                          const IndexList& dualIndices,
                                          IndexList& subgradientIndices)
{
  evaluate(dual, dualIndices);

  subgradientIndices.clear();
  int sum = 0;
  const size_t n = dual.size();
  for (size_t i = 0; i < n; i++)
  {
    int s = getSubgradient(i);
    if (s != 0)
    {
      sum += s*s;
      subgradientIndices.push_back(i);
    }
  }

  return sum;
}

} // namespace gna
} // namespace nina

#endif /* LAGRANGEGNA_H_ */
