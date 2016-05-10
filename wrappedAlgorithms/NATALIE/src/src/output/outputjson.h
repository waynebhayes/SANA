/* 
 * outputjson.h
 *
 *  Created on: 19-dec-2011
 *      Author: M. El-Kebir
 */

#ifndef OUTPUTJSON_H_
#define OUTPUTJSON_H_

#include <ostream>
#include <fstream>
#include <set>
#include "output/output.h"

namespace nina {
namespace gna {

template<typename GR, typename BGR>
class OutputJson : public Output<GR, BGR>
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
  typedef ScoreModel<Graph, BpGraph> ScoreModelType;
  typedef typename Parent::OutputType OutputType;
  typedef typename std::set<BpBlueNode> BpBlueNodeSet;
  typedef typename BpBlueNodeSet::const_iterator BpBlueNodeSetIt;

  struct Entry {
    Node _node1;
    Node _node2;
    BpEdge _bpEdge;

    Entry(const Node node1,
          const Node node2,
          const BpEdge bpEdge)
      : _node1(node1)
      , _node2(node2)
      , _bpEdge(bpEdge)
    {
    }
  };

  typedef typename std::vector<Entry> EntryVector;
  typedef typename EntryVector::const_iterator EntryVectorIt;

  int _maxNodes;

public:
  OutputJson(const MatchingGraphType& matchingGraph,
             int maxNodes)
    : Parent(matchingGraph)
    , _maxNodes(maxNodes)
  {
  }

  void write(const BpMatchingMapType& matchingMap,
             OutputType outputType, std::ostream& outFile) const;

