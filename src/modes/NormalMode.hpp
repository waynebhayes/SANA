#ifndef NORMALMODE_HPP_
#define NORMALMODE_HPP_

#include "Mode.hpp"
#include "../Graph.hpp"
#include "../arguments/ArgumentParser.hpp"
#include "../measures/MeasureCombination.hpp"
#include "../methods/Method.hpp"

class NormalMode : public Mode {
public:
	void run(ArgumentParser& args);
};

void createFolders(ArgumentParser& args);
void initGraphs(Graph& G1, Graph& G2, ArgumentParser& args);
void normalMode(Graph& G1, Graph& G2, ArgumentParser& args);

#endif /* NORMALMODE_HPP_ */
