/*
 * matchinggraph.h
 *
 *  Created on: 8-feb-2011
 *      Author: M. El-Kebir
 */

#ifndef MATCHINGGRAPH_H_
#define MATCHINGGRAPH_H_

#include <assert.h>
#include <string>
#include <fstream>
#include <list>
#include <lemon/core.h>
#include <lemon/connectivity.h>
#include <lemon/lgf_reader.h>
#include <lemon/lgf_writer.h>
#include "input/gmlparser.h"
#include "input/graphmlparser.h"
#include "input/stringparser.h"
#include "input/lgfparser.h"
#include "input/csvparser.h"
#include "input/ledaparser.h"
#include "input/parser.h"
#include "verbose.h"
#include "input/bpparser.h"
#include "webserver.h"

namespace nina {
namespace gna {

template<typename GR, typename BGR>
class MatchingGraph
{
public:
  /// The graph type of the input graphs
  typedef GR Graph;
  /// The graph type of the bipartite matching graph
  typedef BGR BpGraph;
  /// Weights on original edges
  typedef typename Graph::template EdgeMap<double> WeightEdgeMap;
  /// Weights on matching edges
  typedef typename BpGraph::template EdgeMap<double> BpWeightEdgeMap;

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

  /// Labels of the nodes
  typedef typename Graph::template NodeMap<std::string> OrigLabelNodeMap;
  /// Mapping from original nodes to matching nodes
  typedef typename Graph::template NodeMap<BpNode> OrigNodeToMatchNodeMap;
  /// Mapping from matching nodes to original nodes
  typedef typename BpGraph::template NodeMap<Node> MatchNodeToOrigNodeMap;
  /// Mapping from matching nodes to original nodes
  typedef typename BpGraph::template NodeMap<std::string> BpStringNodeMap;
  /// Mapping from labels to original nodes
  typedef std::map<std::string, typename Graph::Node> InvOrigLabelNodeMap;
  /// Parser type
  typedef Parser<Graph> ParserType;
  /// Bipartite parser type
  typedef BpParser<Graph, BpGraph> BpParserType;

  /// Matching graph
  BpGraph _gm;
  /// Input graph 1 (\f$G_1\f$)
  Graph _g1;
  /// Input graph 2 (\f$G_2\f$)
  Graph _g2;
  /// Node labels for \f$G_1\f$
  OrigLabelNodeMap _labelG1;
  /// Node labels for \f$G_2\f$
  OrigLabelNodeMap _labelG2;
  /// Inverse map from label to node in G_1
  InvOrigLabelNodeMap _invLabelG1;
  /// Inverse map from label to node in G_2
  InvOrigLabelNodeMap _invLabelG2;
  /// Mapping from original nodes in \f$G_1\f$ to matching nodes
  OrigNodeToMatchNodeMap _g1ToGm;
  /// Mapping from original nodes in \f$G_2\f$ to matching nodes
  OrigNodeToMatchNodeMap _g2ToGm;
  /// Mapping from matching nodes to original nodes
  MatchNodeToOrigNodeMap _gmToG12;
  /// Weights on edges in G_1
  WeightEdgeMap _weightG1;
  /// Weights on edges in G_2
  WeightEdgeMap _weightG2;
  /// Weights on edges in G_m
  BpWeightEdgeMap _weightGm;
  /// Alternative weights on edges in G_m
  BpWeightEdgeMap _weightAltGm;
  /// Number of matching edges
  int _nEdgesGm;
  /// Number of nodes in \f$G_1\f$
  int _nNodesG1;
  /// Number of nodes in \f$G_2\f$
  int _nNodesG2;
  /// Number of edges in \f$G_1\f$
  int _nEdgesG1;
  /// Number of edges in \f$G_2\f$
  int _nEdgesG2;
  /// Allows looking up edges in \f$O(\log d)\f$ time in \f$G_1\f$
  lemon::ArcLookUp<Graph> _arcLookUpG1;
  /// Allows looking up edges in \f$O(\log d)\f$ time in \f$G_2\f$
  lemon::ArcLookUp<Graph> _arcLookUpG2;
  /// Score model
  //const ScoreModel<Graph, BpGraph>* _pScore;

