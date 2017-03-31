/*
 * bplgfparser.h
 *
 *  Created on: 16-feb-2012
 *      Author: M. El-Kebir
 */

#ifndef BPLGFPARSER_H_
#define BPLGFPARSER_H_

#include <lemon/lgf_reader.h>
#include "input/bpparser.h"

namespace nina {
namespace gna {

template<typename GR, typename BGR>
class BpLgfParser : public BpParser<GR, BGR>
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
  typedef typename BpGraph::template NodeMap<std::string> BpStringNodeMap;

  using Parent::_filename;
  using Parent::_pGm;
  using Parent::_pWeightEdgeMap;
  using Parent::_pG1ToGm;
  using Parent::_pG2ToGm;
  using Parent::_pGmToG12;
  using Parent::_nNodesRed;
  using Parent::_nNodesBlue;
  using Parent::_nEdges;
  using Parent::_pParserG1;
  using Parent::_pParserG2;
  using Parent::constructGm;

public:
  BpLgfParser(const std::string& filename,
              const ParserType* pParserG1,
              const ParserType* pParserG2);
  bool parse();
};

template<typename GR, typename BGR>
inline BpLgfParser<GR, BGR>::BpLgfParser(const std::string& filename,
                                         const ParserType* pParserG1,
                                         const ParserType* pParserG2)
  : Parent(filename, pParserG1, pParserG2)
{
}

template<typename GR, typename BGR>
inline bool BpLgfParser<GR, BGR>::parse()
{
  if (!_pGm)
    return false;

  _pGm->clear();
  if (_filename.empty())
  {
    constructGm(true);
    return true;
  }
  else
  {
    //constructGm(false);

    std::ifstream inFile(_filename.c_str());
    if (!inFile.good())
    {
      std::cerr << "Could not open file " << _filename
                << " for reading" << std::endl;
      return false;
    }

    const typename ParserType::InvIdNodeMap* pInvLabelG1 =
            _pParserG1->getInvIdNodeMap();
    const typename ParserType::InvIdNodeMap* pInvLabelG2 =
            _pParserG2->getInvIdNodeMap();

    BpStringNodeMap idMap(*_pGm);

    bpGraphReader(*_pGm, inFile)
      .nodeMap("id", idMap)
      .edgeMap("weight", *_pWeightEdgeMap)
      .run();

    for (BpNodeIt n(*_pGm); n != lemon::INVALID; ++n)
    {
      if (_pGm->red(n))
      {
        typename ParserType::InvIdNodeMap::const_iterator invIt1 = pInvLabelG1->find(idMap[n]);
        if (invIt1 == pInvLabelG1->end())
        {
          std::cerr << "Error: there is no node in G_1 labeled by "
                    << idMap[n] << std::endl;
          return false;
        }

        Node nodeG1 = invIt1->second;
        _pG1ToGm->set(nodeG1, n);
        _pGmToG12->set(n, nodeG1);
      }
      else
      {
        typename ParserType::InvIdNodeMap::const_iterator invIt2 = pInvLabelG2->find(idMap[n]);
        if (invIt2 == pInvLabelG2->end())
        {
          std::cerr << "Error: there is no node in G_2 labeled by "
                    << idMap[n] << std::endl;
          return false;
        }

        assert(_pGm->blue(n));
        Node nodeG2 = invIt2->second;
        _pG2ToGm->set(nodeG2, n);
        _pGmToG12->set(n, nodeG2);
      }
    }

    _nEdges = 0;
    for (BpEdgeIt e(*_pGm); e != lemon::INVALID; ++e)
    {
      _nEdges++;
    }
  }

  return true;
}

} // namespace gna
} // namespace nina

#endif // BPLGFPARSER_H_
