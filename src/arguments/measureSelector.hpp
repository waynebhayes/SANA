#ifndef MEASURESELECTOR_H_
#define MEASURESELECTOR_H_

#include "ArgumentParser.hpp"
#include "../Graph.hpp"
#include "../measures/MeasureCombination.hpp"

namespace measureSelector {

double betaDerivedAlpha(const string& methodName, const string& G1Name, const string& G2Name, double beta);

void initMeasures(MeasureCombination& M, const Graph& G1, const Graph& G2, ArgumentParser& args);

} //namespace measureSelector

#endif /* MEASURESELECTOR_H_ */
