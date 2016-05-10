/*
 * analyse.h
 *
 *  Created on: 22-feb-2012
 *      Author: M.E. van der Wees
 */

#ifndef ANALYSE_H_
#define ANALYSE_H_

#include <string>
#include <list>
#include <set>
#include <vector>
#include <algorithm>
#include <utility>
#include <fstream>
#include <lemon/core.h>
#include <limits>
#include "input/matchinggraph.h"
#include "gna.h"

namespace nina {
namespace gna {

template<typename GR, typename BGR>
class Analyse
{
public:
  /// The graph type of the connected components graph
  typedef GR Graph;
  /// The graph type of the bipartite matching graph
  typedef BGR BpGraph;
  ///
  typedef PairwiseGlobalNetworkAlignment<Graph, BpGraph> GnaType;
  /// Type of a matching map: maps a node to its matching edge
  typedef typename GnaType::BpMatchingMap BpMatchingMap;

protected:
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

  /// Type of the bipartite edge list
  typedef std::list<BpEdge> BpEdgeList;
  /// Type of the bipartite edge list iterator
  typedef typename BpEdgeList::const_iterator BpEdgeListIt;
  /// Type of the bipartite edge pair
  typedef std::pair<BpEdge, BpEdge> BpEdgePair;
  /// Mapping from nodes to edge pair
  typedef typename Graph::template NodeMap<BpEdgePair> EdgePairNodeMap;
  /// Mapping from nodes to original node labels
  typedef typename Graph::template NodeMap<std::string> OrigLabelNodeMap;
  /// Mapping from nodes to connected component number
  typedef typename Graph::template NodeMap<int> ComponentNodeMap;

  /// Type of the node list
  typedef std::set<BpNode> BpNodeSet;
  /// Type of the bipartite node set iterator
  typedef typename BpNodeSet::const_iterator BpNodeSetIt;
  /// Mapping from connected component numbers to nodes
  typedef typename std::vector<BpNodeSet> BpNodeSetVector;
  /// Type of the matching graph
  typedef MatchingGraph<Graph, BpGraph> MatchingGraphType;
  /// Type of an integer vector
  typedef std::vector<int> IntVector;
  /// Type of a double vector
  typedef std::vector<double> DoubleVector;
  /// Type of a score model
  typedef ScoreModel<Graph, BpGraph> ScoreModelType;

  /// Matching graph
  const MatchingGraphType& _matchingGraph;
  /// Score model
  const ScoreModelType* _pScoreModel;
  /// Connected component graph
  Graph _gc;
  /// List with all edges that exist in alignment solution
  BpEdgeList _bpEdgeList;
  /// Map of nodes to edge pair
  EdgePairNodeMap _nodeToEdges;
  /// Map of nodes to original labels
  OrigLabelNodeMap _nodeToLabels;
  /// Map of nodes to connected component numbers
  ComponentNodeMap _nodeToComponent;
  /// Vector with bipartite node sets per component
  BpNodeSetVector _bpNodeSetsVector;
  /// Vector with number of conserved edges per component
  IntVector _conservedEdgeCountVector;
  /// Vector with number of non-conserved edges in G1 per component
  IntVector _nonConservedG1EdgeCountVector;
  /// Vector with number of non-conserved edges in G2 per component
  IntVector _nonConservedG2EdgeCountVector;
  /// Vector with sequence contribution to the score per component
  DoubleVector _sequenceScoreVector;
  /// Vector with topology contribution to the score per component
  DoubleVector _topologyScoreVector;

public:
  /// Constructor
  Analyse(const MatchingGraphType& matchingGraph, const ScoreModelType* pScoreModel);
  /// Create connected component graph \f$G_c\f$
  void makeGc(const BpMatchingMap& alignment);
  /// Get vector with bipartite node sets per component
  BpNodeSetVector getBpNodeSetVector() const { return _bpNodeSetsVector; }
  /// Get number of components
  int getNumberOfComponents() const { return static_cast<int>(_bpNodeSetsVector.size()); }
  /// Create .dot file used for graphical representation of \f$G_c\f$
  void createDotFile();


