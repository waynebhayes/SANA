/*
 * globalproblemconstrained.h
 *
 *  Created on: 8-mar-2013
 *      Author: M. El-Kebir
 */

#ifndef GLOBALPROBLEMCONSTRAINED_H
#define GLOBALPROBLEMCONSTRAINED_H

#include <lemon/core.h>
#include <lemon/bp_matching.h>
#include <lemon/adaptors.h>
#include <vector>
#include <set>
#include "input/matchinggraph.h"
#include "score/scoremodel.h"
#include "lagrange/globalproblembase.h"
#include "lagrange/globalproblem.h"

namespace nina {
namespace gna {

template<typename GR, typename BGR>
class GlobalProblemConstrained : public GlobalProblem<GR, BGR>
{
public:
  /// The graph type of the input graphs
  typedef GR Graph;
  /// The graph type of the bipartite matching graph
  typedef BGR BpGraph;
  /// Parent
  typedef GlobalProblem<GR, BGR> Parent;

  using Parent::_nBlueNodes;
  using Parent::_nRedNodes;
  using Parent::_nEdges;
  using Parent::_gm;
  using Parent::_profit;
  using Parent::_scaledProfit;
  using Parent::_realizedY;
  using Parent::_potMap;
  using Parent::_matchingMap;

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
  /// Type of the set of the fixed matching edges
  typedef typename std::set<BpEdge> BpEdgeSet;
  /// Iterator type of the set of the fixed matching edges
  typedef typename BpEdgeSet::const_iterator BpEdgeSetIt;
  /// Type of node set
  typedef typename std::set<BpNode> BpNodeSet;

private:
  /// Fixed matching edges
  const BpEdgeSet _fixedEdges;
  /// Fixed matching nodes
  BpNodeSet _fixedNodes;
  /// Matching graph copied
  BpGraph _gmCpy;
  /// Mapping of edges from orig to cpy
  BpEdgeMap _mapEdgeToGmCpy;
  /// Mapping of edges from cpy to org
  BpEdgeMap _mapCpyEdgeToGm;
  /// Mapping of nodes from orig to cpy
  BpNodeMap _mapNodeToGmCpy;
  /// Global profits
  BpWeightMap _profitCpy;
  /// Global scaled profits
  BpWeightMap _scaledProfitCpy;

  /// Potential node map used for global MWBM
  typename MWBM::PotMap _potMapCpy;
  /// Matching map used for global MWBM
  typename MWBM::MatchingMap _matchingMapCpy;
  /// Distance map used for global MWBM
  typename MWBM::DistMap _distMapCpy;
  /// Predecessor map used for global MWBM
  typename MWBM::PredMap _predMapCpy;
  /// Heap cross reference used for global MWBM
  typename MWBM::HeapCrossRef _heapCrossRefCpy;
  /// Heap used for global MWBM
  typename MWBM::Heap _heapCpy;
  /// Global MWBM algorithm
  MWBM _gMWBM;

public:
  /// Constructor
  GlobalProblemConstrained(const MatchingGraphType& matchingGraph,
                           const ScoreModelType& scoreModel,
                           const BpEdgeSet& fixedEdges)
    : Parent(matchingGraph, scoreModel)
    , _fixedEdges(fixedEdges)
    , _fixedNodes()
    , _gmCpy()
    , _mapEdgeToGmCpy(_gm, lemon::INVALID)
    , _mapCpyEdgeToGm(_gmCpy, lemon::INVALID)
    , _mapNodeToGmCpy(_gm, lemon::INVALID)
    , _profitCpy(_gmCpy)
    , _scaledProfitCpy(_gmCpy)
    , _potMapCpy(_gmCpy)
    , _matchingMapCpy(_gmCpy)
    , _distMapCpy(_gmCpy)
    , _predMapCpy(_gmCpy)
    , _heapCrossRefCpy(_gmCpy)
    , _heapCpy(_heapCrossRefCpy)
    , _gMWBM(_gmCpy, _scaledProfitCpy)
  {
    // 1. Identify nodes incident to edges in fixedEdges
    for (BpEdgeSetIt edgeIt = fixedEdges.begin();
         edgeIt != fixedEdges.end(); edgeIt++)
    {
      _fixedNodes.insert(_gm.redNode(*edgeIt));
      _fixedNodes.insert(_gm.blueNode(*edgeIt));
    }

    // 2a. Copy nodes that are not in fixedNodes from _gm to _gmCpy
    for (BpRedNodeIt x(_gm); x != lemon::INVALID; ++x)
    {
      if (_fixedNodes.find(x) == _fixedNodes.end())
      {
        BpRedNode cpyX = _gmCpy.addRedNode();
        _mapNodeToGmCpy[x] = cpyX;
      }
    }

    for (BpBlueNodeIt y(_gm); y != lemon::INVALID; ++y)
    {
      if (_fixedNodes.find(y) == _fixedNodes.end())
      {
        BpBlueNode cpyY = _gmCpy.addBlueNode();
        _mapNodeToGmCpy[y] = cpyY;
      }
    }

    // 2b. Copy edges that are not in fixedEdges from _gm to _gmCpy
    for (BpEdgeIt e(_gm); e != lemon::INVALID; ++e)
    {
      if (_fixedEdges.find(e) == _fixedEdges.end())
      {
        BpNode x = _gm.redNode(e);
        BpNode y = _gm.blueNode(e);
        BpNode cpyX = _mapNodeToGmCpy[x];
        BpNode cpyY = _mapNodeToGmCpy[y];

        if (cpyX != lemon::INVALID && cpyY != lemon::INVALID)
        {
          BpEdge cpyE = _gmCpy.addEdge(cpyX, cpyY);
          _mapEdgeToGmCpy[e] = cpyE;
          _mapCpyEdgeToGm[cpyE] = e;
          _profitCpy[cpyE] = scoreModel.getWeightGm(e);
        }
      }
    }

    // 3. Initialize maps
    lemon::mapFill(_gmCpy, _scaledProfitCpy, 0);
    lemon::mapFill(_gmCpy, _potMapCpy, 0);
    lemon::mapFill(_gmCpy, _matchingMapCpy, lemon::INVALID);
    lemon::mapFill(_gmCpy, _distMapCpy, std::numeric_limits<double>::max());
    lemon::mapFill(_gmCpy, _predMapCpy, lemon::INVALID);
    lemon::mapFill(_gmCpy, _heapCrossRefCpy, MWBM::Heap::PRE_HEAP);

    _gMWBM
      .potMap(_potMapCpy)
      .matchingMap(_matchingMapCpy)
      .distMap(_distMapCpy)
      .predMap(_predMapCpy)
      .heap(_heapCpy, _heapCrossRefCpy);
  }

