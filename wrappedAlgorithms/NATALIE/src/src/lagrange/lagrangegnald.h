/*
 * lagrangegnald.h
 *
 *  Created on: 8-mar-2011
 *      Author: M. El-Kebir
 */

#ifndef LAGRANGEGNALD_H_
#define LAGRANGEGNALD_H_

#include <lemon/core.h>
#include <lemon/bp_matching.h>
#include <lemon/adaptors.h>
#include <limits>
#include "lagrange/lagrangegna.h"
#include "verbose.h"

namespace nina {
namespace gna {

/// The number of dual variables is:
/// \f$\frac{1}{2} |V_1|(|V_1|+1)|V_2|^2 = O(|V_1|^2 |V_2|^2)\f$.
///
/// The number of primal variables is:
/// \f$|V_1||V_2| + |V_1|^2|V_2^2 \f$.
///
/// Note that only variables whose weight != 0 are considered.
template<typename GR, typename BGR>
class LagrangeGnaLd : public virtual LagrangeGna<GR, BGR>
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

  typedef typename std::vector<IndexVector> PrimalDefExtX;
  typedef typename PrimalDefExtX::const_iterator PrimalDefExtXIt;

  struct DefExtY
  {
    bool _fwd;
    size_t _idxLambda;
    size_t _idxMu;
  };

  struct DefLambda
  {
    size_t _idxYFwd;
    size_t _idxYBwd;
  };

  typedef std::vector<DefExtY> PrimalDefExtY;
  typedef typename PrimalDefExtY::iterator PrimalDefExtYNonConstIt;
  typedef typename PrimalDefExtY::const_iterator PrimalDefExtYIt;
  typedef std::vector<DefLambda> DualDefLambda;
  typedef typename DualDefLambda::const_iterator DualDefLambdaIt;
  typedef typename std::vector<size_t> DualDefMu;
  typedef typename DualDefMu::const_iterator DualDefMuIt;

  /// Extended x_{ik} variable definitions
  PrimalDefExtX _defExtX;
  /// Extended y_{ikjl} variable definitions
  PrimalDefExtY _defExtY;
  /// lambda_{ikjl} variable definitions (where i < j)
  DualDefLambda _defLambda;
  /// mu_{ikjl} variable definitions (where i \neq j and k \neq l)
  DualDefMu _defMu;

  /// Initialize primal variables
  void initPrimal();
  /// Initialize dual variable definitions
  virtual void initDual();
  /// Transform computed solution into a feasible one
  void computeFeasiblePrimalValue();
  /// Realize y-vars corresponding to given x-var
  void realize(const BpEdge ik);

public:
  /// Constructor
  LagrangeGnaLd(const MatchingGraphType& matchingGraph,
                const ScoreModelType& scoreModel,
                bool integral);
  /// Destructor
  virtual ~LagrangeGnaLd() {}
  /// Return the subgradient corresponding to the i-th multiplier
  int getSubgradient(size_t i) const;
  /// Return the number of dual multipliers corresponding to the problem instance
  size_t getDualDim() const { return _defLambda.size() + _defMu.size(); }
  /// Return dual lower bound, which is -INF
  double getDualVarLowerBound(size_t idxD) const { return idxD < _defLambda.size() ? -std::numeric_limits<double>::max() : 0; }
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
  /// Return counterpart of y variable with idxY
  size_t getCounterpartY(size_t idxY) const;
  /// Return fwd/bwd idxY corresponding to multiplier lambda at idxD
  size_t getIdxY(size_t idxD, bool fwd) const;
  /// Return idxY corresponding to multipllier mu at idxD
  size_t getIdxY(size_t idxD) const;
};

template<typename GR, typename BGR>
inline LagrangeGnaLd<GR, BGR>::LagrangeGnaLd(const MatchingGraphType& matchingGraph,
                                             const ScoreModelType& scoreModel,
                                             bool integral)
  : Parent(matchingGraph, scoreModel, integral)
  , _defExtX()
  , _defExtY()
  , _defLambda()
  , _defMu()
{
}

template<typename GR, typename BGR>
inline void LagrangeGnaLd<GR, BGR>::initPrimal()
{
  _defX.clear();
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
          // there is an edge (k,l) and also (i,j)
          // we now have that w_{ikjl} != 0, so we consider y_{ikjl}
          bool fwd = (bp_i < bp_j || (bp_i == bp_j && bp_k < bp_l));

          BpEdge red_jl = getRedEdge(idxX, jl);
          double weight = 0.5 * _scoreModel.getWeightG1G2(ij, kl);

          if (weight != 0)
          {
            DefY def_y = {red_jl, weight};
            _defY.push_back(def_y);

            DefExtY defext_y = {fwd,
                                std::numeric_limits<size_t>::max(),
                                std::numeric_limits<size_t>::max()};

            _defExtY.push_back(defext_y);
            _yIdx2xIdx.push_back(idxX);
          }
        }
      }
    }
  }

  _x = PrimalVector(_defX.size(), false);
  _y = PrimalVector(_defY.size(), false);
  _defExtX = PrimalDefExtX(_defX.size());
  _pGlobal->init();
}

