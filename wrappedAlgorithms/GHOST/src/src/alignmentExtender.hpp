#include <string>
#include <vector>
#include <tgmath.h>
#include <boost/unordered_map.hpp>
#include <boost/bimap/bimap.hpp>
#include <boost/bimap/unordered_set_of.hpp>
#include "graph.hpp"
#include "hungarian.hpp"

using std::pair;
using std::string;
using std::vector;
using std::cout;

typedef boost::bimaps::unordered_set_of<string> uset;
typedef boost::bimaps::bimap<uset, uset> bmap;
typedef boost::unordered_map<pair<string,string>, D_alpha> distmap;

void adjustWeights(Graph &g1, Graph &g2, bmap *align, vector<D_alpha> *matches){
  for(int i = 0; i < matches->size(); i++){
    int count = 0;
    vlist *adj1 = g1.neighbors2((*matches)[i].get_n1());
    vlist *adj2 = g2.neighbors2((*matches)[i].get_n2());
    for(auto it = adj1->begin(); it != adj1->end(); it++){
      auto mapped = align->left.find(*it);
      if(mapped != align->left.end() && adj2->find(mapped->second) != adj2->end()) count++;
    }
    D_alpha d = (*matches)[i];
    d.set_da(d.get_da() / (count/2.0 + 1));
    (*matches)[i] = d;
  }
}

//If the vertices in best are not already part of a closer alignment, align them
void insertIfBetter(bmap* alignment, D_alpha best, distmap& dalphas)
{ 
  bmap::left_const_iterator left = alignment->left.find(best.get_n1());
  bmap::right_const_iterator right = alignment->right.find(best.get_n2());
  double d = best.get_da();
  if((left == alignment->left.end() || 
      d < dalphas.at(make_pair(left->first,left->second)).get_da())
      && (right == alignment->right.end() || 
      d < dalphas.at(make_pair(right->second,right->first)).get_da())){
    //Remove any existing alignments, add this one
    alignment->left.erase(best.get_n1());
    alignment->right.erase(best.get_n2());
    alignment->insert(bmap::value_type(best.get_n1(), best.get_n2()));
  }
}

//Returns a vector of the D_alphas for the k nearest n2 nodes for each n1 node,
//leaving out any that are already aligned
vector<D_alpha> computePairwiseScores(vlist *n1, vlist *n2, bmap& alignment, distmap& dalphas, int k)
{
  vector<D_alpha> ans;
  for(auto it1 = n1->begin(); it1 != n1->end(); ++it1){
    if(alignment.left.find(*it1) != alignment.left.end()) continue; //Skip if already aligned
    vector<D_alpha> maxheap;
    for(auto it2 = n2->begin(); it2 != n2->end(); ++it2){
      if(alignment.right.find(*it2) != alignment.right.end()) continue; //Skip if already aligned

      D_alpha d = dalphas.at(make_pair(*it1,*it2));
      //If we don't have k yet, add it
      if(maxheap.size() < k || k==-1){
        maxheap.push_back(d);
        push_heap(maxheap.begin(), maxheap.end(), CompareD_alphaL());
      //Otherwise add and trim back down to k
      } else if(d.get_da() < maxheap.front().get_da()){
        pop_heap(maxheap.begin(), maxheap.end(), CompareD_alphaL());
        maxheap.pop_back();
        maxheap.push_back(d);
        push_heap(maxheap.begin(), maxheap.end(), CompareD_alphaL());
      }
    }
    ans.reserve(ans.size()+maxheap.size());
    ans.insert(ans.end(), maxheap.begin(), maxheap.end());
  }
  return ans;
}

int find(vector<string>& v, string s){
  for(int i=0; i < v.size(); i++) if(v[i]==s) return i;
  return -1;
}

//Set up and perform the alignment algorithm on the list of potential matches
vector<D_alpha> performMatching(vector<D_alpha>& matches){
  vector<string> nodes1;
  vector<string> nodes2;

  vector<vector<double>> matrix;
  int maxwidth = 0;
  for(auto it = matches.begin(); it != matches.end(); it++){
    string s1 = it->get_n1(), s2 = it->get_n2();
    int i = find(nodes1, s1);
    if(i == -1){
      i = nodes1.size();
      nodes1.push_back(s1);
      matrix.resize(i+1);
    }
    int j = find(nodes2, s2);
    if(j == -1){
      j = nodes2.size();
      nodes2.push_back(s2);
      if(j >= maxwidth) maxwidth = j+1;
    }
    if(j >= matrix[i].size()) matrix[i].resize(j+1, 100);
    matrix[i][j] = it->get_da();
  }
  if(maxwidth < matrix.size()) maxwidth = matrix.size();

  float** mat = new float*[matrix.size()];
  for(int i = 0; i < matrix.size(); i++){
    mat[i] = new float[maxwidth];
    for(int j = 0; j < maxwidth; j++){
      if(j >= matrix[i].size()) mat[i][j] = 100;
      else mat[i][j] = matrix[i][j];
    }
  }
  ssize_t** res = kuhn_match(mat, matrix.size(), maxwidth);
  vector<D_alpha> ans;
  for(int i = 0; i < matrix.size(); i++){
    if(res[i][0] < nodes1.size() && res[i][1] < nodes2.size()){
      string a = nodes1[res[i][0]], b = nodes2[res[i][1]];
      for(auto it = matches.begin(); it != matches.end(); it++){
        if(it->get_n1() == a && it->get_n2() == b){
          ans.push_back(*it);
          break;
        }
      }
    }
    delete[] res[i];
    delete[] mat[i];
  }
  delete[] res;
  delete[] mat;
  return ans;
}

//Filter the matches with D_seq>beta
bool checkSeq(D_alpha d) { return d.get_ds() > 1; }

//Algorithm2: from a seed pair, extend the alignment locally
void extendAlignment(D_alpha seed, Graph& g1, Graph& g2, bmap *alignment, vector<D_alpha>& dalphas, distmap& d, int k)
{
  vector<D_alpha> maxP;
  maxP.push_back(seed);

  while(!maxP.empty()){
    D_alpha best = maxP.front();
    pop_heap(maxP.begin(), maxP.end(), CompareD_alphaL());
    maxP.pop_back();
    insertIfBetter(alignment, best, d);
    
    vlist *neighbors1 = g1.neighbors2(best.get_n1());
    vlist *neighbors2 = g2.neighbors2(best.get_n2());

    vector<D_alpha> matches = computePairwiseScores(neighbors1, neighbors2, *alignment, d, k);
    if(matches.size() == 0) continue;

    //Do the alignment problem
    matches = performMatching(matches);
    //Filter out matches that are already aligned or have D_seq>beta
    auto end = remove_if(matches.begin(), matches.end(), checkSeq);

    for(auto it = matches.begin(); it != end; ++it){
      alignment->insert(bmap::value_type(it->get_n1(), it->get_n2()));
      maxP.push_back(*it);
      push_heap(maxP.begin(), maxP.end(), CompareD_alphaL());
    }
  }
}