  virtual ~GlobalProblemConstrained() {}

  /// Set edge profit
  virtual void setProfit(BpEdge e, double p)
  {
    _profit.set(e, p);

    BpEdge cpyE = _mapEdgeToGmCpy[e];
    if (cpyE != lemon::INVALID)
    {
      _profitCpy.set(cpyE, p);
    }
  }

  /// Increment edge profit by p
  virtual void incrProfit(BpEdge e, double p)
  {
    _profit[e] += p;

    BpEdge cpyE = _mapEdgeToGmCpy[e];
    if (cpyE != lemon::INVALID)
    {
      _profitCpy[cpyE] += p;
    }
  }

  /// Solve
  virtual void solve()
  {
    _gMWBM.run();

    // add fixed edges to the matching
    for (BpEdgeSetIt edgeIt = _fixedEdges.begin();
         edgeIt != _fixedEdges.end(); edgeIt++)
    {
      _matchingMap[_gm.redNode(*edgeIt)] = *edgeIt;
      _matchingMap[_gm.blueNode(*edgeIt)] = *edgeIt;
    }

    // copy over the result
    for (BpNodeIt v(_gm); v != lemon::INVALID; ++v)
    {
      BpNode cpyV = _mapNodeToGmCpy[v];
      if (cpyV != lemon::INVALID)
      {
        BpEdge cpyE = _matchingMapCpy[cpyV];
        if (cpyE != lemon::INVALID)
        {
          BpEdge e = _mapCpyEdgeToGm[cpyE];
          _matchingMap[v] = e;
          _potMap[v] = _potMapCpy[cpyV];
        }
      }
      else
      {
        if (_gm.red(v) && _matchingMap[v] != lemon::INVALID)
        {
          _potMap[v] = _scaledProfit[_matchingMap[v]];
        }
        if (_gm.blue(v) && _matchingMap[v] != lemon::INVALID)
        {
          _potMap[v] = 0;
        }
      }
    }
  }

  /// Scale profits
  virtual void scale()
  {
    Parent::scale();

    // copy scaled profits
    for (BpEdgeIt e(_gm); e != lemon::INVALID; ++e)
    {
      BpEdge cpyE = _mapEdgeToGmCpy[e];
      if (cpyE != lemon::INVALID)
      {
        _scaledProfitCpy[cpyE] = _scaledProfit[e];
      }
    }
  }

  /// Clear matching edge
  virtual void clearMatching(BpEdge e)
  {
    Parent::clearMatching(e);

    BpEdge cpyE = _mapEdgeToGmCpy[e];
    if (cpyE != lemon::INVALID)
    {
      _matchingMapCpy.set(_gmCpy.u(cpyE), lemon::INVALID);
      _matchingMapCpy.set(_gmCpy.v(cpyE), lemon::INVALID);
    }
  }
};

} // namespace gna
} // namespace nina

#endif // GLOBALPROBLEMCONSTRAINED_H
