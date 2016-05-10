/*
 * edgelistparser.h
 *
 *  Created on: 26-dec-2012
 *      Author: M. El-Kebir
 *
 */

#ifndef EDGELISTPARSER_H_
#define EDGELISTPARSER_H_

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <map>
#include <string>
#include <lemon/core.h>
#include "input/parser.h"
#include "verbose.h"

namespace nina
{

template<typename GR>
class EdgeListParser : public Parser<GR>
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
  typedef typename Parent::WeightEdgeMap WeightEdgeMap;

  using Parent::_filename;
  using Parent::_pG;
  using Parent::_pIdNodeMap;
  using Parent::_pInvIdNodeMap;
  using Parent::_pWeightEdgeMap;
  using Parent::_nNodes;
  using Parent::_nEdges;

public:
  EdgeListParser(const std::string& filename);
  bool parse();
};

template<typename GR>
inline EdgeListParser<GR>::EdgeListParser(const std::string& filename)
  : Parent(filename)
{
}

template<typename GR>
inline bool EdgeListParser<GR>::parse()
{
  if (!_pG)
    return false;

  if (!_pInvIdNodeMap)
    return false;

  _pG->clear();
  _nNodes = _nEdges = 0;

  lemon::DynArcLookUp<Graph> arcLookUp(*_pG);

  std::ifstream inFile(_filename.c_str());
  if (!inFile.good())
  {
    std::cerr << "Error: could not open file "
               << _filename << " for reading" << std::endl;
    return false;
  }

  std::string line;
  while (std::getline(inFile, line))
  {
    if (!line.empty() && line[0] == '#')
      continue;

    std::string protein1, protein2;

    std::stringstream lineStream(line);
    lineStream >> protein1 >> protein2;

    Node node1;
    if (_pInvIdNodeMap->find(protein1) == _pInvIdNodeMap->end())
    {
      node1 = _pG->addNode();
      if (_pIdNodeMap) _pIdNodeMap->set(node1, protein1);
      (*_pInvIdNodeMap)[protein1] = node1;
      _nNodes++;
    }
    else
    {
      node1 = (*_pInvIdNodeMap)[protein1];
    }

    Node node2;
    if (_pInvIdNodeMap->find(protein2) == _pInvIdNodeMap->end())
    {
      node2 = _pG->addNode();
      if (_pIdNodeMap) _pIdNodeMap->set(node2, protein2);
      (*_pInvIdNodeMap)[protein2] = node2;
      _nNodes++;
    }
    else
    {
      node2 = (*_pInvIdNodeMap)[protein2];
    }

    if (node1 == node2)
    {
      if (g_verbosity >= VERBOSE_DEBUG)
      {
        std::cout << "Warning: node '" << protein1
                  << "' in '"  << _filename << "' has a self-loop. Skipped."
                  << std::endl;
      }
    }
    else if (arcLookUp(node1, node2) != lemon::INVALID)
    {
      if (g_verbosity >= VERBOSE_DEBUG)
      {
        std::cout << "Warning: multiple edge ('"
                  << protein1 << "','" << protein2
                  << "') in '"  << _filename << "'. Skipped." << std::endl;
      }
    }
    else
    {
      Edge e = _pG->addEdge(node1, node2);
      _nEdges++;
      if (_pWeightEdgeMap) _pWeightEdgeMap->set(e, 1);
    }
  }

  return true;
}

} // namespace nina

#endif /* EDGELISTPARSER_H_ */
