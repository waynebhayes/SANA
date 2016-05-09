/* 
 * outputgml.h
 *
 *  Created on: 2-mar-2011
 *      Author: M. El-Kebir
 */

#ifndef OUTPUTGML_H_
#define OUTPUTGML_H_

#include <ostream>
#include <fstream>
#include "output/output.h"

namespace nina {
namespace gna {

template<typename GR, typename BGR>
class OutputGml : public Output<GR, BGR>
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
  OutputGml(const MatchingGraphType& matchingGraph)
    : Parent(matchingGraph)
  {
  };

  void write(const BpMatchingMapType& matchingMap,
             OutputType outputType, std::ostream& outFile) const;

  std::string getExtension() const
  {
    return ".gml";
  }
};

template<typename GR, typename BGR>
inline void OutputGml<GR, BGR>::write(const BpMatchingMapType& matchingMap,
                                      OutputType outputType, 
                                      std::ostream& out) const
{
  const BpGraph& gm = _matchingGraph.getGm();
  const Graph& g1 = _matchingGraph.getG1();
  const Graph& g2 = _matchingGraph.getG2();

  out << "graph [" << std::endl
    << "\tdirected 0" << std::endl;

  // output nodes
  for (BpNodeIt n(gm); n != lemon::INVALID; ++n)
  {
    if (outputType == Parent::FULL || matchingMap[n] != lemon::INVALID)
    {
      out << "\tnode [ id " << gm.id(n) 
        << " label \"" << _matchingGraph.getLabelGm(n) << "\""
        << " comment \"" << (gm.red(n) ? "G_1" : "G_2" ) 
        << "\" ]" << std::endl;
    }
  }

  // output matching edges
  for (BpEdgeIt e(gm); e != lemon::INVALID; ++e)
  {
    if (matchingMap[gm.redNode(e)] == e)
    {
      out << "\tedge [ source " << gm.id(gm.redNode(e)) 
        << " target " << gm.id(gm.blueNode(e)) 
        << " label \"matching edge\""
        << " ]" << std::endl;
    }
  }

  // output original edges
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
           out << "\tedge [ source " << gm.id(u)
            << " target " << gm.id(v)
            << " label \"edge in G_1\""
            << " ]" << std::endl;
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
           out << "\tedge [ source " << gm.id(u)
            << " target " << gm.id(v)
            << " label \"edge in G_2\""
            << " ]" << std::endl;
      }
    }
  }

  out << "]" << std::endl;
}

} // namespace gna
} // namespace nina

#endif /* OUTPUTGML_H_ */
