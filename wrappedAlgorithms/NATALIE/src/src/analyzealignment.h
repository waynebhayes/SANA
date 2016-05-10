/* 
 * analysealignment.h
 *
 *  Created on: 18-jun-2011
 *      Author: M. El-Kebir
 */

#ifndef ANALYSEALIGNMENT_H_
#define ANALYSEALIGNMENT_H_

#include <fstream>
#include "score/scoremodel.h"
#include "input/matchinggraph.h"

namespace nina {
namespace gna {

template<typename GR, typename BGR>
class AnalyzeAlignment
{
public:
  /// The graph type of the input graphs
  typedef GR Graph;
  /// The graph type of the bipartite matching graph
  typedef BGR BpGraph;
  /// MatchingGraph type
  typedef MatchingGraph<Graph, BpGraph> MatchingGraphType;
  /// Score model type
  typedef ScoreModel<Graph, BpGraph> ScoreModelType;

private:
  TEMPLATE_GRAPH_TYPEDEFS(Graph);
  typedef typename Graph::template NodeMap<Node> AlignmentMap;
  typedef typename BpGraph::Node BpNode;
  typedef typename BpGraph::Edge BpEdge;
  typedef typename std::vector<double> DoubleVector;
  typedef typename std::vector<int> IntVector;
  typedef typename IntVector::const_iterator IntVectorIt;
  typedef typename DoubleVector::const_iterator DoubleVectorIt;

  const MatchingGraphType& _matchingGraph;
  const ScoreModelType& _scoreModel;
  AlignmentMap _alignmentG1;
  AlignmentMap _alignmentG2;

  Graph _g;
  BoolNodeMap _isInG1;
  AlignmentMap _fromG1toG;
  AlignmentMap _fromG2toG;
  AlignmentMap _fromGtoG12;

  int _mappedNodes;
  int _mappedEdges;
  int _nComponents;
  double _EC;
  double _ICS;
  double _S3;
  double _sigmaScore;
  double _tauScore;
  IntNodeMap _componentMap;
  IntVector _edgeCountPerComponent1;
  IntVector _edgeCountPerComponent2;
  IntVector _nodeCountPerComponent1;
  IntVector _nodeCountPerComponent2;
  int _largestComponentIdx;

  double computeScore(bool useSigma);
  double computeEC();
  double computeICS();
  double computeS3();
  void computeComponents();

public:
  AnalyzeAlignment(const MatchingGraphType& matchingGraph, const ScoreModelType& scoreModel);
  bool init(const std::string& resultFileName);

  void analyze();
  int getNumberOfComponents() const
  {
    return _nComponents; 
  }
  int getNodesInComponent(int idx) const
  {
    assert(0 <= idx && idx < _nComponents);
    return _nodeCountPerComponent1[idx] + _nodeCountPerComponent2[idx];
  }
  int getLargestComponentSize() const 
  { 
    return getNodesInComponent(_largestComponentIdx);
  }
  double getClusterCoefficient(int idx, bool g1) const
  {
    assert(0 <= idx && idx < _nComponents);
    if (g1)
    {
      return _edgeCountPerComponent1[idx] / 
        static_cast<double>(_nodeCountPerComponent1[idx] * (_nodeCountPerComponent1[idx] - 1) / 2);
    }
    else
    {
      return _edgeCountPerComponent2[idx] / 
        static_cast<double>(_nodeCountPerComponent2[idx] * (_nodeCountPerComponent2[idx] - 1) / 2);
    }
  }
};

} // namespace gna
} // namespace nina

#endif /* ANALYSEALIGNMENT_H_ */
