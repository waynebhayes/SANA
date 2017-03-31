/*
 * scoremodelcoexpression.h
 *
 *  Created on: 30-mar-2012
 *     Authors: M. El-Kebir and M.E. van der Wees
 */

#ifndef SCOREMODELCOEXPRESSIONDISCRETE_H
#define SCOREMODELCOEXPRESSIONDISCRETE_H

#include <assert.h>
#include <string>
#include <lemon/core.h>
#include "score/scoremodel.h"
#include "input/matchinggraph.h"
#include "verbose.h"

namespace nina {
namespace gna {

template<typename GR, typename BGR>
class ScoreModelCoexpressionDiscrete : public ScoreModel<GR, BGR>
{
public:
  /// The graph type of the input graphs
  typedef GR Graph;
  /// The graph type of the bipartite matching graph
  typedef BGR BpGraph;
  /// Base class type
  typedef ScoreModel<GR, BGR> Parent;

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

  double _corrThreshold;

public:
  /// Constructor
  ScoreModelCoexpressionDiscrete(const MatchingGraph<Graph, BpGraph>& matchingGraph, double beta, double corrThreshold);
  /// Destructor
  virtual ~ScoreModelCoexpressionDiscrete() {}

  std::string getScoreFunction() const { return "Discrete"; }

  /// Return weight of edge in G_m
  double getWeightGm(BpEdge ik) const
  {
    // Min-max normalisation
    double currentScore = std::min(_scoreUpperbound, _matchingGraph.getWeightGm(ik));

    if (_maxScore == _minScore)
      return 0;
    else
      return _beta * (currentScore - _minScore) / (_maxScore - _minScore);
  }

  /// Return weight of pair of edges (ij, kl) in G_1 and G_2 resp
  double getWeightG1G2(Edge ij, Edge kl) const
  {
    double weight1 = _matchingGraph.getWeightG1(ij);
    double weight2 = _matchingGraph.getWeightG2(kl);

    if (fabs(weight1) < _corrThreshold || fabs(weight2) < _corrThreshold)
      return 0;

    // No penalty for opposite signs in coexpression values (CHANGED MEK)
    else if ((weight1 <= -_corrThreshold && weight2 >= _corrThreshold) || (weight2 <= -_corrThreshold && weight1 >= _corrThreshold))
      //return -1;
      return 0;

    // Profit for equal signs in coexpression values
    else
      return 1;
  }

  virtual void init();
};

template<typename GR, typename BGR>
inline ScoreModelCoexpressionDiscrete<GR, BGR>::ScoreModelCoexpressionDiscrete(const MatchingGraph<Graph, BpGraph>& matchingGraph, double beta, double corrThreshold)
  : Parent(matchingGraph, beta, false, true)
  , _corrThreshold(corrThreshold)
{
  _scoreUpperbound = 5000.;
}

template<typename GR, typename BGR>
inline void ScoreModelCoexpressionDiscrete<GR, BGR>::init()
{
  Parent::init();

  // let's set beta correctly:
  _beta = 1.0 / (1.0 + std::min(_matchingGraph.getNodeCountG1(), _matchingGraph.getNodeCountG2()));

  if (g_verbosity >= VERBOSE_ESSENTIAL)
  {
    std::cout << "Breaking ties using beta = " << _beta << std::endl;
  }
}

} // namespace gna
} // namespace nina

#endif // SCOREMODELCOEXPRESSIONDISCRETE_H
