/* 
 * outputdot.h
 *
 *  Created on: 2-mar-2011
 *      Author: M. El-Kebir
 */

#ifndef OUTPUTDOT_H_
#define OUTPUTDOT_H_

#include <ostream>
#include <fstream>
#include "output/output.h"

namespace nina {
namespace gna {

template<typename GR, typename BGR>
class OutputDot : public Output<GR, BGR>
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
  OutputDot(const MatchingGraphType& matchingGraph)
    : Parent(matchingGraph)
  {
  }

  void write(const BpMatchingMapType& matchingMap,
             OutputType outputType, std::ostream& outFile) const;

  std::string getExtension() const
  {
    return ".dot";
  }
};

template<typename GR, typename BGR>
inline void OutputDot<GR, BGR>::write(const BpMatchingMapType& matchingMap,
                                      OutputType outputType, 
                                      std::ostream& out) const
{
  const BpGraph& gm = _matchingGraph.getGm();
  const Graph& g1 = _matchingGraph.getG1();
  const Graph& g2 = _matchingGraph.getG2();

  out << "graph G" << std::endl << "{" << std::endl;

  // nodes in G_1
  out << "\t{" << std::endl
    << "\t\trank=same;" << std::endl;
  for (BpRedNodeIt r(gm); r != lemon::INVALID; ++r)
  {
    if (outputType == Parent::FULL || matchingMap[r] != lemon::INVALID)
    {
      out << "\t\t" << gm.id(r) << "[label=\"" 
        << _matchingGraph.getLabelGm(r) << "\"];" << std::endl;
    }
  }
  out << "\t}" << std::endl;

  // nodes in G_2
  out << "\t{" << std::endl
    << "\t\trank=same;" << std::endl;
  for (BpBlueNodeIt b(gm); b != lemon::INVALID; ++b)
  {
    if (outputType == Parent::FULL || matchingMap[b] != lemon::INVALID)
    {
      out << "\t\t" << gm.id(b) << "[label=\"" 
        << _matchingGraph.getLabelGm(b) << "\"];" << std::endl;
    }
  }
  out << "\t}" << std::endl;

  // matching edges
  for (BpEdgeIt e(gm); e != lemon::INVALID; ++e)
  {
    if (matchingMap[gm.redNode(e)] == e)
    {
      out << gm.id(gm.redNode(e)) 
        << " -- " << gm.id(gm.blueNode(e)) 
        << ";" << std::endl;
    }
  }

  if (outputType != Parent::MINIMAL)
  {
    // in G_1
    for (EdgeIt e(g1); e != lemon::INVALID; ++e)
    {
      BpNode u = _matchingGraph.mapG1ToGm(g1.u(e));
      BpNode v = _matchingGraph.mapG1ToGm(g1.v(e));

      if (outputType == Parent::FULL || 
          (matchingMap[u] != lemon::INVALID && matchingMap[v] != lemon::INVALID))
      {
        out << gm.id(u)
          << " -- " << gm.id(v)
          << ";" << std::endl;
      }
    }

    // in G_2
    for (EdgeIt e(g2); e != lemon::INVALID; ++e)
    {
      BpNode u = _matchingGraph.mapG2ToGm(g2.u(e));
      BpNode v = _matchingGraph.mapG2ToGm(g2.v(e));

      if (outputType == Parent::FULL || 
          (matchingMap[u] != lemon::INVALID && matchingMap[v] != lemon::INVALID))
      {
        out << gm.id(u)
          << " -- " << gm.id(v)
          << ";" << std::endl;
      }
    }
  }

  out << "}" << std::endl;
}

} // namespace gna
} // namespace nina

#endif /* OUTPUTDOT_H_ */
