/*
 * bpparser.h
 *
 *  Created on: 08-feb-2012
 *      Author: M. El-Kebir
 */

#ifndef BPPARSER_H_
#define BPPARSER_H_

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <map>
#include <string>
#include <lemon/core.h>
#include "input/parser.h"

namespace nina {
namespace gna {

template<typename GR, typename BGR>
class BpParser
{
public:
  typedef GR Graph;
  typedef BGR BpGraph;

protected:
  TEMPLATE_GRAPH_TYPEDEFS(Graph);

  typedef typename BpGraph::Node BpNode;
  typedef typename BpGraph::Edge BpEdge;
  typedef typename BpGraph::Arc BpArc;
  typedef typename BpGraph::NodeIt BpNodeIt;
  typedef typename BpGraph::EdgeIt BpEdgeIt;
  typedef typename BpGraph::IncEdgeIt BpIncEdgeIt;
  typedef typename BpGraph::RedNode BpRedNode;
  typedef typename BpGraph::BlueNode BpBlueNode;
  typedef typename BpGraph::RedNodeIt BpRedNodeIt;
  typedef typename BpGraph::BlueNodeIt BpBlueNodeIt;

public:
  typedef typename BpGraph::template EdgeMap<double> BpWeightEdgeMap;
  typedef Parser<Graph> ParserType;
  typedef typename Graph::template NodeMap<BpNode> OrigNodeToMatchNodeMap;
  typedef typename BpGraph::template NodeMap<Node> MatchNodeToOrigNodeMap;

protected:
  const std::string _filename;
  BpGraph* _pGm;
  BpWeightEdgeMap* _pWeightEdgeMap;
  BpWeightEdgeMap* _pWeightAltEdgeMap;
  OrigNodeToMatchNodeMap* _pG1ToGm;
  OrigNodeToMatchNodeMap* _pG2ToGm;
  MatchNodeToOrigNodeMap* _pGmToG12;
  int _nNodesRed;
  int _nNodesBlue;
  int _nEdges;
  const ParserType* _pParserG1;
  const ParserType* _pParserG2;

  void constructGm(bool complete);

public:
  BpParser(const std::string& filename,
           const ParserType* pParserG1,
           const ParserType* pParserG2);
  BpParser(const ParserType* pParserG1,
           const ParserType* pParserG2);
  virtual ~BpParser() {}
  virtual bool parse();

  const std::string& getFilename()
  {
    return _filename;
  }

  int getRedNodeCount() const
  {
    return _nNodesRed;
  }

  int getBlueNodeCount() const
  {
    return _nNodesBlue;
  }

  int getEdgeCount() const
  {
    return _nEdges;
  }

  const BpGraph* getGraph() const
  {
    return _pGm;
  }

  BpGraph* getGraph()
  {
    return _pGm;
  }

  void setGraph(BpGraph* pGm)
  {
    _pGm = pGm;
  }

  const BpWeightEdgeMap* getWeightEdgeMap() const
  {
    return _pWeightEdgeMap;
  }

  BpWeightEdgeMap* getWeightEdgeMap()
  {
    return _pWeightEdgeMap;
  }

  void setWeightEdgeMap(BpWeightEdgeMap* pWeightEdgeMap)
  {
    _pWeightEdgeMap = pWeightEdgeMap;
  }

  const BpWeightEdgeMap* getWeightAltEdgeMap() const
  {
    return _pWeightAltEdgeMap;
  }

  BpWeightEdgeMap* getWeightAltEdgeMap()
  {
    return _pWeightAltEdgeMap;
  }

  void setWeightAltEdgeMap(BpWeightEdgeMap* pWeightAltEdgeMap)
  {
    _pWeightAltEdgeMap = pWeightAltEdgeMap;
  }

  const OrigNodeToMatchNodeMap* getG1ToGmMap() const
  {
    return _pG1ToGm;
  }

  OrigNodeToMatchNodeMap* getG1ToGmMap()
  {
    return _pG1ToGm;
  }

  void setG1ToGmMap(OrigNodeToMatchNodeMap* pG1ToGm)
  {
    _pG1ToGm = pG1ToGm;
  }

