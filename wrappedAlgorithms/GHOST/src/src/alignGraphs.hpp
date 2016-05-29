#include <vector>
#include <algorithm>
#include <string>
#include <boost/bimap/bimap.hpp>
#include <boost/bimap/unordered_set_of.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <time.h>
#include "graph.hpp"
#include "alignmentExtender.hpp"

using std::string;
using std::ifstream;
using std::ofstream;
using std::vector;
using std::cout;

typedef boost::posix_time::microsec_clock bclock;
typedef boost::posix_time::ptime ptime;

distmap getDistMap(vector<D_alpha>& v)
{
  distmap m;
  for(auto it = v.begin(); it != v.end(); it++) m.insert(make_pair(make_pair(it->get_n1(), it->get_n2()), *it));
  return m;
}

pair<D_alpha, vector<D_alpha>> getNextSeed(double skipProb, vector<D_alpha> *minP)
{
  vector<D_alpha> skipped;
  D_alpha d = minP->front();
  while(1){
    pop_heap (minP->begin(), minP->end(),CompareD_alphaG());
    minP->pop_back();
    if(rand() > ((1.0-skipProb)*RAND_MAX) && minP->size() > 0){
      skipped.push_back(d);
      d = minP->front();
    }else break;
  }
  return std::make_pair(d, skipped);
}

/* initialize bmap f
 * while minP not empty, 
 *  pop D_alpha d
 *  if no part of d in f, GreedyQAPEXtend(G,H,d,f) */
bmap seedAndExtend(Graph& G, Graph& H, vector<D_alpha>& minP, int k, double skipProb)
{
  srand(time(0));
  vector<D_alpha> allDists(minP);
  distmap dmap = getDistMap(allDists);
  bmap f;
  while(minP.size()>0)
  {
    auto next = getNextSeed(skipProb, &minP);
    D_alpha d = next.first;

    string n1 = d.get_n1();
    string n2 = d.get_n2();
    if(f.left.find(n1) == f.left.end() && f.right.find(n2) == f.right.end())
      extendAlignment(d, G, H, &f, allDists, dmap, k);

    for(int i = 0; i < next.second.size(); i++){
      minP.push_back(next.second[i]);
      push_heap(minP.begin(), minP.end(), CompareD_alphaG());
    }
  }
  return f;
}

/* prints all key-value pairs in f */
void printMap(bmap f, string gname, string hname)
{
  ofstream fout (gname+"_vs_"+hname+".af");
  bmap::left_const_iterator iter = f.left.begin(),
  iend = f.left.end();
  for(; iter != iend; ++iter)
  {
    fout << iter->first << "\t" << iter->second << "\n";
  }
  fout.close();
}

void printICS(Graph& G, Graph& H, bmap& result)
{
  int matchingEdges = 0;
  int edgesH = 0;
  for(auto it = result.left.begin(); it != result.left.end(); it++)
  {
    vlist *adj1 = G.neighbors2(it->first);
    vlist *adj2 = H.neighbors2(it->second);
    for(auto it2 = adj1->begin(); it2 != adj1->end(); it2++){
      auto f_a = result.left.find(*it2);
      if(f_a != result.left.end() && adj2->find(f_a->second) != adj2->end()){
        matchingEdges++;
        if(*it2 == it->first) matchingEdges++; //Double count self loops
      }
    }
    for(auto it2 = adj2->begin(); it2 != adj2->end(); it2++) 
      if(result.right.find(*it2) != result.right.end()){
        edgesH++;
        if(*it2 == it->second) edgesH++; //Double count self loops
      }
  }
  vector<string> nodesG = G.nodes();
  int edgesG = 0;
  for(auto it = nodesG.begin(); it != nodesG.end(); it++){
    vlist *n = G.neighbors2(*it);
    edgesG += n->size();
    if(n->find(*it) != n->end()) edgesG++; //Double count self loops
  }

  double ec = ((double)matchingEdges / edgesG) * 100.0;
  double ics = ((double)matchingEdges / edgesH) * 100.0;
  cout << "Edge correctness " << matchingEdges/2 << " / " << edgesG/2 << " = " << ec << "\%\n";
  cout << "ICS = " << ics << "\%\n";
}

bmap alignGraphs(Graph& G, Graph& H, vector<D_alpha>& distances, int k, double skipProb)
{
  cout << "aligning graphs...\n";
  vector<D_alpha> minP;  // empty vector is a heap
  for(auto it = distances.begin(); it != distances.end(); it++)
  {
    minP.push_back(*it);
    push_heap(minP.begin(),minP.end(),CompareD_alphaG());
  }

  ptime t = bclock::local_time();
  bmap result = seedAndExtend(G, H, minP, k, skipProb);
  cout << "aligned " << result.size() << " nodes in " << (bclock::local_time()-t).total_milliseconds() << " milliseconds\n";

  printICS(G, H, result);
  return result;
}
