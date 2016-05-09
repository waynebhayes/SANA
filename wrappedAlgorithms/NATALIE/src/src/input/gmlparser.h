/* 
 * gmlparser.h
 *
 *  Created on: 7-feb-2011
 *      Author: M. El-Kebir
 */

#ifndef GMLPARSER_H_
#define GMLPARSER_H_

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <map>
#include <string>
#include <lemon/core.h>
#include "gml-parser/gml_parser.h"
#include "input/parser.h"

namespace nina
{

template<typename GR>
class GMLParser : public Parser<GR>
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
  typedef typename Graph::template EdgeMap<std::string> LabelEdgeMap;
  typedef typename Graph::template NodeMap<long> RealIdNodeMap;
  typedef std::map<long, typename Graph::Node> InvRealIdNodeMap;

  using Parent::_filename;
  using Parent::_pG;
  using Parent::_pIdNodeMap;
  using Parent::_pInvIdNodeMap;
  using Parent::_pWeightEdgeMap;
  using Parent::_nNodes;
  using Parent::_nEdges;

private:
  struct GML_pair* _pList;
  struct GML_stat* _pStat;
  InvRealIdNodeMap _invRealIdNodeMap;
  RealIdNodeMap* _pRealIdNodeMap;
  LabelEdgeMap* _pLabelEdgeMap;

  bool parseGraph();
  bool parseNodes();
  bool parseEdges();

public:
  GMLParser(const std::string& filename);
  virtual ~GMLParser();
  bool parse();
  void destroyParseTree();

  const InvRealIdNodeMap& getInvRealIdNodeMap() const
  {
    return _invRealIdNodeMap;
  }

  InvRealIdNodeMap& getInvRealIdNodeMap()
  {
    return _invRealIdNodeMap;
  }

  const RealIdNodeMap* getRealIdNodeMap() const
  {
    return _pRealIdNodeMap;
  }

  RealIdNodeMap* getRealIdNodeMap()
  {
    return _pRealIdNodeMap;
  }

  void setRealIdNodeMap(RealIdNodeMap* pRealIdNodeMap)
  {
    _pRealIdNodeMap = pRealIdNodeMap;
  }

  const LabelEdgeMap* getLabelEdgeMap() const
  {
    return _pLabelEdgeMap;
  }

  LabelEdgeMap* getLabelEdgeMap()
  {
    return _pLabelEdgeMap;
  }

  void setLabelEdgeMap(LabelEdgeMap* pLabelEdgeMap)
  {
    _pLabelEdgeMap = pLabelEdgeMap;
  }
};

template<typename GR>
inline GMLParser<GR>::GMLParser(const std::string& filename)
  : Parent(filename)
  , _pList(NULL)
  , _pStat(new GML_stat())
  , _invRealIdNodeMap()
  , _pRealIdNodeMap(NULL)
  , _pLabelEdgeMap(NULL)
{
  _pStat->key_list = NULL;
}

template<typename GR>
inline bool GMLParser<GR>::parse()
{
  if (!_pG && !_pInvIdNodeMap)
    return false;

  _pG->clear();
  _nNodes = _nEdges = 0;

  FILE* file = fopen(_filename.c_str(), "r");
  if (!file)
  {
    fprintf(stderr, "Error: could not open file %s for reading\n",
            _filename.c_str());
    return false;
  }

  _pList = GML_parser(file, _pStat, 0);
  if (_pList && _pStat->err.err_num == GML_OK)
  {
    bool res = parseGraph();
    destroyParseTree();
    return res;
  }
  else
  {
    fprintf(stderr, 
        "Error: an error occurred while reading line %d column %d of %s:\n",
        _pStat->err.line, _pStat->err.column, _filename.c_str());

    switch (_pStat->err.err_num)
    {
      case GML_UNEXPECTED:
        fprintf(stderr, "  unexpected character\n");
        break;
      case GML_SYNTAX:
        fprintf(stderr, "  syntax error\n");
        break;
      case GML_PREMATURE_EOF:
        fprintf(stderr, "  premature EOF in string\n");
        break;
      case GML_TOO_MANY_DIGITS:
        fprintf(stderr, "  number with too many digits\n");
        break;
      case GML_OPEN_BRACKET:
        fprintf(stderr, "  open bracket(s) left at EOF\n");
        break;
      case GML_TOO_MANY_BRACKETS:
        fprintf(stderr, "  too many closing brackets\n");
        break;
      default:
        break;
    }

    destroyParseTree();
    return false;
  }
}

template<typename GR>
inline GMLParser<GR>::~GMLParser()
{
  destroyParseTree();
}

template<typename GR>
inline void GMLParser<GR>::destroyParseTree()
{
  if (_pList)
  {
    GML_free_list(_pList, _pStat->key_list);
    _pList = NULL;
  }
  if (_pStat)
  {
    delete _pStat;
    _pStat = NULL;
  }
}

