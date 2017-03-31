#pragma once

#include <stdlib.h>
#include <string>
#include <vector>
#include <boost/unordered_set.hpp>
#include <boost/unordered_map.hpp>
#include <iostream>

using std::string;
using std::vector;

typedef boost::unordered_set<string> vlist;
typedef boost::unordered_map<string, vlist> adjacencyMap;

// adjacency list representation
// multiples of the same edge not allowed
class Graph
{
  adjacencyMap m;
  adjacencyMap dm;
  bool directed;
  string name;
  public:
    void setName(string s) {name=s;};
    string getName() {return name;};
    void addVertex(string v) {vlist e; m[v]=e;};
    void addEdge(string v1, string v2) 
      {m[v1].insert(v2); m[v2].insert(v1);
       if(directed) {dm[v1].insert(v2);}};
    vector<string> nodes();
    vlist neighbors(string v) 
      {if(directed) return dm[v];
       else return m[v];};
    vlist* neighbors2(string v) {return &m[v];};
    void direct(bool d) {directed=d;};
    void print();
};

vector<string> Graph::nodes()
{
  adjacencyMap::iterator iter = m.begin(),
  iend = m.end();
  vector<string> result;
  for(; iter != iend; ++iter)
    result.push_back(iter->first);
  return result;
}

void Graph::print()
{
  adjacencyMap::iterator iter = m.begin(),
  iend = m.end();
  for(; iter != iend; ++iter)
  {
    std::cout << iter->first << ": [";
    vlist::iterator vit = (iter->second).begin(),
    vend = (iter->second).end();
    if(vit == vend)
      std::cout << "]\n";
    else
    {
      vlist::iterator vtemp = (iter->second).begin();
      vtemp++;
      for(; vtemp != vend; vit++, vtemp++)
        std::cout << (*vit) << ", ";
      std::cout << (*vit) << "]\n";
    }
  }
}
