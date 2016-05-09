/*
 * bpidentityparser.h
 *
 *  Created on: 18-feb-2012
 *      Author: M. El-Kebir
 */

#ifndef BPIDENTITYPARSER_H_
#define BPIDENTITYPARSER_H_

#include "input/bpparser.h"
#include "input/identityparser.h"

namespace nina {
namespace gna {

template<typename GR, typename BGR>
class BpIdentityParser : public BpParser<GR, BGR>
{
public:
  /// Graph type
  typedef GR Graph;
  /// Bipartite graph type
  typedef BGR BpGraph;
  /// Base class type
  typedef BpParser<GR, BGR> Parent;

private:
  TEMPLATE_GRAPH_TYPEDEFS(Graph);

  typedef typename Parent::BpNode BpNode;
  typedef typename Parent::BpEdge BpEdge;
  typedef typename Parent::BpArc BpArc;
  typedef typename Parent::BpNodeIt BpNodeIt;
  typedef typename Parent::BpEdgeIt BpEdgeIt;
  typedef typename Parent::BpIncEdgeIt BpIncEdgeIt;
  typedef typename Parent::BpRedNode BpRedNode;
  typedef typename Parent::BpBlueNode BpBlueNode;
  typedef typename Parent::BpRedNodeIt BpRedNodeIt;
  typedef typename Parent::BpBlueNodeIt BpBlueNodeIt;

public:
  typedef typename Parent::BpWeightEdgeMap BpWeightEdgeMap;
  typedef typename Parent::ParserType ParserType;
  typedef typename Parent::OrigNodeToMatchNodeMap OrigNodeToMatchNodeMap;
  typedef typename Parent::MatchNodeToOrigNodeMap MatchNodeToOrigNodeMap;
  typedef IdentityParser<Graph> IdentityParserType;
  typedef typename IdentityParserType::NodeRefMap NodeRefMap;
  typedef typename IdentityParserType::NodeCrossRefMap NodeCrossRefMap;
  typedef typename BpGraph::template NodeMap<BpNode> BpNodeRefMap;
  typedef typename BpGraph::template EdgeMap<BpEdge> BpEdgeRefMap;
  typedef typename BpGraph::template NodeMap<BpNode> BpNodeCrossRefMap;

  using Parent::_filename;
  using Parent::_pGm;
  using Parent::_pWeightEdgeMap;
  using Parent::_pG1ToGm;
  using Parent::_pG2ToGm;
  using Parent::_pGmToG12;
  using Parent::_nNodesRed;
  using Parent::_nNodesBlue;
  using Parent::_nEdges;
  using Parent::constructGm;

private:
  const BpGraph& _sourceGm;
  const MatchNodeToOrigNodeMap& _sourceGmToG12;
  const BpWeightEdgeMap* _pSourceWeightEdgeMap;
  const IdentityParserType* _pIdentityParserG1;
  const IdentityParserType* _pIdentityParserG2;
  BpNodeRefMap _nodeRefMap;
  BpEdgeRefMap _edgeRefMap;
  BpNodeCrossRefMap* _pNodeCrossRefMap;

public:
  BpIdentityParser(const BpGraph& sourceGm,
                   const MatchNodeToOrigNodeMap& sourceGmToG12,
                   const BpWeightEdgeMap* pSourceWeightEdgeMap,
                   const IdentityParserType* pParserG1,
                   const IdentityParserType* pParserG2);

  bool parse();

  BpEdge map(BpEdge org) const
  {
    assert(org != lemon::INVALID);
    return _edgeRefMap[org];
  }
};

template<typename GR, typename BGR>
inline BpIdentityParser<GR, BGR>::
    BpIdentityParser(const BpGraph& sourceGm,
                     const MatchNodeToOrigNodeMap& sourceGmToG12,
                     const BpWeightEdgeMap* pSourceWeightEdgeMap,
                     const IdentityParserType* pParserG1,
                     const IdentityParserType* pParserG2)
  : Parent(pParserG1, pParserG2)
  , _sourceGm(sourceGm)
  , _sourceGmToG12(sourceGmToG12)
  , _pSourceWeightEdgeMap(pSourceWeightEdgeMap)
  , _pIdentityParserG1(pParserG1)
  , _pIdentityParserG2(pParserG2)
  , _nodeRefMap(_sourceGm)
  , _edgeRefMap(_sourceGm)
  , _pNodeCrossRefMap(NULL)
{
}

template<typename GR, typename BGR>
inline bool BpIdentityParser<GR, BGR>::parse()
{
  if (!_pGm || !_pG1ToGm || !_pG2ToGm || !_pGmToG12)
    return false;

  _pGm->clear();

  lemon::BpGraphCopy<BpGraph, BpGraph> copy(_sourceGm, *_pGm);
  copy.nodeRef(_nodeRefMap);
  copy.edgeRef(_edgeRefMap);

  delete _pNodeCrossRefMap;
  _pNodeCrossRefMap = new BpNodeCrossRefMap(*_pGm);
  copy.nodeCrossRef(*_pNodeCrossRefMap);

  if (_pSourceWeightEdgeMap && _pWeightEdgeMap)
  {
    copy.edgeMap(*_pSourceWeightEdgeMap, *_pWeightEdgeMap);
  }

  copy.run();

  _nNodesRed = 0;
  const NodeRefMap& nodeRefG1 = _pIdentityParserG1->getNodeRefMap();
  for (BpRedNodeIt sourceBpNode(_sourceGm);
       sourceBpNode != lemon::INVALID; ++sourceBpNode)
  {
    BpNode targetBpNode = _nodeRefMap[sourceBpNode];

    Node sourceNode = _sourceGmToG12[sourceBpNode];
    Node targetNode = nodeRefG1[sourceNode];

    _pG1ToGm->set(targetNode, targetBpNode);
    _pGmToG12->set(targetBpNode, targetNode);

    _nNodesRed++;
  }

  _nNodesBlue = 0;
  const NodeRefMap& nodeRefG2 = _pIdentityParserG2->getNodeRefMap();
  for (BpBlueNodeIt sourceBpNode(_sourceGm);
       sourceBpNode != lemon::INVALID; ++sourceBpNode)
  {
    BpNode targetBpNode = _nodeRefMap[sourceBpNode];

    Node sourceNode = _sourceGmToG12[sourceBpNode];
    Node targetNode = nodeRefG2[sourceNode];

    _pG2ToGm->set(targetNode, targetBpNode);
    _pGmToG12->set(targetBpNode, targetNode);

    _nNodesBlue++;
  }

  _nEdges = 0;
  for (BpEdgeIt e(_sourceGm); e != lemon::INVALID; ++e)
    _nEdges++;

  return true;
}

} // namespace gna
} // namespace nina

#endif // BPIDENTITYPARSER_H_
