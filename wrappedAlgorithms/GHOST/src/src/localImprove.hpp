#include <vector>
#include <string>
#include <tgmath.h>
#include <stdlib.h>
#include <time.h>

#include "graph.hpp"
#include "spectralToDistance.hpp"

using std::string;
using std::vector;
using std::pair;

typedef boost::unordered_map<pair<string,string>, D_alpha> distmap;

struct move{
  string u, w, v, up;
  double s0, s1, s2;
  move(string v, string up) : v(v),up(up){s0 = s1 = s2 = 0; u = w = "";} 
  void apply(bmap *f)
  {
    f->left.erase(u);
    if(up != "") f->left.erase(up);
    f->insert(bmap::value_type(u, v));
    if(up != "") f->insert(bmap::value_type(up, w));
    up = u;
  }
  void calcScores(Graph *G, Graph *H, blastmap *dists, bmap *f)
  {
    s0 = s1 = s2 = 0;
    if(u != up){
      vlist *neighborsu = G->neighbors2(u);
      vlist *neighborsw = H->neighbors2(w);
      vlist *neighborsv = H->neighbors2(v);
      vlist *neighborsup = up == "" ? NULL : G->neighbors2(up);
      //Go thru all the edges that u is a part of, to see what changes
      for(auto it = neighborsu->begin(); it != neighborsu->end(); it++){
        auto mapped = f->left.find(*it);
        if(mapped != f->left.end()){
          string s = mapped->second;
          //There is a matching edge to w that we are losing
          if(neighborsw->find(s) != neighborsw->end()) s0--;
          if(s == v) s = w;
          else if(s == w) s = v;
          //There is a matching edge to v that we are adding
          if(neighborsv->find(s) != neighborsv->end()) s0++;
        }
      }
      if(neighborsup){
        //Go thru all the edges that up is a part of, to see what changes
        for(auto it = neighborsup->begin(); it != neighborsup->end(); it++){
          auto mapped = f->left.find(*it);
          if(mapped != f->left.end()){
            string s = mapped->second;
            //There is a matching edge to v that we are losing
            if(neighborsv->find(s) != neighborsv->end()) s0--;
            if(s == v) s = w;
            else if(s == w) s = v;
            //There is a matching edge to w that we are adding
            if(neighborsw->find(s) != neighborsw->end()) s0++;
          }
        }
      }
    }

    if(dists){
      auto uw = dists->find(make_pair(u,w));
      auto uv = dists->find(make_pair(u,v));
      s1 = (uw != dists->end() ? uw->second : 1) - (uv != dists->end() ? uv->second : 1);

      if(up != ""){
        uw = dists->find(make_pair(up,w));
        uv = dists->find(make_pair(up,v));
        s2 = (uv != dists->end() ? uv->second : 1) - (uw != dists->end() ? uw->second : 1);
      }
    }
  }
};

void calcScoresWorker(Graph *G, Graph *H, blastmap *dists, bmap *f, string u, string w, move **ms, int count)
{
  for(int i = 0; i < count; i++){
    move *m = ms[i];
    m->u = u;
    m->w = w;
    m->calcScores(G, H, dists, f);
  }
}

int searchIter(Graph& G, Graph& H, blastmap *dists, bmap *f, double ratio, int numthreads)
{
  if(numthreads == -1) numthreads = boost::thread::hardware_concurrency();
  if(numthreads < 2) numthreads = 2;
  boost::threadpool::pool threads(numthreads);

  int totalDelt = 0;
  vector<string> hverts = H.nodes();
  int numMoves = hverts.size();
  vector<string> lefts;
  for(auto it = f->left.begin(); it != f->left.end(); it++) 
    lefts.push_back(it->first);

  move** moves = new move*[numMoves];
  int i = 0;
  for(auto it = hverts.begin(); it != hverts.end(); it++){
    auto up = f->right.find(*it);
    moves[i++] = new move(*it, up == f->right.end() ? "" : up->second);
  }

  ProgressBar pbar(lefts.size(), bclock::local_time());
  srand(time(0));
  
  for(auto it = lefts.begin(); it != lefts.end(); it++){
    string u = *it;
    string w = f->left.at(u);
    
    move *bestMove = NULL;
    bool constrained = rand() > (ratio*RAND_MAX);

    for(int threadsMade = 0, i = 0; threadsMade < numthreads; threadsMade++){
      int n = (numMoves - i)/(numthreads - threadsMade);
      auto worker = boost::bind(&calcScoresWorker, &G, &H, dists, f, u, w, moves + i, n);
      i += n;
      threads.schedule(worker);
    }
    threads.wait();

    for(int i = 0; i < numMoves; i++){
      move *m = moves[i];
      //Check if move is acceptible
      //if(m->s0 > 0 && m->s1 >= 0 && (!constrained || m->s2 >= 0)){
      if(m->s0 >= 0){
        if(!bestMove) bestMove = m;
        else if(m->s0 > bestMove->s0) bestMove = m;
        else if(m->s0 == bestMove->s0 && m->s1 > bestMove->s1) bestMove = m;
        else if(m->s0 == bestMove->s0 && m->s1 == bestMove->s1 && m->s2 > bestMove->s2) bestMove = m;
      }
    }

    if(bestMove){
      bestMove->apply(f);
      totalDelt += bestMove->s0;
      
      auto upt = f->right.find(bestMove->w);
      string up = upt == f->right.end() ? "" : upt->second;
      for(int i = 0; i < numMoves; i++)
        if(moves[i]->v == bestMove->w){moves[i]->up = up; break;}
    }
    pbar.update();
  }
  return totalDelt; 
}

int localImprove(Graph& G, Graph& H, blastmap *dists, bmap *f, int iters, double ratio, int numP)
{
  if(iters == 0) return 0;
  vector<double> ratios;
  double sum = 0;
  int ri = iters;
  if(iters == -1) ri = 10;
  for(int i=0; i < ri; i++){
    double d = exp(-i);
    sum += d;
    ratios.push_back(d);
  }
  sum = ratio/sum;
  int totalDelt = 0;
  for(int i=0; i < iters || iters == -1; i++){
    ptime t = bclock::local_time();
    double unconstrained;
    if(i < ri) unconstrained = ratios[i] * sum;
    else unconstrained = ratios[ri-1] * sum;
    if(unconstrained > 1) unconstrained = 1;
    cout << "running PISWAP iteration, " << (unconstrained * 100) << "\% unconstrained\n";
    int d = searchIter(G, H, dists, f, unconstrained, numP);
    cout << "\nadded " << d << " edges in " << (bclock::local_time() - t).total_milliseconds() << "ms\n";
    totalDelt += d;
    if(d == 0) break;
  }
  return totalDelt;
}
