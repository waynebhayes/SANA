/*
 * ledaparser.h
 *
 *  Created on: 25-jan-2012
 *     Authors: M. El-Kebir
 */

#ifndef LEDAPARSER_H_
#define LEDAPARSER_H_

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
class LedaParser : public Parser<GR>
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
  LedaParser(const std::string& filename);
  bool parse();

private:
  bool parseNodes(std::istream& inFile, int& row);
  bool parseEdges(std::istream& inFile, int& row);
};

template<typename GR>
inline LedaParser<GR>::LedaParser(const std::string& filename)
  : Parent(filename)
{
}

template<typename GR>
inline bool LedaParser<GR>::parse()
{
  static const std::string header("LEDA.GRAPH");

  if (!_pG)
    return false;

  _pG->clear();
  _nNodes = _nEdges = 0;

  std::ifstream inFile(_filename.c_str());
  if (!inFile.good())
  {
    std::cerr << "Error: could not open file " 
              << _filename << " for reading" << std::endl;
    return false;
  }

  /* we parse the first three lines, starting with the header */
  int row = 0;
  std::string line, nodeDataType, edgeDataType, dir_undir_flag;

  // skip comments
  do {
    std::getline(inFile, line);
    row++;
  }
  while (line.size() && line[0] == '#');

  if (line.substr(0, header.size()) != header)
  {
    std::cerr << "Error: expected header '" << header
              << "' at line " << row << std::endl;
    return false;
  }

  /* now we parse the node data type */

  // skip comments
  do {
    std::getline(inFile, line);
    row++;
  }
  while (line.size() && line[0] == '#');

  nodeDataType = line;
  
  /* next is the edge data type */

  // skip comments
  do {
    std::getline(inFile, line);
    row++;
  }
  while (line.size() && line[0] == '#');

  edgeDataType = line;

  /* next is the directed/undirected flag */

  // skip comments
  do {
    std::getline(inFile, line);
    row++;
  }
  while (line.size() && line[0] == '#');

  dir_undir_flag = line;

  /* now it's time for the nodes and finally the edges */
  bool res = parseNodes(inFile, row) && parseEdges(inFile, row);

  inFile.close();
  return res;
}

template<typename GR>
inline bool LedaParser<GR>::parseNodes(std::istream& inFile, int& row)
{
  std::string line;

  // skip comments
  do {
    std::getline(inFile, line);
    row++;
  }
  while (line.size() && line[0] == '#');

  std::stringstream lineStream(line);
  
  lineStream >> _nNodes;
  if (!lineStream.eof())
  {
    std::cerr << "Error: expected number of nodes at line " << row << std::endl;
    std::cerr << "Nodes = (" << _nNodes <<")"<< std::endl;
  //  return false;
  }

  for (int i = 0; i < _nNodes; i++)
  {
    if (inFile.eof())
    {
      std::cerr << "Error: premature end-of-file. Expecting " 
                << _nNodes << " nodes" << std::endl;
      return false;
    }

    // skip comments
    do {
      std::getline(inFile, line);
      row++;
    }
    while (line.size() && line[0] == '#');

    if(line[line.size()-1] != '|')
        line = line.substr(0,line.size() - 1); // fixing the bug

    if (!(line.size() > 4 && line[0] == '|' && line[1] == '{'
          && line[line.size()-2] == '}' && line[line.size()-1] == '|'))
    {
      std::cerr << "Error: invalid node at line " << row << std::endl;
      std::cerr << line << std::endl; 
      std::cerr<< line.size() <<" (" << line <<")" << std::endl;
      return false;
    }

    Node n = _pG->addNode();
    if (_pIdNodeMap)
    {
      std::string id = line.substr(2, line.size() - 4);
      if (_pInvIdNodeMap->find(id) != _pInvIdNodeMap->end())
      {
        std::cerr << "Error: duplicate node id '" << id
                  << "' at line " << row << std::endl;
        return false;
      }
      else
      {
        _pIdNodeMap->set(n, id);
        (*_pInvIdNodeMap)[id] = n;
      }
    }
  }

  return true;
}

template<typename GR>
inline bool LedaParser<GR>::parseEdges(std::istream& inFile, int& row)
{
  std::string line;

  // skip comments
  do {
    std::getline(inFile, line);
    row++;
  }
  while (line.size() && line[0] == '#');

  std::stringstream lineStream(line);
  lineStream >> _nEdges;
  if (!lineStream.eof())
  {
    std::cerr << "Error: expected number of edges at line " << row << std::endl;
    std::cerr << line << std::endl;
    std::cerr << "Edges = (" << _nEdges <<")"<< std::endl;
    //return false;
  }

  for (int i = 0; i < _nEdges; i++)
  {
    if (inFile.eof())
    {
      std::cerr << "Error: premature end-of-file. Expecting " 
                << _nEdges << "edges" << std::endl;
      return false;
    }

    // skip comments
    do {
      std::getline(inFile, line);
      row++;
    }
    while (line.size() && line[0] == '#');

    int source, target, revEdge;
    double weight = 1;
    char c;

    int res = sscanf(line.c_str(), "%d %d %d |{%lf}|%c",
                     &source, &target, &revEdge, &weight, &c);

    // std::cerr << source << "/" << target <<"/" << revEdge << "/" << weight << "/" << (unsigned short)c << std::endl;

    if (!(res == 3 || res == 4))
    {
      std::cerr << "Error: invalid edge at line " << row << std::endl;
      std::cerr << line << std::endl;
      return false;
    }

    Node u = source-1 <= _pG->maxNodeId() ?
                _pG->nodeFromId(source-1) : lemon::INVALID;
    Node v = target-1 <= _pG->maxNodeId() ?
                _pG->nodeFromId(target-1) : lemon::INVALID;
    if (u == lemon::INVALID)
    {
      std::cerr << "Error: invalid edge at line " << row 
                << ". No node with id " << source << std::endl;
      return false;
    }
    else if (v == lemon::INVALID)
    {
      std::cerr << "Error: invalid edge at line " << row 
                << ". No node with id " << target << std::endl;
      return false;
    }

    Edge e = _pG->addEdge(u, v); 
    if (_pWeightEdgeMap) _pWeightEdgeMap->set(e, weight);
  }

  return true;
}

} // namespace nina

#endif /* LEDAPARSER_H_ */
