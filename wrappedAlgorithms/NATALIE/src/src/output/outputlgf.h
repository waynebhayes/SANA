/*
 * outputlgf.h
 *
 *  Created on: 3-mar-2011
 *      Author: M. El-Kebir
 */

#ifndef OUTPUTLGF_H_
#define OUTPUTLGF_H_

#include <ostream>
#include <fstream>
#include <assert.h>
#include <lemon/lgf_reader.h>
#include "output/output.h"

namespace nina {
namespace gna {

template<typename GR, typename BGR>
class OutputLgf : public Output<GR, BGR>
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

  typedef typename Graph::template NodeMap<std::string> LabelNodeMap;
  typedef typename Graph::template EdgeMap<double> WeightEdgeMap;
  typedef typename BpGraph::template NodeMap<Node> BpNodeMap;

  typedef typename Parent::MatchingGraphType MatchingGraphType;
  typedef typename Parent::BpMatchingMapType BpMatchingMapType;
  typedef typename Parent::OutputType OutputType;

public:
  OutputLgf(const MatchingGraphType& matchingGraph)
    : Parent(matchingGraph)
  {
  };

  void write(const BpMatchingMapType& matchingMap,
             OutputType outputType, std::ostream& outFile) const;

  std::string getExtension() const
  {
    return ".lgf";
  }
};

template<typename GR, typename BGR>
inline void OutputLgf<GR, BGR>::write(const BpMatchingMapType& matchingMap,
                                      OutputType outputType, 
                                      std::ostream& out) const
{
  const BpGraph& gm = _matchingGraph.getGm();
  const Graph& g1 = _matchingGraph.getG1();
  const Graph& g2 = _matchingGraph.getG2();

  Graph g;

  g.reserveNode(_matchingGraph.getNodeCountG1() 
      + _matchingGraph.getNodeCountG2());

  BpNodeMap map(gm);
  LabelNodeMap label(g);
  BoolNodeMap nodeClass(g);
  WeightEdgeMap weight(g);
  BoolEdgeMap match(g);

  // output nodes
  for (BpNodeIt v(gm); v != lemon::INVALID; ++v)
  {
    if (outputType == Parent::FULL || matchingMap[v] != lemon::INVALID)
    {
      Node new_v = g.addNode();
      label[new_v] = _matchingGraph.getLabelGm(v);
      nodeClass[new_v] = gm.red(v);
      map[v] = new_v;
    }
  }

  // output matching edges
  for (BpRedNodeIt r(gm); r != lemon::INVALID; ++r)
  {
    BpEdge e = matchingMap[r];
    if (e != lemon::INVALID)
    {
      assert(map[r] != lemon::INVALID && map[gm.blueNode(e)] != lemon::INVALID);
      Edge new_e = g.addEdge(map[r], map[gm.blueNode(e)]);
      weight[new_e] = _matchingGraph.getWeightGm(e);
      match[new_e] = true;
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
        assert(map[u] != lemon::INVALID && map[v] != lemon::INVALID);
        Edge new_e = g.addEdge(map[u], map[v]);
        weight[new_e] = 0;
        match[new_e] = false;
      }
    }

    // in G_2
    for (EdgeIt e(g); e != lemon::INVALID; ++e)
    {
      BpNode u = _matchingGraph.mapG2ToGm(g2.u(e));
      BpNode v = _matchingGraph.mapG2ToGm(g2.v(e));

      if (outputType == Parent::FULL || 
          (matchingMap[u] != lemon::INVALID && matchingMap[v] != lemon::INVALID))
      {
        assert(map[u] != lemon::INVALID && map[v] != lemon::INVALID);
        Edge new_e = g.addEdge(map[u], map[v]);
        weight[new_e] = 0;
        match[new_e] = false;
      }
    }
  }

  graphWriter(g, out)
    .nodeMap("label", label)
    .nodeMap("class", nodeClass)
    .edgeMap("weight", weight)
    .edgeMap("match", match)
    .run();
}

} // namespace gna
} // namespace nina

#endif /* OUTPUTLGF_H_ */
