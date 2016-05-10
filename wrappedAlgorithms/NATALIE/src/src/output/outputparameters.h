/*
 * outputparameters.h
 *
 *  Created on: 23-feb-2012
 *      Author: M. El-Kebir
 */

#ifndef OUTPUTPARAMETERS_H_
#define OUTPUTPARAMETERS_H_

#include <ostream>
#include <fstream>
#include "output/output.h"

namespace nina {
namespace gna {

template<typename GR, typename BGR>
class OutputParameters : public Output<GR, BGR>
{
public:
  /// The graph type of the input graphs
  typedef GR Graph;
  /// The graph type of the bipartite matching graph
  typedef BGR BpGraph;
  /// Base class type
  typedef Output<Graph, BpGraph> Parent;

  using Parent::_matchingGraph;

private:
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
  typedef typename Parent::MatchingGraphType MatchingGraphType;
  typedef typename Parent::BpMatchingMapType BpMatchingMapType;
  typedef typename Parent::OutputType OutputType;

  std::string _g2;
  double _beta;
  double _confThreshold;
  double _eValCutOff;

public:
  OutputParameters(const MatchingGraphType& matchingGraph,
                   const std::string& g2,
                   double beta,
                   double confThreshold,
                   double eValCutOff)
    : Parent(matchingGraph)
    , _g2(g2)
    , _beta(beta)
    , _confThreshold(confThreshold)
    , _eValCutOff(eValCutOff)
  {
  }

  void write(const BpMatchingMapType& matchingMap,
             OutputType outputType, std::ostream& outFile) const;

  std::string getExtension() const
  {
    return "-params.csv";
  }
};

template<typename GR, typename BGR>
inline void OutputParameters<GR, BGR>::write(const BpMatchingMapType& matchingMap,
                                      OutputType outputType,
                                      std::ostream& out) const
{
  out << "\"Target network name\"" << ",\"" << _g2 << "\"" << std::endl
      << "\"Target network size\"" << ",\"" << _matchingGraph.getNodeCountG2()
      << " nodes and " << _matchingGraph.getEdgeCountG2() << " edges\"" << std::endl
      << "\"Query network size\"" << ",\"" << _matchingGraph.getNodeCountG1()
      << " nodes and " << _matchingGraph.getEdgeCountG1() << " edges\"" << std::endl
      << "\"Number of matching edges\"" << ",\"" << _matchingGraph.getEdgeCountGm()
      << " edges\"" << std::endl
      << "\"E-value cut-off\"" << ",\"" << _eValCutOff << "\"" << std::endl
      << "\"Confidence threshold\"" << ",\"" << _confThreshold*100 << "%\"" << std::endl;
//      << "\"Beta\"" << ",\"" << _beta*100 << "%\"" << std::endl;
}

} // namespace gna
} // namespace nina

#endif // OUTPUTPARAMETERS_H_