  int getNodeCount(int i) const
  {
    assert(0 <= i && i < static_cast<int>(_bpNodeSetsVector.size()));
    return static_cast<int>(_bpNodeSetsVector[i].size());
  }

  int getAlignedPairCount(int i) const
  {
    assert(0 <= i && i < static_cast<int>(_bpNodeSetsVector.size()));
    assert(_bpNodeSetsVector[i].size() % 2 == 0);
    return static_cast<int>(_bpNodeSetsVector[i].size()) / 2;
  }

  int getConservedEdgeCount(int i) const
  {
    assert(0 <= i && i < static_cast<int>(_conservedEdgeCountVector.size()));
    return _conservedEdgeCountVector[i];
  }

  int getNonConservedG1EdgeCount(int i) const
  {
    assert(0 <= i && i < static_cast<int>(_nonConservedG1EdgeCountVector.size()));
    return _nonConservedG1EdgeCountVector[i];
  }

  int getNonConservedG2EdgeCount(int i) const
  {
    assert(0 <= i && i < static_cast<int>(_nonConservedG2EdgeCountVector.size()));
    return _nonConservedG2EdgeCountVector[i];
  }

  double getSequenceScore(int i) const
  {
    assert(0 <= i && i < static_cast<int>(_sequenceScoreVector.size()));
    return _sequenceScoreVector[i];
  }

  double getTopologyScore(int i) const
  {
    assert(0 <= i && i < static_cast<int>(_topologyScoreVector.size()));
    return _topologyScoreVector[i];
  }