template<typename GR, typename BGR>
inline void LagrangeGnaLd<GR, BGR>::initDual()
{
  _defMu.clear();
  _defLambda.clear();

  // initialize dual variable definitions
  // TODO: do something smarter here
  const size_t n = _defY.size();
  for (size_t idxY1 = 0; idxY1 < n; idxY1++)
  {
    // init mu
    if (_defY[idxY1]._weight < 0)
    {
      _defExtY[idxY1]._idxMu = _defMu.size();
      _defMu.push_back(idxY1);
    }

    for (size_t idxY2 = idxY1 + 1; idxY2 < n; idxY2++)
    {
      if (getOrgEdge(idxY1, true) == getOrgEdge(idxY2, false) 
        && getOrgEdge(idxY1, false) == getOrgEdge(idxY2, true))
      {
        DefExtY& def_ext_y1 = _defExtY[idxY1];
        DefExtY& def_ext_y2 = _defExtY[idxY2];

        assert(def_ext_y1._fwd != def_ext_y2._fwd);

        def_ext_y1._idxLambda = def_ext_y2._idxLambda = _defLambda.size();
        if (def_ext_y1._fwd)
        {
          DefLambda def_lambda = {idxY1, idxY2};
          _defLambda.push_back(def_lambda);
        }
        else
        {
          DefLambda def_lambda = {idxY2, idxY1};
          _defLambda.push_back(def_lambda);
        }
      }
    }
  }

  // initialize _defExtX
  for (size_t idxMu = 0; idxMu < _defMu.size(); idxMu++)
  {
    const size_t idxY = _defMu[idxMu];
    const size_t idxX1 = _idxMapX[getOrgEdge(idxY, true)];
    const size_t idxX2 = _idxMapX[getOrgEdge(idxY, false)];

    _defExtX[idxX1].push_back(idxMu);
    _defExtX[idxX2].push_back(idxMu);
  }
}

template<typename GR, typename BGR>
inline int LagrangeGnaLd<GR, BGR>::getSubgradient(size_t i) const
{
  assert(0 <= i && i < _defLambda.size() + _defMu.size());

  if (i < _defLambda.size())
  {
    const DefLambda& def_lambda = _defLambda[i];
    assert(_defExtY[def_lambda._idxYFwd]._fwd && !_defExtY[def_lambda._idxYBwd]._fwd);

    return _y[def_lambda._idxYFwd] - _y[def_lambda._idxYBwd];
  }
  else
  {
    const size_t idxY = _defMu[i - _defLambda.size()];
    return (_y[idxY] ? 1 : 0)
      - (_x[_idxMapX[getOrgEdge(idxY, true)]] ? 1 : 0)
      - (_x[_idxMapX[getOrgEdge(idxY, false)]] ? 1 : 0)
      + 1;
  }
}

