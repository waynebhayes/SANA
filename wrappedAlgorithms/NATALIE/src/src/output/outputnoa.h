/*
 * outputnoa.h
 *
 *  Created on: 22-feb-2012
 *      Author: M. El-Kebir
 */

#ifndef OUTPUTNOA_H_
#define OUTPUTNOA_H_

#include <ostream>
#include <fstream>
#include "output/output.h"

namespace nina {
namespace gna {

template<typename GR, typename BGR>
class OutputNoa : public Output<GR, BGR>
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

public:
  OutputNoa(const MatchingGraphType& matchingGraph)
    : Parent(matchingGraph)
  {
  }

  void write(const BpMatchingMapType& matchingMap,
             OutputType outputType, std::ostream& outFile) const;

  std::string getExtension() const
  {
    return ".noa";
  }
};

template<typename GR, typename BGR>
inline void OutputNoa<GR, BGR>::write(const BpMatchingMapType& matchingMap,
                                      OutputType outputType,
                                      std::ostream& out) const
{
  const BpGraph& gm = _matchingGraph.getGm();

  // output matched nodes
  out << "Node type" << std::endl;
  for (BpRedNodeIt r(gm); r != lemon::INVALID; ++r)
  {
    BpEdge e = matchingMap[r];
    if (e != lemon::INVALID)
    {
      out << _matchingGraph.getLabelGm(r)
          << " = G1"
          << std::endl;
    }
  }

  for (BpBlueNodeIt b(gm); b != lemon::INVALID; ++b)
  {
    BpEdge e = matchingMap[b];
    if (e != lemon::INVALID)
    {
      out << _matchingGraph.getLabelGm(b)
          << " = G2"
          << std::endl;
    }
  }
}

} // namespace gna
} // namespace nina

#endif /* OUTPUTNOA_H_ */
