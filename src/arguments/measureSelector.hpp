#ifndef MEASURESELECTOR_H_
#define MEASURESELECTOR_H_

#include "ArgumentParser.hpp"
#include "../Graph.hpp"
#include "../measures/MeasureCombination.hpp"

double betaDerivedAlpha(string methodName, string G1Name, string G2Name, double beta);

void initMeasures(MeasureCombination& M, Graph& G1, Graph& G2, ArgumentParser& args);
void initMeasures(MeasureCombination& M, vector<Graph>& GV, Graph& SN, ArgumentParser& args);

#endif /* MEASURESELECTOR_H_ */
