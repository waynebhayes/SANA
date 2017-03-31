#pragma once

#include <string>
#include <stdio.h>
#include <vector>
#include <map>
#include <iostream>
#include "graph.hpp"
#include "Eigen/Dense"

using namespace Eigen;
using std::string;
using std::vector;
using std::map;
using std::cout;

// simple implementation for a adj graph
// use for alternate distance metric
class AdjGraph
{
  int s;
  MatrixXd m;
  vector<string> itsm;
  map<string,int> stim;
  public:
    void init(Graph *G);
    int* size(){return &s;};
    MatrixXd* matrix(){return &m;};
    string stringOf(int i){return itsm[i];};
    int indexOf(string s){return stim[s];};
    void print() {cout << m << "\n";};
};

void AdjGraph::init(Graph *G)
{
  itsm = (*G).nodes();
  s = itsm.size();
  m.resize(s,s);
  m.fill(0);
  for(int i=0;i<s;i++)
    stim[itsm[i]] = i;
  for(int i=0;i<s;i++)
  {
    string src=itsm[i];
    vlist v = (*G).neighbors(src);
    vlist::iterator it=v.begin(), vend=v.end();
    for(;it!=vend;it++)
      m(stim[src],stim[*it])=1;
  }
}

