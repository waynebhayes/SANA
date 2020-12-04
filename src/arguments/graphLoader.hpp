#ifndef GRAPHLOADER_H_
#define GRAPHLOADER_H_

#include "ArgumentParser.hpp"
#include "../Graph.hpp"
#include <vector>
void initGraphs(Graph& G1, Graph& G2, ArgumentParser& args);
void initGraphs(std::vector<Graph>& graphs, ArgumentParser& args);

#endif /* GRAPHLOADER_H_ */
