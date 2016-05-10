/*
 * matchinggraphshuffle.h
 *
 *  Created on: 21-mar-2013
 *      Author: M. El-Kebir
 */

#ifndef MATCHINGGRAPHSHUFFLE_H
#define MATCHINGGRAPHSHUFFLE_H

#include <vector>
#include <tr1/random>
#include "input/matchinggraph.h"

namespace nina {
namespace gna {

template<typename GR, typename BGR>
class MatchingGraphShuffle : public MatchingGraph<GR, BGR>
{
public:
  /// The graph type of the input graphs
  typedef GR Graph;
  /// The graph type of the bipartite matching graph
  typedef BGR BpGraph;
  /// Parent type
  typedef MatchingGraph<GR, BGR> Parent;

protected:
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

  /// Labels of the nodes
  typedef typename Parent::OrigLabelNodeMap OrigLabelNodeMap;
  /// Mapping from original nodes to matching nodes
  typedef typename Parent::OrigNodeToMatchNodeMap OrigNodeToMatchNodeMap;
  /// Mapping from matching nodes to original nodes
  typedef typename Parent::MatchNodeToOrigNodeMap MatchNodeToOrigNodeMap;
  /// Mapping from matching nodes to original nodes
  typedef typename Parent::BpStringNodeMap BpStringNodeMap;
  /// Mapping from labels to original nodes
  typedef typename Parent::InvOrigLabelNodeMap InvOrigLabelNodeMap;
  /// Weights on original edges
  typedef typename Parent::WeightEdgeMap WeightEdgeMap;
  /// Weights on matching edges
  typedef typename Parent::BpWeightEdgeMap BpWeightEdgeMap;
  /// Parser type
  typedef typename Parent::ParserType ParserType;
  /// Bipartite parser type
  typedef typename Parent::BpParserType BpParserType;
  /// Node vector type
  typedef typename std::vector<Node> NodeVector;

  using Parent::_gm;
  using Parent::_g1;
  using Parent::_g2;
  using Parent::_labelG1;
  using Parent::_labelG2;
  using Parent::_invLabelG1;
  using Parent::_invLabelG2;
  using Parent::_g1ToGm;
  using Parent::_g2ToGm;
  using Parent::_gmToG12;
  using Parent::_weightG1;
  using Parent::_weightG2;
  using Parent::_weightGm;
  using Parent::_weightAltGm;
  using Parent::_nEdgesGm;
  using Parent::_nNodesG1;
  using Parent::_nNodesG2;
  using Parent::_nEdgesG1;
  using Parent::_nEdgesG2;
  using Parent::_arcLookUpG1;
  using Parent::_arcLookUpG2;

