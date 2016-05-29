#include <string>
#include <iostream>
#include <fstream>
#include <boost/unordered_map.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include "graph.hpp"

using std::string;
using std::ifstream;
using std::cout;

typedef boost::posix_time::microsec_clock bclock;
typedef boost::posix_time::ptime ptime;

// read from simpler file format
Graph readFromNet(string fileName, bool directed)
{ 
  ptime t = bclock::local_time();
  Graph result;
  result.direct(directed);
  ifstream fin(fileName);
  if(!fin.good()) 
  {
    cout << "error loading file: " << fileName << "\n";
    exit(0);
  }
  unsigned start = fileName.find_last_of("/");
  unsigned end = fileName.find(".net");
  if(start == string::npos) start = -1;
  string name = fileName.substr(start+1,end-start-1);
  result.setName(name);

  int N,E;
  fin >> N >> E;
  for(int i=0;i<N;i++)
  {
    string n;
    fin >> n;
    result.addVertex(n);
  }
  for(int i=0;i<E;i++)
  {
    string n1,n2;
    fin >> n1 >> n2;
    result.addEdge(n1, n2);
  }
  fin.close();
  cout << "extracted: " << result.getName() << ".net in " <<
    (bclock::local_time() - t).total_milliseconds() << " milliseconds\n";
  return result;
}

