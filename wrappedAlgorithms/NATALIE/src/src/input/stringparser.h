/*
 * stringparser.h
 *
 *  Created on: 26-may-2011
 *      Author: M. El-Kebir
 *
 * It may make sense to consider another edge weight 
 * (instead of combined_score)
 */

#ifndef STRINGPARSER_H_
#define STRINGPARSER_H_

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
class StringParser : public Parser<GR>
{
public:
  /// Graph type
  typedef GR Graph;
  /// Base class type
  typedef Parser<GR> Parent;

private:
  TEMPLATE_GRAPH_TYPEDEFS(Graph);

  double _threshold;

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
  StringParser(const std::string& filename, double threshold);
  bool parse();
};

template<typename GR>
inline StringParser<GR>::StringParser(const std::string& filename,
                                      double threshold)
  : Parent(filename)
  , _threshold(threshold)
{
}

template<typename GR>
inline bool StringParser<GR>::parse()
{
  if (!_pG)
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

    std::string value;
    while (lineStream.good())
      lineStream >> value;
 
    std::stringstream valueStream(value);
    double combined_score = 0;
    valueStream >> combined_score;

    if (!valueStream.eof())
    {
      if (g_verbosity >= VERBOSE_DEBUG)
      {
        std::cout << "Warning: no real-valued score ('"
                  << protein1 << "','" << protein2
                  << "') in '"  << _filename << "'. Skipped." << std::endl;
      }

      // skip the edge
      continue;
    }

    if (combined_score < _threshold * 1000)
    {
      if (g_verbosity >= VERBOSE_DEBUG)
      {
        std::cout << "Warning: score ('"
                  << protein1 << "','" << protein2
                  << "') in '"  << _filename << "' below the threhold. Skipped." << std::endl;
      }

      // skip the edge
      continue;
    }

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
      if (_pWeightEdgeMap) _pWeightEdgeMap->set(e, combined_score / 1000.0);
    }
  }

  return true;
}

} // namespace nina

#endif /* STRINGPARSER_H_ */