  double getScore(int i) const
  {
    assert(0 <= i && i < static_cast<int>(_topologyScoreVector.size()));
    assert(0 <= i && i < static_cast<int>(_sequenceScoreVector.size()));
    return _topologyScoreVector[i] + _sequenceScoreVector[i];
  }


protected:
  /// Make list of all alignment edges
  void makeEdgeList(const BpMatchingMap& alignment);
  /// Create \f$G_c\f$ nodes for each connected component
  void makeGcNodes();
  /// Create edges in \f$G_c\f$
  void makeGcEdges();
  /// Label nodes in \f$G_c\f$
  void labelGcNodes();
  /// Fill vector with bipartite node sets and return number of components
  void fillVectors(const BpMatchingMap& alignment);
};

template<typename GR, typename BGR>
inline Analyse<GR, BGR>::Analyse(const MatchingGraphType& matchingGraph,
                                 const ScoreModelType* pScoreModel)
  : _matchingGraph(matchingGraph)
  , _pScoreModel(pScoreModel)
  , _nodeToEdges(_gc)
  , _nodeToLabels(_gc)
  , _nodeToComponent(_gc)
  , _bpNodeSetsVector()
  , _conservedEdgeCountVector()
  , _nonConservedG1EdgeCountVector()
  , _nonConservedG2EdgeCountVector()
  , _sequenceScoreVector()
  , _topologyScoreVector()
{
}

template<typename GR, typename BGR>
inline void Analyse<GR, BGR>::makeGc(const BpMatchingMap& alignment)
{
  makeEdgeList(alignment);
  makeGcNodes();
  makeGcEdges();
  labelGcNodes();
  fillVectors(alignment);
}

template<typename GR, typename BGR>
inline void Analyse<GR, BGR>::makeEdgeList(const BpMatchingMap& alignment)
{
  const BpGraph& gm = _matchingGraph.getGm();
  for (BpRedNodeIt n(gm); n != lemon::INVALID; ++n)
  {
    BpEdge edge = alignment[n];

    if (edge != lemon::INVALID)
      _bpEdgeList.push_back(edge);
  }
}

template<typename GR, typename BGR>
inline void Analyse<GR, BGR>::makeGcNodes()
{
  const BpGraph& gm = _matchingGraph.getGm();

  int count = 0;
  for (BpEdgeListIt e1 = _bpEdgeList.begin(); e1 != _bpEdgeList.end(); e1++)
  {
    BpEdgeListIt e2 = e1;
    e2++;
    for (; e2 != _bpEdgeList.end(); e2++)
    {
      Node red1 = _matchingGraph.mapGmToG1(gm.redNode(*e1));
      Node red2 = _matchingGraph.mapGmToG1(gm.redNode(*e2));
      Node blue1 = _matchingGraph.mapGmToG2(gm.blueNode(*e1));
      Node blue2 = _matchingGraph.mapGmToG2(gm.blueNode(*e2));

      if ((_matchingGraph.getEdgeG1(red1, red2) != lemon::INVALID) && (_matchingGraph.getEdgeG2(blue1, blue2) != lemon::INVALID))
      {
        // Add node to gc
        Node n = _gc.addNode();
        BpEdgePair edgePair(*e1, *e2);
        _nodeToEdges[n] = edgePair;
        count++;
      }
    }
  }
  //std::cout << "Number of nodes in gc: " << count << std::endl;
}

template<typename GR, typename BGR>
inline void Analyse<GR, BGR>::makeGcEdges()
{
  int count = 0;
  for (NodeIt n1(_gc); n1 != lemon::INVALID; ++n1)
  {
    NodeIt n2 = n1;
    ++n2;
    for (; n2 != lemon::INVALID; ++n2)
    {
      if ((_nodeToEdges[n1].first == _nodeToEdges[n2].first) || (_nodeToEdges[n1].first == _nodeToEdges[n2].second) ||
          (_nodeToEdges[n1].second == _nodeToEdges[n2].first) || (_nodeToEdges[n1].second == _nodeToEdges[n2].second))
      {
        // Add edge to gc
        _gc.addEdge(n1, n2);
        count++;
      }
    }
  }
  //std::cout << "Number of edges in gc: " << count << std::endl;
}

template<typename GR, typename BGR>
inline void Analyse<GR, BGR>::labelGcNodes()
{
  for (NodeIt n(_gc); n != lemon::INVALID; ++n)
  {
    const BpGraph& gm = _matchingGraph.getGm();
    BpEdge e1 = _nodeToEdges[n].first;
    BpEdge e2 = _nodeToEdges[n].second;
    Node red1 = _matchingGraph.mapGmToG1(gm.redNode(e1));
    Node blue1 = _matchingGraph.mapGmToG2(gm.blueNode(e1));
    Node red2 = _matchingGraph.mapGmToG1(gm.redNode(e2));
    Node blue2 = _matchingGraph.mapGmToG2(gm.blueNode(e2));

    // Gc label containing all gene IDs in gc node
    const std::string& label = "\"" + _matchingGraph.getLabelG1(red1) + ", " + _matchingGraph.getLabelG2(blue1)
                             + ", " + _matchingGraph.getLabelG1(red2) + ", " + _matchingGraph.getLabelG2(blue2) + "\"";
    _nodeToLabels[n] = label;
  }
}

template<typename GR, typename BGR>
inline void Analyse<GR, BGR>::createDotFile()
{
  std::ofstream dotFile;
  dotFile.open("GcGraph.dot");
  dotFile << "graph Gc {\n";
  for (EdgeIt e(_gc); e != lemon::INVALID; ++e)
  {
    Node node1 = _gc.u(e);
    Node node2 = _gc.v(e);
    dotFile << "\t" + _nodeToLabels[node1] + " -- " + _nodeToLabels[node2] + ";\n";
  }
  dotFile << "}\n";
  dotFile.close();
}

template<typename GR, typename BGR>
inline void Analyse<GR, BGR>::fillVectors(const BpMatchingMap& alignment)
{
  const BpGraph& gm = _matchingGraph.getGm();
  int numberOfComponents = lemon::connectedComponents(_gc, _nodeToComponent);

  _bpNodeSetsVector = BpNodeSetVector(numberOfComponents);
  _conservedEdgeCountVector = _nonConservedG1EdgeCountVector = _nonConservedG2EdgeCountVector = IntVector(numberOfComponents, 0);
  _sequenceScoreVector = _topologyScoreVector = DoubleVector(numberOfComponents, 0);

  // Fill bipartite node set vector
  for (NodeIt n (_gc); n != lemon::INVALID; ++n)
  {
    int componentNumber = _nodeToComponent[n];
    BpEdge e1 = _nodeToEdges[n].first;
    BpEdge e2 = _nodeToEdges[n].second;

    _bpNodeSetsVector[componentNumber].insert(gm.redNode(e1));
    _bpNodeSetsVector[componentNumber].insert(gm.blueNode(e1));
    _bpNodeSetsVector[componentNumber].insert(gm.redNode(e2));
    _bpNodeSetsVector[componentNumber].insert(gm.blueNode(e2));
  }

  for (int i = 0; i < numberOfComponents; i++)
  {
    // Create node pairs belonging to the same component
    for (BpNodeSetIt it1 = _bpNodeSetsVector[i].begin(); it1 != _bpNodeSetsVector[i].end(); it1++)
    {
      BpNodeSetIt it2 = it1;
      it2++;
      for (; it2 != _bpNodeSetsVector[i].end(); it2++)
      {
        if (gm.red(*it1) && gm.red(*it2))
        {
          // Check if the red nodes share a common edge, and their blue counterparts do so as well
          Node u1 = _matchingGraph.mapGmToG1(*it1);
          Node v1 = _matchingGraph.mapGmToG1(*it2);
          Edge e1 = _matchingGraph.getEdgeG1(u1, v1);
          if (e1 != lemon::INVALID)
          {
            // By definition components contain only aligned node pairs
            assert(alignment[*it1] != lemon::INVALID && alignment[*it2] != lemon::INVALID);
            Node u2 = _matchingGraph.mapGmToG2(gm.blueNode(alignment[*it1]));
            Node v2 = _matchingGraph.mapGmToG2(gm.blueNode(alignment[*it2]));
            Edge e2 = _matchingGraph.getEdgeG2(u2, v2);
            if (e2 == lemon::INVALID)
            {
              _nonConservedG1EdgeCountVector[i]++;
            }
            else
            {
              _conservedEdgeCountVector[i]++;
              // Compute topology contribution to the score
              _topologyScoreVector[i] += _pScoreModel->getWeightG1G2(e1, e2);
            }
          }
        }
        else if (gm.blue(*it1) && gm.blue(*it2))
        {
          // Check if the blue nodes share a common edge, and their red counterparts do so as well
          Node u2 = _matchingGraph.mapGmToG2(*it1);
          Node v2 = _matchingGraph.mapGmToG2(*it2);
          if (_matchingGraph.getEdgeG2(u2, v2) != lemon::INVALID)
          {
            // By definition components contain only aligned node pairs
            assert(alignment[*it1] != lemon::INVALID && alignment[*it2] != lemon::INVALID);
            Node u1 = _matchingGraph.mapGmToG1(gm.redNode(alignment[*it1]));
            Node v1 = _matchingGraph.mapGmToG1(gm.redNode(alignment[*it2]));
            if (_matchingGraph.getEdgeG1(u1, v1) == lemon::INVALID)
            {
              _nonConservedG2EdgeCountVector[i]++;
            }
          }
        }
      }

      // Compute sequence contribution to the score
      if (gm.red(*it1))
      {
        BpEdge e = alignment[*it1];
        _sequenceScoreVector[i] += _pScoreModel->getWeightGm(e);
      }
    }
  }
}

} // namespace gna
} // namespace nina

#endif // ANALYSE_H_