template<typename GR>
inline bool GMLParser<GR>::parseGraph()
{
  assert(_pList);

  if (strcmp(_pList->key, "graph") != 0 || _pList->kind != GML_LIST)
  {
    fprintf(stderr, "Error: empty graph\n");
    return false;
  }

  if (!parseNodes()) return false;
  if (!parseEdges()) return false;

  return true;
}

template<typename GR>
inline bool GMLParser<GR>::parseNodes()
{
  assert(_pList && strcmp(_pList->key, "graph") == 0 &&
         _pList->kind == GML_LIST);

  // create all nodes
  struct GML_pair* it = _pList->value.list;
  while (it)
  {
    // are we dealing with a node?
    if (strcmp(it->key, "node") == 0 && it->kind == GML_LIST)
    {
      // find the node id and possibly the label
      long id = -1;
      std::string label;
      struct GML_pair* it2 = it->value.list;
      while (it2 && (id == -1 || label == ""))
      {
        if (strcmp(it2->key, "id") == 0 && it2->kind == GML_INT)
        {
          id = it2->value.integer;
        }
        else if (strcmp(it2->key, "label") == 0 && it2->kind == GML_STRING)
        {
          label = it2->value.string;
        }

        it2 = it2->next;
      }

      if (id == -1)
      {
        // node without an id
        fprintf(stderr, "Error: node without an id\n");
        return false;
      }
      else if (label == "")
      {
        fprintf(stderr, "Warning: node %ld does not have a label\n", id);
        return false;
      }
      else if (_invRealIdNodeMap.find(id) != _invRealIdNodeMap.end())
      {
        // duplicate node id
        fprintf(stderr, "Error: duplicate node id: %ld\n", id);
        return false;
      }
      else if (_pInvIdNodeMap->find(label) != _pInvIdNodeMap->end())
      {
        // duplicate node label
        fprintf(stderr, "Error: duplicate node label: %s\n", label.c_str());
        return false;
      }
      else
      {
        Node n = _pG->addNode();
        if (_pRealIdNodeMap) _pRealIdNodeMap->set(n, id);
        _invRealIdNodeMap[id] = n;
        _nNodes++;

        if (_pIdNodeMap) _pIdNodeMap->set(n, label);
        (*_pInvIdNodeMap)[label] = n;
      }
    }

    it = it->next;
  }

  return true;
}

template<typename GR>
inline bool GMLParser<GR>::parseEdges()
{
  assert(_pList && strcmp(_pList->key, "graph") == 0 &&
         _pList->kind == GML_LIST);

  // create all edges
  struct GML_pair* it = _pList->value.list;
  while (it)
  {
    // are we dealing with a edge?
    if (strcmp(it->key, "edge") == 0 && it->kind == GML_LIST)
    {
      // find source id, target id and possible label, and weight
      std::string label;
      double weight = 1;
      long source_id = -1, target_id = -1;
      struct GML_pair* it2 = it->value.list;
      while (it2 && (source_id == -1 || target_id == -1 || label == ""))
      {
        if (strcmp(it2->key, "source") == 0 && it2->kind == GML_INT)
        {
          source_id = it2->value.integer;
        }
        else if(strcmp(it2->key, "target") == 0 && it2->kind == GML_INT)
        {
          target_id = it2->value.integer;
        }
        else if (strcmp(it2->key, "label") == 0 && it2->kind == GML_STRING)
        {
          label = it2->value.string;
        }
        else if (strcmp(it2->key, "weight") == 0 && it2->kind == GML_DOUBLE)
        {
          weight = it2->value.floating;
        }

        it2 = it2->next;
      }

      if (source_id == -1 || target_id == -1)
      {
        // edge without a source and target node id
        fprintf(stderr, "Error: edge without a source/target node id\n");
        return false;
      }
      else if (_invRealIdNodeMap.find(source_id) == _invRealIdNodeMap.end())
      {
        fprintf(stderr, 
            "Error: edge references a non-existing node with (source) id %ld\n",
            source_id);
        return false;
      }
      else if (_invRealIdNodeMap.find(target_id) == _invRealIdNodeMap.end())
      {
        fprintf(stderr, 
            "Error: edge references a non-existing node"
            "with (target) id %ld\n", target_id);
        return false;
      }
      else
      {
        Node u = _invRealIdNodeMap.find(source_id)->second;
        Node v = _invRealIdNodeMap.find(target_id)->second;
        Edge e = _pG->addEdge(u, v);
        if (_pLabelEdgeMap) _pLabelEdgeMap->set(e, label);
        if (_pWeightEdgeMap) _pWeightEdgeMap->set(e, weight);
        _nEdges++;
      }
    }

    it = it->next;
  }

  return true;
}

} // namespace nina

#endif /* GMLPARSER_H_ */
