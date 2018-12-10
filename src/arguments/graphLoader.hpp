#ifndef GRAPHLOADER_H_
#define GRAPHLOADER_H_

#include "ArgumentParser.hpp"
#include "../Graph.hpp"
extern bool _graphsSwitched;
void initGraphs(Graph& G1, Graph& G2, ArgumentParser& args);

#endif /* GRAPHLOADER_H_ */