  const OrigNodeToMatchNodeMap* getG2ToGmMap() const
  {
    return _pG2ToGm;
  }

  OrigNodeToMatchNodeMap* getG2ToGmMap()
  {
    return _pG2ToGm;
  }

  void setG2ToGmMap(OrigNodeToMatchNodeMap* pG2ToGm)
  {
    _pG2ToGm = pG2ToGm;
  }

  const MatchNodeToOrigNodeMap* getGmToG12Map() const
  {
    return _pGmToG12;
  }

  MatchNodeToOrigNodeMap* getGmToG12Map()
  {
    return _pGmToG12;
  }

  void setGmToG12Map(MatchNodeToOrigNodeMap* pGmToG12)
  {
    _pGmToG12 = pGmToG12;
  }
};

template<typename GR, typename BGR>
inline BpParser<GR, BGR>::BpParser(const std::string& filename,
                                   const ParserType* pParserG1,
                                   const ParserType* pParserG2)
  : _filename(filename)
  , _pGm(NULL)
  , _pWeightEdgeMap(NULL)
  , _pWeightAltEdgeMap(NULL)
  , _pG1ToGm(NULL)
  , _pG2ToGm(NULL)
  , _pGmToG12(NULL)
  , _nNodesRed(0)
  , _nNodesBlue(0)
  , _nEdges(0)
  , _pParserG1(pParserG1)
  , _pParserG2(pParserG2)
{
}

template<typename GR, typename BGR>
inline BpParser<GR, BGR>::BpParser(const ParserType* pParserG1,
                                   const ParserType* pParserG2)
  : _filename()
  , _pGm(NULL)
  , _pWeightEdgeMap(NULL)
  , _pWeightAltEdgeMap(NULL)
  , _pG1ToGm(NULL)
  , _pG2ToGm(NULL)
  , _pGmToG12(NULL)
  , _nNodesRed(0)
  , _nNodesBlue(0)
  , _nEdges(0)
  , _pParserG1(pParserG1)
  , _pParserG2(pParserG2)
{
}

template<typename GR, typename BGR>
inline bool BpParser<GR, BGR>::parse()
{
  constructGm(true);
  return true;
}

template<typename GR, typename BGR>
inline void BpParser<GR, BGR>::constructGm(bool complete)
{
  assert(_pGm);
  assert(_pG1ToGm);
  assert(_pG2ToGm);
  assert(_pGmToG12);
  assert(_pParserG1);
  assert(_pParserG2);

  const Graph* pG1 = _pParserG1->getGraph();
  const Graph* pG2 = _pParserG2->getGraph();

  assert(pG1 && pG2);

  // construct matching graph:
  // - red nodes correspond to nodes in G1
  // - blue nodes correspond to nodes in G2
  _pGm->clear();
  _nEdges = 0;
  _nNodesRed = _pParserG1->getNodeCount();
  _nNodesBlue = _pParserG2->getNodeCount();

  _pGm->reserveNode(_nNodesRed + _nNodesBlue);

  // construct the mappings
  for (NodeIt n(*pG1); n != lemon::INVALID; ++n)
  {
    BpRedNode r = _pGm->addRedNode();
    _pG1ToGm->set(n, r);
    _pGmToG12->set(r, n);
  }
  for (NodeIt n(*pG2); n != lemon::INVALID; ++n)
  {
    BpBlueNode b = _pGm->addBlueNode();
    _pG2ToGm->set(n, b);
    _pGmToG12->set(b, n);
  }

  if (complete)
  {
    // add all edges
    _nEdges = _nNodesRed * _nNodesBlue;
    _pGm->reserveEdge(_nEdges);

    for (BpRedNodeIt r(*_pGm); r != lemon::INVALID; ++r)
    {
      for (BpBlueNodeIt b(*_pGm); b != lemon::INVALID; ++b)
      {
        BpEdge e = _pGm->addEdge(r, b);
        if (_pWeightEdgeMap) _pWeightEdgeMap->set(e, 0);
      }
    }
  }
}

} // namespace gna
} // namespace nina

#endif /* BPPARSER_H_ */
