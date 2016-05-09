/*
 * lagrangegnasi.h
 *
 *  Created on: 9-mar-2011
 *      Author: M. El-Kebir
 */

#ifndef LAGRANGEGNASI_H_
#define LAGRANGEGNASI_H_

#include <lemon/core.h>
#include <lemon/bp_matching.h>
#include <lemon/adaptors.h>
#include <limits>
#include "lagrange/lagrangegna.h"
#include "verbose.h"

namespace nina {
namespace gna {

/// The number of dual variables is:
/// \f$O(|V_1||V_2|^2 |V_2||V_1|^2)\f$.
///
/// Note that only variables whose wei
template<typename GR, typename BGR>
class LagrangeGnaSi : public virtual LagrangeGna<GR, BGR>
{
public:
  /// The graph type of the input graphs
  typedef GR Graph;
  /// The graph type of the bipartite matching graph
  typedef BGR BpGraph;
  /// Base class type
  typedef LagrangeGna<GR, BGR> Parent;
  /// Type of a map assigning a boolean to every matching edge
  typedef typename Parent::BpMatchingMap BpMatchingMap;
  /// Type of a matching map: maps a node to its matching edge
  typedef typename Parent::BpBoolMap BpBoolMap;
  /// Type of the matching graph
  typedef typename Parent::MatchingGraphType MatchingGraphType;
  /// Type of the score model
  typedef typename Parent::ScoreModelType ScoreModelType;

  using Parent::_matchingGraph;
  using Parent::_scoreModel;
  using Parent::_gm;
  using Parent::_defX;
  using Parent::_defY;
  using Parent::_x;
  using Parent::_y;
  using Parent::_idxMapX;
  using Parent::_yIdx2xIdx;
  using Parent::_primalValue;
  using Parent::_feasiblePrimalValue;
  using Parent::_pGlobal;
  using Parent::_integral;
  using Parent::getOrgEdge;
  using Parent::getOrgNode;
  using Parent::getRedEdge;
  using Parent::getDualSlackX;

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
  typedef typename Parent::BpWeightMap BpWeightMap;
  typedef typename Parent::BpIdxMap BpIdxMap;
  typedef typename Parent::BpNodeMap BpNodeMap;
  typedef typename Parent::BpEdgeMap BpEdgeMap;
  typedef typename Parent::MWBM MWBM;
  typedef typename Parent::GlobalProblemType GlobalProblemType;
  typedef typename Parent::LocalProblem LocalProblem;
  typedef typename Parent::DualVector DualVector;
  typedef typename Parent::PrimalVector PrimalVector;
  typedef typename Parent::SubgradientVector SubgradientVector;
  typedef typename Parent::PrimalConstIt PrimalConstIt;
  typedef typename Parent::PrimalIt PrimalIt;
  typedef typename Parent::DualConstIt DualConstIt;
  typedef typename Parent::DualIt DualIt;
  typedef typename Parent::SubgradientConstIt SubgradientConstIt;
  typedef typename Parent::SubgradientIt SubgradientIt;
  typedef typename Parent::IndexList IndexList;
  typedef typename Parent::IndexListIt IndexListIt;
  typedef typename Parent::IndexListConstIt IndexListConstIt;
  typedef typename Parent::IndexVector IndexVector;
  typedef typename Parent::IndexVectorIt IndexVectorIt;
  typedef typename Parent::IndexVectorConstIt IndexVectorConstIt;
  typedef typename Parent::PrimalDefX PrimalDefX;
  typedef typename Parent::PrimalDefXIt PrimalDefXIt;
  typedef typename Parent::DefY DefY;
  typedef typename Parent::PrimalDefY PrimalDefY;
  typedef typename Parent::PrimalDefYIt PrimalDefYIt;

  struct DefExtX
  {
    IndexList _lambdaHIndices;
    IndexList _lambdaVIndices;

    DefExtX()
      : _lambdaHIndices()
      , _lambdaVIndices()
    {
    }
  };

  struct DefExtY
  {
    size_t _idxLambdaH;
    size_t _idxLambdaV;

    DefExtY()
      : _idxLambdaH(std::numeric_limits<size_t>::max())
      , _idxLambdaV(std::numeric_limits<size_t>::max())
    {
    }
  };

