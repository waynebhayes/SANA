/*
 * globalproblembase.h
 *
 *  Created on: 8-mar-2013
 *      Author: M. El-Kebir
 */

#ifndef GLOBALPROBLEMBASE_H
#define GLOBALPROBLEMBASE_H

#include <lemon/core.h>
#include <lemon/bp_matching.h>
#include <lemon/adaptors.h>
#include <vector>
#include "input/matchinggraph.h"
#include "score/scoremodel.h"

namespace nina {
namespace gna {

template<typename GR, typename BGR>
class GlobalProblemBase
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
  typedef lemon::MaxWeightedBipartiteMatching<BpGraph, BpWeightMap> MWBM;
  typedef typename std::vector<size_t> IndexList;
  typedef std::vector<IndexList> RealizedYVector;

public:
  /// Type of a map assigning a boolean to every matching edge
  typedef typename BpGraph::template EdgeMap<bool> BpBoolMap;
  /// Type of a matching map: maps a node to its matching edge
  typedef typename BpGraph::template NodeMap<BpEdge> BpMatchingMap;
  /// Type of the matching graph
  typedef MatchingGraph<Graph, BpGraph> MatchingGraphType;
  /// Type of the score model
  typedef ScoreModel<Graph, BpGraph> ScoreModelType;

protected:
  /// Number of red nodes in matching graph
  int _nRedNodes;
  /// Number of blue nodes in matching graph
  int _nBlueNodes;
  /// Number of edges in matching graph
  int _nEdges;

  /// Constructor
  GlobalProblemBase(int nRedNodes, int nBlueNodes, int nEdges)
    : _nRedNodes(nRedNodes)
    , _nBlueNodes(nBlueNodes)
    , _nEdges(nEdges)
  {
  }

public:
  /// Virtual destructor
  virtual ~GlobalProblemBase() {}

  /// Reset
  virtual void reset() = 0;

  /// Init
  virtual void init() = 0;

  /// Get realized y-vars corresponding to idxX
  virtual IndexList& getRealizedY(size_t idxX) = 0;

  /// Get realized y-vars corresponding to idxX
  virtual const IndexList& getRealizedY(size_t idxX) const = 0;

  /// Get scaled edge profit
  virtual double getScaledProfit(BpEdge e) const = 0;

  /// Get edge profit
  virtual double getProfit(BpEdge e) const = 0;

  /// Set edge profit
  virtual void setProfit(BpEdge e, double p) = 0;

  /// Increment edge profit by p
  virtual void incrProfit(BpEdge e, double p) = 0;

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

  /// Solve
  virtual void solve() = 0;

  /// Scale profits
  virtual void scale() = 0;

  /// Get matching edge
  virtual BpEdge matching(BpNode u) const = 0;

  /// Clear matching edge
  virtual void clearMatching(BpEdge e) = 0;

  /// Potential
  virtual double getPot(BpNode n) const = 0;
};

} // namespace gna
} // namespace nina

#endif // GLOBALPROBLEMBASE_H
