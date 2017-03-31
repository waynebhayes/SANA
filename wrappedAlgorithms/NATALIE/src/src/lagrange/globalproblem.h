/*
 * globalproblem.h
 *
 *  Created on: 8-mar-2013
 *      Author: M. El-Kebir
 */

#ifndef LAGRANGEGNAGLOBALPROBLEM_H
#define LAGRANGEGNAGLOBALPROBLEM_H

#include <lemon/core.h>
#include <lemon/bp_matching.h>
#include <lemon/adaptors.h>
#include <vector>
#include "input/matchinggraph.h"
#include "score/scoremodel.h"
#include "lagrange/globalproblembase.h"

namespace nina {
namespace gna {

template<typename GR, typename BGR>
class GlobalProblem : public GlobalProblemBase<GR, BGR>
{
public:
  /// The graph type of the input graphs
  typedef GR Graph;
  /// The graph type of the bipartite matching graph
  typedef BGR BpGraph;
  /// Parent
  typedef GlobalProblemBase<GR, BGR> Parent;

  using Parent::_nBlueNodes;
  using Parent::_nRedNodes;
  using Parent::_nEdges;

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
  typedef typename Parent::IndexList IndexList;
  typedef typename Parent::RealizedYVector RealizedYVector;

public:
  /// Type of a map assigning a boolean to every matching edge
  typedef typename Parent::BpBoolMap BpBoolMap;
  /// Type of a matching map: maps a node to its matching edge
  typedef typename Parent::BpMatchingMap BpMatchingMap;
  /// Type of the matching graph
  typedef typename Parent::MatchingGraphType MatchingGraphType;
  /// Type of the score model
  typedef typename Parent::ScoreModelType ScoreModelType;

protected:
  /// Matching graph
  const BpGraph& _gm;
  /// Global profits
  BpWeightMap _profit;
  /// Global scaled profits
  BpWeightMap _scaledProfit;
  /// A list of y-s that will be realized if x_{ik} is set to 1
  RealizedYVector _realizedY;

  /// Potential node map used for global MWBM
  typename MWBM::PotMap _potMap;
  /// Matching map used for global MWBM
  typename MWBM::MatchingMap _matchingMap;

private:
  /// Distance map used for global MWBM
  typename MWBM::DistMap _distMap;
  /// Predecessor map used for global MWBM
  typename MWBM::PredMap _predMap;
  /// Heap cross reference used for global MWBM
  typename MWBM::HeapCrossRef _heapCrossRef;
  /// Heap used for global MWBM
  typename MWBM::Heap _heap;
  /// Global MWBM algorithm
  MWBM _gMWBM;

public:
  /// Constructor
  GlobalProblem(const MatchingGraphType& matchingGraph,
                const ScoreModelType& scoreModel)
    : Parent(matchingGraph.getNodeCountG1(),
             matchingGraph.getNodeCountG2(),
             matchingGraph.getEdgeCountGm())
    , _gm(matchingGraph.getGm())
    , _profit(_gm, 0)
    , _scaledProfit(_gm, 0)
    , _realizedY()
    , _potMap(_gm, 0)
    , _matchingMap(_gm, lemon::INVALID)
    , _distMap(_gm, std::numeric_limits<double>::max())
    , _predMap(_gm, lemon::INVALID)
    , _heapCrossRef(_gm, MWBM::Heap::PRE_HEAP)
    , _heap(_heapCrossRef)
    , _gMWBM(_gm, _scaledProfit)
  {
    _gMWBM
      .potMap(_potMap)
      .matchingMap(_matchingMap)
      .distMap(_distMap)
      .predMap(_predMap)
      .heap(_heap, _heapCrossRef);

    for (BpEdgeIt ik(matchingGraph.getGm()); ik != lemon::INVALID; ++ik)
    {
      _profit.set(ik, scoreModel.getWeightGm(ik));
    }
  }

  /// Destructor
  virtual ~GlobalProblem() {}

  /// Reset
  virtual void reset()
  {
    _realizedY = RealizedYVector(_nEdges, IndexList());
  }

  /// Init
  virtual void init()
  {
    _realizedY = RealizedYVector(_nEdges, IndexList());
  }

  /// Get realized y-vars corresponding to idxX
  virtual IndexList& getRealizedY(size_t idxX)
  {
    return _realizedY[idxX];
  }

  /// Get realized y-vars corresponding to idxX
  virtual const IndexList& getRealizedY(size_t idxX) const
  {
    return _realizedY[idxX];
  }

  /// Get scaled edge profit
  virtual double getScaledProfit(BpEdge e) const
  {
    return _scaledProfit[e];
  }

  /// Get edge profit
  virtual double getProfit(BpEdge e) const
  {
    return _profit[e];
  }

  /// Set edge profit
  virtual void setProfit(BpEdge e, double p)
  {
    _profit.set(e, p);
  }

  /// Increment edge profit by p
  virtual void incrProfit(BpEdge e, double p)
  {
    _profit[e] += p;
  }

  /// Solve
  virtual void solve()
  {
    _gMWBM.run();
  }

  /// Scale profits
  virtual void scale()
  {
    _gMWBM.scale(_gm, _profit, _scaledProfit);
  }

  /// Get matching edge
  virtual BpEdge matching(BpNode u) const
  {
    return _matchingMap[u];
  }

  /// Clear matching edge
  virtual void clearMatching(BpEdge e)
  {
    _matchingMap.set(_gm.u(e), lemon::INVALID);
    _matchingMap.set(_gm.v(e), lemon::INVALID);
  }

  /// Potential
  virtual double getPot(BpNode n) const
  {
    return _potMap[n];
  }
};

} // namespace gna
} // namespace nina

#endif // LAGRANGEGNAGLOBALPROBLEM_H
