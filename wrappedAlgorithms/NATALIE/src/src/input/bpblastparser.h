/*
 * bpblastparser.h
 *
 *  Created on: 16-feb-2012
 *      Author: M. El-Kebir
 */

#ifndef BPBLASTPARSER_H_
#define BPBLASTPARSER_H_

#include "input/bpparser.h"
#include "verbose.h"

namespace nina {
namespace gna {

template<typename GR, typename BGR>
class BpBlastParser : public BpParser<GR, BGR>
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

  using Parent::_filename;
  using Parent::_pGm;
  using Parent::_pWeightEdgeMap;
  using Parent::_pWeightAltEdgeMap;
  using Parent::_pG1ToGm;
  using Parent::_pG2ToGm;
  using Parent::_pGmToG12;
  using Parent::_nNodesRed;
  using Parent::_nNodesBlue;
  using Parent::_nEdges;
  using Parent::_pParserG1;
  using Parent::_pParserG2;
  using Parent::constructGm;

private:
  double _eValCutOff;

public:
  BpBlastParser(const std::string& filename,
                const ParserType* pParserG1,
                const ParserType* pParserG2,
                double eValCutOff);
  bool parse();

  double getEValCutOff() const
  {
    return _eValCutOff;
  }
};

template<typename GR, typename BGR>
inline BpBlastParser<GR, BGR>::BpBlastParser(const std::string& filename,
                                             const ParserType* pParserG1,
                                             const ParserType* pParserG2,
                                             double eValCutOff)
  : Parent(filename, pParserG1, pParserG2)
  , _eValCutOff(eValCutOff)
{
}

template<typename GR, typename BGR>
inline bool BpBlastParser<GR, BGR>::parse()
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

    const typename ParserType::InvIdNodeMap* pInvLabelG1 =
            _pParserG1->getInvIdNodeMap();
    const typename ParserType::InvIdNodeMap* pInvLabelG2 =
            _pParserG2->getInvIdNodeMap();

    std::string line;
    while (std::getline(inFile, line))
    {
      if (line[0] == 0)
        continue; // skip comments

      // queryId, subjectId, percIdentity, alnLength,
      // mismatchCount, gapOpenCount, queryStart, queryEnd,
      // subjectStart, subjectEnd, eVal, bitScore
      double percIdentity, alnLength, mismatchCount,
        gapOpenCount, queryStart, queryEnd,
        subjectStart, subjectEnd, eVal, bitScore;

      std::stringstream lineStream(line);

      std::string label_g1, label_g2;
      lineStream >> label_g1 >> label_g2
        >> percIdentity >> alnLength >> mismatchCount
        >> gapOpenCount >> queryStart >> queryEnd
        >> subjectStart >> subjectEnd >> eVal >> bitScore;

      typename ParserType::InvIdNodeMap::const_iterator invIt1 = pInvLabelG1->find(label_g1);
      typename ParserType::InvIdNodeMap::const_iterator invIt2 = pInvLabelG2->find(label_g2);
      if (invIt1 != pInvLabelG1->end() &&
          invIt2 != pInvLabelG2->end() &&
          eVal <= _eValCutOff)
      {
        const Node node1 = invIt1->second;
        const BpRedNode r = _pGm->asRedNode((*_pG1ToGm)[node1]);
        const Node node2 = invIt2->second;
        const BpBlueNode b = _pGm->asBlueNode((*_pG2ToGm)[node2]);

        BpEdge e = arcLookUp(r, b);
        if (e == lemon::INVALID)
        {
          e = _pGm->addEdge(r, b);
          if (_pWeightEdgeMap)
          {
            _pWeightEdgeMap->set(e, bitScore);
          }
          if (_pWeightAltEdgeMap)
          {
            _pWeightAltEdgeMap->set(e, eVal);
          }
          _nEdges++;
        }
        else
        {
          if (_pWeightEdgeMap)
          {
            (*_pWeightEdgeMap)[e] = std::max((*_pWeightEdgeMap)[e], bitScore);
          }
          if (_pWeightAltEdgeMap)
          {
            (*_pWeightAltEdgeMap)[e] = std::min((*_pWeightAltEdgeMap)[e], eVal);
          }
        }
      }
    }
  }

  return true;
}

} // namespace gna
} // namespace nina

#endif // BPBLASTPARSER_H_
