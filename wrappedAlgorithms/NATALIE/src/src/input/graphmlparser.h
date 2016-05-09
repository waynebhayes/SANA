/*
 * graphmlparser.h
 *
 *  Created on: 7-feb-2011
 *      Author: M. El-Kebir
 */

#ifndef GRAPHMLPARSER_H_
#define GRAPHMLPARSER_H_

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <map>
#include <string>
#include <lemon/core.h>
#include "tinyxml/tinyxml.h"
#include "input/parser.h"

namespace nina
{

template<typename GR>
class GraphMLParser : public Parser<GR>
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
  GraphMLParser(const std::string& filename);
  bool parse();

private:
  bool parseNodes(const TiXmlElement* pGraphElement);
  bool parseEdges(const TiXmlElement* pGraphElement);
};

template<typename GR>
inline GraphMLParser<GR>::GraphMLParser(const std::string& filename)
  : Parent(filename)
{
}

template<typename GR>
inline bool GraphMLParser<GR>::parse()
{
  if (!_pG)
    return false;

  _pG->clear();
  _nNodes = _nEdges = 0;

  TiXmlDocument xmlDoc(_filename.c_str());

  if (!xmlDoc.LoadFile())
  {
    std::cerr << "Error: could not open file " << _filename
              << " for reading" << std::endl
              << xmlDoc.ErrorDesc() << std::endl;
    return false;
  }

  TiXmlElement* pRootElement = xmlDoc.RootElement();
  if (!pRootElement || strcmp(pRootElement->Value(), "graphml"))
  {
    std::cerr << "Expected root element with name 'graphml'" << std::endl;
    return false;
  }

  TiXmlElement* pGraphElement = pRootElement->FirstChildElement("graph");
  if (!pRootElement || strcmp(pRootElement->Value(), "graphml"))
  {
    std::cerr << "Missing mandatory element '/graphml/graph'" << std::endl;
    return false;
  }

  return parseNodes(pGraphElement) && parseEdges(pGraphElement);
}

template<typename GR>
inline bool GraphMLParser<GR>::parseNodes(const TiXmlElement* pGraphElement)
{
  for (const TiXmlElement* pNodeElement = pGraphElement->FirstChildElement("node");
      pNodeElement; pNodeElement = pNodeElement->NextSiblingElement("node"))
  {
    const char* pIdStr = pNodeElement->Attribute("id");

    if (pIdStr == NULL)
    {
      std::cerr << "Error: node without an id" << std::endl;
      return false;
    }
    else if (_pInvIdNodeMap->find(pIdStr) != _pInvIdNodeMap->end())
    {
      std::cerr << "Error: duplicate node id: " << pIdStr << std::endl;
      return false;
    }
    else
    {
      Node n = _pG->addNode();
      if (_pIdNodeMap) _pIdNodeMap->set(n, pIdStr);
      (*_pInvIdNodeMap)[pIdStr] = n;
      _nNodes++;
    }
  }

  return true;
}

template<typename GR>
inline bool GraphMLParser<GR>::parseEdges(const TiXmlElement* pGraphElement)
{
  for (const TiXmlElement* pEdgeElement = pGraphElement->FirstChildElement("edge");
      pEdgeElement; pEdgeElement = pEdgeElement->NextSiblingElement("edge"))
  {
    const char* pSourceStr = pEdgeElement->Attribute("source");
    const char* pTargetStr = pEdgeElement->Attribute("target");

    if (pSourceStr == NULL)
    {
      std::cerr << "Error: edge without a source node" << std::endl;
      return false;
    }
    else if (pTargetStr == NULL)
    {
      std::cerr << "Error: edge without a target node" << std::endl;
      return false;
    }
    else if (_pInvIdNodeMap->find(pSourceStr) == _pInvIdNodeMap->end())
    {
      std::cerr << "Error: edge refers to a non-existing source node with id "
                << pSourceStr << std::endl;
      return false;
    }
    else if (_pInvIdNodeMap->find(pTargetStr) == _pInvIdNodeMap->end())
    {
      std::cerr << "Error: edge refers to a non-existing target node with id "
                << pTargetStr << std::endl;
      return false;
    }
    else
    {
      Node u = (*_pInvIdNodeMap)[pSourceStr];
      Node v = (*_pInvIdNodeMap)[pTargetStr];
      Edge e = _pG->addEdge(u, v);
      _nEdges++;
      double weight = 1; // default weight is 1
      if (_pWeightEdgeMap)
      {
        const TiXmlElement* pDataElement = pEdgeElement->FirstChildElement("data");
        if (pDataElement)
        {
          const char* pKeyStr = pDataElement->Attribute("key");
          if (pKeyStr && strcmp(pKeyStr, "weight") == 0)
          {
            if (sscanf(pDataElement->GetText(), "%lf", &weight) != 1)
            {
              std::cerr << "Error: edge ('" << pSourceStr << "','"
                        << pTargetStr << "') has incorrect weight data"
                        << std::endl;
              return false;
            }
          }
          else
          {
            std::cerr << "Error: edge ('" << pSourceStr << "','"
                      << pTargetStr << "') lacks weight data"
                      << std::endl;
            return false;
          }
        }
        _pWeightEdgeMap->set(e, weight);
      }
    }
  }

  return true;
}

} // namespace nina

#endif /* GRAPHMLPARSER_H_ */