  struct DefLambdaH
  {
    BpEdge _jl;   // in _gm
    BpRedNode _i; // in _gm
  };

  struct DefLambdaV
  {
    BpEdge _jl;    // in _gm
    BpBlueNode _k; // in _gm
  };

  static bool compareH(const DefLambdaH& a, const DefLambdaH& b)
  {
    return b._i < a._i || (a._i == b._i && b._jl < a._jl);
  }

  static bool compareV(const DefLambdaV& a, const DefLambdaV& b)
  {
    return b._k < a._k || (a._k == b._k && b._jl < a._jl);
  }

  typedef std::vector<DefExtX> PrimalDefExtX;
  typedef typename PrimalDefExtX::const_iterator PrimalDefExtXIt;

  typedef std::vector<DefExtY> PrimalDefExtY;
  typedef typename PrimalDefExtY::iterator PrimalDefExtYNonConstIt;
  typedef typename PrimalDefExtY::const_iterator PrimalDefExtYIt;

  typedef std::vector<DefLambdaH> DualDefLambdaH;
  typedef std::vector<DefLambdaV> DualDefLambdaV;

  typedef typename DualDefLambdaH::const_iterator DualDefLambdaHIt;
  typedef typename DualDefLambdaV::const_iterator DualDefLambdaVIt;

  typedef typename BpGraph::template NodeMap<bool> BpBoolNodeMap;
  typedef typename BpGraph::template RedNodeMap<size_t> BpRedNodeIdxMap;
  typedef typename BpGraph::template BlueNodeMap<size_t> BpBlueNodeIdxMap;
  
  typedef typename BpGraph::template RedNodeMap<BpBoolMap*> BpRedNodeBoolMap;
  typedef typename BpGraph::template BlueNodeMap<BpBoolMap*> BpBlueNodeBoolMap;

  typedef std::vector<double> DoubleVector;

  /// Extended x_{ik} variable definitions
  PrimalDefExtX _defExtX;
  /// Extended y_{ikjl} variable definitions
  PrimalDefExtY _defExtY;
  /// lambda^h_{jli} variable definitions (where j < i)
  DualDefLambdaH _defLambdaH;
  /// lambda^v_{jlk} variable definitions (where k != l)
  DualDefLambdaV _defLambdaV;
  /// Indicates whether a node is incident to a matching edge corresponding to a y-var
  BpBoolNodeMap _hasY;
  /// Maps a red node i to its first lambda^h_{jli} index
  BpRedNodeIdxMap _idxMapH;
  /// Maps a blue node k to its first lambda^h_{jlk} index
  BpBlueNodeIdxMap _idxMapV;
  /// Subgradient values corresponding to \lambda^h
  SubgradientVector _subgradientH;
  /// Subgradient values corresponding to \lambda^v
  SubgradientVector _subgradientV;
  /// \sum_{j < i} \lambda^h_{ikj}
  DoubleVector _sumH;
  /// \sum_{l != k} \lambda^v_{ikl}
  DoubleVector _sumV;

