/*
 * bpcandlistparser.h
 *
 *  Created on: 10-feb-2012
 *      Author: M. El-Kebir
 */

#ifndef BPCANDLISTPARSER_H_
#define BPCANDLISTPARSER_H_

#include "input/bpparser.h"

namespace nina {
namespace gna {

template<typename GR, typename BGR>
class BpCandListParser : public BpParser<GR, BGR>
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

  double _edgeSimScore;

public:
  typedef typename Parent::BpWeightEdgeMap BpWeightEdgeMap;
  typedef typename Parent::ParserType ParserType;
  typedef typename Parent::OrigNodeToMatchNodeMap OrigNodeToMatchNodeMap;
  typedef typename Parent::MatchNodeToOrigNodeMap MatchNodeToOrigNodeMap;

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
  BpCandListParser(const std::string& filename,
                   const ParserType* pParserG1,
                   const ParserType* pParserG2);
  bool parse();

  double getEdgeSimScore() const
  {
    return _edgeSimScore;
  }
};

template<typename GR, typename BGR>
inline BpCandListParser<GR, BGR>::BpCandListParser(const std::string& filename,
                                                   const ParserType* pParserG1,
                                                   const ParserType* pParserG2)
  : Parent(filename, pParserG1, pParserG2)
  , _edgeSimScore(0)
{
}

template<typename GR, typename BGR>
inline bool BpCandListParser<GR, BGR>::parse()
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
    constructGm(false);

    lemon::DynArcLookUp<BpGraph> arcLookUp(*_pGm);

    std::ifstream inFile(_filename.c_str());
    if (!inFile.good())
    {
      std::cerr << "Could not open file " << _filename
                << " for reading" << std::endl;
      return false;
    }

    //inFile >> _edgeSimScore;

    const typename ParserType::InvIdNodeMap* pInvLabelG1 =
            _pParserG1->getInvIdNodeMap();
    const typename ParserType::InvIdNodeMap* pInvLabelG2 =
            _pParserG2->getInvIdNodeMap();

    std::string line;
    while (std::getline(inFile, line))
    {
      if (line == "")
        continue;

      // every line defines edges between the first label (which is in G_1)
      // and the remaining labels (which are in G_2)
      std::stringstream lineStream(line);

      std::string label_g1, label_g2;
      lineStream >> label_g1;

      typename ParserType::InvIdNodeMap::const_iterator invIt1 = pInvLabelG1->find(label_g1);
      if (invIt1 == pInvLabelG1->end())
      {
        std::cerr << "Error: there is no node in G_1 labeled by "
                  << label_g1 << std::endl;
        return false;
      }

      const Node node1 = invIt1->second;
      const BpRedNode r = _pGm->asRedNode((*_pG1ToGm)[node1]);
      while (lineStream.good())
      {
        lineStream >> label_g2;

        typename ParserType::InvIdNodeMap::const_iterator invIt2 = pInvLabelG2->find(label_g2);
        if (invIt2 == pInvLabelG2->end())
        {
          std::cerr << "Error: there is no node in G_2 labeled by "
                    << label_g2 << std::endl;
          return false;
        }
        else
        {
          const Node node2 = invIt2->second;
          const BpBlueNode b = _pGm->asBlueNode((*_pG2ToGm)[node2]);

          if (arcLookUp(r, b) != lemon::INVALID)
          {
            if (g_verbosity >= VERBOSE_DEBUG)
              std::cout << "Warning: multiple edge ('"
                        << label_g1 << "','" << label_g2
                        << "') in '"  << _filename << "'. Skipped." << std::endl;
          }
          else
          {
            _pGm->addEdge(r, b);
            _nEdges++;
          }
        }
      }
    }

  }

  if (_pWeightEdgeMap)
    mapFill(*_pGm, *_pWeightEdgeMap, _edgeSimScore);

  return true;
}

} // namespace gna
} // namespace nina

#endif // BPCANDLISTPARSER_H_