  bool parse(ParserType* pParser);
  bool parse(BpParserType* pBpParser);

  /// Load G_m in LGF format
  void loadGm(std::istream& in);

public:
  /// Constructor
  MatchingGraph();
  /// Virtual destructor
  virtual ~MatchingGraph() {}
  /// Initialize G_1, G_2 and G_m
  virtual bool init(ParserType* pParserG1,
                    ParserType* pParserG2,
                    BpParserType* pParserGm);
  /// Return the label of a node in \f$G_m\f$
  const std::string& getLabelGm(BpNode n) const;
  /// Return the label of a node in \f$G_1\f$
  const std::string& getLabelG1(Node n) const;
  /// Return the label of a node in \f$G_2\f$
  const std::string& getLabelG2(Node n) const;
  /// Return map of labels of nodes in \f$G_1\f$
  const OrigLabelNodeMap& getMapLabelG1() const { return _labelG1; }
  /// Return map of labels of nodes in \f$G_2\f$
  const OrigLabelNodeMap& getMapLabelG2() const { return _labelG2; }
  /// Get node in \f$G_1\f$ by label
  Node getNodeG1(const std::string& label) const;
  /// Get node in \f$G_2\f$ by label
  Node getNodeG2(const std::string& label) const;
  /// Find an edge between two nodes in \f$G_1\f$
  Edge getEdgeG1(Node u, Node v) const { return _arcLookUpG1(u, v); }
  /// Find an edge between two nodes in \f$G_2\f$
  Edge getEdgeG2(Node u, Node v) const { return _arcLookUpG2(u, v); }
  /// Return node in \f$G_m\f$ corresponding to node n in \f$G_1\f$
  virtual BpNode mapG1ToGm(Node n) const;
  /// Return node in \f$G_m\f$ corresponding to node n in \f$G_2\f$
  virtual BpNode mapG2ToGm(Node n) const;
  /// Return node in \f$G_1\f$ corresponding to node n in \f$G_m\f$
  virtual Node mapGmToG1(BpNode n) const;
  /// Return node in \f$G_2\f$ corresponding to node n in \f$G_m\f$
  virtual Node mapGmToG2(BpNode n) const;
  /// Return \f$G_m\f$
  const BpGraph& getGm() const { return _gm; }
  /// Return \f$G_1\f$
  const Graph& getG1() const { return _g1; }
  /// Return \f$G_2\f$
  const Graph& getG2() const { return _g2; }
  /// Return number of edges in \f$G_m\f$
  int getEdgeCountGm() const { return _nEdgesGm; }
  /// Return number of edges in \f$G_1\f$
  int getEdgeCountG1() const { return _nEdgesG1; }
  /// Return number of edges in \f$G_2\f$
  int getEdgeCountG2() const { return _nEdgesG2; }
  /// Return number of nodes in \f$G_1\f$
  int getNodeCountG1() const { return _nNodesG1; }
  /// Return number of nodes in \f$G_2\f$
  int getNodeCountG2() const { return _nNodesG2; }
  /// Return weight of edge in G_1
  double getWeightG1(Edge ij) const { return _weightG1[ij]; }
  /// Return weight of edge in G_2
  double getWeightG2(Edge kl) const { return _weightG2[kl]; }
  /// Return weight of edge in G_m
  double getWeightGm(BpEdge ik) const { return _weightGm[ik]; }
  /// Return alternative weight of edge in G_m
  double getWeightAltGm(BpEdge ik) const { return _weightAltGm[ik]; }
  /// Return matching edge weight map
  const BpWeightEdgeMap& getWeightGmMap() const { return _weightGm; }
  /// Return matching edge weight map
  BpWeightEdgeMap& getWeightGmMap() { return _weightGm; }
  /// Return matching edge weight map
  const BpWeightEdgeMap& getWeightAltGmMap() const { return _weightAltGm; }
  /// Save G_1 in LGF format
  void saveG1(std::ostream& out) const;
  /// Save G_2 in LGF format
  void saveG2(std::ostream& out) const;
  /// Save G_m in LGF format
  void saveGm(std::ostream& out) const;
};

template<typename GR, typename BGR>
inline MatchingGraph<GR, BGR>::MatchingGraph()
  : _gm()
  , _g1()
  , _g2()
  , _labelG1(_g1)
  , _labelG2(_g2)
  , _invLabelG1()
  , _invLabelG2()
  , _g1ToGm(_g1)
  , _g2ToGm(_g2)
  , _gmToG12(_gm)
  , _weightG1(_g1)
  , _weightG2(_g2)
  , _weightGm(_gm)
  , _weightAltGm(_gm)
  , _nEdgesGm(0)
  , _nNodesG1(0)
  , _nNodesG2(0)
  , _nEdgesG1(0)
  , _nEdgesG2(0)
  , _arcLookUpG1(_g1)
  , _arcLookUpG2(_g2)
{
}

template<typename GR, typename BGR>
inline bool MatchingGraph<GR, BGR>::init(ParserType* pParserG1,
                                         ParserType* pParserG2,
                                         BpParserType* pParserGm)
{
  assert(pParserG1 && pParserG2 && pParserGm);

  pParserG1->setGraph(&_g1);
  pParserG1->setIdNodeMap(&_labelG1);
  pParserG1->setInvIdNodeMap(&_invLabelG1);
  pParserG1->setWeightEdgeMap(&_weightG1);

  pParserG2->setGraph(&_g2);
  pParserG2->setIdNodeMap(&_labelG2);
  pParserG2->setInvIdNodeMap(&_invLabelG2);
  pParserG2->setWeightEdgeMap(&_weightG2);

  if (parse(pParserG1) && parse(pParserG2))
  {
    _nNodesG1 = pParserG1->getNodeCount();
    _nEdgesG1 = pParserG1->getEdgeCount();
    _nNodesG2 = pParserG2->getNodeCount();
    _nEdgesG2 = pParserG2->getEdgeCount();
    _arcLookUpG1.refresh();
    _arcLookUpG2.refresh();
  }
  else
  {
    return false;
  }

  pParserGm->setGraph(&_gm);
  pParserGm->setWeightEdgeMap(&_weightGm);
  pParserGm->setWeightAltEdgeMap(&_weightAltGm);
  pParserGm->setG1ToGmMap(&_g1ToGm);
  pParserGm->setG2ToGmMap(&_g2ToGm);
  pParserGm->setGmToG12Map(&_gmToG12);

  if (!parse(pParserGm))
    return false;
  
  _nEdgesGm = pParserGm->getEdgeCount();
  return true;
}

template<typename GR, typename BGR>
inline bool MatchingGraph<GR, BGR>::parse(ParserType* pParser)
{
  if (!pParser->parse())
    return false;

  const std::string& filename = pParser->getFilename();
  int nNodes = pParser->getNodeCount();
  int nEdges = pParser->getEdgeCount();

  if (!lemon::simpleGraph(*pParser->getGraph()))
  {
    std::cerr << "'" << (g_webserver ? "input graph" : filename)
              << "' is not a simple graph." << std::endl;
    return false;
  }
  else if (g_verbosity >= VERBOSE_ESSENTIAL)
  {
    std::cout << "Successfully parsed '"
              << (g_webserver ? "input graph" : filename)
              << "': contains " << nNodes << " nodes and "
              << nEdges << " edges" << std::endl;
  }

  return true;
}

template<typename GR, typename BGR>
inline bool MatchingGraph<GR, BGR>::parse(BpParserType* pBpParser)
{
  if (!pBpParser->parse())
    return false;

  const std::string& filename = pBpParser->getFilename();
  int nEdges = pBpParser->getEdgeCount();

  if (!lemon::simpleGraph(*pBpParser->getGraph()))
  {
    std::cerr << "'" << (g_webserver ? "input graph" : filename)
              << "' is not a simple graph." << std::endl;
    return false;
  }
  else if (g_verbosity >= VERBOSE_ESSENTIAL)
  {
    std::cout << "Successfully parsed '"
              << (g_webserver || filename.empty() ? "matching graph" : filename)
              << "': contains " << nEdges << " matching edges" << std::endl;
  }

  return true;
}

template<typename GR, typename BGR>
inline const std::string& MatchingGraph<GR, BGR>::getLabelGm(BpNode n) const
{
  assert(n != lemon::INVALID && _gm.valid(n));

  if (_gm.red(n))
  {
    Node r = _gmToG12[n];
    return _labelG1[r];
  }
  else
  {
    assert(_gm.blue(n));
    Node b = _gmToG12[n];
    return _labelG2[b];
  }
}

template<typename GR, typename BGR>
inline void MatchingGraph<GR, BGR>::saveG1(std::ostream& out) const
{
  IntNodeMap idMap(_g1);
  for (NodeIt n(_g1); n != lemon::INVALID; ++n)
  {
    idMap[n] = _gm.id(_g1ToGm[n]);
  }

  graphWriter(_g1, out)
    .nodeMap("id", _labelG1)
    .nodeMap("mapToGm", idMap)
    .edgeMap("weight", _weightG1)
    .run();
}

template<typename GR, typename BGR>
inline void MatchingGraph<GR, BGR>::saveG2(std::ostream& out) const
{
  IntNodeMap idMap(_g2);
  for (NodeIt n(_g2); n != lemon::INVALID; ++n)
  {
    idMap[n] = _gm.id(_g2ToGm[n]);
  }

  graphWriter(_g2, out)
    .nodeMap("id", _labelG2)
    .nodeMap("mapToGm", idMap)
    .edgeMap("weight", _weightG2)
    .run();
}

template<typename GR, typename BGR>
inline void MatchingGraph<GR, BGR>::saveGm(std::ostream& out) const
{
  BpStringNodeMap idMap(_gm);

  for (BpNodeIt n(_gm); n != lemon::INVALID; ++n)
  {
    idMap[n] = getLabelGm(n);
  }

  bpGraphWriter(_gm, out)
    .nodeMap("id", idMap)
    .edgeMap("weight", _weightGm)
    .run();
}

template<typename GR, typename BGR>
inline typename GR::Node MatchingGraph<GR, BGR>::getNodeG1(const std::string& label) const
{
  typename InvOrigLabelNodeMap::const_iterator it = _invLabelG1.find(label);
  if (it == _invLabelG1.end())
    return lemon::INVALID;
  else
    return it->second;
}

template<typename GR, typename BGR>
inline typename GR::Node MatchingGraph<GR, BGR>::getNodeG2(const std::string& label) const
{
  typename InvOrigLabelNodeMap::const_iterator it = _invLabelG2.find(label);
  if (it == _invLabelG2.end())
    return lemon::INVALID;
  else
    return it->second;
}

template<typename GR, typename BGR>
inline const std::string& MatchingGraph<GR, BGR>::getLabelG1(Node n) const
{
  assert(n != lemon::INVALID && _g1.valid(n));
  return _labelG1[n];
}

template<typename GR, typename BGR>
inline const std::string& MatchingGraph<GR, BGR>::getLabelG2(Node n) const
{
  assert(n != lemon::INVALID && _g2.valid(n));
  return _labelG2[n];
}

template<typename GR, typename BGR>
inline typename BGR::Node MatchingGraph<GR, BGR>::mapG1ToGm(Node n) const
{
  assert(n != lemon::INVALID && _g1.valid(n));
  return _g1ToGm[n];
}

template<typename GR, typename BGR>
inline typename BGR::Node MatchingGraph<GR, BGR>::mapG2ToGm(Node n) const
{
  assert(n != lemon::INVALID && _g2.valid(n));
  return _g2ToGm[n];
}

template<typename GR, typename BGR>
inline typename GR::Node MatchingGraph<GR, BGR>::mapGmToG1(BpNode n) const
{
  assert(n != lemon::INVALID && _gm.valid(n));

  if (_gm.red(n))
    return _gmToG12[n];
  else
    return lemon::INVALID;
}

template<typename GR, typename BGR>
inline typename GR::Node MatchingGraph<GR, BGR>::mapGmToG2(BpNode n) const
{
  assert(n != lemon::INVALID && _gm.valid(n));

  if (_gm.blue(n))
    return _gmToG12[n];
  else
    return lemon::INVALID;
}

} // namespace gna
} // namespace nina

#endif /* MATCHINGGRAPH_H_ */
