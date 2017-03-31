/*
 * outputparameters.h
 *
 *  Created on: 19-mar-2012
 *     Authors: M. El-Kebir and M.E. van der Wees
 */

#ifndef OUTPUTPARAMETERSAMC_H
#define OUTPUTPARAMETERSAMC_H

#include <ostream>
#include <fstream>
#include "output/output.h"

namespace nina {
namespace gna {

template<typename GR, typename BGR>
class OutputParametersAmc : public Output<GR, BGR>
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

  std::string _g1;
  std::string _g2;
  std::string _scoreFunction;
  double _beta;
  double _corrThreshold;
  double _eValCutOff;

public:
  OutputParametersAmc(const MatchingGraphType& matchingGraph,
                      const std::string& g1,
                      const std::string& g2,
                      const std::string& scoreFunction,
                      double beta,
                      double corrThreshold,
                      double eValCutOff)
    : Parent(matchingGraph)
    , _g1(g1)
    , _g2(g2)
    , _scoreFunction(scoreFunction)
    , _beta(beta)
    , _corrThreshold(corrThreshold)
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
inline void OutputParametersAmc<GR, BGR>::write(const BpMatchingMapType& matchingMap,
                                                OutputType outputType,
                                                std::ostream& out) const
{
  out << "\"Mouse network name\"" << ",\"" << _g1 << "\"" << std::endl
      << "\"Mouse network size\"" << ",\"" << _matchingGraph.getNodeCountG1()
      << " nodes and " << _matchingGraph.getEdgeCountG1() << " edges\"" << std::endl
      << "\"Human network name\"" << ",\"" << _g2 << "\"" << std::endl
      << "\"Human network size\"" << ",\"" << _matchingGraph.getNodeCountG2()
      << " nodes and " << _matchingGraph.getEdgeCountG2() << " edges\"" << std::endl
      << "\"Score function type\"" << ",\"" << _scoreFunction << "\"" << std::endl
      << "\"E-value cut-off\"" << ",\"" << _eValCutOff << "\"" << std::endl
      << "\"Correlation threshold\"" << ",\"" << _corrThreshold << "\"" << std::endl
      << "\"Beta\"" << ",\"" << _beta*100 << "%\"" << std::endl;
}

} // namespace gna
} // namespace nina

#endif // OUTPUTPARAMETERSAMC_H
