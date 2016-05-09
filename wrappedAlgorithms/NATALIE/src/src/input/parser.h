/*
 * parser.h
 *
 *  Created on: 26-may-2011
 *      Author: M. El-Kebir
 */

#ifndef PARSER_H_
#define PARSER_H_

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <map>
#include <string>
#include <lemon/core.h>
#include <fstream>

template<typename GR,
         typename NWGHT = typename GR::template NodeMap<double>,
         typename NLBL = typename GR::template NodeMap<std::string>,
         typename EWGHT = typename GR::template EdgeMap<double> >
class Parser
{
public:
  typedef GR Graph;

protected:
  TEMPLATE_GRAPH_TYPEDEFS(Graph);

public:
  typedef std::map<std::string, typename Graph::Node> InvIdNodeMap;
  typedef NLBL IdNodeMap;
  typedef NWGHT WeightNodeMap;
  typedef EWGHT WeightEdgeMap;

protected:
  const std::string _filename;
  Graph* _pG;
  IdNodeMap* _pIdNodeMap;
  WeightNodeMap* _pWeightNodeMap;
  WeightEdgeMap* _pWeightEdgeMap;
  InvIdNodeMap* _pInvIdNodeMap;
  int _nNodes;
  int _nEdges;

public:
  Parser(const std::string& filename)
    : _filename(filename)
    , _pG(NULL)
    , _pIdNodeMap(NULL)
    , _pWeightNodeMap(NULL)
    , _pWeightEdgeMap(NULL)
    , _pInvIdNodeMap(NULL)
    , _nNodes(0)
    , _nEdges(0)
  {
  }

  virtual ~Parser() {}
  virtual bool parse() = 0;

  const std::string& getFilename()
  {
    return _filename;
  }

  int getNodeCount() const
  {
    return _nNodes;
  }

  int getEdgeCount() const
  {
    return _nEdges;
  }

  const Graph* getGraph() const
  {
    return _pG;
  }

  Graph* getGraph()
  {
    return _pG;
  }

  void setGraph(Graph* pG)
  {
    _pG = pG;
  }

  const IdNodeMap* getIdNodeMap() const
  {
    return _pIdNodeMap;
  }

  IdNodeMap* getIdNodeMap()
  {
    return _pIdNodeMap;
  }

  void setIdNodeMap(IdNodeMap* pIdNodeMap)
  {
    _pIdNodeMap = pIdNodeMap;
  }

  const WeightEdgeMap* getWeightEdgeMap() const
  {
    return _pWeightEdgeMap;
  }

  WeightEdgeMap* getWeightEdgeMap()
  {
    return _pWeightEdgeMap;
  }

  void setWeightEdgeMap(WeightEdgeMap* pWeightEdgeMap)
  {
    _pWeightEdgeMap = pWeightEdgeMap;
  }

  const WeightNodeMap* getWeightNodeMap() const
  {
    return _pWeightNodeMap;
  }

  WeightNodeMap* getWeightNodeMap()
  {
    return _pWeightNodeMap;
  }

  void setWeightNodeMap(WeightNodeMap* pWeightNodeMap)
  {
    _pWeightNodeMap = pWeightNodeMap;
  }

  const InvIdNodeMap* getInvIdNodeMap() const
  {
    return _pInvIdNodeMap;
  }

  InvIdNodeMap* getInvIdNodeMap()
  {
    return _pInvIdNodeMap;
  }

  void setInvIdNodeMap(InvIdNodeMap* pInvIdNodeMap)
  {
    _pInvIdNodeMap = pInvIdNodeMap;
  }
};

#endif /* PARSER_H_ */
