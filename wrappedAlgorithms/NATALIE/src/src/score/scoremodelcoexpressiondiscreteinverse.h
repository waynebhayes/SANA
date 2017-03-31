/*
 * scoremodelcoexpression.h
 *
 *  Created on: 30-mar-2012
 *     Authors: M. El-Kebir and M.E. van der Wees
 */

#ifndef SCOREMODELCOEXPRESSIONDISCRETEINVERSE_H
#define SCOREMODELCOEXPRESSIONDISCRETEINVERSE_H

#include <assert.h>
#include <string>
#include <lemon/core.h>
#include "score/scoremodel.h"
#include "input/matchinggraph.h"
#include "verbose.h"

namespace nina {
namespace gna {

template<typename GR, typename BGR>
class ScoreModelCoexpressionDiscreteInverse : public ScoreModel<GR, BGR>
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
  /// Max score used for normalisation
  using Parent::_maxScore;
  /// Min score used for normalisation
  using Parent::_minScore;

  double _corrThreshold;

public:
  /// Constructor
  ScoreModelCoexpressionDiscreteInverse(const MatchingGraph<Graph, BpGraph>& matchingGraph, double beta, double corrThreshold);
  /// Destructor
  virtual ~ScoreModelCoexpressionDiscreteInverse() {}

  std::string getScoreFunction() const { return "Discrete Inverse"; }

  /// Return weight of edge in G_m
  double getWeightGm(BpEdge ik) const
  {
    // Min-max normalisation
    return 0;
  }

  /// Return weight of pair of edges (ij, kl) in G_1 and G_2 resp
  double getWeightG1G2(Edge ij, Edge kl) const
  {
    double weight1 = _matchingGraph.getWeightG1(ij);
    double weight2 = _matchingGraph.getWeightG2(kl);

    if (fabs(weight1) < _corrThreshold || fabs(weight2) < _corrThreshold)
      return 0;

    // Profit for opposite signs in coexpression values (CHANGED MEK)
    else if ((weight1 <= -_corrThreshold && weight2 >= _corrThreshold) || (weight2 <= -_corrThreshold && weight1 >= _corrThreshold))
      //return -1;
      return 1;

    // No penalty for equal signs in coexpression values
    else
      return 0;
  }
};

template<typename GR, typename BGR>
inline ScoreModelCoexpressionDiscreteInverse<GR, BGR>::ScoreModelCoexpressionDiscreteInverse(const MatchingGraph<Graph, BpGraph>& matchingGraph, double beta, double corrThreshold)
  : Parent(matchingGraph, beta, false, true)
  , _corrThreshold(corrThreshold)
{
}

} // namespace gna
} // namespace nina

#endif // SCOREMODELCOEXPRESSIONDISCRETEINVERSE_H
