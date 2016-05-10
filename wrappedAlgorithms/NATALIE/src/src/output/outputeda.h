/*
 * outputeda.h
 *
 *  Created on: 22-feb-2012
 *      Author: M. El-Kebir
 */

#ifndef OUTPUTEDA_H_
#define OUTPUTEDA_H_

#include <ostream>
#include <fstream>
#include "output/output.h"

namespace nina {
namespace gna {

template<typename GR, typename BGR>
class OutputEda : public Output<GR, BGR>
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
  OutputEda(const MatchingGraphType& matchingGraph)
    : Parent(matchingGraph)
  {
  }

  void write(const BpMatchingMapType& matchingMap,
             OutputType outputType, std::ostream& outFile) const;

  std::string getExtension() const
  {
    return ".eda";
  }
};

template<typename GR, typename BGR>
inline void OutputEda<GR, BGR>::write(const BpMatchingMapType& matchingMap,
                                      OutputType outputType,
                                      std::ostream& out) const
{
  const BpGraph& gm = _matchingGraph.getGm();
  const Graph& g1 = _matchingGraph.getG1();
  const Graph& g2 = _matchingGraph.getG2();

  // output matching edges
  out << "Edge type" << std::endl;
  for (BpRedNodeIt r(gm); r != lemon::INVALID; ++r)
  {
    BpEdge e = matchingMap[r];
    if (e != lemon::INVALID)
    {
      out << _matchingGraph.getLabelGm(r)
          << " (typeM) "
          << _matchingGraph.getLabelGm(gm.blueNode(e))
          << " = Gm"
          << std::endl;
    }
  }

  // output edges in G1
  if (outputType != Parent::MINIMAL)
  {
    for (EdgeIt e1(g1); e1 != lemon::INVALID; ++e1)
    {
      Node u1 = g1.u(e1);
      Node v1 = g1.v(e1);

      if (outputType == Parent::FULL ||
          (matchingMap[_matchingGraph.mapG1ToGm(u1)] != lemon::INVALID &&
           matchingMap[_matchingGraph.mapG1ToGm(v1)] != lemon::INVALID))
      {
        out << _matchingGraph.getLabelG1(u1)
            << " (type1) "
            << _matchingGraph.getLabelG1(v1)
            << " = G1"
            << std::endl;
      }
    }

    for (EdgeIt e2(g2); e2 != lemon::INVALID; ++e2)
    {
      Node u2 = g2.u(e2);
      Node v2 = g2.v(e2);

      if (outputType == Parent::FULL ||
          (matchingMap[_matchingGraph.mapG2ToGm(u2)] != lemon::INVALID &&
           matchingMap[_matchingGraph.mapG2ToGm(v2)] != lemon::INVALID))
      {
        out << _matchingGraph.getLabelG2(u2)
            << " (type2) "
            << _matchingGraph.getLabelG2(v2)
            << " = G2"
            << std::endl;
      }
    }
  }
}

} // namespace gna
} // namespace nina

#endif /* OUTPUTEDA_H_ */
