/*
 * scoremodel.h
 *
 *  Created on: 17-jan-2012
 *     Authors: M. El-Kebir and M.E. van der Wees
 */

#ifndef SCOREMODEL_H_
#define SCOREMODEL_H_

#include <assert.h>
#include <string>
#include <limits>
#include <lemon/core.h>
#include "input/matchinggraph.h"
#include "verbose.h"

namespace nina {
namespace gna {

// forward class declaration
template<typename GR, typename BGR>
class MatchingGraph;

template<typename GR, typename BGR>
class ScoreModel
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

  /// Weights on original edges
  typedef typename Graph::template EdgeMap<double> WeightEdgeMap;
  /// Weights on matching edges
  typedef typename BpGraph::template EdgeMap<double> BpWeightEdgeMap;

  /// Matching graph
  const MatchingGraph<Graph, BpGraph>& _matchingGraph;
  /// Scaling
  double _beta;
  /// Indicates whether original edge weights should be discretized (made binary)
  bool _discretizeWeight;
  /// Indicates whether matching edge weights need to be normalized
  bool _normalize;
  /// Upperbound used for clamping
  double _scoreUpperbound;
  /// Max score
  double _maxScore;
  /// Min score
  double _minScore;

public:
  /// Constructor
  ScoreModel(const MatchingGraph<Graph, BpGraph>& matchingGraph, 
             double beta, 
             bool discretizeWeight,
             bool normalize);
  /// Destructor
  virtual ~ScoreModel() {}

  virtual std::string getScoreFunction() const { return "Average"; }

  /// Return weight of edge in G_m
  virtual double getWeightGm(BpEdge ik) const 
  {
    if (_minScore == _maxScore && _minScore == 0)
      return 0;
    else if (_minScore == _maxScore)
      return 1;
    else if (_normalize)
    {
      // Clamping
      double currentScore = std::min(_scoreUpperbound, _matchingGraph.getWeightGm(ik));

      // Min-max normalisation
      return (1 - _beta) * (currentScore - _minScore) / (_maxScore - _minScore);
    }
    else
      return (1 - _beta) * _matchingGraph.getWeightGm(ik);
  }

  /// Return weight of pair of edges (ij, kl) in G_1 and G_2 resp
  virtual double getWeightG1G2(Edge ij, Edge kl) const
  {
    double weight1 = _discretizeWeight ? 1 : _matchingGraph.getWeightG1(ij);
    double weight2 = _discretizeWeight ? 1 : _matchingGraph.getWeightG2(kl);
    // for now we use the average
    return _beta * 0.5 * (weight1 + weight2);
  }

  double getScoreUpperbound() const { return _scoreUpperbound; }
  void setScoreUpperbound(double scoreUpperbound) { _scoreUpperbound = scoreUpperbound; }
  double getMaxScore() const { return _maxScore; }
  void setMaxScore(double maxScore) { _maxScore = maxScore; }
  double getMinScore() const { return _minScore; }
  void setMinScore(double minScore) { _minScore = minScore; }
  bool getNormalize() const { return _normalize; }
  void setNormalize(bool normalize) { _normalize = normalize; }

  virtual void init() 
  {
    if (_matchingGraph.getEdgeCountGm() > 0)
    {
      _maxScore = std::min(mapMaxValue(_matchingGraph.getGm(), _matchingGraph.getWeightGmMap()), _scoreUpperbound);
      _minScore = 0;// mapMinValue(_matchingGraph.getGm(), _matchingGraph.getWeightGmMap());
    }
    else
    {
      _minScore = _maxScore = 0;
    }

    if (g_verbosity >= VERBOSE_ESSENTIAL)
    {
      std::cout << "Using scoring function " << getScoreFunction() << std::endl
                << "Using beta = " << _beta << std::endl;
      if (_normalize)
      {
        std::cout << "Min-max normalization of matching edges in the range ["
                  << _minScore << ", " << _maxScore << "]" << std::endl;
      }
    }
  }
};

template<typename GR, typename BGR>
inline ScoreModel<GR, BGR>::ScoreModel(const MatchingGraph<Graph, BpGraph>& matchingGraph, 
                                       double beta, 
                                       bool discretizeWeight,
                                       bool normalize)
  : _matchingGraph(matchingGraph)
  , _beta(beta)
  , _discretizeWeight(discretizeWeight)
  , _normalize(normalize)
  , _scoreUpperbound(500.)
  , _maxScore(0)
  , _minScore(std::numeric_limits<double>::infinity())
{
}

} // namespace gna
} // namespace nina

#endif /* SCOREMODEL_H_ */