  /// Initialize primal variables
  virtual void initPrimal();
  /// Initialize dual variable definitions
  virtual void initDual();
  /// Transform computed solution into a feasible one
  void computeFeasiblePrimalValue();
  /// Compute \sum_{j < i} \lambda^h_{ikj} given x_{ik}
  void computeSumH(const DualVector& dual, const BpEdge ik);
  /// Compute \sum_{l != k} \lambda^h_{ikl} given x_{ik}
  void computeSumV(const DualVector& dual, const BpEdge ik);
  /// Return dual value corresponding with \lambda^h
  double getDualH(const DualVector& dual, const size_t idxH);
  /// Return dual value corresponding with \lambda^v
  double getDualV(const DualVector& dual, const size_t idV);
  /// Realize y-vars corresponding to given x-var
  void realize(const BpEdge ik);

public:
  /// Constructor
  LagrangeGnaSi(const MatchingGraphType& matchingGraph,
                const ScoreModelType& scoreModel,
                bool integral);
  /// Destructor
  virtual ~LagrangeGnaSi() {}
  /// Return the subgradient corresponding to the i-th multiplier
  int getSubgradient(size_t i) const;
  /// Return the number of dual multipliers corresponding to the problem instance
  size_t getDualDim() const { return _defLambdaH.size() + _defLambdaV.size(); }
  /// Return dual lower bound, which is 0
  double getDualVarLowerBound(size_t) const { return 0; }
  /// Return dual upper bound, which is +INF
  double getDualVarUpperBound(size_t) const { return std::numeric_limits<double>::max(); }
  /// Output primal variable definitions
  void printPrimalDef(std::ostream& out) const;
  /// Output dual variable definitions
  void printDualDef(std::ostream& out) const;
  /// Initialize global profit
  virtual void initGlobalProfit(const BpEdge ik,
                                const size_t idxX,
                                const DualVector& dual);
};

template<typename GR, typename BGR>
inline LagrangeGnaSi<GR, BGR>::LagrangeGnaSi(const MatchingGraphType& matchingGraph,
                                             const ScoreModelType& scoreModel,
                                             bool integral)
  : Parent(matchingGraph, scoreModel, integral)
  , _defExtX()
  , _defExtY()
  , _defLambdaH()
  , _defLambdaV()
  , _hasY(_gm, false)
  , _idxMapH(_gm, std::numeric_limits<size_t>::max())
  , _idxMapV(_gm, std::numeric_limits<size_t>::max())
  , _subgradientH()
  , _subgradientV()
  , _sumH()
  , _sumV()
{
}

template<typename GR, typename BGR>
inline void LagrangeGnaSi<GR, BGR>::initPrimal()
{
  _defX.clear();
  _defExtX.clear();
  _defY.clear();
  _defExtY.clear();
  _yIdx2xIdx.clear();

  const Graph& g1 = _matchingGraph.getG1();

  // initialize primal variable definitions
  size_t idxX = 0;
  for (BpEdgeIt ik(_gm); ik != lemon::INVALID; ++ik, idxX++)
  {
    _defX.push_back(ik);
    _idxMapX[ik] = idxX;

    BpNode bp_i = _gm.redNode(ik);
    BpNode bp_k = _gm.blueNode(ik);
    Node i = _matchingGraph.mapGmToG1(bp_i);
    Node k = _matchingGraph.mapGmToG2(bp_k);

    std::vector<std::pair<BpEdge, double> > edgeVector;
    for (IncEdgeIt ij(g1, i); ij != lemon::INVALID; ++ij)
    {
      Node j = g1.oppositeNode(i, ij);
      BpNode bp_j = _matchingGraph.mapG1ToGm(j);

      for (BpIncEdgeIt jl(_gm, bp_j); jl != lemon::INVALID; ++jl)
      {
        BpNode bp_l = _gm.blueNode(jl);
        Node l = _matchingGraph.mapGmToG2(bp_l);

        Edge kl = _matchingGraph.getEdgeG2(k, l);
        if (kl != lemon::INVALID)
        {
          double weight = 0.5 * _scoreModel.getWeightG1G2(ij, kl);
          edgeVector.push_back(std::make_pair(jl, weight));
        }
      }
    }

    std::sort(edgeVector.begin(), edgeVector.end());
    typename std::vector<std::pair<BpEdge, double> >::const_reverse_iterator it;
    const typename std::vector<std::pair<BpEdge, double> >::const_reverse_iterator crend = edgeVector.rend();
    for (it = edgeVector.rbegin(); it != crend; it++)
    {
      // there is an edge (k,l) and also (i,j)
      // we now have that w_{ikjl} != 0, so we consider y_{ikjl}
      //getRedEdge(idxX, ik); // not really needed
      BpEdge red_jl = getRedEdge(idxX, it->first);
      DefY def_y = {red_jl, it->second};
      _defY.push_back(def_y);

      _yIdx2xIdx.push_back(idxX);
      _hasY[bp_i] = _hasY[bp_k] = true;   
    }
  }

  _x = PrimalVector(_defX.size(), false);
  _y = PrimalVector(_defY.size(), false);
  _defExtX = PrimalDefExtX(_defX.size());
  _defExtY = PrimalDefExtY(_defY.size());
  _sumH = DoubleVector(_defX.size(), 0);
  _sumV = DoubleVector(_defX.size(), 0);
  _pGlobal->init();
}

template<typename GR, typename BGR>
inline void LagrangeGnaSi<GR, BGR>::initDual()
{
  _defLambdaH.clear();
  _defLambdaV.clear();

  const size_t maxIdx = std::numeric_limits<size_t>::max();

  // initialize dual variable definitions: _defLambdaH
  BpRedNodeBoolMap doneRed(_gm, NULL);
  BpBlueNodeBoolMap doneBlue(_gm, NULL);

  size_t idxY = 0;
  for (PrimalDefYIt itY = _defY.begin(); itY != _defY.end(); itY++, idxY++)
  {
    const BpRedNode i = _gm.asRedNodeUnsafe(getOrgNode(idxY, true, true));
    const BpBlueNode k = _gm.asBlueNodeUnsafe(getOrgNode(idxY, true, false));
    const BpEdge jl = getOrgEdge(idxY, false);

    BpBoolMap* pRedMap = doneRed[i];
    if (!pRedMap)
    {
      pRedMap = new BpBoolMap(_gm, false);
      doneRed[i] = pRedMap;
    }
    
    BpBoolMap* pBlueMap = doneBlue[k];
    if (!pBlueMap)
    {
      pBlueMap = new BpBoolMap(_gm, false);
      doneBlue[k] = pBlueMap;
    }

    if (!(*pRedMap)[jl])
    {
      DefLambdaH def_h = {jl, i};
      _defLambdaH.push_back(def_h);
      
      pRedMap->set(jl, true);
    }
    
    if (!(*pBlueMap)[jl])
    {
      DefLambdaV def_v = {jl, k};
      _defLambdaV.push_back(def_v);

      pBlueMap->set(jl, true);
    }
  }

  for (BpRedNodeIt i(_gm); i != lemon::INVALID; ++i)
    delete doneRed[i];
    
  for (BpBlueNodeIt k(_gm); k != lemon::INVALID; ++k)
    delete doneBlue[k];

  std::sort(_defLambdaH.begin(), _defLambdaH.end(), compareH);
  std::sort(_defLambdaV.begin(), _defLambdaV.end(), compareV);

  _subgradientH = SubgradientVector(_defLambdaH.size(), 0);
  _subgradientV = SubgradientVector(_defLambdaV.size(), 0);

  // initialize _defExtX[]._lambdaHIdx
  size_t idxH = 0;
  for (DualDefLambdaHIt itH = _defLambdaH.begin();
    itH != _defLambdaH.end(); itH++, idxH++)
  {
    size_t idxX = _idxMapX[itH->_jl];
    _defExtX[idxX]._lambdaHIndices.push_back(idxH);

    if (_idxMapH[itH->_i] == maxIdx)
      _idxMapH[itH->_i] = idxH;
  }

  // initialize _defExtX[]._lambdaVIdx
  size_t idxV = 0;
  for (DualDefLambdaVIt itV = _defLambdaV.begin();
    itV != _defLambdaV.end(); itV++, idxV++)
  {
    size_t idxX = _idxMapX[itV->_jl];
    _defExtX[idxX]._lambdaVIndices.push_back(idxV);

    if (_idxMapV[itV->_k] == maxIdx)
      _idxMapV[itV->_k] = idxV;
  }

  // initialize _defExtY._idxLambdaH and _defExtY._idxLambdaV
  // do a parallel scan
  size_t idxX = 0;
  idxY = 0;
  PrimalDefYIt itY = _defY.begin();
  PrimalDefExtYNonConstIt itExtY = _defExtY.begin();
  for (PrimalDefXIt itX = _defX.begin(); itX != _defX.end() && itY != _defY.end(); itX++, idxX++)
  {
    const BpEdge bp_ik = _defX[idxX];
    const BpRedNode bp_i = _gm.redNode(bp_ik);
    const BpBlueNode bp_k = _gm.blueNode(bp_ik);

    if (idxX != _yIdx2xIdx[idxY])
      continue;

    size_t idxH = _idxMapH[bp_i];
    DualDefLambdaHIt itH = _defLambdaH.begin() + idxH;
    size_t idxV = _idxMapV[bp_k];
    DualDefLambdaVIt itV = _defLambdaV.begin() + idxV;

    for (; itY != _defY.end() && _yIdx2xIdx[idxY] == idxX; itY++, itExtY++, idxY++)
    {
      const BpEdge jl = getOrgEdge(idxY, false);
      while (jl != itH->_jl)
      {
        itH++;
        idxH++;
      }
      assert(itH->_i == bp_i);

      while (jl != itV->_jl)
      {
        itV++;
        idxV++;
      }
      assert(itV->_k == bp_k);
      
      itExtY->_idxLambdaH = idxH;
      itExtY->_idxLambdaV = idxV;
    }
  }
}

template<typename GR, typename BGR>
inline void LagrangeGnaSi<GR, BGR>::printPrimalDef(std::ostream& out) const
{
  size_t idxX = 0;
  for (PrimalDefXIt itX = _defX.begin(); itX != _defX.end(); itX++, idxX++)
  {
    BpEdge ik = *itX;
    out << idxX << ":\t" 
      << _matchingGraph.getLabelGm(_gm.redNode(ik)) << ","
      << _matchingGraph.getLabelGm(_gm.blueNode(ik)) << "\t";

    const DefExtX& defext_x = _defExtX[idxX];
    
    out << "lambda^h: ";
    for (IndexListConstIt itH = defext_x._lambdaHIndices.begin();
      itH != defext_x._lambdaHIndices.end(); itH++)
    {
      out << *itH << " ";
    }

    out << "\tlambda^v: ";
    for (IndexListConstIt itV = defext_x._lambdaVIndices.begin();
      itV != defext_x._lambdaVIndices.end(); itV++)
    {
      out << *itV << " ";
    }

    out << std::endl;
  }

  size_t idxY = 0;
  PrimalDefYIt itY = _defY.begin();
  PrimalDefExtYIt itExtY = _defExtY.begin();
  for (; itY != _defY.end() && itExtY != _defExtY.end(); itY++, itExtY++, idxY++)
  {
    out << idxY << ":\t"
      << _matchingGraph.getLabelGm(getOrgNode(idxY, true, true)) << ","
      << _matchingGraph.getLabelGm(getOrgNode(idxY, true, false)) << "\t"
      << _matchingGraph.getLabelGm(getOrgNode(idxY, false, true)) << ","
      << _matchingGraph.getLabelGm(getOrgNode(idxY, false, false)) << "\t"
      << itExtY->_idxLambdaH << "\t"
      << itExtY->_idxLambdaV << std::endl;
  }
}

template<typename GR, typename BGR>
inline void LagrangeGnaSi<GR, BGR>::printDualDef(std::ostream& out) const
{
  size_t idxH = 0;
  for (DualDefLambdaHIt itH = _defLambdaH.begin(); 
    itH != _defLambdaH.end(); itH++, idxH++)
  {
    out << idxH << ":\t"
      << _matchingGraph.getLabelGm(_gm.redNode(itH->_jl)) << ","
      << _matchingGraph.getLabelGm(_gm.blueNode(itH->_jl)) << "\t"
      << _matchingGraph.getLabelGm(itH->_i)
      << std::endl;
  }

  size_t idxV = 0;
  for (DualDefLambdaVIt itV = _defLambdaV.begin(); 
    itV != _defLambdaV.end(); itV++, idxV++)
  {
    out << idxV << ":\t"
      << _matchingGraph.getLabelGm(_gm.redNode(itV->_jl)) << ","
      << _matchingGraph.getLabelGm(_gm.blueNode(itV->_jl)) << "\t"
      << _matchingGraph.getLabelGm(itV->_k)
      << std::endl;
  }
}

template<typename GR, typename BGR>
inline int LagrangeGnaSi<GR, BGR>::getSubgradient(size_t i) const
{
  assert(0 <= i && i < _defLambdaH.size() + _defLambdaV.size());
  
  if (i < _defLambdaH.size())
  {
    return _subgradientH[i];
  }
  else
  {
    return _subgradientV[i - _defLambdaH.size()];
  }
}

template<typename GR, typename BGR>
inline void LagrangeGnaSi<GR, BGR>::computeFeasiblePrimalValue()
{
  // TODO: ik kan hier iets slimmers doen:
  // - #duals - #violations == #correct matches;
  if (g_verbosity >= VERBOSE_DEBUG)
  {
    std::cout << "Computing feasible primal solution..." << std::flush;
  }

  // compute a feasible solution based on x
  _feasiblePrimalValue = 0;
  size_t idxY = 0;
  PrimalDefYIt itY = _defY.begin();
  for (; itY != _defY.end(); itY++, idxY++)
  {
    if (_x[_idxMapX[getOrgEdge(idxY, true)]] && _x[_idxMapX[getOrgEdge(idxY, false)]])
      _feasiblePrimalValue += itY->_weight;
  }

  size_t idxX = 0;
  for (PrimalConstIt itX = _x.begin(); itX != _x.end(); itX++, idxX++)
  {
    if (*itX)
      _feasiblePrimalValue += _scoreModel.getWeightGm(_defX[idxX]);
  }

  if (g_verbosity >= VERBOSE_DEBUG)
  {
    std::cout << "Done! Value: " << _feasiblePrimalValue << std::endl << std::endl;
  }
}

template<typename GR, typename BGR>
inline void LagrangeGnaSi<GR, BGR>::initGlobalProfit(const BpEdge ik,
                                                     const size_t idxX,
                                                     const DualVector& dual)
{
  _pGlobal->setProfit(ik, _sumH[idxX] + _sumV[idxX] + _scoreModel.getWeightGm(ik));
}

template<typename GR, typename BGR>
inline void LagrangeGnaSi<GR, BGR>::computeSumH(const DualVector& dual, const BpEdge ik)
{
  const size_t idxX = _idxMapX[ik];
  const IndexList& lambdaH = _defExtX[idxX]._lambdaHIndices;

  _sumH[idxX] = 0;
  for (IndexListConstIt itH = lambdaH.begin(); itH != lambdaH.end(); itH++)
    _sumH[idxX] += getDualH(dual, *itH);

}

template<typename GR, typename BGR>
inline void LagrangeGnaSi<GR, BGR>::computeSumV(const DualVector& dual, const BpEdge ik)
{
  const size_t idxX = _idxMapX[ik];
  const IndexList& lambdaV = _defExtX[idxX]._lambdaVIndices;

  _sumV[idxX] = 0;
  for (IndexListConstIt itV = lambdaV.begin(); itV != lambdaV.end(); itV++)
    _sumV[idxX] += getDualV(dual, *itV);
}

template<typename GR, typename BGR>
inline double LagrangeGnaSi<GR, BGR>::getDualH(const DualVector& dual, const size_t idxH)
{
  assert(0 <= idxH && idxH < _defLambdaH.size());
  return dual[idxH];
}

template<typename GR, typename BGR>
inline double LagrangeGnaSi<GR, BGR>::getDualV(const DualVector& dual, const size_t idxV)
{
  assert(0 <= idxV && idxV < _defLambdaV.size());
  return dual[_defLambdaH.size() + idxV];
}

template<typename GR, typename BGR>
inline void LagrangeGnaSi<GR, BGR>::realize(const BpEdge ik)
{
  const size_t idxX = _idxMapX[ik];
  _x[idxX] = true;

  const DefExtX& defext_x =  _defExtX[idxX];
  for (IndexListConstIt itH = defext_x._lambdaHIndices.begin();
    itH != defext_x._lambdaHIndices.end(); itH++)
  {
    _subgradientH[*itH]++;
  }
  for (IndexListConstIt itV = defext_x._lambdaVIndices.begin();
    itV != defext_x._lambdaVIndices.end(); itV++)
  {
    _subgradientV[*itV]++;
  }

  const IndexList& y_list = _pGlobal->getRealizedY(idxX);
  for (IndexListConstIt itR = y_list.begin(); itR != y_list.end(); itR++)
  {
    const DefExtY& defext_y = _defExtY[*itR];

    _subgradientH[defext_y._idxLambdaH]--;
    _subgradientV[defext_y._idxLambdaV]--;

    _y[*itR] = true;
  }
}

} // namespace gna
} // namespace nina

#endif /* LAGRANGEGNASI_H_ */
