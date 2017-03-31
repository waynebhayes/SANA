#include <string>
#include <iostream>
#include <stdlib.h>
#include <boost/unordered_map.hpp>
#include <utility>
#include <iostream>
#include <fstream>

using std::string;
using std::pair;
using std::ifstream;

typedef boost::unordered_map<pair<string,string>, double> blastMap;

// reads in from .evalues file
blastMap getBlastMap(string filename)
{
  blastMap bevals;
  ifstream fin (filename);
  if(!fin.good()) {std::cout << "sequencescores file is bad\n"; exit(0);}
  while(1)
  {
    string n1,n2;
    double d;
    fin >> n1 >> n2 >> d;
    if(fin.eof()) break;
    bevals[make_pair(n1,n2)] = d;
  }
  fin.close();
  return bevals;
}