  std::string getExtension() const
  {
    return ".json";
  }
};

template<typename GR, typename BGR>
inline void OutputJson<GR, BGR>::write(const BpMatchingMapType& matchingMap,
                                       OutputType outputType, 
                                       std::ostream& out) const
{
  const BpGraph& gm = _matchingGraph.getGm();
  //const Graph& g1 = _matchingGraph.getG1();
  const Graph& g2 = _matchingGraph.getG2();

  // we assume that G_1 is the query network and G_2 the target network
  EntryVector entries;
  BpBlueNodeSet blueNodeSet; 

  // generate the query nodes first which have a match
  for (BpRedNodeIt r(gm); r != lemon::INVALID; ++r)
  {
    BpEdge e = matchingMap[r];
    if (e != lemon::INVALID)
    {
      BpBlueNode b = gm.blueNode(e);
      blueNodeSet.insert(b);

      Entry entry(_matchingGraph.mapGmToG1(r), _matchingGraph.mapGmToG2(b), e);
      if (static_cast<int>(entries.size()) < _maxNodes)
        entries.push_back(entry);
    }
  }

  // generate the query nodes that do not have a match, TODO?
  if (outputType != Parent::MINIMAL)
  {
    for (BpRedNodeIt r(gm); r != lemon::INVALID; ++r)
    {
      BpEdge e = matchingMap[r];
      if (e == lemon::INVALID)
      {
        Entry entry(_matchingGraph.mapGmToG1(r), lemon::INVALID, lemon::INVALID);
        if (static_cast<int>(entries.size()) < _maxNodes)
          entries.push_back(entry);
      }
    }
  }

  // now we generate the target nodes without a counter part in G_1
  if (outputType != Parent::MINIMAL)
  {
    BpBlueNodeSet blueNodesToAdd;
    for (BpBlueNodeSetIt it = blueNodeSet.begin(); it != blueNodeSet.end(); it++)
    {
      Node v = _matchingGraph.mapGmToG2(*it);
      for (IncEdgeIt e(g2, v); e != lemon::INVALID; ++e)
      {
        Node w = g2.oppositeNode(v, e);
        BpBlueNode blue_w = gm.asBlueNode(_matchingGraph.mapG2ToGm(w));
        if (blueNodeSet.find(blue_w) == blueNodeSet.end())
        {
          blueNodesToAdd.insert(blue_w);
        }
      }
    }

    // generate target nodes without a counter part in G_1
    for (BpBlueNodeSetIt it = blueNodesToAdd.begin(); it != blueNodesToAdd.end(); it++)
    {
      Entry entry(lemon::INVALID, _matchingGraph.mapGmToG2(*it), lemon::INVALID);
      if (static_cast<int>(entries.size()) < _maxNodes)
        entries.push_back(entry);
    }
  }

  // time to generate output, we start with nodes
  out << "{" << std::endl;
  out << "\t\"nodes\": [" << std::endl;
  int nodeIdx = 0;
  bool first = true;
  for (EntryVectorIt it = entries.begin(); it != entries.end(); it++, nodeIdx++)
  {
      if (!first)
      {
        out << "," << std::endl;
      }
      else
      {
        first = false;
      }

    out << "\t\t{ \"id\": " << nodeIdx++
      << ","
      << "\"idQ\": \"" << (it->_node1 != lemon::INVALID ? _matchingGraph.getLabelG1(it->_node1) : "") << "\""
      << ","
      << "\"idT\": \"" << (it->_node2 != lemon::INVALID ? _matchingGraph.getLabelG2(it->_node2) : "") << "\""
      << ","
      << "\"bit\": \"" << (it->_bpEdge != lemon::INVALID ? _matchingGraph.getWeightGm(it->_bpEdge) : 0) << "\""
      << ","
      << "\"eval\": \"" << (it->_bpEdge != lemon::INVALID ? _matchingGraph.getWeightAltGm(it->_bpEdge) : 0) << "\""
      << " }";
  }
  out << std::endl << "\t]," << std::endl;

  // now the edges
  out << "\t\"links\": [" << std::endl;

  first = true;
  int nodeIdx1 = 0;
  for (EntryVectorIt it1 = entries.begin(); it1 != entries.end(); it1++, nodeIdx1++)
  {
    int nodeIdx2 = nodeIdx1;
    for (EntryVectorIt it2 = it1; it2 != entries.end(); it2++, nodeIdx2++)
    {
      bool queryEdge =
          (it1->_node1 != lemon::INVALID && it2->_node1 != lemon::INVALID) &&
          _matchingGraph.getEdgeG1(it1->_node1, it2->_node1) != lemon::INVALID;

      bool nonConservedTargetEdge =
          (it1->_node1 != lemon::INVALID && it2->_node1 != lemon::INVALID) &&
          (it1->_node2 != lemon::INVALID && it2->_node2 != lemon::INVALID) &&
          _matchingGraph.getEdgeG1(it1->_node1, it2->_node1) == lemon::INVALID &&
          _matchingGraph.getEdgeG2(it1->_node2, it2->_node2) != lemon::INVALID;

      bool targetEdge =
          (it1->_node2 != lemon::INVALID && it2->_node2 != lemon::INVALID) &&
          _matchingGraph.getEdgeG2(it1->_node2, it2->_node2) != lemon::INVALID;

      if ((queryEdge || targetEdge || nonConservedTargetEdge) && !first)
      {
        out << "," << std::endl;
      }
      else if (queryEdge || targetEdge || nonConservedTargetEdge)
      {
        first = false;
      }

      if (queryEdge && targetEdge)
      {
        out << "\t\t{ \"source\":" << nodeIdx1
            << ",\"target\":" << nodeIdx2 << "," << "\"cat\":2"
            << ",\"conf1\":\"" << _matchingGraph.getWeightG1(_matchingGraph.getEdgeG1(it1->_node1, it2->_node1)) << "\""
            << ",\"conf2\":\"" << _matchingGraph.getWeightG2(_matchingGraph.getEdgeG2(it1->_node2, it2->_node2)) << "\""
            << " }";
      }
      else if (queryEdge)
      {
        out << "\t\t{ \"source\":" << nodeIdx1
            << ",\"target\":" << nodeIdx2 << "," << "\"cat\":1"
            << ",\"conf1\":\"" << _matchingGraph.getWeightG1(_matchingGraph.getEdgeG1(it1->_node1, it2->_node1)) << "\""
            << " }";
      }
      else if (targetEdge && !nonConservedTargetEdge)
      {
        out << "\t\t{ \"source\":" << nodeIdx1
            << ",\"target\":" << nodeIdx2 << "," << "\"cat\":0"
            << ",\"conf2\":\"" << _matchingGraph.getWeightG2(_matchingGraph.getEdgeG2(it1->_node2, it2->_node2)) << "\""
            << " }";
      }
      else if (nonConservedTargetEdge)
      {
        out << "\t\t{ \"source\":" << nodeIdx1
            << ",\"target\":" << nodeIdx2 << "," << "\"cat\":3"
            << ",\"conf2\":\"" << _matchingGraph.getWeightG2(_matchingGraph.getEdgeG2(it1->_node2, it2->_node2)) << "\""
            << " }";
      }
    }
  }
  out << std::endl << "\t]" << std::endl;
  out << "}" << std::endl;
}

} // namespace gna
} // namespace nina

#endif /* OUTPUTJSON_H_ */
