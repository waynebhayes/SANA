#pragma once

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <math.h>
#include <tgmath.h>
#include <stdlib.h>

#include <boost/unordered_map.hpp>
#include <boost/thread.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

#include "threadpool.hpp"

#include "gzReader.hpp"
#include "dalpha.hpp"

using std::vector;
using std::string;
using std::cout;
using std::pair;

typedef boost::posix_time::microsec_clock bclock;
typedef boost::posix_time::ptime ptime;
typedef boost::unordered_map<pair<string, string>, double> blastmap;

//Fancy fast log function from fastonebigheader.h
static inline float fastlog2 (float x)
{
  union { float f; uint32_t i; } vx = { x };
  union { uint32_t i; float f; } mx = { (vx.i & 0x007FFFFF) | 0x3f000000 };
  float y = vx.i;
  y *= 1.1920928955078125e-7f;
  return y - 124.22551499f
           - 1.498030302f * mx.f
           - 1.72587999f / (0.3520887068f + mx.f);
}
int min(int a, int b){return a<b?a:b;}

//Helpers for D_alpha calculation
double klDiv(double *p1, double *p2, int count)
{
  double s = 0;
  for(int i = 0; i < count; i++)
    if(p1[i] * p2[i] > 0)
      s += p1[i] * 0.69314718f * fastlog2(p1[i]/p2[i]);
  return s<0?0:s;
}

double jsDist(double *p1, double *p2, int s)
{
  double avg[s];
  for(int i = 0; i < s; i++)
    avg[i] = .5 * (p1[i] + p2[i]);
  return sqrt(.5 * klDiv(p1, avg, s) + .5 * klDiv(p2, avg, s));
}

double DTopo(LevelInfo *v1, LevelInfo *v2, int s)
{
  double dist = 0;
  for(int i = 0; i < s; i++)
    dist += jsDist(&(v1[i].signature)[0], &(v2[i].signature)[0],
        min(v1[i].signature.size(), v2[i].signature.size()));
  return dist/s;
}

//Calculates the D_alphas from node n to all nodes in m2
void distanceWorker(spectramap::value_type n, spectramap* m2,
    D_alpha** result, string* out, ProgressBar *pbar)
{
  auto end = m2->end();
  string name = n.first;
  vector<LevelInfo> info = n.second;
  ostringstream res;
  int i=0;
  for(auto it = m2->begin(); it != end; ++it){
    double topo = DTopo(&info[0], &(it->second)[0], min(info.size(), it->second.size()));
    res << name << "\t" << it->first << "\t" << topo << "\n";
    D_alpha* d = new D_alpha(name, it->first, topo);
    result[i++] = d;
  }
  pbar->update();
  *out = res.str();
}

void applyAlpha(double a, double b, vector<D_alpha> *scores, blastmap* blastscores)
{
  if(!blastscores) return;

  if(a == -1){
    vector<double> maxheap;
    for(auto it = scores->begin(); it != scores->end(); it++){
      if(it->get_da() != 0) maxheap.push_back(it->get_da());
      push_heap(maxheap.begin(), maxheap.end());
      if(maxheap.size() > (int)(.0005 * scores->size())){
        pop_heap(maxheap.begin(), maxheap.end());
        maxheap.pop_back();
      }
    }
    double structval = maxheap.front();

    maxheap.clear();
    for(auto it = scores->begin(); it != scores->end(); it++){
      auto sc = blastscores->find(make_pair(it->get_n1(), it->get_n2()));
      if(sc == blastscores->end()) continue;
      double score = sc->second;
      if(score != 0) maxheap.push_back(score);
      push_heap(maxheap.begin(), maxheap.end());
      if(maxheap.size() > (int)(.0005 * blastscores->size())){
        pop_heap(maxheap.begin(), maxheap.end());
        maxheap.pop_back();
      }
    }
    if(maxheap.size() == 0) a = 1;
    else {
      double seqval = maxheap.front();
      a = seqval / (seqval + structval);
    }
  }

  double maxScore = -1;
  for(auto it = scores->begin(); it != scores->end(); it++){
    double seq;
    auto d = blastscores->find(make_pair(it->get_n1(), it->get_n2()));
    if(d != blastscores->end()){
      seq = d->second;
      if(seq > b) seq = b;
      if(seq > maxScore) maxScore = seq;
    }else seq = b;
    it->set_seq(seq);
  }

  for(auto it = scores->begin(); it != scores->end(); it++){
    double seq = it->get_ds();
    if(seq < b) it->set_seq(seq/maxScore);
    else it->set_seq(1.1);
    it->update_da(a);
  }
}