template<typename GR, typename BGR>
inline void LagrangeGnaLd<GR, BGR>::printDualDef(std::ostream& out) const
{
  size_t idxD = 0;
  for (DualDefLambdaIt itD = _defLambda.begin(); 
    itD != _defLambda.end(); itD++, idxD++)
  {
    const DefLambda& def_lambda = *itD;
    out << "Lambda_" << idxD << ":\t"
      << def_lambda._idxYFwd
      << "\t"
      << def_lambda._idxYBwd
      << std::endl;
  }

  idxD = 0;
  for (DualDefMuIt itD = _defMu.begin();
    itD != _defMu.end(); itD++, idxD++)
  {
    out << "Mu_" << idxD << ":\t"
      << *itD
      << std::endl;
  }
}

template<typename GR, typename BGR>
inline void LagrangeGnaLd<GR, BGR>::printPrimalDef(std::ostream& out) const
{
  size_t idxX = 0;
  for (PrimalDefXIt itX = _defX.begin(); itX != _defX.end(); itX++, idxX++)
  {
    BpEdge ik = *itX;
    out << idxX << ":\t" 
      << _matchingGraph.getLabelGm(_gm.redNode(ik)) << ","
      << _matchingGraph.getLabelGm(_gm.blueNode(ik)) << std::endl;
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
      << itY->_weight << "\t"
      << (itExtY->_fwd ? "fwd" : "bwd") << "\t"
      << itExtY->_idxLambda << "\t"
      << itExtY->_idxMu << "\t" << std::endl;
  }
}

template<typename GR, typename BGR>
inline void LagrangeGnaLd<GR, BGR>::computeFeasiblePrimalValue()
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
  PrimalDefExtYIt itExtY = _defExtY.begin();
  for (; itY != _defY.end() && itExtY != _defExtY.end(); itY++, itExtY++, idxY++)
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
    std::cout << "Done! Value: " << _feasiblePrimalValue << std::endl;
  }
}

template<typename GR, typename BGR>
inline void LagrangeGnaLd<GR, BGR>::initGlobalProfit(const BpEdge ik,
                                                     const size_t idxX,
                                                     const DualVector& dual)
{
  double profit = _scoreModel.getWeightGm(ik);

  const IndexVector& relevantDuals = _defExtX[idxX];

  for (IndexVectorConstIt itIdxD = relevantDuals.begin();
       itIdxD != relevantDuals.end(); itIdxD++)
  {
    profit -= dual[*itIdxD + _defLambda.size()];
  }

  _pGlobal->setProfit(ik, profit);
}

template<typename GR, typename BGR>
inline void LagrangeGnaLd<GR, BGR>::realize(const BpEdge ik)
{
  const size_t idxX = _idxMapX[ik];
  _x[idxX] = true;

  const IndexList& y_list = _pGlobal->getRealizedY(idxX);
  for (IndexListConstIt itR = y_list.begin(); itR != y_list.end(); itR++)
  {
    _y[*itR] = true;
  }
}

template<typename GR, typename BGR>
inline size_t LagrangeGnaLd<GR, BGR>::getCounterpartY(size_t idxY) const
{
  assert(0 <= idxY && idxY < _defExtY.size());

  if (_defExtY[idxY]._fwd)
    return _defLambda[_defExtY[idxY]._idxLambda]._idxYBwd;
  else
    return _defLambda[_defExtY[idxY]._idxLambda]._idxYFwd;
}

template<typename GR, typename BGR>
inline size_t LagrangeGnaLd<GR, BGR>::getIdxY(size_t idxD, bool fwd) const
{
  assert(0 <= idxD && idxD < _defLambda.size());
  return fwd ? _defLambda[idxD]._idxYFwd : _defLambda[idxD]._idxYBwd;
}

} // namespace gna
} // namespace nina

#endif /* LAGRANGEGNALD_H_ */
