#pragma once

#include <unordered_map>
#include <set>
#include <tuple>
#include <string>
#include "boost/multi_array.hpp"
using namespace std;

#include "Network.h"

typedef boost::multi_array<double,2> GOCDict;

GOCDict* loadGOC(Network* net1, Network* net2,
                        string file1, string file2);

unordered_map<node, set<int> > loadAnnotations(Network* net, string fp);
