/*
 * outputneato.h
 *
 *  Created on: 30-dec-2011
 *      Author: M. El-Kebir
 */

#ifndef OUTPUTNEATO_H_
#define OUTPUTNEATO_H_

#include <ostream>
#include <fstream>
#include <set>
#include "output/output.h"

namespace nina {
namespace gna {

template<typename GR, typename BGR>
class OutputNeato : public Output<GR, BGR>
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

public:
  OutputNeato(const MatchingGraphType& matchingGraph)
    : Parent(matchingGraph)
  {
  };

  void write(const BpMatchingMapType& matchingMap,
             OutputType outputType, std::ostream& outFile) const;

  std::string getExtension() const
  {
    return ".dot";
  }
};

template<typename GR, typename BGR>
inline void OutputNeato<GR, BGR>::write(const BpMatchingMapType& matchingMap,
                                      OutputType outputType, 
                                      std::ostream& out) const
{
  const BpGraph& gm = _matchingGraph.getGm();
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
      entries.push_back(entry);
    }
  }

  // generate the query nodes first which have a match
  for (BpRedNodeIt r(gm); r != lemon::INVALID; ++r)
  {
    BpEdge e = matchingMap[r];
    if (e == lemon::INVALID)
    {
      Entry entry(_matchingGraph.mapGmToG1(r), lemon::INVALID, lemon::INVALID);
      entries.push_back(entry);
    }
  }

  // now we generate the target nodes without a counter part in G_1
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
    entries.push_back(entry);
  }


  // time to generate output, we start with nodes
  out << "graph G" << std::endl << "{" << std::endl;
  out << "\t{" << std::endl;
  int nodeIdx = 0;
  for (EntryVectorIt it = entries.begin(); it != entries.end(); it++, nodeIdx++)
  {
    std::string label;
    if (it->_node1 != lemon::INVALID && it->_node2 != lemon::INVALID)
    {
      label = _matchingGraph.getLabelG1(it->_node1) + "_" + _matchingGraph.getLabelG2(it->_node2);
      out << "\t\t" << nodeIdx << "[label=\"" << label << "\""
        << ",shape=doublecircle,style=filled,fillcolor=grey,color=red"
        << "];" << std::endl;
    }
    else if (it->_node1 != lemon::INVALID)
    {
      label = _matchingGraph.getLabelG1(it->_node1);
      out << "\t\t" << nodeIdx << "[label=\"" << label << "\""
        << ",shape=circle,style=filled,fillcolor=red,color=red"
        << "];" << std::endl;
    }
    else if (it->_node2 != lemon::INVALID)
    {
      label = _matchingGraph.getLabelG2(it->_node2);
      out << "\t\t" << nodeIdx << "[label=\"" << label << "\""
        << ",shape=circle,style=filled,fillcolor=grey,color=grey"
        << "];" << std::endl;
    }
  }
  out << "\t}" << std::endl;

  int nodeIdx1 = 0;
  for (EntryVectorIt it1 = entries.begin(); it1 != entries.end(); it1++, nodeIdx1++)
  {
    int nodeIdx2 = nodeIdx1;
    for (EntryVectorIt it2 = it1; it2 != entries.end(); it2++, nodeIdx2++)
    {
      bool queryEdge = (it1->_node1 != lemon::INVALID && it2->_node1 != lemon::INVALID) ? 
        _matchingGraph.getEdgeG1(it1->_node1, it2->_node1) != lemon::INVALID : false;

      bool targetEdge = (it1->_node2 != lemon::INVALID && it2->_node2 != lemon::INVALID) ? 
        _matchingGraph.getEdgeG2(it1->_node2, it2->_node2) != lemon::INVALID : false;

      if (queryEdge)
      {
        out << "\t" << nodeIdx1 << " -- " << nodeIdx2 << " " << "[color=red]" << std::endl;
      }
      if (targetEdge)
      {
        out << "\t" << nodeIdx1 << " -- " << nodeIdx2 << " " << "[color=grey]" << std::endl;
      }
    }
  }

  out << "}" << std::endl;
}

} // namespace gna
} // namespace nina

#endif /* OUTPUTNEATO_H_ */
