/*
 * nataliecplex.h
 *
 *  Created on: 7-may-2012
 *      Author: M. El-Kebir
 */

#ifndef NATALIECPLEX_H
#define NATALIECPLEX_H

#include "natalie.h"
#include "cplex/cplexgna.h"
#include "cplex/cplexgnald.h"
#include "cplex/cplexgnasi.h"
#include "cplex/cplexgnaqp.h"

namespace nina {
namespace gna {

template<typename GR, typename BGR>
class NatalieCplex : public Natalie<GR, BGR>
{
public:
  /// The graph type of the input graphs
  typedef GR Graph;
  /// The graph type of the bipartite matching graph
  typedef BGR BpGraph;
  /// Base class type
  typedef Natalie<GR, BGR> Parent;
  /// Type of a map assigning a boolean to every matching edge
  typedef typename Parent::BpBoolMap BpBoolMap;
  /// Type of a matching map: maps a node to its matching edge
  typedef typename Parent::BpMatchingMap BpMatchingMap;
  /// Type of the matching graph
  typedef typename Parent::MatchingGraphType MatchingGraphType;
  /// Type of options
  typedef typename Parent::Options OptionsType;

  using Parent::_pMatchingGraph;
  using Parent::_pScoreModel;
  using Parent::_outputs;
  using Parent::_options;
  using Parent::_pLagrangeGna;
  using Parent::_pSolver;
  using Parent::_elapsedTime;
  using Parent::_score;
  using Parent::_upperBound;
  using Parent::setScoreModel;
  using Parent::setRelaxation;

  typedef typename Parent::ParserType ParserType;
  typedef typename Parent::BpParserType BpParserType;

  /// Type of formulation
  typedef enum {
                 LAGRANGIAN_DECOMPOSITION,
                 SUBPROBLEM_ISOLATION,
                 QUADRATIC
               } FormulationType;

  typedef CplexGna<GR, BGR> CplexGnaType;
  typedef CplexGnaLd<GR, BGR> CplexGnaLdType;
  typedef CplexGnaSi<GR, BGR> CplexGnaSiType;
  typedef CplexGnaQp<GR, BGR> CplexGnaQpType;

protected:
  FormulationType _formulation;
  bool _lp;
  CplexGnaType* _pCplexGna;

public:
  NatalieCplex(FormulationType formulation,
               bool lp);
  NatalieCplex(const OptionsType& options,
               FormulationType formulation,
               bool lp);
  virtual ~NatalieCplex();
  virtual int solve(int);
  virtual int getNumberOfSolutions() const { return 1; }
  void setFormulation(FormulationType formulation);
  virtual void getSolution(BpBoolMap& m, int) const;
  virtual void getSolution(BpMatchingMap& m, int) const;
  /// Initializes the graphs using the specified parsers
  virtual bool init(ParserType* pParserG1,
                    ParserType* pParserG2,
                    BpParserType* pParserGm);
};

template<typename GR, typename BGR>
inline void NatalieCplex<GR, BGR>::setFormulation(FormulationType formulation)
{
  delete _pCplexGna;
  _pCplexGna = NULL;

  switch (formulation)
  {
    case LAGRANGIAN_DECOMPOSITION:
      setRelaxation(Parent::LAGRANGIAN_DECOMPOSITION_CACHED);
      _pCplexGna = new CplexGnaLdType(*_pMatchingGraph,
                                      _pScoreModel,
                                      dynamic_cast<typename Parent::LagrangeGnaLdCachedType*>(_pLagrangeGna),
                                      _lp);
      break;
    case SUBPROBLEM_ISOLATION:
      setRelaxation(Parent::SUBPROBLEM_ISOLATION_CACHED);
      _pCplexGna = new CplexGnaSiType(*_pMatchingGraph,
                                      _pScoreModel,
                                      dynamic_cast<typename Parent::LagrangeGnaSiCachedType*>(_pLagrangeGna),
                                      _lp);
      break;
    case QUADRATIC:
      setRelaxation(Parent::SUBPROBLEM_ISOLATION_CACHED);
      _pCplexGna = new CplexGnaQpType(*_pMatchingGraph,
                                      _pScoreModel,
                                      dynamic_cast<typename Parent::LagrangeGnaSiCachedType*>(_pLagrangeGna),
                                      _lp);
      break;
  }
}

template<typename GR, typename BGR>
inline NatalieCplex<GR, BGR>::~NatalieCplex()
{
  delete _pCplexGna;
}

template<typename GR, typename BGR>
inline NatalieCplex<GR, BGR>::NatalieCplex(FormulationType formulation,
                                           bool lp)
  : Parent()
  , _formulation(formulation)
  , _lp(lp)
  , _pCplexGna(NULL)
{
}

template<typename GR, typename BGR>
inline NatalieCplex<GR, BGR>::NatalieCplex(const OptionsType& options,
                                           FormulationType formulation,
                                           bool lp)
  : Parent(options)
  , _formulation(formulation)
  , _lp(lp)
  , _pCplexGna(NULL)
{
}

template<typename GR, typename BGR>
inline bool NatalieCplex<GR, BGR>::init(ParserType* pParserG1,
                                        ParserType* pParserG2,
                                        BpParserType* pParserGm)
{
  bool res = false;
  if (pParserG1 && pParserG2 && pParserGm)
    res = _pMatchingGraph->init(pParserG1, pParserG2, pParserGm);

  if (res)
  {
    if (!_pScoreModel)
      setScoreModel(static_cast<typename Parent::ScoreFunctionEnum>(_options._scoreModel), pParserG1, pParserG2);

    _pScoreModel->init();

    if (!_pLagrangeGna)
      setFormulation(_formulation);

    _pLagrangeGna->init();
  }

  return res;
}


template<typename GR, typename BGR>
inline int NatalieCplex<GR, BGR>::solve(int)
{
  assert(_pCplexGna);
  _pCplexGna->init();
  return _pCplexGna->solve(_options._timeLimit);
}

template<typename GR, typename BGR>
inline void NatalieCplex<GR, BGR>::getSolution(BpBoolMap& m, int) const
{
  _pCplexGna->getMatching(m);
}

template<typename GR, typename BGR>
inline void NatalieCplex<GR, BGR>::getSolution(BpMatchingMap& m, int) const
{
  _pCplexGna->getMatching(m);
}

} // namespace gna
} // namespace nina

#endif // NATALIECPLEX_H
