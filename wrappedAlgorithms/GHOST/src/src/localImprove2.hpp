#include <vector>
#include <string>
#include <tgmath.h>
#include <stdlib.h>
#include <time.h>

#include "graph.hpp"
#include "spectralToDistance.hpp"
#include "tabou_qap.hpp"

using std::string;
using std::vector;
using std::pair;

typedef boost::unordered_map<pair<string,string>, D_alpha> distmap;

int matchingEdges(Graph& G, Graph& H, bmap& result)
{
  int matchingEdges = 0;
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
  }
  return matchingEdges / 2;
}

vlist extendNeighborhood(vlist *l, Graph g)
{
  vlist result;
  for(auto it = l->begin(); it != l->end(); it++){
    result.insert(*it);
    vlist *n = g.neighbors2(*it);
    for(auto it2 = n->begin(); it2 != n->end(); it2++) 
      result.insert(*it2);
  }
  return result;
}

int searchIter2(Graph& G, Graph& H, bmap *f, int numthreads)
{
  int totalDelt = 0;
  vector<string> hverts = H.nodes();
  vector<string> lefts;
  for(auto it = f->left.begin(); it != f->left.end(); it++) 
    lefts.push_back(it->first);

  ProgressBar pbar(lefts.size(), bclock::local_time());
  srand(time(0));
  
  for(auto it = lefts.begin(); it != lefts.end(); it++){
    string u = *it;
    string w = f->left.at(u);

    vlist nl;
    vlist nr;
    nl.insert(u);
    nr.insert(w);
    for(int i = 0; i < 2; i++){
      nl = extendNeighborhood(&nl, G);
      nr = extendNeighborhood(&nr, H);
    }

    vector<string> leftNodes;
    vector<string> rightNodes;

    for(auto it = nl.begin(); it != nl.end(); it++){
      if(find(leftNodes.begin(), leftNodes.end(), *it) == leftNodes.end())
        leftNodes.push_back(*it);
      auto a = f->left.find(*it);
      if(a != f->left.end()){
        if(find(rightNodes.begin(), rightNodes.end(), a->second) == rightNodes.end())
          rightNodes.push_back(a->second);
      }
    }
    for(auto it = nr.begin(); it != nr.end(); it++){
      if(find(rightNodes.begin(), rightNodes.end(), *it) == rightNodes.end())
        rightNodes.push_back(*it);
      auto a = f->right.find(*it);
      if(a != f->right.end()){
        if(find(leftNodes.begin(), leftNodes.end(), a->second) == leftNodes.end())
          leftNodes.push_back(a->second);
      }
    }

    vector<int> a, b;
    int n = leftNodes.size();
    if(rightNodes.size() > n) n = rightNodes.size();
    vector<int> seed;
    vector<int> usedVals;
    for(int i = 0; i < n; i++) seed.push_back(-1);

    for(int i = 0; i < n; i++){
      string s = "";
      if(i < leftNodes.size() && f->left.find(leftNodes[i]) != f->left.end()) 
        s = f->left.find(leftNodes[i])->second;

      vlist *adj1 = i < leftNodes.size() ? G.neighbors2(leftNodes[i]) : NULL;
      vlist *adj2 = i < rightNodes.size() ? H.neighbors2(rightNodes[i]) : NULL;

      for(int j = 0; j < n; j++){
        if(adj1 && j < leftNodes.size() && adj1->find(leftNodes[j]) != adj1->end()) a.push_back(1);
        else a.push_back(0);
        if(adj2 && j < rightNodes.size() && adj2->find(rightNodes[j]) != adj2->end()) b.push_back(-1);
        else b.push_back(0);
        if(j < rightNodes.size() && rightNodes[j] == s){
          seed[i] = j;
          usedVals.push_back(j);
        }
      }
    }

    make_heap(usedVals.begin(), usedVals.end());
    int j = n-1;
    for(int i = 0; i < n; i++){
      if(seed[i] == -1){
        while(usedVals.front() == j){
          j--;
          pop_heap(usedVals.begin(), usedVals.end());
          usedVals.pop_back();
        }
        seed[i] = j;
        j--;
      } 
    }

    vector<int> ans = doAlignment(n, a, b, 5000, &seed[0], false);

    for(int i = 0; i < leftNodes.size(); i++){
      f->left.erase(leftNodes[i]);
    }
    for(int i = 0; i < rightNodes.size(); i++){
      f->right.erase(rightNodes[i]);
    }

    for(int i = 0; i < leftNodes.size(); i++){
      if(ans[i] <= rightNodes.size())
        f->insert(bmap::value_type(leftNodes[i], rightNodes[ans[i]-1]));
    }

    pbar.update();
  }
  return totalDelt; 
}

int localImprove2(Graph& G, Graph& H, bmap *f, int iters, int numP)
{
  int totalDelt = 0;
  for(int i=0; i < iters || iters == -1; i++){
    ptime t = bclock::local_time();
    int d = searchIter2(G, H, f, numP);
    cout << "\nadded " << d << " edges in " << (bclock::local_time() - t).total_milliseconds() << "ms\n";
    totalDelt += d;
    if(d == 0) break;
  }
  return totalDelt;
}