  /// G_1 node vector
  NodeVector _g1Vec;
  /// G_2 node vector
  NodeVector _g2Vec;
  /// Mapping from original nodes in \f$G_1\f$ to matching nodes (shuffled)
  OrigNodeToMatchNodeMap _g1ToGmShuffled;
  /// Mapping from original nodes in \f$G_2\f$ to matching nodes (shuffled)
  OrigNodeToMatchNodeMap _g2ToGmShuffled;
  /// Mapping from matching nodes to original nodes (shuffled)
  MatchNodeToOrigNodeMap _gmToG12Shuffled;
  /// Random number generator
  std::tr1::mt19937 _eng;

public:
  /// Constructor
  MatchingGraphShuffle();
  /// Virtual destructor
  ~MatchingGraphShuffle() {}
  /// Initialize G_1, G_2 and G_m
  virtual bool init(ParserType* pParserG1,
                    ParserType* pParserG2,
                    BpParserType* pParserGm);
  virtual BpNode mapG1ToGm(Node n) const;
  /// Return node in \f$G_m\f$ corresponding to node n in \f$G_2\f$
  virtual BpNode mapG2ToGm(Node n) const;
  /// Return node in \f$G_1\f$ corresponding to node n in \f$G_m\f$
  virtual Node mapGmToG1(BpNode n) const;
  /// Return node in \f$G_2\f$ corresponding to node n in \f$G_m\f$
  virtual Node mapGmToG2(BpNode n) const;
  /// Shuffle
  void shuffle();
  /// Unshuffle
  void unshuffle();
  /// Set seed
  void setSeed(unsigned int seed) { _eng.seed(seed); }
};

template<typename GR, typename BGR>
inline MatchingGraphShuffle<GR, BGR>::MatchingGraphShuffle()
  : Parent()
  , _g1Vec()
  , _g2Vec()
  , _g1ToGmShuffled(_g1)
  , _g2ToGmShuffled(_g2)
  , _gmToG12Shuffled(_gm)
  , _eng()
{
}

template<typename GR, typename BGR>
inline bool MatchingGraphShuffle<GR, BGR>::init(ParserType* pParserG1,
                                                ParserType* pParserG2,
                                                BpParserType* pParserGm)
{
  if (!Parent::init(pParserG1, pParserG2, pParserGm))
    return false;

  _g1Vec.resize(_nNodesG1, lemon::INVALID);
  size_t idx = 0;
  for (NodeIt i(_g1); i != lemon::INVALID; ++i, idx++)
  {
    _g1Vec[idx] = i;
  }

  _g2Vec.resize(_nNodesG2, lemon::INVALID);
  idx = 0;
  for (NodeIt k(_g2); k != lemon::INVALID; ++k, idx++)
  {
    _g2Vec[idx] = k;
  }

  unshuffle();

  return true;
}

template<typename GR, typename BGR>
inline void MatchingGraphShuffle<GR, BGR>::unshuffle()
{
  lemon::mapCopy(_g1, _g1ToGm, _g1ToGmShuffled);
  lemon::mapCopy(_g2, _g2ToGm, _g2ToGmShuffled);
  lemon::mapCopy(_gm, _gmToG12, _gmToG12Shuffled);
}

template<typename GR, typename BGR>
inline void MatchingGraphShuffle<GR, BGR>::shuffle()
{
  // shuffle g1 to gm mapping first
  size_t n1 = static_cast<size_t>(_nNodesG1);
  for (size_t i = n1 - 1; i >= 1; i--)
  {
    std::tr1::uniform_int<size_t> unif(0, i);
    size_t j = unif(_eng);
    std::swap(_g1ToGmShuffled[_g1Vec[i]], _g1ToGmShuffled[_g1Vec[j]]);
  }

  // shuffle g2 to gm mapping
  size_t n2 = static_cast<size_t>(_nNodesG2);
  for (size_t i = n2 - 1; i >= 1; i--)
  {
    std::tr1::uniform_int<size_t> unif(0, i);
    size_t j = unif(_eng);
    std::swap(_g2ToGmShuffled[_g2Vec[i]], _g2ToGmShuffled[_g2Vec[j]]);
  }

  // update gmtog12 mapping
  for (NodeIt i(_g1); i != lemon::INVALID; ++i)
  {
    _gmToG12Shuffled[_g1ToGmShuffled[i]] = i;
  }

  for (NodeIt k(_g2); k != lemon::INVALID; ++k)
  {
    _gmToG12Shuffled[_g2ToGmShuffled[k]] = k;
  }
}

template<typename GR, typename BGR>
inline typename MatchingGraphShuffle<GR, BGR>::BpNode
MatchingGraphShuffle<GR, BGR>::mapG1ToGm(Node n) const
{
  assert(n != lemon::INVALID && _g1.valid(n));
  return _g1ToGmShuffled[n];
}

template<typename GR, typename BGR>
inline typename MatchingGraphShuffle<GR, BGR>::BpNode
MatchingGraphShuffle<GR, BGR>::mapG2ToGm(Node n) const
{
  assert(n != lemon::INVALID && _g2.valid(n));
  return _g2ToGmShuffled[n];
}

template<typename GR, typename BGR>
inline typename GR::Node MatchingGraphShuffle<GR, BGR>::mapGmToG1(BpNode n) const
{
  assert(n != lemon::INVALID && _gm.valid(n));

  if (_gm.red(n))
    return _gmToG12Shuffled[n];
  else
    return lemon::INVALID;
}

template<typename GR, typename BGR>
inline typename GR::Node MatchingGraphShuffle<GR, BGR>::mapGmToG2(BpNode n) const
{
  assert(n != lemon::INVALID && _gm.valid(n));

  if (_gm.blue(n))
    return _gmToG12Shuffled[n];
  else
    return lemon::INVALID;
}

} // namespace gna
} // namespace nina

#endif // MATCHINGGRAPHSHUFFLE_H
