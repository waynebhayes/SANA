/* 
 * lgfparser.h
 *
 *  Created on: 17-jan-2012
 *      Author: M. El-Kebir
 *
 */

#ifndef LGFPARSER_H_
#define LGFPARSER_H_

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <map>
#include <string>
#include <lemon/core.h>
#include <lemon/lgf_reader.h>
#include "input/parser.h"
#include "verbose.h"

namespace nina
{

template<typename GR>
class LgfParser : public Parser<GR>
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
  typedef typename Parent::WeightNodeMap WeightNodeMap;
  typedef typename Parent::WeightEdgeMap WeightEdgeMap;

  using Parent::_filename;
  using Parent::_pG;
  using Parent::_pIdNodeMap;
  using Parent::_pInvIdNodeMap;
  using Parent::_pWeightNodeMap;
  using Parent::_pWeightEdgeMap;
  using Parent::_nNodes;
  using Parent::_nEdges;

public:
  LgfParser(const std::string& filename);
  bool parse();
};

template<typename GR>
inline LgfParser<GR>::LgfParser(const std::string& filename)
  : Parent(filename)
{
}

template<typename GR>
inline bool LgfParser<GR>::parse()
{
  if (!_pG)
    return false;

  _pG->clear();

  std::ifstream inFile(_filename.c_str());
  if (!inFile.good())
  {
    std::cerr << "Error: could not open file " 
              << _filename << " for reading" << std::endl;
    return false;
  }

  lemon::GraphReader<Graph> graphReader(*_pG, inFile);

  if (_pIdNodeMap)
    graphReader.nodeMap("id", *_pIdNodeMap);

  if (_pWeightNodeMap)
    graphReader.nodeMap("weight", *_pWeightNodeMap);

  if (_pWeightEdgeMap)
    graphReader.edgeMap("weight", *_pWeightEdgeMap);
    
  graphReader.run();

  inFile.close();

  for (NodeIt n(*_pG); n != lemon::INVALID; ++n)
  {
    if (_pIdNodeMap)
    {
      const std::string& label = (*_pIdNodeMap)[n];
      if (_pInvIdNodeMap->find(label) == _pInvIdNodeMap->end())
      {
        (*_pInvIdNodeMap)[label] = n;
      }
      else
      {
        std::cerr << "Error: duplicate node id: " << label << std::endl;
        return false;
      }
    }
    _nNodes++;
  }

  for (EdgeIt e(*_pG); e != lemon::INVALID; ++e) _nEdges++;

  return true;
}

} // namespace nina

#endif /* LGFPARSER_H_ */
