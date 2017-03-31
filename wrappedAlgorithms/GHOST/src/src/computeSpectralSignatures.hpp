#include <iostream>
#include <stdlib.h>
#include <string>
#include <vector>
#include <boost/unordered_map.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include "progressBar.hpp"
#include "graph.hpp"
#include "laplacian.hpp"
#include "gzWriter.hpp"

#include <boost/thread.hpp>
#include "threadpool.hpp"

using std::cout;
using boost::threadpool::pool;
typedef boost::posix_time::microsec_clock bclock;
typedef boost::posix_time::ptime ptime;

// all three functions compute the spectrum to use
// first is exact, other two are approximations
void spectrum(Graph *input, int numHops, string source, 
              vector<LevelData> *output, ProgressBar *p)
{
  AdjacencyMatrix m;
  for(int i=1;i<=numHops;i++)
  {
    m.extend(input, source);
    (*output).push_back(LevelData(m.getPrev(), m.getEigen(), m.getDensity()));
  }
  (*p).update();
}

void spectrumR(Graph *input, int numHops, string source, 
              vector<LevelData> *output, ProgressBar *p)
{
  AdjacencyMatrix m;
  for(int i=1;i<=numHops;i++)
  {
    m.extend(input, source);
    (*output).push_back(LevelData(m.getPrev(), m.rayleighEigen(), 
                                  m.getDensity()));
  }
  (*p).update();
}

void spectrumI(Graph *input, int numHops, string source, 
              vector<LevelData> *output, ProgressBar *p)
{
  AdjacencyMatrix m;
  for(int i=1;i<=numHops;i++)
  {
    m.extend(input, source);
    (*output).push_back(LevelData(m.getPrev(), m.inverseEigen(), 
                                  m.getDensity()));
  }
  (*p).update();
}

// uses graph to generate spectrums of induced subgraphs
// of a radius 1-#hops from every possible source node
void computeSpectralSignatures(Graph *input, int numHops, int numP, string mode)
{
  ptime t = bclock::local_time();
  if(numP < 1) numP = boost::thread::hardware_concurrency();
  if(numP < 1) numP = 2;
  pool tpool(numP);
  vector<string> nodes = (*input).nodes();
  int numNodes = nodes.size();
  GzWriter w((*input).getName()+".sig.gz");
  ProgressBar p(numNodes, t);
  cout << "creating: " << (*input).getName() << ".sig.gz\n";
  w.writeInt(numNodes);
  w.writeInt(numHops);
  
  levelMap levelmap;
  vector< vector<LevelData> > data;
  data.resize(numNodes);
  if(mode=="")
    for(int i=0;i<numNodes;i++)
      tpool.schedule(
        boost::bind(&spectrum, input, numHops, nodes[i], &(data[i]), &p)
      );
  else if(mode=="rayleigh")
    for(int i=0;i<numNodes;i++)
      tpool.schedule(
        boost::bind(&spectrumR, input, numHops, nodes[i], &(data[i]), &p)
      );
  else if(mode=="inverseIter")
    for(int i=0;i<numNodes;i++)
      tpool.schedule(
        boost::bind(&spectrumI, input, numHops, nodes[i], &(data[i]), &p)
      );
  else
  {
    cout << "sigApprox should be either rayleigh or inverseIter\n";
    exit(0);
  }
  tpool.wait();
  cout << "\n";

  for(int i=0;i<numNodes;i++)
    levelmap[nodes[i]] = data[i];
  w.writeData(levelmap);
  cout << "created: " << (*input).getName() << ".sig.gz in " <<
    (bclock::local_time() - t).total_milliseconds() << " milliseconds\n";
}

