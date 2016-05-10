/*
 * identityparser.h
 *
 *  Created on: 18-feb-2012
 *      Author: M. El-Kebir
 */

#ifndef IDENTITYPARSER_H_
#define IDENTITYPARSER_H_

#include "input/parser.h"

namespace nina
{

template<typename GR>
class IdentityParser : public Parser<GR>
{
public:
  /// Graph type
  typedef GR Graph;
  /// Base class type
  typedef Parser<GR> Parent;

private:
  TEMPLATE_GRAPH_TYPEDEFS(Graph);

public:
  typedef typename Parent::InvIdNodeMap InvIdNodeMap;
  typedef typename Parent::IdNodeMap IdNodeMap;
  typedef typename Parent::WeightNodeMap WeightNodeMap;
  typedef typename Parent::WeightEdgeMap WeightEdgeMap;
  typedef typename Graph::template NodeMap<Node> NodeRefMap;
  typedef typename Graph::template NodeMap<Node> NodeCrossRefMap;

  using Parent::_filename;
  using Parent::_pG;
  using Parent::_pIdNodeMap;
  using Parent::_pInvIdNodeMap;
  using Parent::_pWeightNodeMap;
  using Parent::_pWeightEdgeMap;
  using Parent::_nNodes;
  using Parent::_nEdges;

private:
  const Graph& _sourceG;
  const WeightNodeMap* _pSourceWeightNodeMap;
  const WeightEdgeMap* _pSourceWeightEdgeMap;
  const IdNodeMap* _pSourceIdNodeMap;
  /// Source to target
  NodeRefMap _nodeRefMap;
  /// Target to source
  NodeCrossRefMap* _pNodeCrossRefMap;

public:
  IdentityParser(const Graph& sourceG,
                 const WeightNodeMap* pWeightNodeMap,
                 const WeightEdgeMap* pWeightEdgeMap,
                 const IdNodeMap* pIdNodeMap);
  ~IdentityParser();
  bool parse();

  const NodeRefMap& getNodeRefMap() const
  {
    return _nodeRefMap;
  }

  const NodeCrossRefMap& getNodeCrossRefMap() const
  {
    assert(_pNodeCrossRefMap);
    return _pNodeCrossRefMap;
  }
};

template<typename GR>
inline IdentityParser<GR>::IdentityParser(const Graph& sourceG,
                                          const WeightNodeMap* pWeightNodeMap,
                                          const WeightEdgeMap* pWeightEdgeMap,
                                          const IdNodeMap* pIdNodeMap)
  : Parent(std::string())
  , _sourceG(sourceG)
  , _pSourceWeightNodeMap(pWeightNodeMap)
  , _pSourceWeightEdgeMap(pWeightEdgeMap)
  , _pSourceIdNodeMap(pIdNodeMap)
  , _nodeRefMap(_sourceG)
  , _pNodeCrossRefMap(NULL)
{
}

template<typename GR>
inline IdentityParser<GR>::~IdentityParser()
{
  delete _pNodeCrossRefMap;
}

template<typename GR>
inline bool IdentityParser<GR>::parse()
{
  if (!_pG)
    return false;

  _pG->clear();

  lemon::GraphCopy<Graph, Graph> copy(_sourceG, *_pG);
  copy.nodeRef(_nodeRefMap);

  delete _pNodeCrossRefMap;
  _pNodeCrossRefMap = new NodeCrossRefMap(*_pG);
  copy.nodeCrossRef(*_pNodeCrossRefMap);

  if (_pSourceIdNodeMap && _pIdNodeMap)
  {
    copy.nodeMap(*_pSourceIdNodeMap, *_pIdNodeMap);
  }

  if (_pSourceWeightNodeMap && _pWeightNodeMap)
  {
    copy.nodeMap(*_pSourceWeightNodeMap, *_pWeightNodeMap);
  }

  if (_pSourceWeightEdgeMap && _pWeightEdgeMap)
  {
    copy.edgeMap(*_pSourceWeightEdgeMap, *_pWeightEdgeMap);
  }

  copy.run();

  _nNodes = lemon::countNodes(*_pG);
  _nEdges = lemon::countEdges(*_pG);

  if (_pIdNodeMap && _pInvIdNodeMap)
  {
    if (_pSourceIdNodeMap)
    {
      for (NodeIt n(*_pG); n != lemon::INVALID; ++n)
      {
        (*_pInvIdNodeMap)[(*_pIdNodeMap)[n]] = n;
      }
    }
    else
    {
      for (NodeIt n(*_pG); n != lemon::INVALID; ++n)
      {
        int id = _pG->id(n);
        char buf[128];
        sprintf(buf, "%d", id);

        std::string str(buf);

        _pIdNodeMap->set(n, str);
        (*_pInvIdNodeMap)[str] = n;
      }
    }
  }

  return true;
}

} // namespace nina

#endif // IDENTITYPARSER_H_
