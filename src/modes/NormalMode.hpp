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

private:
	void makeReport(const Graph& G1, Graph& G2, const Alignment& A,
	  const MeasureCombination& M, Method* method, ofstream& stream);

	void saveReport(const Graph& G1, Graph& G2, const Alignment& A,
	  const MeasureCombination& M, Method* method, string reportFile);
};

void createFolders(ArgumentParser& args);
void initGraphs(Graph& G1, Graph& G2, ArgumentParser& args);
Method* initMethod(Graph& G1, Graph& G2, ArgumentParser& args, MeasureCombination& M);
void initMeasures(MeasureCombination& M, Graph& G1, Graph& G2, ArgumentParser& args);
void normalMode(Graph& G1, Graph& G2, ArgumentParser& args);

#endif /* NORMALMODE_HPP_ */
