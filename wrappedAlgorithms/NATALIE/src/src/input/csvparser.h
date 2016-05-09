/* 
 * csvparser.h
 *
 *  Created on: 12-dec-2011
 *     Authors: M. El-Kebir, M.E. van der Wees
 */

#ifndef CSVPARSER_H_
#define CSVPARSER_H_

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <map>
#include <string>
#include <lemon/core.h>
#include <math.h>
#include "input/parser.h"
#include "verbose.h"

namespace nina
{

template<typename GR>
class CSVParser : public Parser<GR>
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

private:
  double _corrThreshold;

public:
  CSVParser(const std::string& filename, double corrThreshold);
  bool parse();
};

template<typename GR>
inline CSVParser<GR>::CSVParser(const std::string& filename,
                                double corrThreshold)
  : Parent(filename)
  , _corrThreshold(corrThreshold)
{
}

template<typename GR>
inline bool CSVParser<GR>::parse()
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
    std::stringstream lineStream(line);

    std::string protein1, protein2;
    double combined_score;

    std::getline(lineStream, protein1, ',');
    std::getline(lineStream, protein2, ',');
    lineStream >> combined_score;

    if (!lineStream.eof())
    {
      if (g_verbosity >= VERBOSE_DEBUG)
      {
        std::cout << "Warning: no real-valued score ('"
                  << protein1 << "','" << protein2
                  << "') in '"  << _filename << "'. Skipped." << std::endl;
      }
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
                  << "' in '"  << _filename
                  << "' has a self-loop. Skipped." << std::endl;
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
    else if (fabs(1 - combined_score) >= _corrThreshold)
    { 
      // Edge does not exist yet
      // Store only edges with weight higher than threshold
      Edge e = _pG->addEdge(node1, node2);
      _nEdges++;
      if (_pWeightEdgeMap) _pWeightEdgeMap->set(e, 1 - combined_score);
    }
  }

  inFile.close(); // dit is wel zo netjes
  return true;
}

} // namespace nina

#endif /* CSVPARSER_H_ */
