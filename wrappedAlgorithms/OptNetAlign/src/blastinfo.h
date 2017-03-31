#pragma once

#include <unordered_map>
#include <tuple>
#include <string>
#include "boost/multi_array.hpp"
#include "Network.h"
using namespace std;

typedef boost::multi_array<double,2> BLASTDict;

BLASTDict* loadBLASTInfo(Network* net1, Network* net2, string filename);