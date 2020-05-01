#ifndef COMPUTEGRAPHLETSWRAPPER_H_
#define COMPUTEGRAPHLETSWRAPPER_H_

#include <vector>
#include <string>
#include "utils/utils.hpp"
#include "Graph.hpp"

using namespace std;

//wrapper around the computeGraphlets algorithm/file
namespace computeGraphletsWrapper {

vector<vector<uint>> loadGraphletDegreeVectors(const Graph& G, uint maxGraphletSize);
vector<vector<uint>> computeGraphletDegreeVectors(const Graph& G, uint maxGraphletSize);
void saveGraphletsAsSigs(const Graph& G, uint maxGraphletSize, const string& outFile);

} //namespace computeGraphletsWrapper

#endif /* COMPUTEGRAPHLETSWRAPPER_H_ */
