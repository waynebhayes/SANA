#ifndef MEASURESELECTOR_H_
#define MEASURESELECTOR_H_

#include "ArgumentParser.hpp"
#include "../Graph.hpp"
#include "../measures/MeasureCombination.hpp"

void initMeasures(MeasureCombination& M, Graph& G1, Graph& G2, ArgumentParser& args);

#endif /* MEASURESELECTOR_H_ */
