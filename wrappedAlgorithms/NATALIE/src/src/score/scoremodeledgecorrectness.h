/*
 * scoremodeledgecorrectness.h
 *
 *  Created on: 26-dec-2012
 *     Authors: M. El-Kebir
 */

#ifndef SCOREMODELEDGECORRECTNESS_H
#define SCOREMODELEDGECORRECTNESS_H

#include <assert.h>
#include <string>
#include <lemon/core.h>
#include "score/scoremodel.h"
#include "input/matchinggraph.h"
#include "verbose.h"

namespace nina {
namespace gna {

template<typename GR, typename BGR>
class ScoreModelEdgeCorrectness : public ScoreModel<GR, BGR>
{
public:
  /// The graph type of the input graphs
  typedef GR Graph;
  /// The graph type of the bipartite matching graph
  typedef BGR BpGraph;
  /// Base class type
  typedef ScoreModel<GR, BGR> Parent;
  /// Matching graph type
  typedef MatchingGraph<Graph, BpGraph> MatchingGraphType;

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

  /// Weights on original edges
  typedef typename Parent::WeightEdgeMap WeightEdgeMap;
  /// Weights on matching edges
  typedef typename Parent::BpWeightEdgeMap BpWeightEdgeMap;

  /// Matching graph
  using Parent::_matchingGraph;
  /// Scaling
  using Parent::_beta;
  /// Max score used for normalisation
  using Parent::_maxScore;
  /// Min score used for normalisation
  using Parent::_minScore;
  /// Upperbound used for clamping
  using Parent::_scoreUpperbound;

public:
  /// Constructor
  ScoreModelEdgeCorrectness(const MatchingGraphType& matchingGraph);
  /// Destructor
  virtual ~ScoreModelEdgeCorrectness();

  /// Return weight of edge in G_m
  double getWeightGm(BpEdge ik) const
  {
    double currentScore = std::min(_scoreUpperbound, _matchingGraph.getWeightGm(ik));

    if (_maxScore == _minScore)
      return 0;
    else
      return _beta * (currentScore - _minScore) / (_maxScore - _minScore);
  }

  /// Return weight of pair of edges (ij, kl) in G_1 and G_2 resp
  double getWeightG1G2(Edge ij, Edge kl) const
  {
    return 1.0 / _numberOfEdges;
  }

  std::string getScoreFunction() const { return "Edge correctness"; }

  virtual void init();

private:
  double _numberOfEdges;
};

template<typename GR, typename BGR>
inline ScoreModelEdgeCorrectness<GR, BGR>::ScoreModelEdgeCorrectness(const MatchingGraphType& matchingGraph)
  : Parent(matchingGraph, 1, false, false)
  , _numberOfEdges(0)
{
  _scoreUpperbound = 5000.;
}

template<typename GR, typename BGR>
inline ScoreModelEdgeCorrectness<GR, BGR>::~ScoreModelEdgeCorrectness()
{
}

template<typename GR, typename BGR>
inline void ScoreModelEdgeCorrectness<GR, BGR>::init()
{
  Parent::init();

  _numberOfEdges = static_cast<double>(std::min(_matchingGraph.getEdgeCountG1(), _matchingGraph.getEdgeCountG2()));

  // let's set beta correctly:
  _beta = 1.0 / _numberOfEdges *
      1.0 / (1.0 + std::min(_matchingGraph.getNodeCountG1(), _matchingGraph.getNodeCountG2()));

  if (g_verbosity >= VERBOSE_ESSENTIAL)
  {
    std::cout << "Computing edge correctness by normalizing with " << _numberOfEdges << std::endl;
    std::cout << "Breaking ties using beta = " << _beta << std::endl;
  }
}

} // namespace gna
} // namespace nina

#endif // SCOREMODELEDGECORRECTNESS_H