//Calculates and writes all D_topo values from the given spectral signature files
//Returns
vector<D_alpha> getDistances(string file1, string file2, string outputname, double a, double b, blastmap* blastscores, int numP)
{
  ptime t = bclock::local_time();
  cout << "loading sigs file: " << file1 << "\n";
  spectramap m1 = loadSigs(file1);
  cout << "loaded " << m1.size() << " vertices in " <<
    (bclock::local_time() - t).total_milliseconds() << "ms\n";

  t = bclock::local_time();
  cout << "loading sigs file: " << file2 << "\n";
  spectramap m2 = loadSigs(file2);
  cout << "loaded " << m2.size() << " vertices in " <<
    (bclock::local_time() - t).total_milliseconds() << "ms\n";

  ofstream out(outputname.c_str());

  t = bclock::local_time();

  vector<D_alpha**> results;
  vector<string*> outputs;
  int numthreads = numP==-1?boost::thread::hardware_concurrency():numP;
  if(numthreads < 2) numthreads = 2;
  cout << "\ncalculating D_alphas on " << numthreads << " threads\n";
  boost::threadpool::pool threads(numthreads);

  ProgressBar pbar(m1.size(), t);
  for(spectramap::iterator it1 = m1.begin(); it1 != m1.end(); ++it1){
    results.push_back(new D_alpha*[m2.size()]);
    outputs.push_back(new string);
    threads.schedule(boost::bind(&distanceWorker, *it1, &m2, results.back(),
        outputs.back(), &pbar));
  }

  threads.wait();
  cout << "\nfinished calculating " << (m1.size()*m2.size()) << " distances in " <<
    (bclock::local_time()-t).total_seconds() << "s\n";

  vector<D_alpha> allDistances;
  for(int i=0; i<results.size(); i++){
    out << *(outputs[i]);
    delete outputs[i];
    for(int j=0; j<m2.size(); j++){allDistances.push_back(*(results[i][j])); delete results[i][j];}
    delete[] results[i];
  }

  applyAlpha(a, b, &allDistances, blastscores);

  out.close();
  return allDistances;
}

vector<D_alpha> getDistancesFromFile(string file, double alpha, double beta, blastmap* blastscores)
{
  cout << "reading distances from " << file << "\n";
  vector<D_alpha> allDistances;

  ifstream fin(file);
  if(!fin.good()) {cout << ".sig.gz file" << file << "not found\n"; exit(0);}
  char c = fin.get();
  int alen = 0;
  int blen = 0;
  int dlen = 0;
  char a[128];
  char b[128];
  char d[128];
  int filling = 0;
  while(fin.good()){
    if(c == '\n'){
      d[dlen] = '\0';
      allDistances.push_back(D_alpha(string(a), string(b), atof(d)));
      alen = blen = dlen = 0;
      filling = 0;
    }
    else if(c == '\r'){}
    else if (c == '\t'){
      if(filling == 0) a[alen] = '\0';
      else if(filling == 1) b[blen] = '\0';
      filling++;
    }
    else if(filling == 0) a[alen++] = c;
    else if(filling == 1) b[blen++] = c;
    else d[dlen++] = c;
    c = fin.get();
  }

  applyAlpha(alpha, beta, &allDistances, blastscores);
  cout << "finished reading " << allDistances.size() << " distances\n";

  return allDistances;
}
